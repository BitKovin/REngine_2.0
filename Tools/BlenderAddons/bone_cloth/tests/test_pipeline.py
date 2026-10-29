import sys, json, math, os
import bpy
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import props, presets, pipeline, doctor, export, model_data, mesh_prep, bones, poses, stages

bone_cloth.register()
arm = bpy.data.objects['root']
setup = props.new_setup(arm, "coat")
r = setup.cloth_objects.add()
r.obj = bpy.data.objects['coat']
presets.apply(setup, 'LONG_COAT')
issues = pipeline.auto_setup(setup, arm)
print("---- issues")
for i in issues:
    print(i.severity, i.id, i.message)
print("---- stages", {k: stages.status(setup, arm, k) for k in stages.ORDER})

skel, verts, strips = doctor.engine_strips(setup, arm, pipeline.frame_of(setup, arm))
print("---- engine strips:", len(strips))
for s in strips:
    print([skel.names[j.bone] for j in s.joints], "side", s.side, "tip %.3f" % s.tip_length)

for c in setup.colliders:
    print("collider", c.bone, c.to, round(c.fitted_radius, 3), round(c.fitted_end, 3))
for kind in ("open", "volume"):
    q = poses.read_action(getattr(setup.poses, kind).action)
    print(kind, "bones", len(q), "max angle deg", round(max(math.degrees(v.angle) for v in q.values()), 1) if q else None)

out = r"C:\Users\bogda_\AppData\Local\Temp\claude\F--Git-mapeditor\e80fbd3e-512f-4dab-a14d-b25501a867ef\scratchpad\export_test"
t = setup.export.targets.add()
t.glb_path = os.path.join(out, "coat_test.glb")
msgs, iss = export.run(setup, arm)
print("---- export", msgs)
print(open(os.path.join(out, "coat_test.glb.skmm")).read())
