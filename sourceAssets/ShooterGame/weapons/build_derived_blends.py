"""ONE-TIME BUILDER (already run): derives weapons/thrown/<name>.blend and weapons/thirdPerson/<name>.blend from the
viewmodel blend. From now on those two files are edited by hand and exported with export_static.py.
The original text of this script said: exports everything the game needs of the throwable this blend file is for, into
source/Games/ShooterGame/GameData/models/player/weapons/<name>/ :

    <name>.glb          the viewmodel, every clip as its own animation
    <name>_thrown.glb   the static model of the object in flight, on the ground, and as a pickup
    <name>_tp.glb       the third-person model: the player skeleton with the object skinned to the right hand

    blender -b sourceAssets/ShooterGame/dynamite.blend --python sourceAssets/ShooterGame/export_throwable.py
    blender -b sourceAssets/ShooterGame/molotov.blend  --python sourceAssets/ShooterGame/export_throwable.py

or both at once: sourceAssets/ShooterGame/export_throwables.bat

The weapon is taken from the blend file's name (dynamite -> clips dyn_*, molotov -> clips mol_*). Set the environment
variable EXPORT_DIR to write somewhere else. Textures bigger than TEXTURE_SIZE are scaled down in the exported files only;
the blend file keeps the full size.

Why not File > Export for the viewmodel: the skeleton `root` follows the control rig `ArmsCtrl` through constraints, and one
action per clip drives both rigs. The glTF exporter would sample only one clip, so every clip is sampled here (constraints
on) and written as a self-contained action on `root` (constraints muted), then all clips are exported together as NLA
tracks. Every stage starts from the saved blend file again; the blend file itself is never saved. Run with blender -b.
"""
import bpy, os, math
import numpy as np
from mathutils import Matrix, Quaternion, Vector, Euler

TEXTURE_SIZE = 512

WEAPONS = {
    "dynamite": dict(prefix="dyn", meshes=["Dynamite", "DynamiteSpark", "Lighter", "LighterFlame"],
                     thrown=dict(label="thrown_dynamite", parts=["Dynamite"], up_bone="wick_1")),
    "molotov": dict(prefix="mol", meshes=["Molotov", "MolotovRag", "MolotovFlame", "Lighter", "LighterFlame"],
                    thrown=dict(label="thrown_molotov", parts=["Molotov", "MolotovRag"], up_bone="rag_2")),
}
CLIPS = ["idle", "draw", "throw_start", "throw_charge", "throw_end"]

# third person: where the object sits in the right hand, in the frame of the hand's weapon_r bone (cm, degrees)
TP_SCALE = 0.8
TP_LOC = (0.0, 4.4, 1.0)
TP_ROT = (-90.0, 0.0, 0.0)

if bpy.app.background is False:
    raise SystemExit("run this with blender -b (it changes the open scene)")

SRC = bpy.data.filepath
name = os.path.splitext(os.path.basename(SRC))[0]
if name not in WEAPONS:
    raise SystemExit("the blend file must be called dynamite.blend or molotov.blend, not %s.blend" % name)
cfg = WEAPONS[name]
prefix = cfg["prefix"]

def find_repo(start):
    """the repository root: the first folder above this file that holds both source/ and sourceAssets/"""
    d = os.path.abspath(os.path.dirname(start))
    while True:
        if os.path.isdir(os.path.join(d, "source")) and os.path.isdir(os.path.join(d, "sourceAssets")):
            return d
        up = os.path.dirname(d)
        if up == d:
            raise SystemExit("cannot find the repository root above %s (set REPO_ROOT)" % start)
        d = up


repo = os.environ.get("REPO_ROOT") or find_repo(SRC)
weapons_dir = os.path.join(repo, "source", "Games", "ShooterGame", "GameData", "models", "player", "weapons")
OUT = os.environ.get("EXPORT_DIR") or os.path.join(os.environ.get("TEMP", "/tmp"), "throwable_build", name)
THROWN_BLEND = os.path.join(os.path.dirname(SRC), "thrown", name + ".blend")
TP_BLEND = os.path.join(os.path.dirname(SRC), "thirdPerson", name + ".blend")
PISTOL_TP = os.path.join(weapons_dir, "pistol", "pistol_tp.glb")
os.makedirs(OUT, exist_ok=True)


def reopen():
    bpy.ops.wm.open_mainfile(filepath=SRC)


def shrink_textures():
    for img in bpy.data.images:
        if img.type == 'IMAGE' and img.size[0] > TEXTURE_SIZE and img.size[1] > TEXTURE_SIZE:
            print("TEXTURE", img.name, tuple(img.size), "->", TEXTURE_SIZE)
            img.scale(TEXTURE_SIZE, TEXTURE_SIZE)


def select_only(objs):
    for o in bpy.context.view_layer.objects:
        o.select_set(False)
    for o in objs:
        o.hide_set(False)
        o.select_set(True)
    bpy.context.view_layer.objects.active = objs[0]


# ================================================================================================ thrown model
def stage_thrown():
    """The static model: the props in their rest pose, in meters, +Z up, origin in the middle of the body."""
    reopen()
    t = cfg["thrown"]
    root = bpy.data.objects["root"]
    sc = bpy.context.scene
    root.data.pose_position = 'POSE'
    if root.animation_data:
        root.animation_data.action = None
        root.animation_data.use_nla = False
    bpy.data.objects["ArmsCtrl"]["Manny_Follow"] = 0.0
    for pb in root.pose.bones:
        pb.location = (0, 0, 0)
        pb.rotation_quaternion = (1, 0, 0, 0)
        pb.rotation_euler = (0, 0, 0)
        pb.scale = (1, 1, 1)
    bpy.context.view_layer.update()

    if name == "molotov":
        # the rest pose lets the rag hang down the bottle: stand it up out of the neck, with a small droop
        SINK, DROOP = 3.0, (10, 18)
        pb = root.pose.bones
        ev0 = bpy.data.objects["Molotov"].evaluated_get(bpy.context.evaluated_depsgraph_get())
        me0 = bpy.data.meshes.new_from_object(ev0)
        inv = root.matrix_world.inverted() @ bpy.data.objects["Molotov"].matrix_world
        P = np.array([(inv @ v.co)[:] for v in me0.vertices])
        c = P.mean(axis=0)
        ax = Vector(np.linalg.svd(P - c, full_matrices=False)[2][0]).normalized()   # bottle axis from the mesh itself
        neck_dir = ax if (pb["rag_1"].head - Vector(c)).dot(ax) > 0 else -ax
        d1 = (pb["rag_1"].tail - pb["rag_1"].head).normalized()
        R = d1.rotation_difference(neck_dir).to_matrix().to_4x4()
        m0 = pb["rag_1"].matrix.copy()
        m0.translation = (0, 0, 0)
        new = R @ m0
        new.translation = pb["rag_1"].head.copy() - neck_dir * SINK
        pb["rag_1"].matrix = new
        bpy.context.view_layer.update()
        for n, a in (("rag_2", DROOP[0]), ("rag_3", DROOP[1])):
            pb[n].rotation_mode = 'XYZ'
            pb[n].rotation_euler = (math.radians(a), 0, math.radians(a * 0.4))
        bpy.context.view_layer.update()

    dg = bpy.context.evaluated_depsgraph_get()

    def world_mesh(n):
        o = bpy.data.objects[n]
        ev = o.evaluated_get(dg)
        me = bpy.data.meshes.new_from_object(ev, preserve_all_data_layers=True, depsgraph=dg)
        me.transform(o.matrix_world)
        return me

    def wpt(bone, tail=False):
        pb = root.pose.bones[bone]
        return root.matrix_world @ (pb.tail if tail else pb.head)

    mes = [world_mesh(n) for n in t["parts"]]
    # frame: +Z along the body's long axis (towards the wick / rag), origin in the middle of the body
    body = np.array([v.co[:] for v in mes[0].vertices])        # the first part is the body (bottle / bundle)
    c = body.mean(axis=0)
    vt = np.linalg.svd(body - c, full_matrices=False)[2]
    axis, side = Vector(vt[0]), Vector(vt[1])
    if axis.dot((wpt(t["up_bone"]) - Vector(c)).normalized()) < 0:
        axis = -axis
    x = side - axis * side.dot(axis)
    x.normalize()
    y = axis.cross(x)
    Rm = Matrix(((x.x, x.y, x.z), (y.x, y.y, y.z), (axis.x, axis.y, axis.z)))
    loc = [Rm @ Vector(p) for p in body]
    mn = Vector((min(l.x for l in loc), min(l.y for l in loc), min(l.z for l in loc)))
    mx = Vector((max(l.x for l in loc), max(l.y for l in loc), max(l.z for l in loc)))
    M = Matrix.Translation(-(mn + mx) * 0.5) @ Rm.to_4x4()

    objs = []
    for n, me in zip(t["parts"], mes):
        me.transform(M)
        me.name = t["label"] + "_" + n
        o = bpy.data.objects.new(me.name, me)
        sc.collection.objects.link(o)
        objs.append(o)
    select_only(objs)
    if len(objs) > 1:
        bpy.ops.object.join()
    obj = bpy.context.view_layer.objects.active
    obj.name = t["label"]
    obj.vertex_groups.clear()
    for m in list(obj.modifiers):
        obj.modifiers.remove(m)
    print("THROWN dims (m)", [round(v, 4) for v in obj.dimensions])

    # where the effects attach, in the model's own space (the code has them as constants)
    point = "spark" if name == "dynamite" else "rag_3"
    print("THROWN effect point (%s tip)" % point, [round(v, 4) for v in (M @ wpt(point, tail=True))])

    select_only([obj])
    path = os.path.join(OUT, name + "_thrown.glb")
    bpy.ops.export_scene.gltf(filepath=path, export_format='GLB', use_selection=True, export_animations=False,
                              export_apply=False, export_yup=True)
    print("EXPORTED", path, os.path.getsize(path))

    # keep only the model in the scene and save it as the thrown blend (full size textures)
    for o in list(bpy.data.objects):
        if o is not obj:
            bpy.data.objects.remove(o)
    bpy.ops.outliner.orphans_purge(do_local_ids=True, do_linked_ids=True, do_recursive=True)
    os.makedirs(os.path.dirname(THROWN_BLEND), exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=THROWN_BLEND, copy=True, relative_remap=True)
    print("SAVED", THROWN_BLEND)


# ================================================================================================ third-person model
def stage_tp():
    """The player skeleton (from pistol_tp.glb) with the thrown model skinned rigidly to weapon_r."""
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=PISTOL_TP)
    arm = [o for o in bpy.data.objects if o.type == 'ARMATURE'][0]
    for o in list(bpy.data.objects):
        if o.type == 'MESH':
            bpy.data.objects.remove(o)
    before = set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=os.path.join(OUT, name + "_thrown.glb"))
    new = [o for o in bpy.data.objects if o not in before and o.type == 'MESH'][0]
    for o in list(bpy.data.objects):
        if o not in before and o is not new:
            bpy.data.objects.remove(o)
    new.parent = None
    new.name = "w_r"
    bone = arm.data.bones["weapon_r"]
    M = (bone.matrix_local @ Matrix.Translation(TP_LOC)
         @ Euler([math.radians(a) for a in TP_ROT], 'XYZ').to_matrix().to_4x4() @ Matrix.Scale(100 * TP_SCALE, 4))
    new.data.transform(new.matrix_world)
    new.matrix_world = Matrix.Identity(4)
    new.data.transform(M)
    new.parent = arm
    new.matrix_parent_inverse = Matrix.Identity(4)
    new.vertex_groups.new(name="weapon_r").add(list(range(len(new.data.vertices))), 1.0, 'REPLACE')
    new.modifiers.new("Armature", 'ARMATURE').object = arm
    select_only([arm, new])
    bpy.context.view_layer.objects.active = arm
    path = os.path.join(OUT, name + "_tp.glb")
    bpy.ops.export_scene.gltf(filepath=path, export_format='GLB', use_selection=True, export_animations=False)
    print("EXPORTED", path, os.path.getsize(path))

    for o in list(bpy.data.objects):
        if o is not arm and o is not new:
            bpy.data.objects.remove(o)
    bpy.ops.outliner.orphans_purge(do_local_ids=True, do_linked_ids=True, do_recursive=True)
    os.makedirs(os.path.dirname(TP_BLEND), exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=TP_BLEND, copy=True, relative_remap=True)
    print("SAVED", TP_BLEND)



stage_thrown()
stage_tp()
