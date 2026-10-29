"""Step 7: leg capsules, fitted exactly as the engine fits them (DESIGN.md section 8.7)."""
import numpy as np

from . import engine_contract as ec
from . import frames, landmarks, model_data


def add_defaults(setup, arm):
    """Thighs and calves of both legs, replacing the list."""
    setup.colliders.clear()
    for bone, to in landmarks.leg_capsules(setup.character):
        c = setup.colliders.add()
        c.bone, c.to = bone, to


def fit_all(setup, arm, data=None):
    """Fills fitted_radius / fitted_end from the vertices of everything that will be exported."""
    skel, verts = data if data is not None else model_data.collect(arm)
    out = []
    for c in setup.colliders:
        if c.bone not in skel.index or c.to not in skel.index:
            c.fitted_radius = c.fitted_end = 0.0
            continue
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


def capsules(setup, arm):
    """[(start, end, r0, r1)] in world space."""
    out = []
    for c in setup.colliders:
        if not c.enabled or c.bone not in arm.data.bones or c.to not in arm.data.bones:
            continue
        r0, r1 = radii(c)
        out.append((frames.bone_head(arm, c.bone), frames.bone_head(arm, c.to), r0, r1))
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
