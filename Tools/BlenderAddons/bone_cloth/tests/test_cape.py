"""A cape: a curved sheet behind the back, weighted to the chest. Auto Setup with the Cape preset, the engine's view of the
strips, and the swing preview in a headwind (the cape must be blown back, away from the body).
Run on an unprepared copy of the coat scene: BC_TEST_BLEND=<coat copy> BC_NO_PREPARE=1"""
import math, os, sys
import bpy, bmesh
import numpy as np
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import props, presets, pipeline, doctor, bones, mesh_prep, sim as sim_mod, frames, landmarks

# the scene: the armature with its old cloth rig removed, no cloth mesh yet
exec(open(os.path.join(os.path.dirname(os.environ.get("BC_TEST_SCRIPT", __file__)), "prepare_coat.py")).read()) if False else None
bone_cloth.register()
arm = bpy.data.objects["root"]
bpy.context.view_layer.objects.active = arm
for o in bpy.data.objects:
    if o.type == 'MESH' and o.name in ("coat", "coat.001"):
        o.hide_viewport = True
        o.hide_set(True)

# a cape: a cylinder segment 22 cm behind the spine axis, from the shoulders (z 1.52) to the knees (0.55), +-70 degrees
mesh = bpy.data.meshes.new("cape")
bm = bmesh.new()
rows, cols = 12, 15
grid = []
for i in range(rows + 1):
    z = 1.52 - (1.52 - 0.55) * i / rows
    row = []
    for j in range(cols + 1):
        a = math.radians(180 - 70 + 140 * j / cols)
        r = 0.2 + 0.06 * (i / rows)                              # flares a little toward the hem
        row.append(bm.verts.new((r * math.sin(a), 0.01 - r * math.cos(a) * -1.0 * -1.0, z)))
    grid.append(row)
for i in range(rows):
    for j in range(cols):
        bm.faces.new((grid[i][j], grid[i][j + 1], grid[i + 1][j + 1], grid[i + 1][j]))
bm.to_mesh(mesh)
bm.free()
cape = bpy.data.objects.new("cape", mesh)
bpy.context.scene.collection.objects.link(cape)
for name in ("spine_03", "spine_02", "pelvis", "clavicle_l", "clavicle_r"):
    cape.vertex_groups.new(name=name)
for v in mesh.vertices:
    cape.vertex_groups["spine_03" if v.co.z > 1.2 else "spine_02"].add([v.index], 1.0, 'REPLACE')
cape.parent = arm
cape.matrix_parent_inverse = arm.matrix_world.inverted()
mod = cape.modifiers.new("Armature", 'ARMATURE')
mod.object = arm
bpy.context.view_layer.update()

setup = props.new_setup(arm, "cape")
setup.cloth_objects.add().obj = cape
presets.apply(setup, "CAPE")
issues = pipeline.auto_setup(setup, arm, log=lambda m: print("  log:", m))
print("roots:", bones.roots(setup))
print("issues:", [(i.severity, i.id, i.message[:80]) for i in issues if i.severity != "info"])
frame = pipeline.frame_of(setup, arm)
skel, verts, strips = doctor.engine_strips(setup, arm, frame)
print("engine strips:", len(strips), [(len(s.joints), s.side) for s in strips])
assert len(strips) >= 4 and not doctor.blocking(issues)
tops = [skel.head(s.joints[0].bone)[2] for s in strips]
print("strip tops (m):", np.round(tops, 2).tolist())
assert all(1.3 < z < 1.7 for z in tops)

# swing preview: a 12 m/s headwind makes the volume layer blow the cape back
arm.animation_data_create()
arm.animation_data.action = None
sc = bpy.context.scene
setup.preview.motion, setup.preview.wind_speed, setup.preview.wind_from, setup.preview.gust = 'STAND', 12.0, 0.0, 0.0
mw = lambda: np.array(arm.matrix_world)
hem = lambda: np.array([mw() @ np.append(np.array(arm.pose.bones[s["bones"][-1]].tail), 1.0) for s in bones.strips(setup, arm)])[:, :3]
bpy.context.view_layer.update()
rest = hem().copy()
sc.frame_set(1)
sim = sim_mod.start(setup, arm, sc)
for f in range(1, 60):
    sc.frame_set(f)
blown = hem().copy()
sim_mod.stop(arm)
back = (blown - rest)[:, 1].mean() * 100            # the character faces -Y: behind is +Y
print("cape hem blown back by %.1f cm in a 12 m/s headwind (volume weight %.2f)" % (back, sim.volume_weight))
assert back > 5.0
print("ALL OK")
