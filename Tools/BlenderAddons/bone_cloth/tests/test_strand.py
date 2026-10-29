"""A single strand (a ponytail): a tube hanging from the head. Auto Setup with the Strand preset, the engine's strips, and the
swing preview in a headwind. Run on an unprepared copy of the coat scene: BC_TEST_BLEND=<coat copy> BC_NO_PREPARE=1"""
import math, os, sys
import bpy, bmesh
import numpy as np
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import props, presets, pipeline, doctor, bones, sim as sim_mod, frames
bone_cloth.register()
arm = bpy.data.objects["root"]
bpy.context.view_layer.objects.active = arm
for o in bpy.data.objects:
    if o.type == 'MESH' and o.name in ("coat", "coat.001"):
        o.hide_viewport = True
        o.hide_set(True)
head = frames.bone_head(arm, "head")
print("head at", head.round(2))

# a ponytail: a square tube from the back of the head, 45 cm long, hanging down and a little back
mesh = bpy.data.meshes.new("tail")
bm = bmesh.new()
rings = []
for i in range(10):
    z = head[2] - 0.02 - 0.45 * i / 9
    y = head[1] + 0.09 + 0.04 * i / 9
    w = 0.025 * (1.0 - 0.5 * i / 9)
    rings.append([bm.verts.new((head[0] + dx, y + dy, z)) for dx, dy in ((w, w), (-w, w), (-w, -w), (w, -w))])
for i in range(9):
    for j in range(4):
        bm.faces.new((rings[i][j], rings[i][(j + 1) % 4], rings[i + 1][(j + 1) % 4], rings[i + 1][j]))
bm.to_mesh(mesh)
bm.free()
tail = bpy.data.objects.new("tail", mesh)
bpy.context.scene.collection.objects.link(tail)
tail.vertex_groups.new(name="head")
tail.vertex_groups["head"].add(list(range(len(mesh.vertices))), 1.0, 'REPLACE')
tail.parent = arm
tail.matrix_parent_inverse = arm.matrix_world.inverted()
mod = tail.modifiers.new("Armature", 'ARMATURE')
mod.object = arm
bpy.context.view_layer.update()

setup = props.new_setup(arm, "tail")
setup.cloth_objects.add().obj = tail
presets.apply(setup, "STRAND")
setup.guides.strand_anchor = "head"
issues = pipeline.auto_setup(setup, arm, log=lambda m: print("  log:", m))
print("issues:", [(i.severity, i.id, i.message[:90]) for i in issues if i.severity != "info"])
frame = pipeline.frame_of(setup, arm)
skel, verts, strips = doctor.engine_strips(setup, arm, frame)
print("engine strips:", [(len(s.joints), round(s.tip_length, 2)) for s in strips])
assert len(strips) == 1 and len(strips[0].joints) == 5 and not doctor.blocking(issues)

# the weights: the first part stays with the head, the rest follows the strand's bones
names = {vg.index: vg.name for vg in tail.vertex_groups}
by_bone = {}
for v in mesh.vertices:
    for g in v.groups:
        if g.weight >= 0.5:
            by_bone.setdefault(names[g.group], []).append(round(v.co.z, 2))
print("vertices owned by each group (z range):", {k: (min(z), max(z)) for k, z in by_bone.items()})
assert "head" in by_bone and sum(1 for k in by_bone if k.startswith("tail_strand")) >= 4

arm.animation_data_create()
arm.animation_data.action = None
sc = bpy.context.scene
setup.preview.motion, setup.preview.wind_speed, setup.preview.wind_from, setup.preview.gust = 'STAND', 10.0, 0.0, 0.0
last = bones.strips(setup, arm)[0]["bones"][-1]
mw = lambda: np.array(arm.matrix_world)
end = lambda: (mw() @ np.append(np.array(arm.pose.bones[last].tail), 1.0))[:3]
bpy.context.view_layer.update()
rest = end().copy()
sim_mod.start(setup, arm, sc)
sc.frame_set(1)
for f in range(1, 50):
    sc.frame_set(f)
blown = end().copy()
sim_mod.stop(arm)
print("tail end blown back by %.1f cm in a 10 m/s headwind" % ((blown - rest)[1] * 100))
assert (blown - rest)[1] > 0.03
print("ALL OK")
