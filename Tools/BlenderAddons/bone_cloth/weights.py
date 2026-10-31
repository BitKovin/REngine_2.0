"""Step 4: cloth amount, blending between and along panels, and the final weights (DESIGN.md section 8.4)."""
import bmesh
import bpy
import numpy as np
from mathutils import Vector, kdtree
from mathutils.bvhtree import BVHTree

from . import bones as bones_mod
from . import frames, landmarks, mesh_prep

AMOUNT_GROUP = "BC amount"
CURVE_TREE = ".BoneCloth"

SMOOTH_PTS = [(0.0, 0.0), (0.25, 0.1), (0.5, 0.5), (0.75, 0.9), (1.0, 1.0)]
BELT_PTS = [(0.0, 0.0), (0.54, 0.40), (0.68, 0.45), (1.0, 1.0)]


# ---- the blend curve ("how it goes") -------------------------------------------------------------------------------

def curve_node(setup, create=True):
    tree = bpy.data.node_groups.get(CURVE_TREE)
    if tree is None:
        if not create:
            return None
        tree = bpy.data.node_groups.new(CURVE_TREE, 'ShaderNodeTree')
        tree.use_fake_user = True
    name = setup.id + ".blend"
    node = tree.nodes.get(name)
    if node is None and create:
        node = tree.nodes.new('ShaderNodeFloatCurve')
        node.name = name
        set_curve(node, SMOOTH_PTS)
    return node


def set_curve(node, pts):
    curve = node.mapping.curves[0]
    while len(curve.points) > 2:
        curve.points.remove(curve.points[1])
    curve.points[0].location = pts[0]
    curve.points[-1].location = pts[-1]
    for x, y in pts[1:-1]:
        curve.points.new(x, y)
    for p in curve.points:
        p.handle_type = 'AUTO'
    node.mapping.update()


def sample_curve(setup, n=65):
    node = curve_node(setup)
    node.mapping.initialize()
    curve = node.mapping.curves[0]
    xs = np.linspace(0.0, 1.0, n)
    ys = np.array([node.mapping.evaluate(curve, float(x)) for x in xs])
    return xs, np.clip(ys, 0.0, 1.0)


# ---- cloth amount ---------------------------------------------------------------------------------------------------

def _smooth(a, b, x):
    t = np.clip((x - a) / max(b - a, 1e-9), 0.0, 1.0)
    return t * t * (3 - 2 * t)


def read_body_weights(src, bone_names, skip):
    """Dense (N, G) original weights for the groups that are bones not in `skip`; returns (W, names)."""
    me = src.data
    n = len(me.vertices)
    cols, names = {}, []
    for vg in src.vertex_groups:
        if vg.name in bone_names and vg.name not in skip:
            cols[vg.index] = len(names)
            names.append(vg.name)
    W = np.zeros((n, len(names)))
    for i, v in enumerate(me.vertices):
        for g in v.groups:
            c = cols.get(g.group)
            if c is not None:
                W[i, c] = g.weight
    return W, names


STRAY_NEAR, STRAY_FAR = 0.10, 0.20     # metres from an excluded bone: arm weights count fully / not at all


def exclude_segments(arm, exclude):
    """(head, tail) of every excluded bone (arms, neck, head), world space."""
    return [(frames.bone_head(arm, n), frames.bone_tail(arm, n)) for n in exclude if n in arm.data.bones]


def _distance_to_segments(pos, segments):
    best = np.full(len(pos), np.inf)
    for a, b in segments:
        ab = b - a
        l2 = float(ab @ ab)
        t = np.clip(((pos - a) @ ab) / l2, 0.0, 1.0) if l2 > 1e-12 else np.zeros(len(pos))
        best = np.minimum(best, np.linalg.norm(pos - (a + np.outer(t, ab)), axis=1))
    return best


def cloth_amount(setup, pos, W_body, body_names, exclude, painted=None, segments=None):
    """c(v) in 0..1, DESIGN.md 8.4.2. segments: where the excluded bones are - arm weights far from them are stray paint
    (a back vertex weighted to an upper arm in the source mesh) and keep nothing on the body."""
    w = setup.weights
    if w.use_painted_amount and painted is not None:
        return np.clip(painted, 0.0, 1.0)
    h = pos[:, 2]
    u = np.clip((w.blend_start - h) / max(w.blend_start - w.blend_full, 1e-6), 0.0, 1.0)
    xs, ys = sample_curve(setup)
    c = np.interp(u, xs, ys)
    for pin in w.pins:
        p = pin.strength * (1.0 - _smooth(pin.half_width, pin.half_width + max(pin.softness, 1e-6), np.abs(h - pin.height)))
        c = c * (1.0 - p)
    if w.exclude_arms and exclude and W_body.shape[1]:
        cols = [i for i, n in enumerate(body_names) if n in exclude]
        if cols:
            a = W_body[:, cols].sum(axis=1)
            if segments:
                a = a * (1.0 - _smooth(STRAY_NEAR, STRAY_FAR, _distance_to_segments(pos, segments)))
            c = c * (1.0 - _smooth(0.5 * w.exclude_threshold, w.exclude_threshold, a))
    return np.clip(c, 0.0, 1.0)


# ---- panels ---------------------------------------------------------------------------------------------------------

class StripGeo:
    def __init__(self, arm, frame, info):
        self.bones = info["bones"]
        self.side = info["side"]
        self.theta_top = info["theta"]
        heads = [frames.bone_head(arm, b) for b in self.bones]
        tail = frames.bone_tail(arm, self.bones[-1])
        pts = heads + [tail]
        z = np.array([p[2] for p in pts])
        # strictly decreasing heights so interpolation is defined
        z = np.minimum.accumulate(z - np.arange(len(z)) * 1e-5)
        self.z = z
        self.theta = np.array([frame.angle(p) for p in pts])
        if self.side == '':
            self.theta[:] = self.theta_top
        self.n = len(self.bones)
        self.strand = bool(info.get("strand"))
        self.points = np.array(heads + [tail])

    def arc_param(self, pos):
        """Closest point of the polyline for each position: (distance along it, its total length), metres."""
        seg = self.points[1:] - self.points[:-1]
        length = np.linalg.norm(seg, axis=1)
        starts = np.concatenate([[0.0], np.cumsum(length)[:-1]])
        best = np.full(len(pos), np.inf)
        arc = np.zeros(len(pos))
        for k in range(len(seg)):
            d = seg[k] / max(length[k], 1e-9)
            u = np.clip((pos - self.points[k]) @ d, 0.0, length[k])
            dist = np.linalg.norm(pos - (self.points[k] + np.outer(u, d)), axis=1)
            better = dist < best
            best[better] = dist[better]
            arc[better] = (starts[k] + u)[better]
        return arc, float(length.sum())

    def sigma_strand(self, pos):
        arc, total = self.arc_param(pos)
        seg = np.linalg.norm(self.points[1:] - self.points[:-1], axis=1)
        edges = np.concatenate([[0.0], np.cumsum(seg)])
        return np.interp(arc, edges, np.arange(len(edges), dtype=float))

    def angle_at(self, h):
        return np.interp(h, self.z[::-1], self.theta[::-1])

    def sigma(self, h):
        return np.interp(h, self.z[::-1], np.arange(len(self.z))[::-1].astype(float))


def _along(strip, h, softness, pos=None):
    """V (N, n): how the strip's own bones share a vertex at height h (or, for a strand, along it)."""
    n = strip.n
    sigma = strip.sigma_strand(pos) if strip.strand else strip.sigma(h)
    if n == 1:
        return np.ones((len(h), 1))
    b = softness
    beta = []
    for j in range(1, n):
        if b <= 1e-6:
            beta.append((sigma >= j).astype(float))
        else:
            beta.append(_smooth(j - b, j + b, sigma))
    V = np.zeros((len(h), n))
    V[:, 0] = 1.0 - beta[0]
    for k in range(1, n - 1):
        V[:, k] = beta[k - 1] - beta[k]
    V[:, n - 1] = beta[n - 2]
    return V


def _sharpen(t, k):
    t = np.clip((t - 0.5) / max(1.0 - k, 1e-6) + 0.5, 0.0, 1.0)
    return t * t * (3 - 2 * t)


def _around(strips, theta, h, closed, vent, sharp):
    """U (N, S): how the panels share a vertex, by angle."""
    N, S = len(theta), len(strips)
    U = np.zeros((N, S))
    for side_sign in (1, -1):
        sel = (theta >= 0) if side_sign == 1 else (theta < 0)
        idx = np.where(sel)[0]
        if not len(idx):
            continue
        th = theta[idx] * side_sign
        hh = h[idx]
        cand = []   # (strip index, function h -> angle in this side's frame)
        for si, s in enumerate(strips):
            if s.side == '':
                cand.append((si, lambda hv, s=s: np.full(len(hv), abs(s.theta_top) if abs(s.theta_top) > 1e-6 else 0.0)))
            elif (s.side == 'l') == (side_sign == 1):
                cand.append((si, lambda hv, s=s: np.abs(s.angle_at(hv))))
            else:
                if closed:
                    cand.append((si, lambda hv, s=s: -np.abs(s.angle_at(hv))))
                if not vent:
                    cand.append((si, lambda hv, s=s: 2 * np.pi - np.abs(s.angle_at(hv))))
        if not cand:
            continue
        A = np.column_stack([fn(hh) for _, fn in cand])
        order = np.argsort(A, axis=1)
        As = np.take_along_axis(A, order, axis=1)
        sidx = np.array([si for si, _ in cand])[order]
        m = A.shape[1]
        cnt = (As <= th[:, None]).sum(axis=1)
        j = cnt - 1
        rows = np.arange(len(idx))
        lo = j < 0
        hi = j >= m - 1
        mid = ~lo & ~hi
        U[idx[lo], sidx[rows[lo], 0]] += 1.0
        U[idx[hi], sidx[rows[hi], m - 1]] += 1.0
        if mid.any():
            r = rows[mid]
            a, b = As[r, j[mid]], As[r, j[mid] + 1]
            t = _sharpen((th[mid] - a) / np.maximum(b - a, 1e-6), sharp)
            np.add.at(U, (idx[mid], sidx[r, j[mid]]), 1.0 - t)
            np.add.at(U, (idx[mid], sidx[r, j[mid] + 1]), t)
    return U


# ---- mesh helpers ---------------------------------------------------------------------------------------------------

def _components(n, e0, e1):
    parent = np.arange(n)

    def find(x):
        while parent[x] != x:
            parent[x] = parent[parent[x]]
            x = parent[x]
        return x

    for a, b in zip(e0, e1):
        ra, rb = find(a), find(b)
        if ra != rb:
            parent[ra] = rb
    return np.array([find(i) for i in range(n)])


def _limit4(W, c, is_cloth, bone_count=4):
    N, G = W.shape
    R = W.copy()
    cloth_cols = np.where(is_cloth)[0]
    if len(cloth_cols) and N:
        sub = W[:, cloth_cols]
        k = min(2, len(cloth_cols))
        top = np.argsort(-sub, axis=1)[:, :k]
        sel = (c >= 0.5)[:, None] & (np.take_along_axis(sub, top, axis=1) > 0)
        rr, kk = np.nonzero(sel)
        R[rr, cloth_cols[top[rr, kk]]] += 1.0
    keep = np.argsort(-R, axis=1)[:, :bone_count]
    out = np.zeros_like(W)
    rows = np.arange(N)[:, None]
    out[rows, keep] = W[rows, keep]
    return out


# ---- build ----------------------------------------------------------------------------------------------------------

def build_object(setup, arm, frame, obj, strip_infos, stats):
    src = mesh_prep.source_of(obj) or obj
    if len(src.data.vertices) != len(obj.data.vertices):
        raise RuntimeError("%s was edited after the setup was made (%d vertices now, %d in the kept copy). Press Fix in the "
                           "Mesh step first" % (obj.name, len(obj.data.vertices), len(src.data.vertices)))
    me = src.data
    n = len(me.vertices)
    co = np.empty(n * 3, dtype=np.float32)
    me.vertices.foreach_get('co', co)
    mw = np.array(mesh_prep.world_matrix(src))
    pos = co.reshape(n, 3).astype(float) @ mw[:3, :3].T + mw[:3, 3]
    w = setup.weights

    cloth_names = set(bones_mod.generated(setup))
    bone_names = set(b.name for b in arm.data.bones)
    W_body, body_names = read_body_weights(src, bone_names, cloth_names)
    exclude = landmarks.exclude_bones(arm, setup.character)

    painted = None
    vg_amount = obj.vertex_groups.get(AMOUNT_GROUP)
    if vg_amount is not None:
        painted = np.zeros(n)
        for i, v in enumerate(obj.data.vertices):
            for g in v.groups:
                if g.group == vg_amount.index:
                    painted[i] = g.weight
    strips = [StripGeo(arm, frame, s) for s in strip_infos]
    if strips and strips[0].strand:
        # a strand is loose from its start on: the cloth amount grows along it, the first 12% blends with the body
        arc, total = strips[0].arc_param(pos)
        c = _smooth(0.0, max(0.12 * total, 0.03), arc)
    else:
        c = cloth_amount(setup, pos, W_body, body_names, exclude, painted, exclude_segments(arm, exclude))
    theta = frame.angle(pos)
    h = pos[:, 2]
    closed = setup.guides.layout == 'CLOSED_RING'
    U = _around(strips, theta, h, closed, setup.guides.back_vent, w.panel_sharpness)

    cloth_cols = []
    for s in strips:
        cloth_cols.extend(s.bones)
    G_body = W_body.shape[1]
    W_cloth = np.zeros((n, len(cloth_cols)))
    col = 0
    for si, s in enumerate(strips):
        V = _along(s, h, w.joint_softness, pos)
        W_cloth[:, col:col + s.n] = U[:, si:si + 1] * V
        col += s.n
    total = W_cloth.sum(axis=1, keepdims=True)
    W_cloth = np.where(total > 1e-9, W_cloth / np.maximum(total, 1e-9), W_cloth)

    body_total = W_body.sum(axis=1, keepdims=True)
    W_body_n = np.where(body_total > 1e-9, W_body / np.maximum(body_total, 1e-9), W_body)
    # a vertex with no body weights at all that the profile leaves on the body gets the cloth anyway
    c = np.where(body_total[:, 0] <= 1e-9, 1.0, c)
    W = np.hstack([(1.0 - c)[:, None] * W_body_n, c[:, None] * W_cloth])
    names = body_names + cloth_cols
    is_cloth = np.array([False] * G_body + [True] * len(cloth_cols))

    e = np.empty(len(me.edges) * 2, dtype=np.int32)
    me.edges.foreach_get('vertices', e)
    e0, e1 = e[0::2].astype(int), e[1::2].astype(int)

    # small loose parts move rigidly with the cloth under them
    if w.rigid_parts and n:
        comp = _components(n, e0, e1)
        ids, counts = np.unique(comp, return_counts=True)
        big = ids[np.argmax(counts)]
        small = []
        for cid in ids:
            if cid == big:
                continue
            m = comp == cid
            size = np.linalg.norm(pos[m].max(axis=0) - pos[m].min(axis=0))
            if size <= w.rigid_max_size:
                small.append(cid)
        if small:
            main = np.where(np.isin(comp, [i for i in ids if i not in small]))[0]
            tree = kdtree.KDTree(len(main))
            for k, i in enumerate(main):
                tree.insert(pos[i], k)
            tree.balance()
            for cid in small:
                m = np.where(comp == cid)[0]
                _, k, _ = tree.find(pos[m].mean(axis=0))
                W[m] = W[main[k]]
                c[m] = c[main[k]]
            stats['rigid_parts'] = stats.get('rigid_parts', 0) + len(small)

    # smoothing, not across the front opening
    if w.smooth_iterations and w.smooth_strength > 0 and len(e0):
        ok = np.ones(len(e0), dtype=bool)
        if not closed:
            front = np.abs(theta) < (setup.guides.opening_half_angle + np.radians(20.0))
            cross = front[e0] & front[e1] & ((theta[e0] >= 0) != (theta[e1] >= 0))
            ok &= ~cross
        a, b = e0[ok], e1[ok]
        deg = np.bincount(np.concatenate([a, b]), minlength=n).astype(float)
        free = (c > 0) & (deg > 0)
        for _ in range(w.smooth_iterations):
            S = np.zeros_like(W)
            np.add.at(S, a, W[b])
            np.add.at(S, b, W[a])
            avg = S / np.maximum(deg, 1.0)[:, None]
            lam = w.smooth_strength
            W = np.where(free[:, None], (1.0 - lam) * W + lam * avg, W)

    stats['over4'] = stats.get('over4', 0) + int(((W > 1e-4).sum(axis=1) > 4).sum())
    W = _limit4(W, c, is_cloth)
    W = np.where(W < w.clean_below, 0.0, W)
    tot = W.sum(axis=1, keepdims=True)
    W = np.where(tot > 1e-9, W / np.maximum(tot, 1e-9), W)

    _write(arm, obj, names, W, set(cloth_cols))
    stats['vertices'] = stats.get('vertices', 0) + n
    stats['moving'] = stats.get('moving', 0) + int((c > 0.5).sum())
    stats['blended'] = stats.get('blended', 0) + int(((U > 0.1).sum(axis=1) > 1).sum())
    col = G_body
    per_bone = {}
    for s in strips:
        for k, b in enumerate(s.bones):
            col_i = names.index(b)
            per_bone[b] = (int((W[:, col_i] >= 0.05).sum()), int((W[:, col_i] >= 0.25).sum()))
    for b, (a, t) in per_bone.items():
        stats.setdefault('bones', {})
        old = stats['bones'].get(b, (0, 0))
        stats['bones'][b] = (old[0] + a, old[1] + t)


def _write(arm, obj, names, W, cloth_names):
    bone_names = set(b.name for b in arm.data.bones)
    for nm in names:
        if nm not in obj.vertex_groups:
            obj.vertex_groups.new(name=nm)
    index = {vg.name: vg.index for vg in obj.vertex_groups}
    non_bone = {vg.index for vg in obj.vertex_groups if vg.name not in bone_names}
    cols = [(index[nm], j) for j, nm in enumerate(names)]
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    dl = bm.verts.layers.deform.verify()
    for v in bm.verts:
        d = v[dl]
        kept = {k: d[k] for k in list(d.keys()) if k in non_bone}
        d.clear()
        row = W[v.index]
        for gi, j in cols:
            if row[j] > 0.0:
                d[gi] = float(row[j])
        for k, val in kept.items():
            d[k] = val
    bm.to_mesh(obj.data)
    bm.free()
    obj.data.update()
    # drop groups of bones that ended up empty. Decide by name first: removing a group renumbers the ones after it
    used = set()
    for v in obj.data.vertices:
        for g in v.groups:
            used.add(g.group)
    empty = [vg.name for vg in obj.vertex_groups if vg.name in cloth_names and vg.index not in used]
    for name in empty:
        obj.vertex_groups.remove(obj.vertex_groups[name])


def build(setup, arm, frame):
    """Builds the weights of every cloth object. Returns the stats dict."""
    infos = bones_mod.strips(setup, arm)
    if not infos:
        raise RuntimeError("Build the bones first")
    stats = {}
    for obj in mesh_prep.cloth_objects(setup):
        if mesh_prep.source_of(obj) is None:
            mesh_prep.ensure_source(obj)
        build_object(setup, arm, frame, obj, infos, stats)
    return stats


def write_amount_group(setup, arm, frame, obj):
    """Shows the computed cloth amount as the paintable group 'BC amount'."""
    src = mesh_prep.source_of(obj) or obj
    me = src.data
    n = len(me.vertices)
    co = np.empty(n * 3, dtype=np.float32)
    me.vertices.foreach_get('co', co)
    mw = np.array(mesh_prep.world_matrix(src))
    pos = co.reshape(n, 3).astype(float) @ mw[:3, :3].T + mw[:3, 3]
    W_body, names = read_body_weights(src, set(b.name for b in arm.data.bones), set(bones_mod.generated(setup)))
    was = setup.weights.use_painted_amount
    setup.weights.use_painted_amount = False
    excl = landmarks.exclude_bones(arm, setup.character)
    c = cloth_amount(setup, pos, W_body, names, excl, None, exclude_segments(arm, excl))
    setup.weights.use_painted_amount = was
    vg = obj.vertex_groups.get(AMOUNT_GROUP) or obj.vertex_groups.new(name=AMOUNT_GROUP)
    vg.remove(range(n))
    for i in range(n):
        if c[i] > 0:
            vg.add([i], float(c[i]), 'REPLACE')
    return c


# ---- mirroring ------------------------------------------------------------------------------------------------------

MIRROR_MIDDLE = 0.005      # vertices this close to the centre plane keep their weights


def mirror_name(name):
    """The bone (group) on the other side: coat_front_01_l <-> coat_front_01_r, thigh_l <-> thigh_r, .L <-> .R."""
    for a, b in (("_l", "_r"), ("_r", "_l"), (".L", ".R"), (".R", ".L"), ("Left", "Right"), ("Right", "Left")):
        if name.endswith(a):
            return name[:-len(a)] + b
    if ":Left" in name:
        return name.replace(":Left", ":Right")
    if ":Right" in name:
        return name.replace(":Right", ":Left")
    return name


def mirror_object(setup, arm, obj, source='L'):
    """Gives every vertex on the other side the weights found at its mirror image on the `source` side ('L' or 'R') of
    the mesh - interpolated across the face that point lands on, left and right bones swapped - so both sides move
    alike even where the two halves of the mesh are not built alike. Weights are limited to 4 and normalised. Returns
    the number of vertices written."""
    frame = frames.compute_frame(arm, setup.character)
    me = obj.data
    mw = obj.matrix_world
    pos = np.array([tuple(mw @ v.co) for v in me.vertices])
    side = (pos - frame.origin) @ frame.left            # > 0: the character's left
    sign = 1.0 if source == 'L' else -1.0
    me.calc_loop_triangles()
    tris = [t for t in me.loop_triangles if all(side[i] * sign >= -MIRROR_MIDDLE for i in t.vertices)]
    if not tris:
        raise RuntimeError("The %s side of %s has no faces to copy from" % ("left" if source == 'L' else "right", obj.name))
    tree = BVHTree.FromPolygons([tuple(pos[i]) for i in range(len(pos))], [tuple(t.vertices) for t in tris])
    groups = {g.index: g.name for g in obj.vertex_groups}
    weights = [{groups[g.group]: g.weight for g in v.groups if g.weight > 0.0} for v in me.vertices]
    written = {}
    for i in np.where(side * sign < -MIRROR_MIDDLE)[0]:
        p = pos[i] - 2.0 * side[i] * frame.left             # the mirror image, on the source side
        hit, _, face, _ = tree.find_nearest(Vector(p))
        if hit is None:
            continue
        a, b, c = (pos[k] for k in tris[face].vertices)
        bary = _barycentric(np.array(hit), a, b, c)
        acc = {}
        for k, t in zip(tris[face].vertices, bary):
            for name, w in weights[k].items():
                acc[mirror_name(name)] = acc.get(mirror_name(name), 0.0) + w * t
        top = sorted(acc.items(), key=lambda kv: -kv[1])[:4]
        total = sum(w for _, w in top)
        written[int(i)] = {n: w / total for n, w in top if w > 1e-4} if total > 0 else {}
    for name in {n for w in written.values() for n in w}:
        if name not in obj.vertex_groups:
            obj.vertex_groups.new(name=name)
    for i, w in written.items():
        for g in list(me.vertices[i].groups):
            if groups.get(g.group) in arm.data.bones or groups.get(g.group) is None:
                obj.vertex_groups[g.group].remove([i])
        for name, value in w.items():
            obj.vertex_groups[name].add([i], value, 'REPLACE')
    me.update()
    return len(written)


def _barycentric(p, a, b, c):
    v0, v1, v2 = b - a, c - a, p - a
    d00, d01, d11 = v0 @ v0, v0 @ v1, v1 @ v1
    d20, d21 = v2 @ v0, v2 @ v1
    den = d00 * d11 - d01 * d01
    if abs(den) < 1e-18:
        return np.array([1.0, 0.0, 0.0])
    v = (d11 * d20 - d01 * d21) / den
    w = (d00 * d21 - d01 * d20) / den
    t = np.clip(np.array([1.0 - v - w, v, w]), 0.0, 1.0)
    return t / max(t.sum(), 1e-12)


def mirror(setup, arm, source='L'):
    """mirror_object on every cloth object. Returns the number of vertices written."""
    return sum(mirror_object(setup, arm, obj, source) for obj in mesh_prep.cloth_objects(setup))
