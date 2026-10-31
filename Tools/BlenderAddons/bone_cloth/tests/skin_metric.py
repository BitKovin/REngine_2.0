"""Skin distortion of the cloth over every clip's poses (headless): triangles squeezed under 15% of their area, edges over
2.5x or under 0.3x, per pose with BC_ROWS=1. BC_REGEN=1 regenerates the clips first (BC_SET=blow_back=3,lift=0.386);
BC_VARIANTS='name:weight_setting=val,...;...' rebuilds the weights with other settings."""
import os, sys
import bpy, numpy as np
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import props, poses, frames, weights, mesh_prep, engine_contract as ec
bone_cloth.register()
arm = bpy.data.objects["root"]
s = props.get_setup(arm)
if arm.animation_data:
    arm.animation_data.action = None
obj = mesh_prep.cloth_objects(s)[0]


def metric(label):
    R = np.array([v.co for v in obj.data.vertices])
    me0 = obj.data
    me0.calc_loop_triangles()
    tris = np.array([t.vertices for t in me0.loop_triangles])
    ed = np.unique(np.sort(np.concatenate([tris[:, [0, 1]], tris[:, [1, 2]], tris[:, [2, 0]]]), axis=1), axis=0)

    def area(X):
        return np.linalg.norm(np.cross(X[tris[:, 1]] - X[tris[:, 0]], X[tris[:, 2]] - X[tris[:, 0]]), axis=1) / 2

    a0 = area(R)
    l0 = np.linalg.norm(R[ed[:, 0]] - R[ed[:, 1]], axis=1)
    rows = []
    with poses.held_pose(arm):
        for key in ec.CLIP_SLOTS:
            act = poses.clip_slot(s, key).action
            if act is None:
                continue
            for fr in (2, 3):
                b = poses.bases_at(act, fr)
                poses.set_bases(arm, {n: q for n, (q, _) in b.items()}, {n: l for n, (_, l) in b.items()})
                bpy.context.view_layer.update()
                ev = obj.evaluated_get(bpy.context.evaluated_depsgraph_get())
                me = ev.to_mesh()
                P = np.array([v.co for v in me.vertices])
                ev.to_mesh_clear()
                ar = area(P) / np.maximum(a0, 1e-9)
                le = np.linalg.norm(P[ed[:, 0]] - P[ed[:, 1]], axis=1) / np.maximum(l0, 1e-9)
                rows.append((key + str(fr), int((ar < 0.15).sum()), int((le > 2.5).sum()), int((le < 0.3).sum()), float(ar.min())))
    if os.environ.get("BC_ROWS"):
        print("ROWS", [(r[0], r[1], r[2] + r[3]) for r in rows])
    tot = [sum(r[i] for r in rows) for i in (1, 2, 3)]
    print("METRIC %-22s squeezed tris %3d, edges >2.5x %3d, edges <0.3x %3d | worst area %.3f (%s)" % (
        label, tot[0], tot[1], tot[2], min(r[4] for r in rows), max(rows, key=lambda r: r[1] + r[2] + r[3])[0]))


if os.environ.get("BC_REGEN"):
    from bone_cloth import clipgen
    for item in filter(None, os.environ.get("BC_SET", "").split(",")):
        k, v = item.split("=")
        setattr(s.poses.gen, k, float(v))
    print("generated:", clipgen.generate(s, arm, force=True))
metric("current weights")
for spec in filter(None, os.environ.get("BC_VARIANTS", "").split(";")):
    name, sets = spec.split(":")
    saved = {}
    for item in filter(None, sets.split(",")):
        k, v = item.split("=")
        saved[k] = getattr(s.weights, k)
        setattr(s.weights, k, type(saved[k])(float(v)))
    frame = frames.compute_frame(arm, s.character)
    weights.build(s, arm, frame)
    metric(name)
    for k, v in saved.items():
        setattr(s.weights, k, v)
print("DONE")
