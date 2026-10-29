"""Test helper: turns the copied player_coat.blend into a 'beginner start' - the old hand-made cloth rig removed, the old
body-weighted `coat` mesh as the cloth to rig. Run inside Blender: exec(open(path).read())"""
import bpy

arm = bpy.data.objects['root']
view = bpy.context.view_layer
if arm.animation_data:
    for d in list(arm.animation_data.drivers):
        arm.animation_data.drivers.remove(d)
    arm.animation_data.action = None
view.objects.active = arm
for o in bpy.context.scene.objects:
    o.select_set(False)
if arm.mode != 'OBJECT':
    bpy.ops.object.mode_set(mode='OBJECT')
bpy.ops.object.mode_set(mode='EDIT')
eb = arm.data.edit_bones
kill = [b.name for b in eb if b.name.startswith(('coat_', 'cloth_root', 'MCH-'))]
for n in kill:
    eb.remove(eb[n])
bpy.ops.object.mode_set(mode='OBJECT')
for o in list(bpy.data.objects):
    if o.name in ('coat.001',):
        bpy.data.objects.remove(o, do_unlink=True)
coat = bpy.data.objects['coat']
coat.hide_viewport = False
coat.hide_set(False)
for pb in arm.pose.bones:
    pb.location = (0, 0, 0)
    pb.rotation_quaternion = (1, 0, 0, 0)
    pb.rotation_euler = (0, 0, 0)
    pb.scale = (1, 1, 1)
bpy.context.scene.frame_set(1)
print("prepared: removed", len(kill), "bones; bones left:", len(arm.data.bones))
