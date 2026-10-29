"""Adopting the hand-made coat rig of player_coat.blend with its .skmm. Run on an UNPREPARED copy of that blend:
BC_TEST_BLEND=<copy of SlasherGame/player_coat.blend> BC_NO_PREPARE=1"""
import json, os, sys
import bpy
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import props, adopt, doctor, bones, stages
bone_cloth.register()

arm = bpy.data.objects["root"]
bpy.context.view_layer.objects.active = arm
n_bones = len(arm.data.bones)
setup = props.new_setup(arm, "coat")
skmm = r"F:\Git\SdlWasmEngine\source\Games\SlasherGame\GameData\models\player\player_coat.glb.skmm"
print("invoking the operator through bpy.ops")
r = bpy.ops.bone_cloth.adopt('EXEC_DEFAULT', roots="", skmm_path=skmm)
print("operator:", r)
setup = props.get_setup(arm)
infos = bones.strips(setup, arm)
print("strips:", len(infos), [(i["column"], i["side"], len(i["bones"])) for i in infos])
print("roots:", bones.roots(setup), "| cloth meshes:", [x.obj.name for x in setup.cloth_objects])
m = setup.motion
print("motion from the file: followRoot %s followTip %s trail %s maxAngleTip %s volume %s spinOpen %s" % (m.followRoot, m.followTip, m.trail, m.maxAngleTip, m.volume, m.spinOpen))
print("colliders:", [(c.bone, c.to, c.auto_radius) for c in setup.colliders])
print("clips:", setup.poses.open.clip_name, bool(setup.poses.open.action), setup.poses.open.mode, "|", setup.poses.volume.clip_name, bool(setup.poses.volume.action))
print("layout:", setup.guides.layout, setup.guides.anchors, [(c.name, round(c.angle, 2)) for c in setup.guides.columns])
assert len(infos) == 6 and all(len(i["bones"]) == 4 for i in infos)
assert m.followRoot == 60.0 and m.followTip == 5.0 and abs(m.spinOpen - 0.8) < 1e-5
assert setup.poses.open.action is not None
assert len(arm.data.bones) == n_bones, "adopting must not add or remove bones"
iss = doctor.run(setup, arm)
print("check:", [(i.severity, i.id) for i in iss])
assert not doctor.blocking(iss)
print("stages:", {k: stages.status(setup, arm, k) for k in stages.ORDER})
legacy = adopt.legacy_rig_bones(setup, arm)
print("legacy rig: %d helper bones, %d constraints, %d drivers" % (len(legacy[0]), legacy[1], legacy[2]))
# an adopted rig is never rebuilt or deleted
try:
    bpy.ops.bone_cloth.build_bones()
    print("build_bones: NO ERROR (should refuse)")
except RuntimeError as e:
    print("build_bones refused:", str(e)[:70])
bpy.ops.bone_cloth.start_over('EXEC_DEFAULT')
print("after start over: bones", len(arm.data.bones), "(unchanged:", len(arm.data.bones) == n_bones, ")")
assert len(arm.data.bones) == n_bones
print("ALL OK")
