"""Renders the rigged coat from the front, side and back: vertices coloured by the panel they follow (grey = body),
bones drawn in front. Output PNGs go to BC_RENDER_DIR."""
import math
import os
import sys
import bpy
import numpy as np
from mathutils import Vector
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import props, presets, pipeline, bones
bone_cloth.register()

out_dir = os.environ.get("BC_RENDER_DIR", os.environ.get("TEMP", "."))
arm = bpy.data.objects['root']
bpy.context.view_layer.objects.active = arm
coat = bpy.data.objects[os.environ.get('BC_CLOTH', 'coat')]
setup = props.get_setup(arm)
if setup is not None:
    bpy.ops.bone_cloth.start_over()
else:
    setup = props.new_setup(arm, "coat")
    setup.cloth_objects.add().obj = coat
presets.apply(setup, os.environ.get("BC_PRESET", "LONG_COAT"))
if os.environ.get("BC_FLARE"):
    setup.rest.flare = float(os.environ["BC_FLARE"])
for item in filter(None, os.environ.get("BC_SET", "").split(",")):
    path, val = item.split("=")
    obj = setup
    parts = path.split(".")
    for part in parts[:-1]:
        obj = getattr(obj, part)
    setattr(obj, parts[-1], type(getattr(obj, parts[-1]))(float(val)) if not isinstance(getattr(obj, parts[-1]), bool) else bool(int(val)))
pipeline.auto_setup(setup, arm, log=lambda m: None)

# colour by panel
panels = {}
palette = [(0.9, 0.2, 0.2), (0.2, 0.7, 0.2), (0.2, 0.4, 0.95), (0.95, 0.75, 0.1), (0.8, 0.2, 0.8), (0.1, 0.8, 0.8),
           (0.9, 0.5, 0.1), (0.5, 0.5, 0.9)]
for info in bones.strips(setup, arm):
    key = info["column"] + info["side"]
    for k, b in enumerate(info["bones"]):
        panels[b] = (palette[len(panels) % len(palette)] if False else None, key, k)
cols = sorted({v[1] for v in panels.values()})
colour_of = {c: palette[i % len(palette)] for i, c in enumerate(cols)}
me = coat.data
names = {vg.index: vg.name for vg in coat.vertex_groups}
attr = me.color_attributes.get("viz") or me.color_attributes.new("viz", 'FLOAT_COLOR', 'POINT')
for i, v in enumerate(me.vertices):
    c = np.zeros(3)
    body = 0.0
    shade = 0.0
    for g in v.groups:
        nm = names[g.group]
        if nm in panels:
            base = np.array(colour_of[panels[nm][1]])
            tone = 0.55 + 0.45 * (panels[nm][2] % 2)         # alternate light/dark along the panel to show the joints
            c += g.weight * base * tone
        else:
            body += g.weight
    c += body * np.array([0.62, 0.62, 0.62])
    attr.data[i].color = (c[0], c[1], c[2], 1.0)
me.color_attributes.active_color = attr

if os.environ.get("BC_HIDE_ARMS"):
    # cut the sleeve faces out of the display mesh so the near arm does not hide the back (test render only)
    import bmesh
    from bone_cloth import landmarks
    ex = landmarks.exclude_bones(arm, setup.character)
    bm = bmesh.new(); bm.from_mesh(coat.data)
    dl = bm.verts.layers.deform.verify()
    gname = {vg.index: vg.name for vg in coat.vertex_groups}
    def armw(v):
        return sum(w for gi, w in v[dl].items() if gname.get(gi) in ex)
    kill = [f for f in bm.faces if all(armw(v) > 0.4 for v in f.verts)]
    bmesh.ops.delete(bm, geom=kill, context='FACES')
    bm.to_mesh(coat.data); bm.free()
if os.environ.get("BC_PREVIEW_OPEN"):
    setup.poses.preview_open = float(os.environ["BC_PREVIEW_OPEN"])
    setup.poses.preview_volume = float(os.environ.get("BC_PREVIEW_VOLUME", "0"))
    bpy.context.view_layer.update()
bend = float(os.environ.get("BC_BEND", "0"))
if bend:
    from mathutils import Quaternion
    for name, f in (("spine_01", 0.3), ("spine_02", 0.4), ("spine_03", 0.3)):
        pb = arm.pose.bones[name]
        pb.rotation_mode = 'QUATERNION'
        pb.rotation_quaternion = Quaternion((1, 0, 0), math.radians(bend * f))
    bpy.context.view_layer.update()
for o in bpy.data.objects:
    if o.type == 'MESH' and o.name != coat.name and "bone_cloth_source_of" not in o:
        o.hide_render = True
        o.hide_viewport = True
scene = bpy.context.scene
scene.render.engine = 'BLENDER_WORKBENCH'
scene.display.shading.color_type = 'VERTEX'
scene.display.shading.light = 'FLAT'
scene.render.resolution_x, scene.render.resolution_y = 600, 900
scene.render.film_transparent = False
world = bpy.data.worlds.new("w") if not scene.world else scene.world
scene.world = world
world.color = (0.12, 0.12, 0.14)
arm.show_in_front = True
arm.data.display_type = 'STICK'
arm.data.collections_all  # noqa
for c in arm.data.collections_all:
    c.is_visible = c.name in ("Cloth", "Cloth Roots")
cam_data = bpy.data.cameras.new("cam")
cam_data.type = 'ORTHO'
cam_data.ortho_scale = 2.1
cam = bpy.data.objects.new("cam", cam_data)
scene.collection.objects.link(cam)
scene.camera = cam
views = {"front": (0, -5, 0.95, 90, 0, 0), "side": (5, 0, 0.95, 90, 0, 90), "back": (0, 5, 0.95, 90, 0, 180), "top": (0, 0, 6, 0, 0, 0)}
views = {k: v for k, v in views.items() if k in os.environ.get("BC_VIEWS", "front,side,back").split(",")}
for name, (x, y, z, rx, ry, rz) in views.items():
    cam.location = (x, y, z)
    cam.rotation_euler = (math.radians(rx), math.radians(ry), math.radians(rz))
    scene.render.filepath = os.path.join(out_dir, "view_%s%s.png" % (name, os.environ.get("BC_SUFFIX", "")))
    bpy.ops.render.render(write_still=True)
    print("rendered", scene.render.filepath)
