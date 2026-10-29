import os, sys, math
import bpy
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import props, doctor, stages, poses, bones, mesh_prep
bone_cloth.register()

arm = bpy.data.objects['root']
bpy.context.view_layer.objects.active = arm
arm.select_set(True)
ops = bpy.ops.bone_cloth
print("create:", ops.create(name='coat', preset='LONG_COAT', mesh='coat'))
setup = props.get_setup(arm)
print("character:", setup.character.hips, setup.character.chest, [o.obj.name for o in setup.cloth_objects])
print("auto:", ops.auto_setup())
print("stages:", {k: stages.status(setup, arm, k) for k in stages.ORDER})
print("issues:", [(i.severity, i.id) for i in doctor.LAST[setup.id]])

# preview sliders
setup.poses.preview_open = 1.0
setup.poses.preview_volume = 0.5
pb = arm.pose.bones['coat_front_02_l']
print("preview basis angle deg:", round(math.degrees(pb.rotation_quaternion.angle), 2))
print("hem moved cm:", round((arm.pose.bones['coat_front_03_l'].tail - arm.data.bones['coat_front_03_l'].tail_local).length * 0.01 * 100, 2) if False else "n/a")
ops.pose_preview_reset()
print("preview reset angle:", round(pb.rotation_quaternion.angle, 4))

# change a setting -> stale -> update all
setup.guides.rows = 4
print("after rows=4:", {k: stages.status(setup, arm, k) for k in stages.ORDER if stages.status(setup, arm, k) != 'ok'})
print("update_all:", ops.update_all())
print("after update:", {k: stages.status(setup, arm, k) for k in stages.ORDER})
print("bones built:", len(bones.generated(setup)))

# hand edit: pose a bone, save as open
arm.pose.bones['coat_front_02_l'].rotation_quaternion = (0.9, 0.1, 0.3, 0.0)
print("capture:", ops.pose_capture(kind='open'))
print("open mode:", setup.poses.open.mode)

# check + export
print("check:", ops.check())
out = r"C:\Users\bogda_\AppData\Local\Temp\claude\F--Git-mapeditor\e80fbd3e-512f-4dab-a14d-b25501a867ef\scratchpad\export_test2"
t = setup.export.targets.add()
t.glb_path = os.path.join(out, "coat_test.glb")
print("export:", ops.export())
print("skmm exists:", os.path.exists(os.path.join(out, "coat_test.glb.skmm")), "glb exists:", os.path.exists(os.path.join(out, "coat_test.glb")))
# start over
print("start_over:", ops.start_over())
print("bones left:", len(bones.generated(setup)), "coat verts", len(bpy.data.objects['coat'].data.vertices), "groups", [g.name for g in bpy.data.objects['coat'].vertex_groups][:5])
