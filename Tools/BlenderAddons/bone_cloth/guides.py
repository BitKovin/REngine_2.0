"""Step 3: measure the cloth and lay out panels and bone positions (DESIGN.md section 8.3)."""
import math

import bpy
import numpy as np
from mathutils import Vector
from mathutils.bvhtree import BVHTree

from . import frames, landmarks, mesh_prep


class ClothGeom:
    """The cloth mesh(es) in world space at rest. Faces dominated by arm bones are left out of the surface BVH."""

    def __init__(self, setup, arm):
        self.verts = np.zeros((0, 3))
        self.arm_weight = np.zeros(0)
        polys = []
        exclude = landmarks.exclude_bones(arm, setup.character) if setup.weights.exclude_arms else set()
        offset = 0
        pieces, arm_w = [], []
        for obj in mesh_prep.cloth_objects(setup):
            src = mesh_prep.source_of(obj) or obj
            me = src.data
            n = len(me.vertices)
            co = np.empty(n * 3, dtype=np.float32)
            me.vertices.foreach_get('co', co)
            mw = np.array(mesh_prep.world_matrix(src))
            w = co.reshape(n, 3).astype(float) @ mw[:3, :3].T + mw[:3, 3]
            names = {vg.index: vg.name for vg in src.vertex_groups}
            aw = np.zeros(n)
            for i, v in enumerate(me.vertices):
                for g in v.groups:
                    if names.get(g.group) in exclude:
                        aw[i] += g.weight
            pieces.append(w)
            arm_w.append(aw)
            for p in me.polygons:
                polys.append((tuple(i + offset for i in p.vertices)))
            offset += n
        if pieces:
            self.verts = np.vstack(pieces)
            self.arm_weight = np.concatenate(arm_w)
        thr = setup.weights.exclude_threshold
        keep = [p for p in polys if all(self.arm_weight[i] < thr for i in p)]
        self.torso_mask = self.arm_weight < thr
        self.bvh = BVHTree.FromPolygons([Vector(v) for v in self.verts], keep) if keep else None
        self.torso = self.verts[self.torso_mask]


def all_hits(geom, origin, direction, max_dist=2.0):
    """Hit distances along a ray, nearest first."""
    hits = []
    o = Vector(origin)
    d = Vector(direction)
    travelled = 0.0
    while travelled < max_dist:
        loc, _, idx, dist = geom.bvh.ray_cast(o, d, max_dist - travelled)
        if loc is None:
            break
        travelled += dist
        hits.append(travelled)
        o = loc + d * 1e-4
        travelled += 1e-4
    return hits


def surface_point(geom, frame, theta, z, placement='OUTER', offset=0.0):
    """Point of the cloth surface at height z in direction theta, or None."""
    if geom.bvh is None:
        return None
    direction = frame.radial(theta)
    origin = np.array([frame.origin[0], frame.origin[1], z])
    hits = all_hits(geom, origin, direction)
    if not hits:
        return None
    t = {'OUTER': hits[-1], 'INNER': hits[0], 'MIDDLE': 0.5 * (hits[0] + hits[-1])}[placement] + offset
    return origin + direction * t


def surface_point_robust(geom, frame, theta, z, placement='OUTER', offset=0.0):
    """As surface_point; when a ray falls into a gap it tries nearby angles."""
    for d in (0, 3, -3, 6, -6, 10, -10, 15, -15):
        p = surface_point(geom, frame, theta + math.radians(d), z, placement, offset)
        if p is not None:
            # keep the requested angle: slide the point onto the ray of theta at the same distance from the axis
            r = np.linalg.norm((p - frame.origin)[:2])
            q = np.array([frame.origin[0], frame.origin[1], z]) + frame.radial(theta) * r
            return q
    return None


# ---- measuring ------------------------------------------------------------------------------------------------------

def measure(setup, arm, frame, geom):
    """Suggested lines and opening, from the cloth mesh and the skeleton."""
    g, ch = setup.guides, setup.character
    out = {}
    z = geom.torso[:, 2] if len(geom.torso) else np.array([0.0])
    out['hem'] = float(z.min()) + 0.01
    out['top'] = float(frames.bone_head(arm, ch.chest)[2])
    if g.anchors == 'CHEST':
        for key in ('clavicle_l', 'clavicle_r'):
            if getattr(ch, key) in arm.data.bones:
                out['top'] = float(frames.bone_head(arm, getattr(ch, key))[2]) - 0.02       # the shoulder line
                break
    if g.anchors == 'SPINE' and len(geom.torso):
        # a short jacket starts at the upper chest, a little above the chest joint, and never above its own cloth
        out['top'] = min(out['top'] + 0.12, float(geom.torso[:, 2].max()) - 0.02)
    wz = landmarks.waist_height(arm, ch)
    out['waist'] = wz if wz is not None else out['top'] - 0.3
    # opening: the widest empty arc around the front, between the waist and the hem
    low = geom.torso[(geom.torso[:, 2] < out['waist']) & (geom.torso[:, 2] > out['hem'])] if len(geom.torso) else geom.torso
    half = 0.0
    if len(low) > 20:
        theta = np.degrees(frame.angle(low))
        hist, _ = np.histogram(np.abs(theta), bins=36, range=(0, 180))
        limit = max(1, int(0.02 * hist.max()))
        k = 0
        while k < len(hist) and hist[k] <= limit:
            k += 1
        half = math.radians(5.0 * k + 5.0) if k > 0 else 0.0
    out['opening'] = half
    return out


def detect_belt(geom, waist_z):
    """A band of high vertex density near the waist, as (centre height, half width), or None."""
    if len(geom.torso) < 100:
        return None
    zs = geom.torso[:, 2]
    lo, hi = waist_z - 0.3, waist_z + 0.3
    bins = np.arange(lo, hi, 0.01)
    hist, edges = np.histogram(zs, bins=bins)
    med = np.median(hist[hist > 0]) if np.any(hist > 0) else 0
    if med <= 0:
        return None
    hot = hist > 2.0 * med
    best, cur = (0, 0), None
    for i, h in enumerate(hot):
        if h and cur is None:
            cur = i
        if (not h or i == len(hot) - 1) and cur is not None:
            end = i if not h else i + 1
            if end - cur > best[1] - best[0]:
                best = (cur, end)
            cur = None
    width = (best[1] - best[0]) * 0.01
    if width < 0.02 or width > 0.12:
        return None
    return float(0.5 * (edges[best[0]] + edges[best[1]])), float(width / 2)


# ---- layout ---------------------------------------------------------------------------------------------------------

def column_names(n):
    if n == 1:
        return ["side"]
    if n == 2:
        return ["front", "back"]
    return ["front"] + ["side"] + ["side%d" % k for k in range(2, n - 1)] + ["back"]


def rebuild_columns(setup, opening=None):
    g = setup.guides
    old = {c.name: (c.top_offset, c.open_mult, c.volume_mult, c.rest_flare_mult) for c in g.columns}
    g.columns.clear()
    if g.layout == 'STRAND':
        return
    n = g.columns_per_side
    names = column_names(n)
    if g.layout == 'BACK_ONLY':
        span = math.radians(80.0)              # the cape covers the back and a little of the sides
        angles = [math.pi - span + span * (i + 0.5) / n for i in range(n)] if n > 1 else [math.pi - span / 2]
        if g.center_back:
            angles = [a - span / (2.0 * n) for a in angles]    # leave room for the panel at the very back
    elif g.layout == 'OPEN_FRONT':
        a0 = g.opening_half_angle if opening is None else opening
        a1 = math.pi
        arc = a1 - a0
        if n == 1:
            angles = [(a0 + a1) / 2]
        else:
            lo, hi = a0 + 0.08 * arc, a1 - 0.05 * arc
            angles = [lo + (hi - lo) * i / (n - 1) for i in range(n)]
    else:
        angles = [math.pi * (i + 1) / (n + 1) for i in range(n)]
    for name, ang in zip(names, angles):
        c = g.columns.add()
        c.name, c.side, c.angle = name, 'L', ang
        if name in old:
            c.top_offset, c.open_mult, c.volume_mult, c.rest_flare_mult = old[name]
        else:
            c.open_mult = 1.0 if ang < math.radians(80) else (0.4 if ang < math.radians(150) else 0.0)
            # following the spine (jackets): the front panels do the opening, so they flare less than the rest
            c.volume_mult = 0.5 if (g.anchors == 'SPINE' and ang < math.radians(80)) else 1.0
    if g.center_front:
        c = g.columns.add()
        c.name, c.side, c.angle = "frontc", 'C', 0.0
    if g.center_back:
        c = g.columns.add()
        c.name, c.side, c.angle = "backc", 'C', math.pi
        c.open_mult = 0.0
        if "backc" in old:
            c.top_offset = old["backc"][0]


def measure_column_tops(setup, frame, geom):
    """Each column starts where its ray first meets the (non-arm) cloth, scanning down from the top line."""
    g = setup.guides
    for c in g.columns:
        if not c.enabled:
            continue
        z = g.top_line
        found = None
        lower = g.hem_line + 0.1 if g.anchors == 'SPINE' else g.waist_line
        while z > lower:
            if surface_point(geom, frame, c.angle, z) is not None:
                found = z
                break
            z -= 0.01
        c.top_offset = float(np.clip((found if found is not None else g.top_line) - g.top_line, -0.5, 0.05))


# ---- plan -----------------------------------------------------------------------------------------------------------

class PlanBone:
    def __init__(self, name, head, tail, parent, theta, connected=False):
        self.name, self.head, self.tail, self.parent = name, np.asarray(head, float), np.asarray(tail, float), parent
        self.theta, self.connected = theta, connected


class Plan:
    def __init__(self):
        self.roots = []      # PlanBone
        self.strips = []     # (column name, side, [PlanBone])
        self.skipped = []    # messages
        self.joined = []     # (upper bone name, lower bone name)

    def all_bones(self):
        out = list(self.roots)
        for _, _, bones in self.strips:
            out.extend(bones)
        return out


def _mirror(frame, p):
    """The point reflected through the body's centre plane."""
    d = p - frame.origin
    return p - 2.0 * np.dot(d, frame.left) * frame.left


def _row_fractions(g):
    n = g.rows
    out = []
    for k in range(n + 1):
        t = k / n
        if g.row_spacing == 'DENSER_AT_HEM':
            t = t ** (1.0 + g.row_bias)
        elif g.row_spacing == 'DENSER_AT_TOP':
            t = t ** (1.0 / (1.0 + g.row_bias))
        out.append(t)
    return out


def _column_points(geom, frame, g, theta, zs, hem_index, twin, mirror_ok):
    """Surface points of one panel at the heights zs (the hem one probed a little above the rim). A missed point
    comes from the mirrored panel when there is one."""
    pts = []
    for k, z in enumerate(zs):
        p = surface_point_robust(geom, frame, theta, z + (0.03 if k == hem_index else 0.0), g.placement, g.surface_offset)
        if p is not None and k == hem_index:
            p = p.copy()
            p[2] = z
        if p is None and twin is not None and twin[k] is not None and mirror_ok:
            p = _mirror(frame, twin[k])
        pts.append(p)
    return pts


def _inset(pts, zs, owners, z_zero, spine_xy, amount, left, min_side=0.03):
    """Moves points toward the spine's joint line: fully down to the lowest joint where the owner changes (the cloth
    hanging from different spine bones must agree on where it hinges), fading to none at z_zero. The top point of a
    panel keeps at least `min_side` metres of lateral offset: the game reads a strip's left or right from that point."""
    pivots = [zs[i] for i in range(1, len(owners)) if owners[i] != owners[i - 1]]
    z_pivot = min(pivots) if pivots else zs[0]
    z_zero = min(z_zero, z_pivot - 0.01)
    top_amount = amount
    if pts[0] is not None:
        c0 = spine_xy(zs[0])
        lateral = abs(float(np.dot(np.append(pts[0][:2] - c0, 0.0), left)))
        top_amount = min(amount, 1.0 - min_side / lateral) if lateral > min_side else 0.0
    out = []
    for k, (p, z) in enumerate(zip(pts, zs)):
        if p is None:
            out.append(None)
            continue
        f = (top_amount if k == 0 else amount) * float(np.clip((z - z_zero) / max(z_pivot - z_zero, 1e-6), 0.0, 1.0))
        c = spine_xy(z)
        q = p.copy()
        q[:2] = c + (p[:2] - c) * (1.0 - f)
        out.append(q)
    return out


def _plan_spine(setup, arm, frame, geom, plan):
    """Short jackets: every panel runs from the top line to the hem, one bone per spine segment, each segment hanging
    from the spine bone at its height (a root bone under it). The engine joins them into one strip, and the strip bends
    where the spine bends."""
    g, ch = setup.guides, setup.character
    prefix = setup.prefix
    path = landmarks.spine_path(arm, ch)
    if not path:
        plan.skipped.append("No spine found between the chest and the hips")
        return plan
    top_z = g.top_line
    heads = [(name, float(frames.bone_head(arm, name)[2])) for name in path]
    # boundaries between segments: the spine joints (bone heads) below the top and above the hem part
    bounds = []
    prev = top_z
    for name, hz in heads:
        if hz < prev - 0.04 and hz > g.hem_line + 0.06:
            bounds.append(hz)
            prev = hz

    def owner(z_mid):
        """The spine bone a piece of cloth at this height rides: the one with the largest head at or below it."""
        best = None
        for name, hz in heads:
            if hz <= z_mid and (best is None or hz > best[1]):
                best = (name, hz)
        return best[0] if best else path[-1]

    axis = frame.origin
    root_of = {}
    # the spine's own joint line: where the body skin pivots
    sz = np.array([hz for _, hz in heads])
    sxy = np.array([frames.bone_head(arm, n)[:2] for n, _ in heads])
    order = np.argsort(sz)

    def spine_xy(z):
        return np.array([np.interp(z, sz[order], sxy[order][:, 0]), np.interp(z, sz[order], sxy[order][:, 1])])

    def owners_of(zs_):
        return [owner(0.5 * (zs_[i] + zs_[i + 1])) for i in range(len(zs_) - 1)]

    def root_for(bone_name, z):
        if bone_name not in root_of:
            rn = "%s_root_%d" % (prefix, len(root_of))
            root_of[bone_name] = rn
            plan.roots.append(PlanBone(rn, [axis[0], axis[1], z], [axis[0], axis[1], z - 0.1], bone_name, 0.0))
        return root_of[bone_name]

    if geom.bvh is None:
        plan.skipped.append("The cloth has no surface to place bones on")
        return plan
    sector = math.pi / max(2, 2 * g.columns_per_side)
    cache = {}
    for col in [c for c in g.columns if c.enabled]:
        thetas = [(col.angle, 'l'), (-col.angle, 'r')] if col.side == 'L' else [(col.angle, '')]
        for theta, suffix in thetas:
            tag = ("_" + suffix) if suffix else ""
            if g.hem_auto and len(geom.torso):
                a = np.arctan2((geom.torso - frame.origin) @ frame.left, (geom.torso - frame.origin) @ frame.forward)
                diff = np.abs((a - theta + math.pi) % (2 * math.pi) - math.pi)
                sel = geom.torso[diff < sector]
                hem_z = float(sel[:, 2].min()) + 0.005 if len(sel) else g.hem_line
            else:
                hem_z = g.hem_line
            tz = top_z + col.top_offset
            last_z = bounds[-1] if bounds else tz
            hem_z = min(hem_z, last_z - 0.05)
            zs = [tz] + bounds
            fr = _row_fractions(g)
            zs += [last_z + (hem_z - last_z) * t for t in fr[1:]]
            hem_index = len(zs) - 1
            twin = cache.get((col.name, 'l'))[0] if suffix == 'r' and (col.name, 'l') in cache else None
            pts = _column_points(geom, frame, g, theta, zs, hem_index, twin, True)
            if g.bone_inset > 0.0:
                pts = _inset(pts, zs, owners_of(zs), bounds[-1] if bounds else hem_z, spine_xy, g.bone_inset, frame.left)
            if any(p is None for p in pts[1:]) and len(geom.torso):
                a_ = np.arctan2((geom.torso - frame.origin) @ frame.left, (geom.torso - frame.origin) @ frame.forward)
                diff = np.abs((a_ - theta + math.pi) % (2 * math.pi) - math.pi)
                sel = geom.torso[diff < sector]
                if pts[-1] is None and len(sel):
                    pts[-1] = sel[np.argmin(sel[:, 2])]
            if pts[0] is None or pts[-1] is None:
                plan.skipped.append("%s_%s%s does not reach the %s" % (prefix, col.name, tag, "top" if pts[0] is None else "hem"))
                continue
            for k in range(1, len(pts) - 1):
                if pts[k] is None:
                    t = (zs[k] - zs[0]) / (zs[-1] - zs[0]) if zs[-1] != zs[0] else 0.5
                    pts[k] = pts[0] + (pts[-1] - pts[0]) * t
            bones = []
            owners = []
            for i in range(len(pts) - 1):
                name = "%s_%s_%02d%s" % (prefix, col.name, i, tag)
                own = owner(0.5 * (zs[i] + zs[i + 1]))
                if bones and own == owners[-1]:
                    parent, connected = bones[-1].name, True
                else:
                    parent, connected = root_for(own, zs[i]), False
                    if bones:
                        plan.joined.append((bones[-1].name, name))
                bones.append(PlanBone(name, pts[i], pts[i + 1], parent, theta, connected))
                owners.append(own)
            plan.strips.append((col.name, suffix, bones))
            cache[(col.name, suffix)] = (pts, None)
    return plan


def _plan_strand(setup, arm, frame, geom, plan):
    """One strip along the mesh: from the picked bone to the far end, through the centre of each slice of the cloth."""
    g = setup.guides
    prefix = setup.prefix
    anchor = g.strand_anchor
    if anchor not in arm.data.bones:
        plan.skipped.append("Pick the bone the strand hangs from (Shape step)")
        return plan
    verts = geom.verts
    if len(verts) < 8:
        plan.skipped.append("The cloth has too few vertices")
        return plan
    start = frames.bone_head(arm, anchor)
    # the strand's direction: from the anchor toward the far end of the cloth
    far = verts[np.argsort(np.linalg.norm(verts - start, axis=1))[-max(3, len(verts) // 20):]].mean(axis=0)
    near_ = verts[np.argsort(np.linalg.norm(verts - start, axis=1))[:max(3, len(verts) // 20)]].mean(axis=0)
    axis = far - near_
    axis = axis / max(np.linalg.norm(axis), 1e-9)
    t = (verts - near_) @ axis
    t0, t1 = float(t.min()), float(t.max())
    n = g.rows
    pts = [near_.copy()]
    half = 0.5 * (t1 - t0) / n
    for k in range(1, n):
        boundary = t0 + (t1 - t0) * k / n
        band = verts[np.abs(t - boundary) <= half]
        pts.append(band.mean(axis=0) if len(band) else near_ + axis * (boundary - float(np.dot(near_ - near_, axis))))
    pts.append(verts[int(np.argmax(t))])              # the free end: the farthest vertex
    # where the strand sits around the body decides which way the wind poses push it (a tail behind is pushed back)
    theta = float(frame.angle(np.mean(pts, axis=0)))
    plan.roots.append(PlanBone(prefix + "_root", pts[0], pts[0] + axis * 0.05, anchor, 0.0))
    bl = []
    for k in range(len(pts) - 1):
        name = "%s_strand_%02d" % (prefix, k)
        parent, connected = (prefix + "_root", False) if k == 0 else (bl[-1].name, True)
        bl.append(PlanBone(name, pts[k], pts[k + 1], parent, theta, connected))
    plan.strips.append(("strand", "", bl))
    return plan


def make_plan(setup, arm, frame, geom):
    g, ch = setup.guides, setup.character
    plan = Plan()
    if g.layout == 'STRAND':
        return _plan_strand(setup, arm, frame, geom, plan)
    if g.anchors == 'SPINE':
        return _plan_spine(setup, arm, frame, geom, plan)
    prefix = setup.prefix
    use_upper = g.upper_part and g.anchors == 'CHEST_AND_HIPS'
    two_roots = use_upper
    axis_xy = frame.origin

    top_z, waist_z = g.top_line, g.waist_line
    cape = g.anchors == 'CHEST'
    if cape:
        waist_z = top_z                         # the panels start at the shoulder line and hang from the chest
    lower_root_name = prefix + ("_root_lower" if two_roots else "_root")
    plan.roots.append(PlanBone(lower_root_name, [axis_xy[0], axis_xy[1], waist_z],
                               [axis_xy[0], axis_xy[1], waist_z - 0.1], ch.chest if cape else ch.hips, 0.0))
    if two_roots:
        plan.roots.append(PlanBone(prefix + "_root_upper", [axis_xy[0], axis_xy[1], top_z],
                                   [axis_xy[0], axis_xy[1], top_z - 0.1], ch.chest, 0.0))
    upper_root = plan.roots[-1].name if two_roots else lower_root_name

    if geom.bvh is None:
        plan.skipped.append("The cloth has no surface to place bones on")
        return plan
    fractions = _row_fractions(g)
    sector = math.pi / max(2, 2 * g.columns_per_side)

    cols = [c for c in g.columns if c.enabled]
    cache = {}
    for col in cols:
        thetas = [(col.angle, 'l'), (-col.angle, 'r')] if col.side == 'L' else [(col.angle, '')]
        for theta, suffix in thetas:
            tag = col.name + ("_" + suffix if suffix else "")
            # sector's lowest cloth vertex gives this panel's hem
            if g.hem_auto and len(geom.torso):
                a = np.arctan2((geom.torso - frame.origin) @ frame.left, (geom.torso - frame.origin) @ frame.forward)
                diff = np.abs((a - theta + math.pi) % (2 * math.pi) - math.pi)
                sel = geom.torso[diff < sector]
                hem_z = float(sel[:, 2].min()) + 0.005 if len(sel) else g.hem_line
            else:
                hem_z = g.hem_line
            hem_z = min(hem_z, waist_z - 0.15)

            twin = cache.get((col.name, 'l')) if suffix == 'r' else None
            pts = []
            last = len(fractions) - 1
            for k, t in enumerate(fractions):
                z = waist_z + (hem_z - waist_z) * t
                # the hem edge itself is a thin rim: probe a little above it, then drop the point onto the rim
                p = surface_point_robust(geom, frame, theta, z + (0.03 if k == last else 0.0), g.placement,
                                         g.surface_offset)
                if p is not None and k == last:
                    p = p.copy()
                    p[2] = z
                if p is None and twin is not None and twin[0][k] is not None:
                    p = _mirror(frame, twin[0][k])
                pts.append(p)
            if pts[-1] is None and len(geom.torso):
                a = np.arctan2((geom.torso - frame.origin) @ frame.left, (geom.torso - frame.origin) @ frame.forward)
                diff = np.abs((a - theta + math.pi) % (2 * math.pi) - math.pi)
                sel = geom.torso[diff < sector]
                if len(sel):
                    pts[-1] = sel[np.argmin(sel[:, 2])]
            if pts[0] is None or pts[-1] is None:
                plan.skipped.append("%s_%s does not reach the %s" % (prefix, tag, "waist" if pts[0] is None else "hem"))
                continue
            for k in range(1, len(pts) - 1):
                if pts[k] is None:
                    pts[k] = pts[0] + (pts[-1] - pts[0]) * fractions[k]
            if g.path == 'STRAIGHT':
                pts = [pts[0] + (pts[-1] - pts[0]) * fractions[k] for k in range(len(pts))]

            bones = []
            has_upper = False
            if use_upper:
                tz = top_z + col.top_offset
                if tz > waist_z + 0.03:
                    ptop = surface_point_robust(geom, frame, theta, tz, g.placement, g.surface_offset)
                    if ptop is None and twin is not None and twin[1] is not None:
                        ptop = _mirror(frame, twin[1])
                    if ptop is not None:
                        bones.append(PlanBone("%s_%s_00%s" % (prefix, col.name, ("_" + suffix) if suffix else ""),
                                              ptop, pts[0], upper_root, theta))
                        has_upper = True
            for k in range(g.rows):
                name = "%s_%s_%02d%s" % (prefix, col.name, k + 1, ("_" + suffix) if suffix else "")
                if k == 0:
                    parent, connected = lower_root_name, False
                else:
                    parent, connected = bones[-1].name, True
                bones.append(PlanBone(name, pts[k], pts[k + 1], parent, theta, connected))
            if has_upper:
                plan.joined.append((bones[0].name, bones[1].name))
            plan.strips.append((col.name, suffix, bones))
            cache[(col.name, suffix)] = (pts, bones[0].head if has_upper else None)
    return plan
