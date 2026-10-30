"""Windowed trailing: a sway back and forth (the pelvis sliding 3 cm left and right at 2 Hz, as in a walk) hardly moves
the hem tips, while a steady run still trails. Run on a copy of the jacket scene:
BC_NO_PREPARE=1 python tests/run_headless.py test_window.py <copy of ShooterGame/player_coat.blend>"""
import math, sys
import bpy
import numpy as np
from mathutils import Matrix, Vector
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import props, poses, sim as sim_mod
bone_cloth.register()

arm = bpy.data.objects["root"]
s = props.get_setup(arm)
if arm.animation_data:
    arm.animation_data.action = None
m = s.motion
m.followRoot, m.followTip, m.trail, m.maxAngleRoot, m.maxAngleTip = 90.0, 90.0, 0.0, 2.0, 16.0
m.swing, m.collision, m.spinOpen = 1.0, 0.0, 0.0
m.windStrengthFront = m.windStrengthSides = m.windStrengthBack = m.spinStrength = 0.0
s.preview.motion, s.preview.wind_speed = 'ANIMATION', 0.0
TIPS = ["coat_front_05_l", "coat_front_05_r", "coat_back_05_l", "coat_back_05_r"]
FPS = 60.0
start = arm.matrix_basis.copy()


def tips_in_body():
    """Hem tips relative to the pelvis, in the armature's axes (cm)."""
    bpy.context.view_layer.update()
    pelvis = np.array(arm.pose.bones["pelvis"].head)
    return np.array([np.array(arm.pose.bones[n].tail) - pelvis for n in TIPS])


def back_tip_depth():
    """How far the back hem tips are from the body's vertical axis through the pelvis (cm), smallest."""
    bpy.context.view_layer.update()
    pelvis = np.array(arm.pose.bones["pelvis"].head)
    d = [np.hypot(*(np.array(arm.pose.bones[n].tail) - pelvis)[:2]) for n in ("coat_back_05_l", "coat_back_05_r")]
    return min(d) * arm.matrix_world.to_scale().x * 100


def run(window, motion):
    m.motionWindow = window
    sim = sim_mod.start(s, arm, bpy.context.scene)
    sim.reset()
    pb = arm.pose.bones["pelvis"]
    rest = None
    samples = []
    depths = []
    for f in range(240):
        t = f / FPS
        if motion == 'spin':
            arm.matrix_basis = start @ Matrix.Rotation(math.radians(180.0) * t, 4, 'Z')    # 180 deg/s
        elif motion == 'sway':
            pb.location = (3.0 * math.sin(2.0 * math.pi * 2.0 * t), 0.0, 0.0)      # armature units: cm
        else:
            speed = -5.0 if motion == 'run' else 4.0                    # forward is -Y; 'back' runs backward
            arm.matrix_basis = start.copy()
            arm.matrix_basis.translation = start.translation + Vector((0.0, speed * t, 0.0))
        bpy.context.view_layer.update()
        sim.step(0.0 if f == 0 else 1.0 / FPS, t)
        p = tips_in_body()
        if f == 0:
            rest = p
        if f >= 120:
            samples.append(p - rest)
            depths.append(back_tip_depth())
    sim_mod.stop(arm)
    pb.location = (0.0, 0.0, 0.0)
    arm.matrix_basis = start
    a = np.array(samples)
    swing_cm = float((a.max(axis=0) - a.min(axis=0)).max()) * arm.matrix_world.to_scale().x * 100
    mean_lag_cm = float(np.linalg.norm(a.mean(axis=0), axis=1).max()) * arm.matrix_world.to_scale().x * 100
    run.depth = min(depths)
    return swing_cm, mean_lag_cm


classic_sway, _ = run(0.0, 'sway')
window_sway, _ = run(0.5, 'sway')
print("sway 3 cm at 2 Hz: hem tips swing %.2f cm classic, %.2f cm with a 0.5 s window" % (classic_sway, window_sway))
_, window_run = run(0.5, 'run')
print("running 5 m/s with the window: hem tips trail %.1f cm" % window_run)
m.lagTip = 0.1            # a strong trail, to show it never carries the back hem into the body
run(0.0, 'sway')
rest_depth = run.depth
run(0.5, 'back')
print("running backward 4 m/s: the back hem tips stay %.1f cm from the body's axis (%.1f cm standing)" % (run.depth, rest_depth))
m.physicsBack = 0.0
_, back_off = run(0.5, 'back')
print("physics back 0: running backward the tips trail %.2f cm" % back_off)
assert run.depth >= rest_depth - 0.3 and back_off < 0.5
m.physicsBack = 1.0
_, spin_on = run(0.5, 'spin')
m.physicsSpin = 0.0
_, spin_off = run(0.5, 'spin')
print("turning 180 deg/s: the hem tips trail %.1f cm, %.2f cm with physics spin 0" % (spin_on, spin_off))
assert spin_off < 0.3 * spin_on, (spin_off, spin_on)
assert window_sway < 0.5 * classic_sway, (window_sway, classic_sway)
assert window_run > 1.0
print("ALL OK")
