"""Exports the thrown model or the third-person model of a throwable from its own blend file into
source/Games/ShooterGame/GameData/models/player/weapons/<name>/ :

    weapons/thrown/<name>.blend      ->  <name>_thrown.glb   the static object in flight, on the ground and as a pickup
                                                              (meters, +Z up, origin in the middle of the body; the code has
                                                              the effect points as constants in its own space)
    weapons/thirdPerson/<name>.blend ->  <name>_tp.glb       the player skeleton with the object skinned to the weapon_r bone

    blender -b sourceAssets/ShooterGame/weapons/thrown/dynamite.blend --python sourceAssets/ShooterGame/weapons/export_static.py

Everything in the file is exported as it is. Set EXPORT_DIR to write somewhere else. Textures bigger than TEXTURE_SIZE are
scaled down in the exported file only. The blend file is never saved. Run with blender -b.

The two files were first derived from the viewmodel blend by build_derived_blends.py; after that they are edited by hand.
"""
import bpy, os

TEXTURE_SIZE = 512

if bpy.app.background is False:
    raise SystemExit("run this with blender -b")

SRC = bpy.data.filepath
name = os.path.splitext(os.path.basename(SRC))[0]
kind = os.path.basename(os.path.dirname(SRC))
if kind not in ("thrown", "thirdPerson"):
    raise SystemExit("the blend file must be in a folder called thrown or thirdPerson, not %s" % kind)


def find_repo(start):
    d = os.path.abspath(os.path.dirname(start))
    while True:
        if os.path.isdir(os.path.join(d, "source")) and os.path.isdir(os.path.join(d, "sourceAssets")):
            return d
        up = os.path.dirname(d)
        if up == d:
            raise SystemExit("cannot find the repository root above %s (set REPO_ROOT)" % start)
        d = up


repo = os.environ.get("REPO_ROOT") or find_repo(SRC)
OUT = os.environ.get("EXPORT_DIR") or os.path.join(repo, "source", "Games", "ShooterGame", "GameData", "models", "player", "weapons", name)
os.makedirs(OUT, exist_ok=True)

for img in bpy.data.images:
    if img.type == 'IMAGE' and img.size[0] > TEXTURE_SIZE and img.size[1] > TEXTURE_SIZE:
        print("TEXTURE", img.name, tuple(img.size), "->", TEXTURE_SIZE)
        img.scale(TEXTURE_SIZE, TEXTURE_SIZE)

objs = [o for o in bpy.context.scene.objects if o.type in ('MESH', 'ARMATURE')]
for o in bpy.context.view_layer.objects:
    o.select_set(False)
for o in objs:
    o.hide_set(False)
    o.select_set(True)

if kind == "thrown":
    bpy.context.view_layer.objects.active = objs[0]
    path = os.path.join(OUT, name + "_thrown.glb")
    bpy.ops.export_scene.gltf(filepath=path, export_format='GLB', use_selection=True, export_animations=False,
                              export_apply=False, export_yup=True)
else:
    bpy.context.view_layer.objects.active = [o for o in objs if o.type == 'ARMATURE'][0]
    path = os.path.join(OUT, name + "_tp.glb")
    bpy.ops.export_scene.gltf(filepath=path, export_format='GLB', use_selection=True, export_animations=False)
print("EXPORTED", path, os.path.getsize(path))
