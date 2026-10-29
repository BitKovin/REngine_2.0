"""Hand-made poses: saved with their slide, mirrored, protected from Update, and matched by the generator.
Run on the reference scene: BC_TEST_BLEND=tests/data/jacket_reference.blend BC_NO_PREPARE=1
(a jacket set up with the Jacket preset; its action coat_open_target holds the symmetric hand-made Open pose)."""
import json, math, os, sys
import bpy
import numpy as np
from mathutils import Quaternion, Vector
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import props, presets, pipeline, poses, bones
bone_cloth.register()
DATA = os.path.join(r"F:\Git\SdlWasmEngine\Tools\BlenderAddons\bone_cloth\tests\data")
arm = bpy.data.objects["root"]
bpy.context.view_layer.objects.active = arm
s = props.get_setup(arm)
frame = pipeline.frame_of(s, arm)
target = json.load(open(os.path.join(DATA, "jacket_open_target.json")))["bones"]
arm.animation_data.action = None
mw = arm.matrix_world


def tails(side):
    return np.array([np.array(mw @ arm.pose.bones["coat_front_%02d_%s" % (k, side)].tail) for k in range(6)])


def apply(d):
    poses.set_bases(arm, {}, {})
    for n, v in d.items():
        pb = arm.pose.bones[n]
        pb.rotation_mode = 'QUATERNION'
        pb.rotation_quaternion = v["rotation_quaternion"]
        pb.location = v["location"]
    bpy.context.view_layer.update()


# 1. the target is symmetric
apply(target)
tl, tr = tails("l"), tails("r")
m = lambda a: a * np.array([-1, 1, 1])
print("target symmetry error (cm, max):", round(float(np.abs(m(tr) - tl).max()) * 100, 2))
target_tails = tr.copy()

# 2. the generator against the target (Open settings of the preset, bones as built)
presets.apply(s, "JACKET")
pipeline.update_poses(s, arm, force=("open", "volume"))
q, l = poses.solve_layer_full(s, arm, frame, "open")
poses.set_bases(arm, {n: q[n] for n in q if "front" in n}, {n: l[n] for n in l if "front" in n})
bpy.context.view_layer.update()
gen = tails("r")
rms = float(np.sqrt(((gen - target_tails) ** 2).sum(axis=1).mean())) * 100
print("generator vs hand-made target, tail rms (cm): %.2f" % rms)
assert rms < 4.0, rms

# 3. hand pose: tweak the right side, mirror it, save; Update keeps it; Back to generated replaces it
poses.set_bases(arm, {}, {})
tw = {n: dict(v) for n, v in target.items()}
apply(tw)
arm.pose.bones["coat_front_05_r"].rotation_quaternion = Quaternion((0, 1, 0), math.radians(25)) @ arm.pose.bones["coat_front_05_r"].rotation_quaternion
bpy.context.view_layer.update()
for pb in arm.pose.bones:
    pb.select = False
names = poses.mirror_pose(s, arm, 'R')
print("mirrored bones:", len(names))
a, b = tails("l"), tails("r")
print("after mirroring R->L, mirror error (cm, max):", round(float(np.abs(m(b) - a).max()) * 100, 2))
assert float(np.abs(m(b) - a).max()) * 100 < 2.0
for pb in arm.pose.bones:
    pb.select = pb.name.startswith("coat_front")
poses.capture_current(s, arm, "open")
act = s.poses.open.action
print("saved: mode", s.poses.open.mode, "| keyed bones", len(poses.read_action(act)), "| location keys", sorted(poses.read_action_locs(act)))
before = poses.action_signature(act)
kept = pipeline.update_poses(s, arm)
print("update keeps:", kept, "| same action:", s.poses.open.action == act, "| unchanged:", poses.action_signature(s.poses.open.action) == before)
assert kept == ["open"] and poses.action_signature(s.poses.open.action) == before

# a pose made by the generator turns into a hand pose when someone keys into it
pipeline.update_poses(s, arm, force=("open",))
print("after Back to generated: hand =", poses.layer_is_hand(s.poses.open), "(must be False)")
assert not poses.layer_is_hand(s.poses.open)
fc = None
for lyr in s.poses.open.action.layers:
    for st in lyr.strips:
        for cb in st.channelbags:
            fc = next(f for f in cb.fcurves if f.data_path.endswith("rotation_quaternion") and f.array_index == 1)
            break
fc.keyframe_points[1].co[1] += 0.1
fc.update()
print("after keying into the clip: hand =", poses.layer_is_hand(s.poses.open), "(must be True)")
assert poses.layer_is_hand(s.poses.open)
# a hand pose belongs to its bones: rebuilding them differently is reported (and it was the reason a rebuilt jacket looked wrong)
poses.capture_current(s, arm, "open")
print("pose made for these bones: layout changed =", poses.layout_changed(arm, s.poses.open))
assert not poses.layout_changed(arm, s.poses.open)
from bone_cloth import doctor
s.guides.max_bone_length = 0.12
pipeline.build_bones(s, arm)
print("after rebuilding with 12 cm bones: layout changed =", poses.layout_changed(arm, s.poses.open), "| check:", [i.id for i in doctor.run(s, arm) if i.severity == "error"])
assert poses.layout_changed(arm, s.poses.open)
assert any(i.id == "HAND-POSE-BONES" for i in doctor.run(s, arm))
print("ALL OK")
