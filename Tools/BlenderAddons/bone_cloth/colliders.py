"""Step 7: leg capsules, fitted exactly as the engine fits them, and the torso's surface capsules, which keep the cloth
itself out of the body (DESIGN.md section 8.7)."""
import numpy as np

from . import bones as bones_mod
from . import engine_contract as ec
from . import frames, landmarks, model_data

SURFACE_CLEARANCE = 0.01     # an auto surface capsule ends this far inside the resting cloth
SURFACE_PERCENTILE = 5       # ...measured to the cloth nearest it, leaving out the closest few vertices (a collar, a belt)
MIN_SIDE_OFFSET = 0.02       # the torso gets side capsules when it is this much wider than it is deep


def add_defaults(setup, arm):
    """Thighs and calves of both legs, and the torso's surface capsules (all layouts but a strand), replacing the list."""
    setup.colliders.clear()
    for bone, to in landmarks.leg_capsules(setup.character):
        c = setup.colliders.add()
        c.bone, c.to, c.covers = bone, to, True
    if setup.guides.layout != 'STRAND':
        add_torso(setup, arm)


def _cloth_points(setup, arm, data=None):
    """Rest positions of the cloth's vertices carried mostly by the strip bones (sleeves on the arms left out)."""
    skel, verts = data if data is not None else model_data.collect(arm)
    cloth = {skel.index[b] for b in bones_mod.generated(setup) if b in skel.index}
    if not cloth or not len(verts.position):
        return np.zeros((0, 3))
    dominant = verts.dominant()
    return verts.position[np.isin(dominant, list(cloth))]


def add_torso(setup, arm, data=None):
    """Surface capsules from the hips to the chest: one on the spine and, for a torso wider than it is deep, one along
    each side, so together they fill an oval. Their size comes from the cloth at rest (fit_all)."""
    hips, chest = setup.character.hips, setup.character.chest
    if hips not in arm.data.bones or chest not in arm.data.bones:
        return
    frame = frames.compute_frame(arm, setup.character)
    a, b = frames.bone_head(arm, hips), frames.bone_head(arm, chest)
    pts = _cloth_points(setup, arm, data)
    side = 0.0
    if len(pts):
        axis = b - a
        t = np.clip((pts - a) @ axis / float(axis @ axis), 0.0, 1.0)
        rel = pts - (a + np.outer(t, axis))
        x, y = np.abs(rel @ frame.left), np.abs(rel @ frame.forward)
        inside = (t > 0.0) & (t < 1.0)
        sides, fronts = inside & (x > y), inside & (y >= x)
        if sides.any() and fronts.any():
            side = float(np.percentile(x[sides], SURFACE_PERCENTILE) - np.percentile(y[fronts], SURFACE_PERCENTILE))
    offsets = [np.zeros(3)]
    if side >= MIN_SIDE_OFFSET:
        offsets += [frame.left * side, -frame.left * side]
    for off in offsets:
        c = setup.colliders.add()
        c.bone, c.to = hips, chest
        c.surface = True
        c.offset = tuple(float(v) for v in off)


def fit_surface(a, b, pts):
    """Radii of an auto surface capsule: as large as the cloth at rest allows, SURFACE_CLEARANCE inside it."""
    axis = b - a
    l2 = float(axis @ axis)
    if not len(pts) or l2 < 1e-12:
        return ec.FALLBACK_COLLIDER_RADIUS, ec.FALLBACK_COLLIDER_RADIUS
    t = np.clip((pts - a) @ axis / l2, 0.0, 1.0)
    d = np.linalg.norm(pts - (a + np.outer(t, axis)), axis=1)
    inside = (t > 0.0) & (t < 1.0)
    out = []
    for half in (inside & (t <= 0.5), inside & (t > 0.5)):
        near = d[half] if half.any() else d[inside] if inside.any() else d
        out.append(max(0.02, float(np.percentile(near, SURFACE_PERCENTILE)) - SURFACE_CLEARANCE))
    return out[0], out[1]


def fit_all(setup, arm, data=None):
    """Fills fitted_radius / fitted_end: leg capsules from the vertices of everything that will be exported (as the game
    fits them), surface capsules from the cloth at rest (the game is given those radii)."""
    data = data if data is not None else model_data.collect(arm)
    skel, verts = data
    pts = None
    out = []
    for c in setup.colliders:
        if c.bone not in skel.index or c.to not in skel.index:
            c.fitted_radius = c.fitted_end = 0.0
            continue
        if c.surface:
            pts = _cloth_points(setup, arm, data) if pts is None else pts
            off = np.array(c.offset)
            r, e = fit_surface(skel.head(skel.index[c.bone]) + off, skel.head(skel.index[c.to]) + off, pts)
        else:
            r, e = ec.fit_collider(skel, verts, skel.index[c.bone], skel.index[c.to],
                                   0.0 if c.auto_radius else c.radius, 0.0 if c.auto_end else c.end_radius)
        c.fitted_radius, c.fitted_end = float(r), float(e)
        out.append((c.bone, c.to, r, e))
    return out


def radii(c):
    """(start, end) radius in metres as the engine will use them."""
    r = c.fitted_radius if c.auto_radius and c.fitted_radius > 0 else c.radius
    e = c.fitted_end if c.auto_end and c.fitted_end > 0 else c.end_radius
    return r, e


def capsules(setup, arm, surface=None):
    """[(start, end, r0, r1)] in world space, at rest. surface: True only the surface capsules, False only the others."""
    out = []
    for c in setup.colliders:
        if not c.enabled or c.bone not in arm.data.bones or c.to not in arm.data.bones:
            continue
        if surface is not None and c.surface != surface:
            continue
        r0, r1 = radii(c)
        off = np.array(c.offset)
        out.append((frames.bone_head(arm, c.bone) + off, frames.bone_head(arm, c.to) + off, r0, r1))
    return out


def penetration(point, caps, padding):
    """Largest depth of the point inside any capsule (plus padding), 0 if it is clear."""
    worst = 0.0
    for a, b, r0, r1 in caps:
        ab = b - a
        l2 = float(np.dot(ab, ab))
        t = float(np.clip(np.dot(point - a, ab) / l2, 0.0, 1.0)) if l2 > 1e-12 else 0.0
        nearest = a + ab * t
        need = r0 + (r1 - r0) * t + padding
        d = float(np.linalg.norm(point - nearest))
        worst = max(worst, need - d)
    return worst
