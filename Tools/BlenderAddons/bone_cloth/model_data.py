"""Reads the armature and the meshes it skins the way the engine's model loader sees them (DESIGN.md section 2)."""
import numpy as np

from . import engine_contract as ec


def bone_order(arm):
    """Bones parents-first."""
    out, seen = [], set()

    def visit(b):
        if b.name in seen:
            return
        if b.parent:
            visit(b.parent)
        seen.add(b.name)
        out.append(b)

    for b in arm.data.bones:
        visit(b)
    return out


def skeleton(arm):
    bones = bone_order(arm)
    index = {b.name: i for i, b in enumerate(bones)}
    names = [b.name for b in bones]
    parent = [index[b.parent.name] if b.parent else -1 for b in bones]
    mw = np.array(arm.matrix_world)
    rest = np.array([mw @ np.array(b.matrix_local) for b in bones])
    return ec.Skeleton(names, parent, rest)


def skinned_meshes(arm, include_hidden=False):
    import bpy
    out = []
    for o in bpy.context.scene.objects:
        if o.type != 'MESH' or "bone_cloth_source_of" in o:
            continue
        skinned = any(m.type == 'ARMATURE' and m.object == arm for m in o.modifiers) or o.parent == arm
        if not skinned:
            continue
        if not include_hidden and (o.hide_viewport or o.hide_get()):
            continue
        out.append(o)
    return out


def mesh_vertices(obj, skel, limit=True):
    """(world positions, bones (n,4), weights (n,4)) of one object at rest; weights limited to 4 and normalised like the
    glTF exporter does."""
    me = obj.data
    n = len(me.vertices)
    pos = np.empty(n * 3, dtype=np.float32)
    me.vertices.foreach_get('co', pos)
    pos = pos.reshape(n, 3).astype(float)
    mw = np.array(obj.matrix_world)
    pos = pos @ mw[:3, :3].T + mw[:3, 3]

    group_bone = {vg.index: skel.index.get(vg.name, -1) for vg in obj.vertex_groups}
    bones = np.full((n, ec.MAX_BONE_INFLUENCE), -1, dtype=int)
    weights = np.zeros((n, ec.MAX_BONE_INFLUENCE))
    for i, v in enumerate(me.vertices):
        pairs = [(group_bone.get(g.group, -1), g.weight) for g in v.groups if g.weight > 0.0]
        pairs = [(b, w) for b, w in pairs if b >= 0]
        if not pairs:
            continue
        pairs.sort(key=lambda p: -p[1])
        pairs = pairs[:ec.MAX_BONE_INFLUENCE]
        total = sum(w for _, w in pairs)
        for k, (b, w) in enumerate(pairs):
            bones[i, k] = b
            weights[i, k] = w / total if limit and total > 0 else w
    return pos, bones, weights


def collect(arm, objects=None):
    """Skeleton plus the vertices of every exported mesh, concatenated."""
    skel = skeleton(arm)
    objects = objects if objects is not None else skinned_meshes(arm)
    P, B, W = [], [], []
    for o in objects:
        p, b, w = mesh_vertices(o, skel)
        P.append(p)
        B.append(b)
        W.append(w)
    if not P:
        return skel, ec.Vertices(np.zeros((0, 3)), np.zeros((0, 4), int), np.zeros((0, 4)))
    return skel, ec.Vertices(np.vstack(P), np.vstack(B), np.vstack(W))
