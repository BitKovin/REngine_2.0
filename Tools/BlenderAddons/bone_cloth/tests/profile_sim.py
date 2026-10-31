"""Times the preview sim (frames per second) and profiles a step: BC_NO_PREPARE=1 python tests/run_headless.py profile_sim.py <scene>."""
import cProfile, io, pstats, sys, time
import bpy
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import props, sim as sim_mod
bone_cloth.register()
arm = bpy.data.objects["root"]
s = props.get_setup(arm)
if arm.animation_data:
    arm.animation_data.action = None
s.preview.motion, s.preview.wind_speed = 'WALK' if 'WALK' in [i.identifier for i in s.preview.bl_rna.properties['motion'].enum_items] else 'ANIMATION', 8.0
sim = sim_mod.start(s, arm, bpy.context.scene)
if sim_mod._handler in bpy.app.handlers.frame_change_post:
    bpy.app.handlers.frame_change_post.remove(sim_mod._handler)
sim.reset()
for f in range(10):
    sim.step(1 / 60, f / 60)
N = 60
t0 = time.time()
for f in range(N):
    sim.step(1 / 60, (f + 10) / 60)
dt = (time.time() - t0) / N
print("STEP %.1f ms (%.0f fps)" % (dt * 1000, 1 / dt))
pr = cProfile.Profile()
pr.enable()
for f in range(30):
    sim.step(1 / 60, (f + 70) / 60)
pr.disable()
out = io.StringIO()
pstats.Stats(pr, stream=out).sort_stats('cumulative').print_stats(14)
print("PROFILE\n" + out.getvalue())
if sim_mod._handler not in bpy.app.handlers.frame_change_post:
    bpy.app.handlers.frame_change_post.append(sim_mod._handler)
scene = bpy.context.scene
scene.frame_set(1)
t0 = time.time()
for f in range(2, 62):
    scene.frame_set(f)
    bpy.context.view_layer.update()
dt = (time.time() - t0) / 60
print("FRAME_SET %.1f ms per frame including Blender's own update (%.0f fps)" % (dt * 1000, 1 / dt))
t0 = time.time()
for f in range(60):
    bpy.context.view_layer.update()
print("EMPTY UPDATE %.1f ms" % ((time.time() - t0) / 60 * 1000))
