"""Short jacket that follows the spine. Run on a saved state of the jacket scene (BC_TEST_BLEND, BC_NO_PREPARE=1):
Auto Setup with the JACKET preset, the engine's view of the strips, then a strong spine bend compared with the mesh as
it deforms with its original body weights."""
import math, os, sys
import bpy
import numpy as np
from mathutils import Quaternion, Vector
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import props, presets, pipeline, doctor, bones, mesh_prep, export
bone_cloth.register()
arm = bpy.data.objects["root"]
bpy.context.view_layer.objects.active = arm
s = props.get_setup(arm)
if s is not None:
    bpy.ops.bone_cloth.start_over()
else:
    s = props.new_setup(arm, "coat")
    s.cloth_objects.add().obj = bpy.data.objects[os.environ.get("BC_CLOTH", "coat_")]
presets.apply(s, "JACKET")
issues = pipeline.auto_setup(s, arm, log=lambda m: print("  log:", m))
print("roots:", bones.roots(s))
print("issues:", [(i.severity, i.id, i.message) for i in issues])
frame = pipeline.frame_of(s, arm)
skel, verts, strips = doctor.engine_strips(s, arm, frame)
for st in strips:
    print("strip", [skel.names[j.bone] for j in st.joints], "side", st.side, "tip %.2f" % st.tip_length)

# ---- spine bend: the jacket must go where the original (body-weighted) mesh goes
cloth = mesh_prep.cloth_objects(s)[0]
src = mesh_prep.source_of(cloth)
ref = bpy.data.objects.new("ref", src.data.copy())
ref.parent = arm
ref.matrix_parent_inverse = src.matrix_parent_inverse.copy()
ref.matrix_basis = src.matrix_basis.copy()
bpy.context.scene.collection.objects.link(ref)
for vg in src.vertex_groups:
    if vg.name not in ref.vertex_groups:
        ref.vertex_groups.new(name=vg.name)
mod = ref.modifiers.new("Armature", 'ARMATURE'); mod.object = arm
bpy.context.view_layer.update()

def evaluated(o):
    ev = o.evaluated_get(bpy.context.evaluated_depsgraph_get())
    me = ev.to_mesh()
    buf = np.empty(len(me.vertices) * 3, dtype=np.float32)
    me.vertices.foreach_get('co', buf)
    ev.to_mesh_clear()
    return buf.reshape(-1, 3)

def bend(deg):
    for name, f in (("spine_01", 0.3), ("spine_02", 0.4), ("spine_03", 0.3)):
        pb = arm.pose.bones[name]
        pb.rotation_mode = 'QUATERNION'
        pb.rotation_quaternion = Quaternion((1, 0, 0), math.radians(deg * f))
    bpy.context.view_layer.update()

bend(0)
a0, b0 = evaluated(cloth), evaluated(ref)
bend(60)
a1, b1 = evaluated(cloth), evaluated(ref)
bend(0)
da, db = (a1 - a0), (b1 - b0)
err = np.linalg.norm(da - db, axis=1)          # bending behaviour only, cm
z = np.array([(cloth.matrix_world @ Vector(v)).z for v in a0])
print("bend 60: displacement mismatch (cm) by height band; ref mean motion in band")
for lo, hi in ((1.45, 2), (1.3, 1.45), (1.15, 1.3), (1.0, 1.15), (0.85, 1.0)):
    m = (z >= lo) & (z < hi)
    if m.any():
        print("  z %.2f-%.2f  n=%3d  mismatch mean %.2f max %.2f   |ref motion| mean %.1f" % (lo, hi, m.sum(), err[m].mean(), err[m].max(), np.linalg.norm(db[m], axis=1).mean()))
print("overall mean %.2f, 95%% %.2f, max %.2f" % (err.mean(), np.percentile(err, 95), err.max()))
bend(0)
out = os.path.join(os.environ.get("BC_OUT", os.environ.get("TEMP", ".")), "jacket_test")
t = s.export.targets.add(); t.glb_path = os.path.join(out, "jacket.glb")
msgs, iss = export.run(s, arm)
print("export:", msgs[-1][-60:])
