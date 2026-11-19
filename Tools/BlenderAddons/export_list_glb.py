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


# --------------------------------------------------------------------------------------------- export
def do_export(context, st, report):
    objs = list(dict.fromkeys(i.obj for i in st.items if i.obj))
    if not objs:
        raise RuntimeError("Export list is empty")
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
    register()
