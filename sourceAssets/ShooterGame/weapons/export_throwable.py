"""Exports the viewmodel of a throwable from its blend file into
source/Games/ShooterGame/GameData/models/player/weapons/<name>/<name>.glb, every clip as its own animation.

    blender -b sourceAssets/ShooterGame/weapons/dynamite.blend --python sourceAssets/ShooterGame/weapons/export_throwable.py
    blender -b sourceAssets/ShooterGame/weapons/molotov.blend  --python sourceAssets/ShooterGame/weapons/export_throwable.py

or everything at once (viewmodels, thrown models, third-person models): sourceAssets/ShooterGame/weapons/export_throwables.bat

The thrown model (<name>_thrown.glb) and the third-person model (<name>_tp.glb) have their own blend files, see
export_static.py: weapons/thrown/<name>.blend and weapons/thirdPerson/<name>.blend.

The weapon is taken from the blend file's name (dynamite -> clips dyn_*, molotov -> clips mol_*). Set the environment
variable EXPORT_DIR to write somewhere else. Textures bigger than TEXTURE_SIZE are scaled down in the exported files only;
the blend file keeps the full size.

Why not File > Export for the viewmodel: the skeleton `root` follows the control rig `ArmsCtrl` through constraints, and one
action per clip drives both rigs. The glTF exporter would sample only one clip, so every clip is sampled here (constraints
on) and written as a self-contained action on `root` (constraints muted), then all clips are exported together as NLA
tracks. Every stage starts from the saved blend file again; the blend file itself is never saved. Run with blender -b.
"""
import bpy, os
from mathutils import Matrix, Quaternion, Vector

TEXTURE_SIZE = 512

WEAPONS = {
    "dynamite": dict(prefix="dyn", meshes=["Dynamite", "DynamiteSpark", "Lighter", "LighterFlame"]),
    "molotov": dict(prefix="mol", meshes=["Molotov", "MolotovRag", "MolotovFlame", "Lighter", "LighterFlame"]),
}
CLIPS = ["idle", "draw", "throw_start", "throw_charge", "throw_end"]


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
OUT = os.environ.get("EXPORT_DIR") or os.path.join(weapons_dir, name)
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


# ================================================================================================ viewmodel
def stage_viewmodel():
    reopen()
    shrink_textures()
    sc = bpy.context.scene
    ctrl = bpy.data.objects["ArmsCtrl"]
    root = bpy.data.objects["root"]
    ctrl.animation_data_create()
    root.animation_data_create()
    root.animation_data.use_nla = True
    for t in list(root.animation_data.nla_tracks):
        root.animation_data.nla_tracks.remove(t)
    ctrl["Manny_Follow"] = 1.0

    def assign_action(obj, action):
        """layered actions need their slot set explicitly: an object that had another action assigned before keeps a stale slot and plays nothing"""
        obj.animation_data.action = action
        obj.animation_data.action_slot = obj.animation_data.action_suitable_slots[0]

    bones = [b.name for b in root.data.bones]
    rest = {b.name: b.matrix_local.copy() for b in root.data.bones}
    parent = {b.name: (b.parent.name if b.parent else None) for b in root.data.bones}

    def basis_of(n, pose_mats):
        """local channel matrix of a bone from armature-space pose matrices"""
        m = pose_mats[n]
        p = parent[n]
        if p is None:
            return rest[n].inverted() @ m
        rest_rel = rest[p].inverted() @ rest[n]
        return rest_rel.inverted() @ (pose_mats[p].inverted() @ m)

    # ---- 1. sample every clip with constraints on
    samples = {}
    for clip in CLIPS:
        act = bpy.data.actions["%s_%s" % (prefix, clip)]
        assign_action(ctrl, act)
        assign_action(root, act)
        f0, f1 = int(round(act.frame_range[0])), int(round(act.frame_range[1]))
        frames = []
        for f in range(f0, f1 + 1):
            sc.frame_set(f)
            ev = root.evaluated_get(bpy.context.evaluated_depsgraph_get())
            mats = {pb.name: pb.matrix.copy() for pb in ev.pose.bones}
            frames.append((f, {n: basis_of(n, mats).decompose() for n in bones}))
        samples[clip] = frames
        print("SAMPLED", clip, f0, f1)

    def moves(n, kind):
        for frames in samples.values():
            for _, row in frames:
                loc, q, s = row[n]
                if kind == "loc" and loc.length > 1e-5: return True
                if kind == "rot" and abs(q.w) < 1 - 1e-6: return True
                if kind == "scl" and (s - Vector((1, 1, 1))).length > 1e-5: return True
        return False

    keyed = {n: {k: moves(n, k) for k in ("loc", "rot", "scl")} for n in bones}   # only channels that move anywhere get keys

    # ---- 2. self-contained actions, no constraints
    ctrl.animation_data.action = None
    root.animation_data.action = None
    for pb in root.pose.bones:
        for c in list(pb.constraints):
            c.mute = True
        pb.rotation_mode = 'QUATERNION'
        pb.location = (0, 0, 0)
        pb.rotation_quaternion = (1, 0, 0, 0)
        pb.scale = (1, 1, 1)
    for d in list(root.animation_data.drivers):
        root.animation_data.drivers.remove(d)

    bpy.context.preferences.edit.keyframe_new_interpolation_type = 'LINEAR'

    for clip in CLIPS:
        act = bpy.data.actions.new(clip)
        root.animation_data.action = act
        prev_q = {}
        for f, row in samples[clip]:
            for n in bones:
                k = keyed[n]
                if not any(k.values()):
                    continue
                pb = root.pose.bones[n]
                loc, q, s = row[n]
                if k["rot"]:
                    pq = prev_q.get(n)
                    if pq is not None and pq.dot(q) < 0:
                        q = Quaternion((-q.w, -q.x, -q.y, -q.z))
                    prev_q[n] = q
                    pb.rotation_quaternion = q
                    pb.keyframe_insert("rotation_quaternion", frame=f, group=n)
                if k["loc"]:
                    pb.location = loc
                    pb.keyframe_insert("location", frame=f, group=n)
                if k["scl"]:
                    pb.scale = s
                    pb.keyframe_insert("scale", frame=f, group=n)
        root.animation_data.action = None
        tr = root.animation_data.nla_tracks.new()
        tr.name = clip
        tr.strips.new(clip, samples[clip][0][0], act)
        print("BAKED", clip, "frames", len(samples[clip]))

    # ---- 3. export
    # Only the skeleton and the props go into the glb. The hands (arms, sleeves, cuff) can be visible in the blend file
    # while animating: the game draws its own arms, so they are never exported, whatever their visibility is.
    HANDS = ("ArmsMesh", "arms", "ArmsSkin_Sleeved", "Sleeve", "Shirt_Cuff", "Lighter_prop")
    assert not any(m in HANDS for m in cfg["meshes"]), "a hand mesh is in the export list"
    select_only([root] + [bpy.data.objects[m] for m in cfg["meshes"]])
    print("EXPORT OBJECTS", [o.name for o in bpy.context.view_layer.objects if o.select_get()])
    path = os.path.join(OUT, name + ".glb")
    bpy.ops.export_scene.gltf(filepath=path, export_format='GLB', use_selection=True, export_animations=True,
                              export_animation_mode='NLA_TRACKS', export_force_sampling=True, export_apply=False)
    print("EXPORTED", path, os.path.getsize(path))


stage_viewmodel()
