"""Crouch-walking sideways (the game's crouch_l / crouch_r from npc/base.glb, the body moving 2.5 m/s to that side): how far
the jacket's vertices go into the leg capsules (past the hip end), with and without the physics - the thighs come up
through the jacket's front as posed, and the leg probes push it back over them. Renders crouch_l at frame BC_SHOT (150)
to BC_RENDER_DIR.
Run on a copy of the jacket scene: BC_NO_PREPARE=1 python tests/run_headless.py test_crouch.py <copy of player_coat.blend>"""
import os, sys
import bpy
import numpy as np
from mathutils import Vector
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import props, colliders, bones as bones_mod, mesh_prep, sim as sim_mod
bone_cloth.register()

GLB = r"F:\Git\SdlWasmEngine\source\Games\ShooterGame\GameData\models\npc\base.glb"
FPS = 60.0
arm = bpy.data.objects["root"]
s = props.get_setup(arm)
scene = bpy.context.scene
before = set(bpy.data.actions)
before_obj = set(bpy.data.objects)
bpy.ops.import_scene.gltf(filepath=GLB)
for o in set(bpy.data.objects) - before_obj:
    bpy.data.objects.remove(o, do_unlink=True)
actions = {a.name: a for a in set(bpy.data.actions) - before}
for a in actions.values():                 # only the bones: the object keys would move the armature off its spot
    for layer in a.layers:
        for strip in layer.strips:
            for bag in strip.channelbags:
                for fc in [fc for fc in bag.fcurves if not fc.data_path.startswith("pose.bones")]:
                    bag.fcurves.remove(fc)
print("imported actions:", sorted(actions)[:40])
s.preview.motion, s.preview.wind_speed = 'ANIMATION', 0.0
start = arm.matrix_basis.copy()
legs = [c for c in s.colliders if c.covers]
cloth_bones = {b for info in bones_mod.strips(s, arm) for b in info["bones"]}


def leg_caps():
    out = []
    for c in legs:
        r0, r1 = colliders.radii(c)
        a = np.array(arm.matrix_world @ arm.pose.bones[c.bone].head)
        b = np.array(arm.matrix_world @ arm.pose.bones[c.to].head)
        out.append((a, b, r0, r1, c.bone))
    return out


def cloth_points():
    dg = bpy.context.evaluated_depsgraph_get()
    out = []
    for o in mesh_prep.cloth_objects(s):
        names = {g.index: g.name for g in o.vertex_groups}
        keep = np.array([sum(g.weight for g in v.groups if names.get(g.group) in cloth_bones) >= 0.5
                         for v in o.data.vertices])
        ev = o.evaluated_get(dg)
        me = ev.to_mesh()
        mw = np.array(o.matrix_world)
        co = np.array([v.co for v in me.vertices]) @ mw[:3, :3].T + mw[:3, 3]
        ev.to_mesh_clear()
        out.append(co[keep])
        cloth_points.owner = [max(((names.get(g.group), g.weight) for g in v.groups if names.get(g.group) in cloth_bones),
                                  key=lambda x: x[1])[0] for v, k in zip(o.data.vertices, keep) if k]
    return np.concatenate(out)


MID = 0.3     # the hip end of a thigh capsule is mostly pelvis: not counted


def inside(co):
    """Per point, the deepest it goes into a leg capsule (m), and which."""
    best, which = np.zeros(len(co)), np.full(len(co), "", dtype=object)
    for a, b, r0, r1, name in leg_caps():
        ab = b - a
        raw = (co - a) @ ab / max(ab @ ab, 1e-12)
        t = np.clip(raw, 0.0, 1.0)
        d = (r0 + (r1 - r0) * t) - np.linalg.norm(co - (a + t[:, None] * ab), axis=1)
        d[raw < MID] = 0.0
        which[d > best] = name
        best = np.maximum(best, d)
    return best, which


bpy.context.view_layer.update()
at_rest = inside(cloth_points())[0] > 0.0        # resting inside a leg (round the hips): not counted
print("jacket points %d, %d of them rest inside a leg" % (len(at_rest), at_rest.sum()))


def depth():
    d, which = inside(cloth_points())
    d[at_rest] = 0.0
    i = int(np.argmax(d))
    return float(d[i]) * 100.0, "%s (%s)" % (which[i], cloth_points.owner[i])


scene.render.engine = 'BLENDER_WORKBENCH'
scene.display.shading.light = 'STUDIO'
scene.display.shading.show_cavity = True
scene.display.shading.show_object_outline = True
scene.render.resolution_x, scene.render.resolution_y = 320, 400
cam_data = bpy.data.cameras.new("cam")
cam_data.type = 'ORTHO'
cam = bpy.data.objects.new("cam", cam_data)
scene.collection.objects.link(cam)
scene.camera = cam
out_dir = os.environ.get("BC_RENDER_DIR", os.environ.get("TEMP", "."))
for o in mesh_prep.cloth_objects(s):
    mat = bpy.data.materials.new("viz_coat")
    mat.diffuse_color = (0.85, 0.3, 0.2, 1.0)
    o.data.materials.clear()
    o.data.materials.append(mat)
scene.display.shading.color_type = 'MATERIAL'
shown = set(mesh_prep.cloth_objects(s)) | {bpy.data.objects[n] for n in ("body", "body.001", "arms") if n in bpy.data.objects}
for o in bpy.data.objects:
    if o.type == 'MESH':
        o.hide_render = o not in shown
VIEWS = [("front", (0, -1, 0.15)), ("left", (1, 0, 0.1)), ("right", (-1, 0, 0.1)), ("back", (0, 1, 0.15)),
         ("under", (0.3, -0.5, -0.8))]


def render(tag):
    centre = np.array(arm.matrix_world @ arm.pose.bones["pelvis"].head)
    tiles = []
    for name, d in VIEWS:
        d = np.array(d, float)
        d /= np.linalg.norm(d)
        cam.location = tuple(centre + d * 4.0)
        cam_data.ortho_scale = 1.3
        cam.rotation_euler = Vector(tuple(-d)).to_track_quat('-Z', 'Z' if name == "under" else 'Y').to_euler()
        path = os.path.join(out_dir, "_tile.png")
        scene.render.filepath = path
        bpy.ops.render.render(write_still=True)
        img = bpy.data.images.load(path, check_existing=False)
        tiles.append(np.array(img.pixels[:]).reshape(img.size[1], img.size[0], 4))
        bpy.data.images.remove(img)
    sheet = np.concatenate(tiles, axis=1)
    img = bpy.data.images.new(tag, sheet.shape[1], sheet.shape[0], alpha=True)
    img.pixels = sheet.ravel().tolist()
    img.filepath_raw = os.path.join(out_dir, "crouch_%s.png" % tag)
    img.file_format = 'PNG'
    img.save()
    print("sheet", img.filepath_raw)


def run(clip, side, physics, shot=None):
    act = next((a for n, a in actions.items() if n.split("|")[-1].startswith(clip) or n == clip), None)
    assert act, clip
    arm.animation_data_create()
    arm.animation_data.action = act
    if hasattr(arm.animation_data, "action_slot") and act.slots:
        arm.animation_data.action_slot = act.slots[0]
    m = s.motion
    keep = (m.swing, m.collision)
    if not physics:
        m.swing, m.collision = 0.0, 0.0
    sim = sim_mod.start(s, arm, scene)
    if sim_mod._handler in bpy.app.handlers.frame_change_post:
        bpy.app.handlers.frame_change_post.remove(sim_mod._handler)
    sim.reset()
    f0, f1 = act.frame_range
    worst, where, worst_f = 0.0, "", 0
    for f in range(240):
        t = f / FPS
        scene.frame_set(int(f0 + (f * 30.0 / FPS) % max(f1 - f0, 1)))
        arm.matrix_basis = start.copy()
        arm.matrix_basis.translation = start.translation + Vector((2.5 * side * t, 0.0, 0.0))   # +X: the left
        bpy.context.view_layer.update()
        sim.step(0.0 if f == 0 else 1.0 / FPS, t)
        if shot is not None and f == shot:
            render("%s_%s" % (clip, "on" if physics else "off"))
        if f >= 60:
            d, w = depth()
            if d > worst:
                worst, where, worst_f = d, w, f
    print("%s %s physics %s: deepest %.1f cm into %s (frame %d)" % (clip, "left" if side > 0 else "right",
          "on" if physics else "off", worst, where, worst_f))
    sim_mod.stop(arm)
    m.swing, m.collision = keep
    arm.matrix_basis = start
    return worst


print("motion:", {k: round(getattr(s.motion, k), 3) for k in ("swing", "collision", "motionWindow", "lagTip", "radius")})
print("legs:", [(c.bone, round(colliders.radii(c)[0], 3), round(colliders.radii(c)[1], 3)) for c in legs])
for clip, side in (("crouch_l", 1.0), ("crouch_r", -1.0)):
    shot = int(os.environ.get("BC_SHOT", "150")) if clip == "crouch_l" else None
    off = run(clip, side, False, shot)
    on = run(clip, side, True, shot)
    assert on < off - 1.5, (clip, off, on)    # the legs push the jacket over the thighs
print("ALL OK")
