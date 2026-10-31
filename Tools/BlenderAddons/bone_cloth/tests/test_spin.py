"""Spinning in place (the preview's Spin 720 deg/s, the scene's fps): how many of the jacket's triangles are flipped or
squeezed under 15% of their area at each frame, and renders of the worst frame from below and behind (BC_RENDER_DIR).
BC_REGEN=1 makes the clips from the scene's generator settings first. Run on a copy of the jacket scene:
BC_NO_PREPARE=1 python tests/run_headless.py test_spin.py <copy>"""
import math, os, sys
import bpy
import numpy as np
from mathutils import Vector
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import props, clipgen, mesh_prep, sim as sim_mod
bone_cloth.register()

arm = bpy.data.objects["root"]
s = props.get_setup(arm)
scene = bpy.context.scene
if arm.animation_data:
    arm.animation_data.action = None
for item in filter(None, os.environ.get("BC_CLIPGEN", "").split(",")):       # e.g. TURN_OPEN=0.2,SIDE_CARRY=0.8
    k, v = item.split("=")
    setattr(clipgen, k, float(v))
if os.environ.get("BC_REGEN"):
    print("generated:", clipgen.generate(s, arm, force=True))
s.preview.motion, s.preview.wind_speed = os.environ.get("BC_MOTION", "SPIN"), float(os.environ.get("BC_WIND", "0"))
obj = mesh_prep.cloth_objects(s)[0]
me0 = obj.data
me0.calc_loop_triangles()
tris = np.array([t.vertices for t in me0.loop_triangles])
R = np.array([v.co for v in me0.vertices])


def nrm_area(X):
    n = np.cross(X[tris[:, 1]] - X[tris[:, 0]], X[tris[:, 2]] - X[tris[:, 0]])
    a = np.linalg.norm(n, axis=1)
    return n / np.maximum(a, 1e-12)[:, None], a / 2


n0, a0 = nrm_area(R)


def posed():
    ev = obj.evaluated_get(bpy.context.evaluated_depsgraph_get())
    me = ev.to_mesh()
    P = np.array([v.co for v in me.vertices])
    ev.to_mesh_clear()
    return P


scene.render.engine = 'BLENDER_WORKBENCH'
scene.display.shading.light = 'STUDIO'
scene.display.shading.color_type = 'MATERIAL'
scene.render.resolution_x, scene.render.resolution_y = 420, 340
cam_data = bpy.data.cameras.new("cam")
cam_data.type = 'ORTHO'
cam = bpy.data.objects.new("cam", cam_data)
scene.collection.objects.link(cam)
scene.camera = cam
mat = bpy.data.materials.new("viz")
mat.diffuse_color = (0.85, 0.3, 0.2, 1.0)
obj.data.materials.clear()
obj.data.materials.append(mat)
shown = {obj} | {bpy.data.objects[n] for n in ("body", "body.001", "arms") if n in bpy.data.objects}
for o in bpy.data.objects:
    if o.type == 'MESH':
        o.hide_render = o not in shown
out_dir = os.environ.get("BC_RENDER_DIR", os.environ.get("TEMP", "."))


def render(tag):
    centre = np.array(arm.matrix_world @ arm.pose.bones["pelvis"].head)
    tiles = []
    for d in ((0, 0.35, -1.0), (0, 1, -0.35)):
        d = np.array(d, float) / np.linalg.norm(d)
        # the body spins: look from where it faces now
        rot = np.array(arm.matrix_world.to_3x3().normalized())
        d = rot @ d
        cam.location = tuple(centre + d * 4.0)
        cam_data.ortho_scale = 1.3
        cam.rotation_euler = Vector(tuple(-d)).to_track_quat('-Z', 'Z').to_euler()
        scene.render.filepath = os.path.join(out_dir, "_tile.png")
        bpy.ops.render.render(write_still=True)
        img = bpy.data.images.load(scene.render.filepath, check_existing=False)
        tiles.append(np.array(img.pixels[:]).reshape(img.size[1], img.size[0], 4))
        bpy.data.images.remove(img)
    sheet = np.concatenate(tiles, axis=1)
    img = bpy.data.images.new(tag, sheet.shape[1], sheet.shape[0], alpha=True)
    img.pixels = sheet.ravel().tolist()
    img.filepath_raw = os.path.join(out_dir, "spin_%s.png" % tag)
    img.file_format = 'PNG'
    img.save()
    print("sheet", img.filepath_raw)


sim = sim_mod.start(s, arm, scene)
scene.frame_set(1)
n1, a1 = nrm_area(posed())
base_flipped = int(((n1 * n0).sum(axis=1) < 0).sum())       # the rest shape itself
base_squeezed = int((a1 / np.maximum(a0, 1e-12) < 0.15).sum())
from bone_cloth import bones as bones_mod
front_tails = [info["bones"][-1] for info in bones_mod.strips(s, arm) if abs(info["theta"]) < math.pi / 2]
centre = np.array(arm.pose.bones[s.character.hips].head)
inv = np.array(arm.matrix_world.inverted())


def front_azimuth():
    """How far round the body the front panels' hems are, degrees from the front (90: straight to the side)."""
    best = 0.0
    for n in front_tails:
        t = np.array(arm.pose.bones[n].tail) - centre
        best = max(best, math.degrees(math.atan2(math.hypot(t[0], 0.0), -t[1])) if True else 0.0)
    return best


vgroups = {g.index: g.name for g in obj.vertex_groups}
kind = np.array(["" for _ in R], dtype=object)
for v in me0.vertices:
    best = max(((vgroups[g.group], g.weight) for g in v.groups if vgroups[g.group].startswith(s.id_prefix if hasattr(s, "id_prefix") else "jaket")),
               key=lambda x: x[1], default=("", 0.0))
    kind[v.index] = best[0].split("_")[1] if best[1] > 0.5 and best[0].count("_") >= 2 else ""
hem = R[:, 2] < R[:, 2].min() + 30.0 / 1.0
front_v, other_v = np.where(hem & (kind == "front"))[0], np.where(hem & np.isin(kind, ["side", "back"]))[0]
body_centre = None


def tucked():
    """Hem vertices of the front panels that lie under (closer to the body than) the side or back cloth at the same angle."""
    P = posed()
    c = P[hem, :2].mean(axis=0) * 0.0 + np.array([0.0, centre[1] * 100.0 if abs(centre[1]) < 2 else centre[1]])
    rel = P[:, :2] - c
    ang = np.degrees(np.arctan2(np.abs(rel[:, 0]), -rel[:, 1]))
    rad = np.hypot(rel[:, 0], rel[:, 1])
    count = 0
    for i in front_v:
        near = other_v[(np.abs(ang[other_v] - ang[i]) < 8.0) & (np.sign(rel[other_v, 0]) == np.sign(rel[i, 0])) & (np.abs(P[other_v, 2] - P[i, 2]) < 8.0)]
        if len(near) and rad[i] < rad[near].max() - 1.5:
            count += 1
    return count


rows = []
tuck = []
azimuth = []
for f in range(2, int(os.environ.get("BC_FRAMES", "150"))):
    scene.frame_set(f)
    bpy.context.view_layer.update()
    P = posed()
    n1, a1 = nrm_area(P)
    flipped = int(((n1 * n0).sum(axis=1) < 0).sum()) - base_flipped
    squeezed = int((a1 / np.maximum(a0, 1e-12) < 0.15).sum()) - base_squeezed
    rows.append((f, flipped, squeezed))
    azimuth.append(front_azimuth())
    tuck.append(tucked())
worst = max(rows, key=lambda r: r[1] + r[2])
print("SPIN frames %d: triangles flipped over the rest shape: mean %.0f max %d, squeezed (under 15%% area) mean %.1f max %d; worst frame %d" % (
    len(rows), np.mean([r[1] for r in rows]), max(r[1] for r in rows), np.mean([r[2] for r in rows]),
    max(r[2] for r in rows), worst[0]))
print("FRONT HEMS %.0f deg round the body at most (mean %.0f); 90 is straight to the side" % (max(azimuth), np.mean(azimuth)))
print("TUCKED front hem vertices (of %d) under the side or back cloth: mean %.1f max %d" % (len(front_v), np.mean(tuck), max(tuck)))
shots = [int(x) for x in os.environ.get("BC_SHOT", "").split(",") if x] or [worst[0]]
for f in shots:
    scene.frame_set(f)
    render("f%d" % f)
sim_mod.stop(arm)
print("ALL OK")
