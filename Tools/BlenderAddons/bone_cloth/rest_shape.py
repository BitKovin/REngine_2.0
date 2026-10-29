"""Step 5: how the cloth hangs at rest (DESIGN.md section 8.5)."""
import json

import bpy
import numpy as np
from mathutils import Matrix, Quaternion, Vector

from . import bones as bones_mod
from . import colliders, frames, mesh_prep, model_data, poses

MAX_EXTRA_STEPS = 20      # 5 mm each: at most 10 cm of extra flare to clear the legs


def solve(setup, arm, frame):
    """{bone: quaternion} for the rest settings, including the extra flare that keeps the cloth off the legs."""
    r = setup.rest
    caps = colliders.capsules(setup, arm) if r.clear_legs else []
    pad = setup.motion.radius + r.clearance
    out, report = {}, []
    for info in bones_mod.strips(setup, arm):
        rig = poses.StripRig(arm, info)
        th = info["theta"]
        sign = 1.0 if th > 1e-3 else (-1.0 if th < -1e-3 else 0.0)
        extra = 0.0
        for _ in range(MAX_EXTRA_STEPS):
            v = (r.flare * poses.column_mult(setup, th, "rest") + extra) * frame.radial(th) \
                + r.open * poses.column_mult(setup, th, "open") * sign * frame.around(th)
            amount = float(np.linalg.norm(v))
            direction = v / amount if amount > 1e-9 else v
            rots = poses.solve_strip(arm, frame, rig, direction, amount, r.bend, 0.3)
            if not caps:
                break
            _, tails = rig.fk(rots)
            if max(colliders.penetration(t, caps, pad) for t in tails[len(tails) // 2:]) <= 0.0:
                break
            extra += 0.005
        if extra > 0:
            report.append("%s +%.1f cm" % (info["bones"][0].replace("_00", "").replace("_01", ""), extra * 100))
        out.update(poses.to_basis(arm, rig, rots))
    return out, report


def preview(setup, arm, frame):
    bases, report = solve(setup, arm, frame)
    poses.set_bases(arm, bases)
    bpy.context.view_layer.update()
    return report


def clear_preview(arm):
    poses.set_bases(arm, {})
    bpy.context.view_layer.update()


def _snapshot(setup, arm):
    snap = {}
    for b in bones_mod.generated(setup) + bones_mod.roots(setup):
        bone = arm.data.bones.get(b)
        if bone:
            snap[b] = {"head": list(bone.head_local), "tail": list(bone.tail_local), "roll": None,
                       "matrix": [list(row) for row in bone.matrix_local]}
    return snap


def apply(setup, arm, frame):
    """Poses the cloth bones into the rest shape and makes that the new rest (bones and meshes)."""
    for pb in arm.pose.bones:
        if "bone_cloth" in pb:
            continue
        if pb.rotation_quaternion.angle > 1e-6 or any(abs(x) > 1e-6 for x in pb.location):
            raise RuntimeError("Reset the pose of the body bones first (%s is posed)" % pb.name)
    for o in mesh_prep.cloth_objects(setup):
        if not mesh_prep._is_identity(mesh_prep.local_matrix(arm, o)):
            raise RuntimeError("%s still has its own transform. Run Fix all in the Mesh step first" % o.name)
    r = setup.rest
    if r.applied:
        reset(setup, arm, frame)
    else:
        r.snapshot = json.dumps(_snapshot(setup, arm))
    bases, report = solve(setup, arm, frame)
    if not bases:
        return report
    poses.set_bases(arm, bases)
    bpy.context.view_layer.update()

    cloth_set = set(bones_mod.generated(setup))
    # the evaluated armature is the one that is always up to date (the original's pose matrices can lag a step)
    arm_eval = arm.evaluated_get(bpy.context.evaluated_depsgraph_get())
    skin, posed = {}, {}
    for name in cloth_set:
        m = arm_eval.pose.bones[name].matrix.copy()
        posed[name] = m
        skin[name] = m @ arm.data.bones[name].matrix_local.inverted()

    for obj in bpy.data.objects:
        if obj.type != 'MESH' or mesh_prep.is_source_copy(obj):
            continue
        if not any(vg.name in cloth_set for vg in obj.vertex_groups):
            continue
        _deform_mesh(obj, arm, skin, cloth_set)

    poses.set_bases(arm, {})
    with bones_mod.mode(arm, 'EDIT'):
        eb = arm.data.edit_bones
        order = [b.name for b in model_data.bone_order(arm) if b.name in cloth_set]
        # lengths first: moving a parent drags the heads of its connected children along
        lengths = {name: eb[name].length for name in order}
        for name in order:
            eb[name].matrix = posed[name]
            eb[name].length = lengths[name]
    r.applied = True
    bpy.context.view_layer.update()
    return report


def _deform_mesh(obj, arm, skin, cloth_set):
    me = obj.data
    n = len(me.vertices)
    # object space -> armature space is the identity for baked cloth (the meshes were baked into the armature's space)
    M = np.zeros((n, 3, 4))
    M[:, :3, :3] = np.eye(3)
    names = {vg.index: vg.name for vg in obj.vertex_groups}
    skin_np = {k: np.array(v)[:3, :] for k, v in skin.items()}
    for i, v in enumerate(me.vertices):
        total = 0.0
        acc = np.zeros((3, 4))
        for g in v.groups:
            nm = names.get(g.group)
            if nm in skin_np and g.weight > 0:
                acc += g.weight * skin_np[nm]
                total += g.weight
        if total > 0:
            rest = np.zeros((3, 4))
            rest[:3, :3] = np.eye(3)
            M[i] = acc + (1.0 - total) * rest

    def transform(coords):
        homo = np.concatenate([coords, np.ones((n, 1))], axis=1)
        return np.einsum('nij,nj->ni', M, homo)

    buf = np.empty(n * 3, dtype=np.float32)
    me.vertices.foreach_get('co', buf)
    me.vertices.foreach_set('co', transform(buf.reshape(n, 3).astype(float)).astype(np.float32).ravel())
    if me.shape_keys:
        for kb in me.shape_keys.key_blocks:
            b = np.empty(n * 3, dtype=np.float32)
            kb.data.foreach_get('co', b)
            kb.data.foreach_set('co', transform(b.reshape(n, 3).astype(float)).astype(np.float32).ravel())
    me.update()


def reset(setup, arm, frame=None):
    """Back to the bones as generated, and the meshes from their source copies."""
    r = setup.rest
    if not r.snapshot:
        r.applied = False
        return
    snap = json.loads(r.snapshot)
    with bones_mod.mode(arm, 'EDIT'):
        eb = arm.data.edit_bones
        order = [b.name for b in model_data.bone_order(arm) if b.name in snap]
        lengths = {name: eb[name].length for name in order}
        for name in order:
            eb[name].matrix = Matrix(snap[name]["matrix"])
            eb[name].length = lengths[name]
    for o in mesh_prep.cloth_objects(setup):
        mesh_prep.restore_from_source(o)
    r.applied = False
    bpy.context.view_layer.update()
