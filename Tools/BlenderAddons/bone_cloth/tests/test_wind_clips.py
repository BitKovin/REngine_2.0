"""Wind and turn clips, torso surface capsules and probes, on a copy of the jacket scene (player_coat.blend):
BC_NO_PREPARE=1 python tests/run_headless.py test_wind_clips.py <copy of player_coat.blend>
Makes the front clip from the Open pose, a left clip and its mirror, a turn clip; checks the preview blend, the mirror,
the probes holding the cloth out of the torso in the swing preview, the check step and the export."""
import json, math, os, struct, sys, tempfile
import bpy
import numpy as np
from mathutils import Quaternion, Vector
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import colliders, doctor, export, pipeline, poses, props, sim as sim_mod
from bone_cloth import engine_contract as ec
bone_cloth.register()

arm = bpy.data.objects["root"]
bpy.context.view_layer.objects.active = arm
s = props.get_setup(arm)
assert s is not None, "the scene has no Bone Cloth setup"
if arm.animation_data:
    arm.animation_data.action = None
sc = bpy.context.scene
sc.frame_set(1)
poses.set_bases(arm, {}, {})
names = poses.cloth_bone_names(s, arm)
mw = arm.matrix_world
# the scene's own tuning would change what the checks below expect: clips play as made, no spin opening
m = s.motion
m.windStrengthFront = m.windStrengthSides = m.windStrengthBack = m.spinStrength = 1.0
m.swing = m.collision = 1.0
m.spinOpen = 0.0


def tails(bones):
    bpy.context.view_layer.update()
    return np.array([np.array(mw @ arm.pose.bones[n].tail) for n in bones])


# 1. the front clip from the Open pose (frame 2), as the panel's button does
front = poses.new_clip(s, arm, "windFront")
q_open, l_open = poses.read_action(s.poses.open.action), poses.read_action_locs(s.poses.open.action)
poses.key_into_clip(front, 2, {b: (q_open.get(b, Quaternion()), l_open.get(b, Vector())) for b in names})
samples = poses.clip_samples(front)
assert len(samples) == ec.CLIP_SAMPLE_COUNT
first_angle = max(q.angle for q, _ in samples[0].values())
last_err = max((samples[-1][b][0].rotation_difference(q_open[b])).angle for b in names if b in q_open)
print("front clip: first sample turns %.4f deg, last differs from Open by %.4f deg" % (math.degrees(first_angle),
                                                                                        math.degrees(last_err)))
assert first_angle < 1e-4 and last_err < 1e-3

# 2. preview: full headwind is the Open pose; halfway is half of it
s.poses.preview_wind, s.poses.preview_wind_from, s.poses.preview_turn = 1.0, 0.0, 0.0
poses.apply_preview(bpy.context)
for b in names:
    if b in q_open:
        assert arm.pose.bones[b].rotation_quaternion.rotation_difference(q_open[b]).angle < 1e-3, b
print("preview at full headwind matches the Open pose")

# 3. a left clip (the left front panel swung out) mirrored to the right: the right panel's tails mirror the left's
left = poses.new_clip(s, arm, "windLeft")
left_bones = [n for n in names if n.startswith("coat_front") and n.endswith("_l")]
right_bones = [n[:-2] + "_r" for n in left_bones]
pose = {b: (Quaternion((0, 0, 1), math.radians(25)), Vector()) for b in left_bones}
pose.update({b: (Quaternion((1, 0, 0), math.radians(-10)), Vector()) for b in right_bones})
poses.key_into_clip(left, 2, pose)
poses.set_bases(arm, {}, {})
rest_l, rest_r = tails(left_bones), tails(right_bones)      # the rig is not exactly symmetric at rest
poses.set_bases(arm, {b: q for b, (q, _) in pose.items()})
tl_left, tr_left = tails(left_bones), tails(right_bones)
s.poses.windRight.mirrored = True
poses.mirror_clip(s, arm, "windRight")
right_pose = poses.bases_at(s.poses.windRight.action, 2)
poses.set_bases(arm, {b: right_pose[b][0] for b in names if b in right_pose},
                {b: right_pose[b][1] for b in names if b in right_pose})
tl_right, tr_right = tails(left_bones), tails(right_bones)
flip = np.array([-1.0, 1.0, 1.0])
err = max(np.abs((tr_right - rest_r) * flip - (tl_left - rest_l)).max(),
          np.abs((tl_right - rest_l) * flip - (tr_left - rest_r)).max())
print("mirror: how far the right clip moves each tail vs the left clip, mirrored (cm, max): %.3f" % (err * 100))
assert err < 0.005
poses.set_bases(arm, {}, {})

# 4. a turn clip
turn = poses.new_clip(s, arm, "turnLeft")
poses.key_into_clip(turn, 2, {b: (Quaternion((0, 0, 1), math.radians(-15)), Vector()) for b in names})
poses.mirror_clip(s, arm, "turnRight")

# 5. torso surface capsules
before = len(s.colliders)
colliders.add_torso(s, arm)
colliders.fit_all(s, arm)
for c in list(s.colliders)[before:]:
    r0, r1 = colliders.radii(c)
    print("torso capsule %s-%s offset %s radii %.3f / %.3f" % (c.bone, c.to, tuple(round(v, 3) for v in c.offset), r0, r1))
assert len(s.colliders) > before and all(c.surface for c in list(s.colliders)[before:])

# 6. the swing preview: a 25 m/s headwind with the probes, then without the surface capsules
s.motion.windStart, s.motion.windFull = 2.0, 12.0


def probe_depth(sim, probes):
    """Deepest one of the joints' probes (`probes`: per joint, taken at the start) sits in a surface capsule it does not
    ignore, now (cm)."""
    worst = 0.0
    caps = [c for c in sim.colliders]
    for st in sim.strips:
        for j in st.joints:
            for local, ignore in probes[j.name]:
                p = j.head + j.basis @ local
                for k, (a, b, r0, r1, surface, sl, el) in enumerate(caps):
                    if surface and not ignore & (1 << k):
                        start, end = (sim.M[a] @ np.append(sl, 1.0))[:3], (sim.M[b] @ np.append(el, 1.0))[:3]
                        worst = max(worst, ec.capsule_depth(p, start, end, r0, r1)[0])
    return worst * 100


def run(wind, wind_from, frames=30, motion='STAND', use_probes=True):
    s.preview.motion, s.preview.wind_speed, s.preview.wind_from, s.preview.gust = motion, wind, math.radians(wind_from), 0.0
    sc.frame_set(1)
    sim = sim_mod.start(s, arm, sc)
    taken = {j.name: list(j.probes) for st in sim.strips for j in st.joints}
    if not use_probes:
        for st in sim.strips:
            for j in st.joints:
                j.probes = []
    for f in range(1, frames + 1):
        sc.frame_set(f)
    probes = sum(len(v) for v in taken.values())
    depth = probe_depth(sim, taken)
    air = ec.spin_air(sim.wind_air[0], sim.wind_air[1], s.motion.spinOpen, sim.spin_weight, s.motion.windFull,
                      sim.wind_speed)
    shares = [t for layer in ec.clip_layers(air[0], air[1],
                                            sim_mod.ramp(s.motion.windStart, s.motion.windFull, float(np.hypot(*air))),
                                            sim.turn_amount) for t in layer]
    sim_mod.stop(arm)
    return probes, depth, shares


expected = {0.0: {"windFront": 1.0}, 45.0: {"windFront": 0.5, "windLeft": 0.5}, 90.0: {"windLeft": 1.0},
            270.0: {"windRight": 1.0}}
for wind_from, want in expected.items():
    probes, depth, shares = run(25.0, wind_from)
    got = {ec.CLIP_SLOTS[k]: round(w, 2) for k, _, w in shares if w > 0.01}
    print("wind from %3d deg: %d probes, deepest probe in the torso %.2f cm, clips %s" % (wind_from, probes, depth, got))
    assert probes > 0 and depth < 0.5 and got == want, got
# the turn clip (every bone turned 15 degrees) takes the cloth into the torso: the probes hold it out (the spin's front
# opening is left off here, it would carry the cloth clear of the torso)
s.motion.spinOpen = 0.0
_, depth_with, shares = run(0.0, 0.0, frames=20, motion='SPIN')
_, depth_without, _ = run(0.0, 0.0, frames=20, motion='SPIN', use_probes=False)
print("spinning left (clips %s): the cloth's probes reach %.2f cm into the torso, %.2f cm without the probes" % (
    [(ec.CLIP_SLOTS[k], round(w, 2)) for k, _, w in shares], depth_with, depth_without))
assert shares and ec.CLIP_SLOTS[shares[0][0]] == "turnLeft"
assert depth_without > 1.0 and depth_with < 0.5

# 6b. Swing and Collision at 0: the cloth is the clips' pose exactly, even running in a side wind
s.motion.swing, s.motion.collision = 0.0, 0.0
s.preview.motion, s.preview.wind_speed, s.preview.wind_from, s.preview.gust = 'RUN', 8.0, math.radians(60.0), 0.0
sc.frame_set(1)
sim = sim_mod.start(s, arm, sc)
for f in range(1, 31):
    sc.frame_set(f)
air = ec.spin_air(sim.wind_air[0], sim.wind_air[1], s.motion.spinOpen, sim.spin_weight, s.motion.windFull, sim.wind_speed)
layers = ec.clip_layers(air[0], air[1], sim_mod.ramp(s.motion.windStart, s.motion.windFull, float(np.hypot(*air))),
                        sim.turn_amount, poses.strengths(s))
expected = poses.blend_layers(poses.clip_sample_sets(s), layers)
worst = max((arm.pose.bones[b].rotation_quaternion.rotation_difference(q).angle for b, (q, _) in expected.items()
             if b in arm.pose.bones), default=0.0)
sim_mod.stop(arm)
print("swing 0, collision 0: the cloth differs from the clips' pose by at most %.3f deg" % math.degrees(worst))
assert worst < math.radians(0.05)
s.motion.swing, s.motion.collision = 1.0, 1.0

# 7. the check step
issues = doctor.run(s, arm)
for i in issues:
    if i.id.startswith(("POSE-", "CLIP-")):
        print("check:", i.id, "-", i.message)

# 8. the .skmm entry and the model's clips
entry = export.skmm_entry(s, arm)
text = export.entry_text(entry)
data = json.loads(text)
print("skmm clips:", {k: data[k] for k in ec.CLIP_SLOTS}, "windStart", data["windStart"], "windFull", data["windFull"])
print("skmm surface colliders:", [c for c in data["colliders"] if c.get("surface")])
assert data["windFront"] == front.name and data["windRight"] == s.poses.windRight.action.name
assert data["openClip"] == "" and data["volumeClip"] == ""
path = os.path.join(tempfile.gettempdir(), "bc_wind_clips_test.glb")
export.export_glb(s, arm, path)
with open(path, "rb") as f:
    f.read(12)
    length, _ = struct.unpack("<II", f.read(8))
    gltf = json.loads(f.read(length))
anims = {a["name"]: a for a in gltf.get("animations", [])}
for k in ec.CLIP_SLOTS:
    act = poses.clip_slot(s, k).action
    if act is not None:
        assert act.name in anims, (act.name, list(anims))
print("model clips:", sorted(anims))
print("ALL OK")
