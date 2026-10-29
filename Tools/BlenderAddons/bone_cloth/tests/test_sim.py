"""The swing preview behaves like the engine's cloth: rests when nothing happens, trails a running body, opens in a
headwind, opens on a fast spin, snaps on a teleport. Run on the reference jacket scene:
BC_TEST_BLEND=tests/data/jacket_reference.blend BC_NO_PREPARE=1"""
import math, os, sys
import bpy
import numpy as np
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import props, sim as sim_mod, pipeline
bone_cloth.register()

arm = bpy.data.objects["root"]
bpy.context.view_layer.objects.active = arm
arm.animation_data.action = None
s = props.get_setup(arm)
sc = bpy.context.scene
LAST = ["coat_front_05_l", "coat_front_05_r", "coat_back_05_l", "coat_back_05_r"]
frame0 = pipeline.frame_of(s, arm)


def hem_tails():
    mw = np.array(arm.matrix_world)
    return np.array([mw @ np.append(np.array(arm.pose.bones[n].tail), 1.0) for n in LAST])[:, :3]


def relative(tails):
    """Hem tails in the body's own frame (origin and yaw of the armature object)."""
    mw = np.array(arm.matrix_world)
    r = mw[:3, :3].copy()
    for k in range(3):
        r[:, k] /= np.linalg.norm(r[:, k])
    return (tails - mw[:3, 3]) @ r


def run(motion, frames, wind=0.0, wind_from=0.0, stop_at=None):
    s.preview.motion, s.preview.wind_speed, s.preview.wind_from, s.preview.gust = motion, wind, math.radians(wind_from), 0.0
    sc.frame_set(1)
    sim = sim_mod.start(s, arm, sc)
    out = []
    for f in range(1, frames + 1):
        sc.frame_set(f)
        out.append(relative(hem_tails()))
    state = {"spin": sim.spin_weight, "open": [st.open_weight for st in sim.strips], "volume": sim.volume_weight}
    sim_mod.stop(arm)
    return out, state


from bone_cloth import poses
poses.set_bases(arm, {}, {})          # the file holds the hand-made pose: start from the rest shape
bpy.context.view_layer.update()
rest = relative(hem_tails())
base_volume = s.motion.volume
s.motion.volume = 0.0                  # the preset keeps 30% of the Volume pose always on: that is not the rest shape
stand, st = run('STAND', 40)
print("stand still: max hem offset from rest (cm): %.2f" % (np.abs(stand[-1] - rest).max() * 100))
assert np.abs(stand[-1] - rest).max() < 0.01

runr, st = run('RUN', 60)
s.motion.volume = base_volume
back = lambda t: t[:, 1]                        # the character faces -Y in the body frame: behind is +Y
print("run 5 m/s: hem trails behind rest by (cm):", ((back(runr[-1]) - back(rest)) * 100).round(1).tolist())
assert (back(runr[-1]) - back(rest)).mean() > 0.03

wind, st = run('STAND', 60, wind=12.0, wind_from=0.0)
lateral = np.abs(wind[-1][:2, 0]) - np.abs(rest[:2, 0])
print("headwind 12 m/s: open weights", [round(w, 2) for w in st["open"]], "| front hem sideways change (cm):", (lateral * 100).round(1).tolist(), "| volume", round(st["volume"], 2))
assert max(st["open"]) > 0.9 and lateral.mean() > 0.04

sp, st = run('SPIN', 40)
print("spin 720 deg/s: spin weight %.2f" % st["spin"])
assert st["spin"] > 0.5

tp, st = run('TELEPORT', 130)
# the jump happens at t = 2 s. The engine reads the jump as a huge body velocity (a gale for a frame or two: the wind
# layers open briefly), then the weights settle; the strips themselves snap to rest, they do not stretch.
jump = int(2.0 * sc.render.fps / sc.render.fps_base) + 1
before = np.abs(tp[jump - 2] - rest).max() * 100
after = np.abs(tp[jump + 1] - rest).max() * 100
settled = np.abs(tp[jump + 30] - rest).max() * 100
print("teleport: offset from rest before the jump %.1f cm, right after %.1f cm, 30 frames later %.1f cm" % (before, after, settled))
assert after < 25.0 and abs(settled - before) < 3.0

assert all(np.isfinite(a).all() for a in stand + runr + wind + sp + tp)
for name in ("coat_front_00_l", "coat_front_03_r"):
    q = arm.pose.bones[name].matrix_basis
    assert np.isfinite(np.array(q)).all()
print("pose restored after stop:", float(np.abs(np.array(arm.pose.bones["coat_front_03_l"].matrix_basis) - np.eye(4)).max()))
print("ALL OK")
