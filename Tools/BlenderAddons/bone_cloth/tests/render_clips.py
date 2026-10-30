"""Renders the wind and turn clips the way the game mixes them, as contact sheets: one row per pose, five views (front,
three-quarter, left side, back, top), the cloth coloured by panel (darker toward the hem) over the grey body.
BC_NO_PREPARE=1 python tests/run_headless.py render_clips.py <copy of ShooterGame/player_coat.blend>
Output: BC_RENDER_DIR (default TEMP)/clips_<sheet>.png and clips_legend.txt. BC_GENERATE=0 keeps the scene's clips,
BC_SET=gen.blow_back=1.5,... changes the generator's settings, BC_SHEETS picks sheets."""
import math
import os
import sys
import bpy
import numpy as np
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import bones, clipgen, poses, props
from bone_cloth import engine_contract as ec
bone_cloth.register()

out_dir = os.environ.get("BC_RENDER_DIR", os.environ.get("TEMP", "."))
arm = bpy.data.objects["root"]
bpy.context.view_layer.objects.active = arm
if arm.animation_data:
    arm.animation_data.action = None
setup = props.get_setup(arm)
for item in filter(None, os.environ.get("BC_SET", "").split(",")):
    path, val = item.split("=")
    obj = setup.poses
    parts = path.split(".")
    for part in parts[:-1]:
        obj = getattr(obj, part)
    setattr(obj, parts[-1], float(val))
if os.environ.get("BC_PARAMETRIC"):           # as if there were no hand-made Open pose: the generator's own hinge
    setup.poses.open.mode = 'GENERATED'
    if setup.poses.open.action is not None and "bc_sig" in setup.poses.open.action:
        del setup.poses.open.action["bc_sig"]
if os.environ.get("BC_GENERATE", "1") == "1":
    print("generated:", clipgen.generate(setup, arm, force=True))
sets = poses.clip_sample_sets(setup)
cloth = poses.cloth_bone_names(setup, arm)

# ---- look: panels in colour, the body grey ----
palette = {"front_l": (0.85, 0.25, 0.2), "front_r": (0.2, 0.45, 0.9), "back_l": (0.95, 0.7, 0.15),
           "back_r": (0.25, 0.75, 0.35), "side_l": (0.8, 0.3, 0.8), "side_r": (0.2, 0.8, 0.8)}
panel = {}
for info in bones.strips(setup, arm):
    key = "%s_%s" % (info["column"], info["side"].lower()) if info["side"] else info["column"]
    for k, b in enumerate(info["bones"]):
        panel[b] = (np.array(palette.get(key, (0.7, 0.7, 0.7))), k / max(len(info["bones"]) - 1, 1))
cloth_objs = [r.obj for r in setup.cloth_objects if r.obj]
for o in cloth_objs:
    names = {g.index: g.name for g in o.vertex_groups}
    attr = o.data.color_attributes.get("viz") or o.data.color_attributes.new("viz", 'FLOAT_COLOR', 'POINT')
    for i, v in enumerate(o.data.vertices):
        c, body = np.zeros(3), 0.0
        for g in v.groups:
            n = names.get(g.group)
            if n in panel:
                base, along = panel[n]
                c += g.weight * base * (1.0 - 0.45 * along)
            else:
                body += g.weight
        c += body * np.array([0.55, 0.55, 0.58])
        attr.data[i].color = (c[0], c[1], c[2], 1.0)
    o.data.color_attributes.active_color = attr
shown = set(cloth_objs) | ({o for o in bpy.data.objects if o.type == 'MESH' and o.name.startswith("body")}
                           if not os.environ.get("BC_NO_BODY") else set())
for o in bpy.data.objects:
    if o.type == 'MESH':
        o.hide_render = o not in shown
for o in shown:
    if o not in cloth_objs:
        attr = o.data.color_attributes.get("viz") or o.data.color_attributes.new("viz", 'FLOAT_COLOR', 'POINT')
        for d in attr.data:
            d.color = (0.5, 0.5, 0.52, 1.0)
        o.data.color_attributes.active_color = attr

scene = bpy.context.scene
scene.render.engine = 'BLENDER_WORKBENCH'
scene.display.shading.light = 'STUDIO'
scene.display.shading.color_type = 'VERTEX'
scene.display.shading.show_cavity = True
scene.display.shading.show_object_outline = True
res = os.environ.get("BC_RES", "300x380").split("x")
scene.render.resolution_x, scene.render.resolution_y = int(res[0]), int(res[1])
scene.render.film_transparent = False
scene.world = scene.world or bpy.data.worlds.new("w")
scene.world.color = (0.16, 0.16, 0.18)
cam_data = bpy.data.cameras.new("cam")
cam_data.type = 'ORTHO'
cam = bpy.data.objects.new("cam", cam_data)
scene.collection.objects.link(cam)
scene.camera = cam
centre = np.array(arm.matrix_world @ arm.data.bones["spine_01"].head_local)
# name: (direction from the centre to the camera, ortho scale); the character faces -Y, its left is +X
VIEWS = [("front", (0, -1, 0), 1.25), ("3/4", (0.7, -0.7, 0.25), 1.25), ("left", (1, 0, 0), 1.25),
         ("back", (0, 1, 0), 1.25), ("back high", (0.25, 0.75, 0.6), 1.25), ("back low", (-0.3, 0.9, -0.15), 0.95),
         ("top", (0, 0.001, 1), 0.9), ("below", (0.0, 0.35, -1.0), 1.0),
         ("under back", (0.15, 0.6, -0.8), 0.6), ("under front", (-0.15, -0.6, -0.8), 0.6)]
if os.environ.get("BC_VIEWS"):
    VIEWS = [v for v in VIEWS if v[0] in os.environ["BC_VIEWS"].split(",")]


def render_pose(bases, locs, tag):
    poses.set_bases(arm, bases, locs)
    bpy.context.view_layer.update()
    tiles = []
    for name, d, scale in VIEWS:
        d = np.array(d, float)
        d /= np.linalg.norm(d)
        target = centre + np.array([0.0, 0.0, 0.0 if name == "top" else (-0.1 if name.startswith("under") else 0.1)])
        cam.location = tuple(target + d * 4.0)
        cam_data.ortho_scale = scale
        # the top view looks down from chest height (no hat, head or shoulders): the cloth round the torso
        cam_data.clip_start = 4.0 - (1.42 - target[2]) if name == "top" else 0.1
        direction = -d
        from mathutils import Vector
        cam.rotation_euler = Vector(tuple(direction)).to_track_quat('-Z', 'Y').to_euler()
        path = os.path.join(out_dir, "_tile.png")
        scene.render.filepath = path
        bpy.ops.render.render(write_still=True)
        img = bpy.data.images.load(path, check_existing=False)
        px = np.array(img.pixels[:]).reshape(img.size[1], img.size[0], 4)
        bpy.data.images.remove(img)
        tiles.append(px)
    return np.concatenate([np.pad(t, ((4, 4), (4, 4), (0, 0)), constant_values=0.05) for t in tiles], axis=1)


def save_sheet(rows, name):
    sheet = np.concatenate(rows, axis=0)
    img = bpy.data.images.new(name, sheet.shape[1], sheet.shape[0], alpha=True)
    img.pixels = sheet.ravel().tolist()
    img.filepath_raw = os.path.join(out_dir, "clips_%s%s.png" % (name, os.environ.get("BC_SUFFIX", "")))
    img.file_format = 'PNG'
    img.save()
    print("sheet", img.filepath_raw)


def pose_of(shares):
    """One layer [(slot, position, weight)], or engine_contract.clip_layers' list of them."""
    p = poses.blend_layers(sets, shares if shares and isinstance(shares[0], list) else [shares])
    return {b: q for b, (q, _) in p.items()}, {b: l for b, (_, l) in p.items()}


SLOT = {k: i for i, k in enumerate(ec.CLIP_SLOTS)}
SHEETS = {
    "front_back": [("rest", []), ("headwind 50% (frame 2)", [(SLOT["windFront"], 0.5, 1.0)]),
                   ("headwind 75%", [(SLOT["windFront"], 0.75, 1.0)]), ("headwind 100% (frame 3)", [(SLOT["windFront"], 1.0, 1.0)]),
                   ("tailwind 50%", [(SLOT["windBack"], 0.5, 1.0)]), ("tailwind 100%", [(SLOT["windBack"], 1.0, 1.0)])],
    "sides": [("wind from left 50%", [(SLOT["windLeft"], 0.5, 1.0)]), ("wind from left 100%", [(SLOT["windLeft"], 1.0, 1.0)]),
              ("wind from right 100%", [(SLOT["windRight"], 1.0, 1.0)]),
              ("front-left 45deg 100%", ec.clip_layers(-1.0, -1.0, 1.0, 0.0)),
              ("back-left 135deg 100%", ec.clip_layers(1.0, -1.0, 1.0, 0.0))],
    "turns": [("turn left 50%", [(SLOT["turnLeft"], 0.5, 1.0)]), ("turn left 100%", [(SLOT["turnLeft"], 1.0, 1.0)]),
              ("turn right 100%", [(SLOT["turnRight"], 1.0, 1.0)]),
              ("headwind 60% + turn left 60%", ec.clip_layers(-1.0, 0.0, 0.6, 0.6)),
              ("headwind 100% + turn left 30%", ec.clip_layers(-1.0, 0.0, 1.0, 0.3))],
}
SHEETS["hem"] = [("rest", []), ("headwind 50% (frame 2)", [(SLOT["windFront"], 0.5, 1.0)]),
                 ("headwind 75%", [(SLOT["windFront"], 0.75, 1.0)]), ("headwind 100% (frame 3)", [(SLOT["windFront"], 1.0, 1.0)])]
SHEETS["turn"] = [("rest", []), ("turn left 50% (frame 2)", [(SLOT["turnLeft"], 0.5, 1.0)]),
                  ("turn left 100% (frame 3)", [(SLOT["turnLeft"], 1.0, 1.0)])]
SHEETS["turns_both"] = [("turn left 100%", [(SLOT["turnLeft"], 1.0, 1.0)]), ("turn right 100%", [(SLOT["turnRight"], 1.0, 1.0)]),
                        ("headwind 100% (compare)", [(SLOT["windFront"], 1.0, 1.0)])]
SHEETS["seam"] = [("headwind 50% = your Open pose", [(SLOT["windFront"], 0.5, 1.0)]),
                  ("headwind 100%", [(SLOT["windFront"], 1.0, 1.0)]), ("turn left 100%", [(SLOT["turnLeft"], 1.0, 1.0)]),
                  ("turn right 100%", [(SLOT["turnRight"], 1.0, 1.0)])]
SHEETS["turncmp"] = [("hand turn left 50%", [(SLOT["turnLeft"], 0.5, 1.0)]), ("hand turn left 100%", [(SLOT["turnLeft"], 1.0, 1.0)]),
                     ("generated 50% (in the windBack slot)", [(SLOT["windBack"], 0.5, 1.0)]),
                     ("generated 100%", [(SLOT["windBack"], 1.0, 1.0)])]
SHEETS["strong"] = [("headwind 100%", [(SLOT["windFront"], 1.0, 1.0)]), ("wind from left 100%", [(SLOT["windLeft"], 1.0, 1.0)]),
                    ("tailwind 100%", [(SLOT["windBack"], 1.0, 1.0)]), ("turn left 100%", [(SLOT["turnLeft"], 1.0, 1.0)])]
wanted = os.environ.get("BC_SHEETS", "front_back,sides,turns").split(",")
with open(os.path.join(out_dir, "clips_legend.txt"), "w") as f:
    for sheet in wanted:
        f.write("%s: views front | 3/4 | left | back | back high | top; rows: %s\n" % (sheet, " / ".join(n for n, _ in SHEETS[sheet])))
with poses.held_pose(arm):
    for sheet in wanted:
        rows = []
        for label, shares in SHEETS[sheet]:
            flat = [t for layer in shares for t in layer] if shares and isinstance(shares[0], list) else shares
            print("pose", sheet, label, [(ec.CLIP_SLOTS[s], round(p, 2), round(w, 2)) for s, p, w in flat])
            rows.append(render_pose(*pose_of(shares), label))
        save_sheet(rows[::-1], sheet)      # image rows go bottom-up
print("ALL OK")
