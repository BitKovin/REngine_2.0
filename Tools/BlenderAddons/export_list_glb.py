bl_info = {
    "name": "Export List GLB",
    "author": "",
    "version": (1, 0, 0),
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
                    "exported skeleton, e.g.  ctrl_*, IK_*, root_ctrl")
    exclude_collections: StringProperty(
        name="Exclude Bone Collections", default="",
        description="Bone collection names, separated by commas. Every bone in them is left out")
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
    returns {removed bone name: nearest kept ancestor name or None}"""
    heir = {}
    bones = arm_obj.data.bones
    for n in remove:
        p = bones[n].parent
        while p and p.name in remove:
            p = p.parent
        heir[n] = p.name if p else None
    bpy.context.view_layer.objects.active = arm_obj
    bpy.ops.object.mode_set(mode='EDIT')
    eb = arm_obj.data.edit_bones
    for n in remove:                                       # reparent first, so removal never orphans a kept bone
        for c in list(eb[n].children):
            if c.name not in remove:
                c.parent = eb[heir[n]] if heir[n] else None
    for n in remove:
        eb.remove(eb[n])
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


# --------------------------------------------------------------------------------------------- export
def do_export(context, st, report):
    objs = [i.obj for i in st.items if i.obj]
    if not objs:
        raise RuntimeError("Export list is empty")
    names, cols = _split(st.exclude_names), _split(st.exclude_collections)
    temp_objs, temp_meshes, temp_arms = [], [], []
    mapping = {}
    prev_sel = [o for o in context.view_layer.objects if o.select_get()]
    prev_active = context.view_layer.objects.active
    prev_mode = context.mode
    if prev_mode != 'OBJECT':
        bpy.ops.object.mode_set(mode='OBJECT')
    try:
        # 1. private copies of everything in the list
        for o in objs:
            c = o.copy()
            if o.data is not None and o.type in {'MESH', 'ARMATURE', 'CURVE'}:
                c.data = o.data.copy()
                (temp_meshes if o.type == 'MESH' else temp_arms).append(c.data)
            if o.type == 'MESH' and c.data.shape_keys is None:
                pass
            context.scene.collection.objects.link(c)
            c.matrix_world = o.matrix_world
            temp_objs.append(c)
            mapping[o] = c
        # 2. keep parenting / armature modifiers inside the copied set
        for o, c in mapping.items():
            if o.parent in mapping:
                world = c.matrix_world.copy()
                c.parent = mapping[o.parent]
                c.matrix_world = world
            for m in c.modifiers:
                if m.type == 'ARMATURE' and m.object in mapping:
                    m.object = mapping[m.object]
        # 3. bake shape keys
        if st.bake_shape_keys:
            for c in temp_objs:
                if c.type == 'MESH':
                    for o_ in context.view_layer.objects:
                        o_.select_set(False)
                    c.select_set(True)
                    context.view_layer.objects.active = c
                    bake_shape_keys(c)
        # 4. exclude bones
        removed_total = 0
        for o, c in mapping.items():
            if o.type != 'ARMATURE':
                continue
            remove = excluded_bones(c.data, names, cols)
            if not remove:
                continue
            heir = strip_bones(c, remove)
            removed_total += len(remove)
            for m in temp_objs:
                if m.type != 'MESH':
                    continue
                if any(md.type == 'ARMATURE' and md.object is c for md in m.modifiers):
                    merge_groups(m, heir)
        # 5. export only the copies
        for o_ in context.view_layer.objects:
            o_.select_set(False)
        for c in temp_objs:
            c.select_set(True)
        context.view_layer.objects.active = temp_objs[0]
        path = bpy.path.abspath(st.filepath)
        if not path.lower().endswith(".glb"):
            path += ".glb"
        bpy.ops.export_scene.gltf(filepath=path, export_format='GLB', use_selection=True,
                                  export_animations=st.export_animations, export_apply=False)
        report({'INFO'}, f"Exported {len(temp_objs)} objects to {path} ({removed_total} bones excluded)")
        return path
    finally:
        for c in temp_objs:
            bpy.data.objects.remove(c, do_unlink=True)
        for d in temp_meshes:
            bpy.data.meshes.remove(d)
        for d in temp_arms:
            bpy.data.armatures.remove(d)
        for o_ in context.view_layer.objects:
            o_.select_set(False)
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
