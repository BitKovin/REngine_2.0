bl_info = {
    "name": "Export List GLB",
    "author": "",
    "version": (1, 1, 0),
    "blender": (4, 2, 0),
    "location": "3D Viewport > Sidebar (N) > Export",
    "description": "Export only the objects in an export list to .glb, with current shape keys baked in and "
                   "optional bones excluded (e.g. control bones)",
    "category": "Import-Export",
}

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time

import bpy
from fnmatch import fnmatchcase
from bpy.props import (BoolProperty, CollectionProperty, IntProperty, PointerProperty, StringProperty)
from bpy.types import Operator, Panel, PropertyGroup, UIList


# --------------------------------------------------------------------------------------------- data
class ELG_Item(PropertyGroup):
    obj: PointerProperty(name="Object", type=bpy.types.Object)


class ELG_Settings(PropertyGroup):
    items: CollectionProperty(type=ELG_Item)
    active: IntProperty()
    filepath: StringProperty(name="File", subtype='FILE_PATH', default="//export.glb")
    bake_shape_keys: BoolProperty(
        name="Bake Shape Keys", default=True,
        description="Bake the current shape key mix into the mesh and leave no shape keys in the output")
    exclude_names: StringProperty(
        name="Exclude Bones", default="",
        description="Bone names or patterns (* and ?), separated by commas. Matching bones are left out of the "
                    "exported skeleton, e.g.  ctrl_*, IK_*, root_ctrl. Leave empty to keep the skeleton untouched")
    exclude_collections: StringProperty(
        name="Exclude Bone Collections", default="",
        description="Bone collection names, separated by commas. Every bone in them is left out")
    deform_bones_only: BoolProperty(
        name="Deform Bones Only", default=False,
        description="Same as 'Use Deform Bones' in the glTF exporter (Data > Armature)")
    export_animations: BoolProperty(
        name="Animations", default=False, description="Also export the armature's actions")



# --------------------------------------------------------------------------------------------- helpers
def _split(text):
    return [t.strip() for t in text.split(",") if t.strip()]


def excluded_bones(arm_data, names, collections):
    """names of bones of arm_data that must not be exported"""
    out = set()
    for b in arm_data.bones:
        if any(fnmatchcase(b.name, p) for p in names):
            out.add(b.name)
    for cn in collections:
        col = arm_data.collections_all.get(cn)
        if col:
            out.update(b.name for b in col.bones)
    return out


def bake_shape_keys(obj):
    """obj owns a private copy of its mesh; replace its shape keys by their current mix."""
    me = obj.data
    if not me.shape_keys or len(me.shape_keys.key_blocks) == 0:
        return
    mix = obj.shape_key_add(name="__mix", from_mix=True)
    co = [0.0] * (len(me.vertices) * 3)
    mix.data.foreach_get("co", co)
    obj.active_shape_key_index = 0
    for kb in list(me.shape_keys.key_blocks)[::-1]:
        obj.shape_key_remove(kb)
    me.vertices.foreach_set("co", co)
    me.update()


def strip_bones(arm_obj, remove):
    """delete bones `remove` from the (copied) armature, children go to the nearest kept ancestor.
    arm_obj must be the only selected object and the active one.
    returns {removed bone name: nearest kept ancestor name or None}"""
    heir = {}
    bones = arm_obj.data.bones
    for n in remove:
        p = bones[n].parent
        while p and p.name in remove:
            p = p.parent
        heir[n] = p.name if p else None
    bpy.ops.object.mode_set(mode='EDIT')
    try:
        eb = arm_obj.data.edit_bones
        for n in remove:                                   # reparent first, so removal never orphans a kept bone
            for c in list(eb[n].children):
                if c.name not in remove:
                    # a connected bone would be snapped to its NEW parent's tail when leaving edit mode,
                    # which moves the joint. Disconnect first so its head stays exactly where it was.
                    c.use_connect = False
                    c.parent = eb[heir[n]] if heir[n] else None
        for n in remove:
            eb.remove(eb[n])
    finally:
        bpy.ops.object.mode_set(mode='OBJECT')
    return heir


def merge_groups(mesh_obj, heir):
    """weights of removed bones go to their nearest kept ancestor, then the removed groups are deleted"""
    vg = mesh_obj.vertex_groups
    for n, h in heir.items():
        g = vg.get(n)
        if not g:
            continue
        if h:
            dst = vg.get(h) or vg.new(name=h)
            for v in mesh_obj.data.vertices:
                for e in v.groups:
                    if e.group == g.index:
                        dst.add([v.index], e.weight, 'ADD')
        vg.remove(g)


def _rename(idb, new_name, log):
    """rename an ID and remember the old name in log (library data can't be renamed)"""
    if idb.library is not None:
        return
    log.append((idb, idb.name))
    idb.name = new_name


def _select_for_export(context, ob, unhidden, skipped):
    if ob.name not in context.view_layer.objects:
        skipped.append(ob.name)
        return False
    if ob.hide_get():
        ob.hide_set(False)
        unhidden.append(ob)
    ob.select_set(True)
    if not ob.select_get():
        skipped.append(ob.name)
        return False
    return True


# --------------------------------------------------------------------------------------------- baked export
# Some files animate a control rig ("ArmsCtrl") whose pose the exported skeleton ("root") only follows through constraints.
# The glTF exporter applies an action to the skeleton itself and cannot follow that, so every clip would come out in the pose
# the control rig happens to have. With Animations on and that rig in the file, the clips are baked onto the skeleton first,
# in a background Blender working on a temporary copy of the open file (this same script is run there, see bake_worker).
CONTROL_RIG = "ArmsCtrl"                                   # the object the actions are keyed on
EXPORT_RIG = "root"                                        # the skeleton that is exported and follows it
LOCATION_BONES = {"clavicle_l", "clavicle_r", "upperarm_l", "upperarm_r", "weapon"}   # bones whose location is baked, too
WORKER_FLAG = "--elg-bake-worker"


def reset_controls(ctrl):
    """Every pose bone of the armature back to its rest transform. A clip only keys the controls it moves: whatever it does not key keeps the
    value the clip evaluated before left there, so the baked pose would depend on the order the clips are baked in."""
    for pb in ctrl.pose.bones:
        pb.location = (0.0, 0.0, 0.0)
        pb.rotation_quaternion = (1.0, 0.0, 0.0, 0.0)
        pb.rotation_euler = (0.0, 0.0, 0.0)
        pb.scale = (1.0, 1.0, 1.0)


def bake_action(action, ctrl, manny):
    """The action of the control rig as a new action on the skeleton: what Blender shows of the skeleton for each frame."""
    from bpy_extras import anim_utils
    driven = {b.name[4:] for b in ctrl.data.bones if b.name.startswith("MCH-")}
    if "weapon.R" in ctrl.data.bones and "weapon" in manny.data.bones:
        driven.add("weapon")
    reset_controls(ctrl)
    reset_controls(manny)               # the skeleton keeps the last pose of the previous bake, and its constraints let it leak in
    for pb in manny.pose.bones:
        pb.select = pb.name in driven
    ctrl["Manny_Follow"] = 1.0
    ctrl.animation_data.action = action
    ctrl.animation_data.action_slot = action.slots[0]
    manny.animation_data_create()
    manny.animation_data.action = None
    bpy.context.view_layer.update()
    start, end = (int(v) for v in action.frame_range)
    opts = anim_utils.BakeOptions(only_selected=True, do_pose=True, do_object=False, do_visual_keying=True,
                                  do_constraint_clear=False, do_parents_clear=False, do_clean=False,
                                  do_location=True, do_rotation=True, do_scale=False, do_bbone=False, do_custom_props=False)
    baked = anim_utils.bake_action(manny, action=None, frames=range(start, end + 1), bake_options=opts)
    baked.name = f"{action.name}_manny"
    baked.use_fake_user = True
    for layer in baked.layers:                              # location only where bones really move (bone lengths never change)
        for strip in layer.strips:
            for bag in strip.channelbags:
                for fc in [f for f in bag.fcurves if f.data_path.endswith(".location")]:
                    if fc.data_path.split('"')[1] not in LOCATION_BONES:
                        bag.fcurves.remove(fc)
    baked.use_frame_range = True
    baked.frame_start, baked.frame_end = start, end
    baked.use_cyclic = action.use_cyclic
    return baked


def bake_worker():
    """Runs in the background Blender on the temporary copy: bakes every control rig action onto the skeleton and writes
    the glb. The copy is thrown away afterwards, nothing is saved."""
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    p = argparse.ArgumentParser()
    p.add_argument(WORKER_FLAG, action="store_true")
    p.add_argument("--out", required=True)
    p.add_argument("--objects", required=True, help="comma separated objects written to the glb")
    a = p.parse_args(argv)

    ctrl, manny = bpy.data.objects[CONTROL_RIG], bpy.data.objects[EXPORT_RIG]
    export_objects = [bpy.data.objects[n] for n in a.objects.split(",") if n]
    ctrl.animation_data_create()
    todo = [x for x in bpy.data.actions if x.slots
            and any(sl.target_id_type == "OBJECT" and sl.name_display == CONTROL_RIG for sl in x.slots)]
    if not todo:
        raise SystemExit("no action of the control rig")
    for act in todo:
        if act.frame_range[1] <= act.frame_range[0]:        # a held pose is one key: give it a second frame so the clip has a length
            act.use_frame_range = True
            act.frame_end = act.frame_start + 1
        elif act.use_cyclic and act.frame_range[1] - act.frame_range[0] > 2:
            act.use_frame_range = True                      # a loop ends on its first pose: the game loops it, so that
            act.frame_end -= 1                              # frame would be shown twice
    baked = {}
    for n, act in enumerate(todo):
        baked[act.name] = bake_action(act, ctrl, manny)
        print(f"[{n + 1}/{len(todo)}] baked {act.name}", flush=True)
    ctrl.animation_data.action = None
    ctrl["Manny_Follow"] = 0.0
    for name, act in baked.items():                 # the exporter names an animation after its action: free the plain names
        bpy.data.actions.remove(bpy.data.actions[name])
        act.name = name
    for act in list(bpy.data.actions):              # only the baked actions are written
        if act.name not in baked:
            bpy.data.actions.remove(act)
    for pb in manny.pose.bones:                     # a channel the actions do not key is written from the bone's current pose: rest
        pb.location = (0.0, 0.0, 0.0)
        pb.rotation_quaternion = (1.0, 0.0, 0.0, 0.0)
        pb.rotation_euler = (0.0, 0.0, 0.0)
        pb.scale = (1.0, 1.0, 1.0)
    manny.animation_data_create()
    manny.animation_data.action = None
    for ob in export_objects:
        ob.hide_set(False)
        ob.hide_viewport = False
    for ob in bpy.context.view_layer.objects:
        ob.select_set(False)
    for ob in export_objects:
        ob.select_set(True)
    bpy.context.view_layer.objects.active = manny
    bpy.ops.export_scene.gltf(filepath=a.out, export_format="GLB", use_selection=True, export_animations=True,
                              export_animation_mode="ACTIONS", export_force_sampling=True, export_frame_range=False,
                              export_anim_slide_to_zero=True, export_apply=False, export_skins=True, export_yup=True)
    print("exported", a.out, "with", len(baked), "animations")


def uses_control_rig(st, objs):
    """True when the actions must be baked from the control rig before they can be exported"""
    rig = bpy.data.objects.get(CONTROL_RIG)
    return bool(st.export_animations and rig is not None and rig.type == 'ARMATURE'
                and rig not in objs)


def export_baked(context, st, objs, report):
    """Hand a temporary copy of the open file to a background Blender that bakes every control rig action onto the
    exported skeleton and writes the glb. The open file, its pose, its actions and its selection stay as they are."""
    if _split(st.exclude_names) or _split(st.exclude_collections) or st.deform_bones_only:
        raise RuntimeError("Excluded bones / Deform Bones Only cannot be combined with the baked animation export "
                           "(clear them to export with animations)")
    path = bpy.path.abspath(st.filepath)
    if not path.lower().endswith(".glb"):
        path += ".glb"
    if context.mode != 'OBJECT':
        bpy.ops.object.mode_set(mode='OBJECT')
    tmp = tempfile.mkdtemp(prefix="elg_bake_")
    try:
        copy = os.path.join(tmp, "copy.blend")
        bpy.ops.wm.save_as_mainfile(filepath=copy, copy=True, compress=False)
        out = os.path.join(tmp, "out.glb")                 # written next to the copy first: a failed run never touches the real glb
        cmd = [bpy.app.binary_path, "-b", copy, "--python", os.path.abspath(__file__), "--", WORKER_FLAG, "--out", out,
               "--objects", ",".join(o.name for o in objs)]
        report({'INFO'}, "Baking control rig actions in a background Blender, please wait ...")
        t0 = time.time()
        run = subprocess.run(cmd, capture_output=True, text=True)
        log = (run.stdout + "\n" + run.stderr).strip().splitlines()
        if run.returncode != 0 or not os.path.isfile(out):
            raise RuntimeError("Baked export failed: " + " | ".join(log[-6:]))
        done = [l for l in log if l.startswith("exported ")]
        os.makedirs(os.path.dirname(path), exist_ok=True)
        shutil.copyfile(out, path)
        report({'INFO'}, f"Exported {len(objs)} objects to {path} ({done[-1] if done else 'baked'}, {time.time() - t0:.0f} s)")
        return path
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


# --------------------------------------------------------------------------------------------- export
def do_export(context, st, report):
    objs = list(dict.fromkeys(i.obj for i in st.items if i.obj))
    if not objs:
        raise RuntimeError("Export list is empty")
    if uses_control_rig(st, objs):
        return export_baked(context, st, objs, report)
    names, cols = _split(st.exclude_names), _split(st.exclude_collections)

    # ---- decide what really has to be touched. Everything else is exported as the ORIGINAL object, so
    # ---- its name, transform, parent inverse and the whole skeleton are identical to a plain glb export.
    removals = {}                                          # armature object -> bone names to drop
    for o in objs:
        if o.type == 'ARMATURE':
            r = excluded_bones(o.data, names, cols)
            if r:
                removals[o] = r

    copy_set = set(removals)
    for o in objs:
        if o.type != 'MESH':
            continue
        has_keys = o.data.shape_keys is not None and len(o.data.shape_keys.key_blocks) > 0
        if (st.bake_shape_keys and has_keys) or \
                any(m.type == 'ARMATURE' and m.object in removals for m in o.modifiers):
            copy_set.add(o)
    grew = True
    while grew:                                            # children of a copied parent must follow the copy
        grew = False
        for o in objs:
            if o not in copy_set and o.parent in copy_set:
                copy_set.add(o)
                grew = True

    temp_objs, temp_data = [], []
    mapping = {}                                           # original -> copy
    renamed = []                                           # (id, original name) of originals renamed meanwhile
    unhidden = []
    skipped = []
    data_real = {}

    prev_sel = [o for o in context.view_layer.objects if o.select_get()]
    prev_active = context.view_layer.objects.active
    if context.mode != 'OBJECT':
        bpy.ops.object.mode_set(mode='OBJECT')
    try:
        # 1. private copies, only of what needs changes. The original steps aside by name so the copy
        #    can carry the exact original name (node names in the glb stay unchanged, no ".001").
        for idx, o in enumerate(o for o in objs if o in copy_set):
            real = o.name
            _rename(o, f"__elg_tmp_{idx}", renamed)
            c = o.copy()
            c.name = real
            if o.type in {'MESH', 'ARMATURE'} and o.data is not None:
                od = o.data
                if od not in data_real:
                    data_real[od] = od.name
                    _rename(od, f"__elg_tmp_d{len(data_real)}", renamed)
                c.data = od.copy()
                c.data.name = data_real[od]
                temp_data.append(c.data)
            context.scene.collection.objects.link(c)
            temp_objs.append(c)
            mapping[o] = c

        # 2. parenting / armature modifiers between copies. Setting .parent from Python resets the parent
        #    inverse matrix, so it is put back afterwards; no matrix_world juggling at all.
        for o, c in mapping.items():
            if o.parent in mapping:
                ptype, pbone = c.parent_type, c.parent_bone
                mpi = c.matrix_parent_inverse.copy()
                c.parent = mapping[o.parent]
                if c.parent_type != ptype:
                    c.parent_type = ptype
                if ptype == 'BONE' and c.parent_bone != pbone:
                    c.parent_bone = pbone
                c.matrix_parent_inverse = mpi
            for m in c.modifiers:
                if m.type == 'ARMATURE' and m.object in mapping:
                    m.object = mapping[m.object]

        # 3. bake shape keys
        if st.bake_shape_keys:
            for c in temp_objs:
                if c.type == 'MESH':
                    bake_shape_keys(c)

        # 4. exclude bones (only armatures that actually lose bones were copied)
        removed_total = 0
        for o, remove in removals.items():
            c = mapping[o]
            for o_ in context.view_layer.objects:
                o_.select_set(False)
            c.select_set(True)
            context.view_layer.objects.active = c
            heir = strip_bones(c, remove)
            removed_total += len(remove)
            for m in temp_objs:
                if m.type == 'MESH' and any(md.type == 'ARMATURE' and md.object == c for md in m.modifiers):
                    merge_groups(m, heir)

        # 5. export: copies where they exist, originals everywhere else
        for o_ in context.view_layer.objects:
            o_.select_set(False)
        chosen = [mapping.get(o, o) for o in objs]
        picked = [e for e in chosen if _select_for_export(context, e, unhidden, skipped)]
        if not picked:
            raise RuntimeError("None of the listed objects could be selected for export")
        context.view_layer.objects.active = picked[0]
        context.view_layer.update()

        path = bpy.path.abspath(st.filepath)
        if not path.lower().endswith(".glb"):
            path += ".glb"
        bpy.ops.export_scene.gltf(filepath=path, export_format='GLB', use_selection=True,
                                  export_animations=st.export_animations, export_apply=False,
                                  export_def_bones=st.deform_bones_only)
        if skipped:
            report({'WARNING'}, "Not exported (hidden / not in view layer): " + ", ".join(skipped))
        report({'INFO'}, f"Exported {len(picked)} objects to {path} ({removed_total} bones excluded)")
        return path
    finally:
        for c in temp_objs:
            try:
                bpy.data.objects.remove(c, do_unlink=True)
            except ReferenceError:
                pass
        for d in temp_data:
            try:
                if isinstance(d, bpy.types.Mesh):
                    bpy.data.meshes.remove(d)
                else:
                    bpy.data.armatures.remove(d)
            except ReferenceError:
                pass
        for idb, old in renamed:                           # copies are gone, so the names are free again
            try:
                idb.name = old
            except ReferenceError:
                pass
        for o_ in context.view_layer.objects:
            o_.select_set(False)
        for o_ in unhidden:
            if o_.name in context.view_layer.objects:
                o_.hide_set(True)
        for o_ in prev_sel:
            if o_.name in context.view_layer.objects:
                o_.select_set(True)
        if prev_active and prev_active.name in context.view_layer.objects:
            context.view_layer.objects.active = prev_active


# --------------------------------------------------------------------------------------------- UI
class ELG_OT_add_selected(Operator):
    bl_idname = "export_list_glb.add_selected"
    bl_label = "Add Selected"
    bl_description = "Add the selected objects to the export list"

    def execute(self, context):
        st = context.scene.export_list_glb
        have = {i.obj for i in st.items}
        for o in context.selected_objects:
            if o not in have:
                st.items.add().obj = o
        return {'FINISHED'}


class ELG_OT_remove(Operator):
    bl_idname = "export_list_glb.remove"
    bl_label = "Remove"
    bl_description = "Remove the active entry from the export list"

    def execute(self, context):
        st = context.scene.export_list_glb
        if 0 <= st.active < len(st.items):
            st.items.remove(st.active)
            st.active = max(0, st.active - 1)
        return {'FINISHED'}


class ELG_OT_clear(Operator):
    bl_idname = "export_list_glb.clear"
    bl_label = "Clear"

    def execute(self, context):
        context.scene.export_list_glb.items.clear()
        return {'FINISHED'}


class ELG_OT_export(Operator):
    bl_idname = "export_list_glb.export"
    bl_label = "Export GLB"

    def execute(self, context):
        try:
            do_export(context, context.scene.export_list_glb, self.report)
        except Exception as e:
            self.report({'ERROR'}, str(e))
            return {'CANCELLED'}
        return {'FINISHED'}


class ELG_UL_items(UIList):
    def draw_item(self, context, layout, data, item, icon, active_data, active_propname, index):
        if item.obj:
            layout.label(text=item.obj.name, icon='OUTLINER_OB_' + item.obj.type if item.obj.type in
                         {'MESH', 'ARMATURE', 'EMPTY', 'CURVE'} else 'OBJECT_DATA')
        else:
            layout.label(text="<missing>", icon='ERROR')


class ELG_PT_panel(Panel):
    bl_label = "Export List GLB"
    bl_idname = "ELG_PT_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = "Export"

    def draw(self, context):
        st = context.scene.export_list_glb
        lay = self.layout
        row = lay.row()
        row.template_list("ELG_UL_items", "", st, "items", st, "active", rows=4)
        col = row.column(align=True)
        col.operator("export_list_glb.add_selected", icon='ADD', text="")
        col.operator("export_list_glb.remove", icon='REMOVE', text="")
        col.operator("export_list_glb.clear", icon='TRASH', text="")
        lay.prop(st, "bake_shape_keys")
        lay.prop(st, "export_animations")
        if st.export_animations and bpy.data.objects.get(CONTROL_RIG):
            sub = lay.column(align=True)
            sub.label(text="Control rig actions are baked (about 20 s)", icon='INFO')
        lay.prop(st, "deform_bones_only")
        box = lay.box()
        box.label(text="Bones left out of the skeleton:")
        box.prop(st, "exclude_names", text="Names")
        box.prop(st, "exclude_collections", text="Collections")
        lay.prop(st, "filepath", text="")
        lay.operator("export_list_glb.export", icon='EXPORT')


classes = (ELG_Item, ELG_Settings, ELG_OT_add_selected, ELG_OT_remove, ELG_OT_clear, ELG_OT_export,
           ELG_UL_items, ELG_PT_panel)


def register():
    for c in classes:
        bpy.utils.register_class(c)
    bpy.types.Scene.export_list_glb = PointerProperty(type=ELG_Settings)


def unregister():
    del bpy.types.Scene.export_list_glb
    for c in reversed(classes):
        bpy.utils.unregister_class(c)


if __name__ == "__main__":
    if WORKER_FLAG in sys.argv:
        bake_worker()
    else:
        register()
