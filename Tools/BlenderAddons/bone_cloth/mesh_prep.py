"""Step 2: mesh checks and fixes, the hidden source copy (DESIGN.md sections 6.2 and 8.2)."""
import bmesh
import bpy
import numpy as np
from mathutils import Matrix, kdtree

SOURCE_COLLECTION = "Bone Cloth Sources"


def cloth_objects(setup):
    return [r.obj for r in setup.cloth_objects if r.obj and r.obj.type == 'MESH']


def is_source_copy(obj):
    return obj is not None and "bone_cloth_source_of" in obj


def source_of(obj):
    """The hidden source copy of a cloth object, or None."""
    name = obj.get("bc_source")
    return bpy.data.objects.get(name) if name else None


def armature_modifiers(obj):
    return [m for m in obj.modifiers if m.type == 'ARMATURE']


def _is_identity(m, tol=1e-4):
    return all(abs(m[i][j] - (1.0 if i == j else 0.0)) < tol for i in range(4) for j in range(4))


def world_matrix(obj):
    """World matrix from the parent chain. Objects in an excluded collection (the source copies) are never evaluated,
    so their matrix_world cannot be trusted."""
    m = obj.matrix_basis.copy()
    if obj.parent:
        m = world_matrix(obj.parent) @ obj.matrix_parent_inverse @ m
    return m


def local_matrix(arm, obj):
    return arm.matrix_world.inverted() @ obj.matrix_world


# ---- checks ---------------------------------------------------------------------------------------------------------

def check(setup, arm):
    """List of (id, severity, message, fixable). Severity: 'ok', 'info', 'warn', 'error'."""
    items = []
    objs = cloth_objects(setup)
    if not objs:
        return [("no_objects", 'error', "No cloth mesh picked yet. Add the coat mesh in the Character step.", False)]
    for o in objs:
        tag = o.name
        if not armature_modifiers(o) or armature_modifiers(o)[0].object != arm or len(armature_modifiers(o)) > 1 \
                or o.modifiers[0].type != 'ARMATURE':
            items.append(("modifier:" + tag, 'warn', "%s: needs one Armature modifier first in the stack, pointing at %s"
                          % (tag, arm.name), True))
        elif armature_modifiers(o)[0].use_deform_preserve_volume:
            items.append(("modifier:" + tag, 'warn', "%s: Preserve Volume is on; the game skins linearly, so Blender would "
                          "show a different shape" % tag, True))
        if o.parent != arm or not _is_identity(local_matrix(arm, o)):
            items.append(("transform:" + tag, 'warn', "%s: its transform differs from the armature's (it is rotated or "
                          "scaled). Bake it into the mesh" % tag, True))
        weighted = [v for v in o.data.vertices if len(v.groups)]
        if len(weighted) < len(o.data.vertices):
            items.append(("weightless:" + tag, 'warn', "%s: %d vertices have no weights" % (tag,
                          len(o.data.vertices) - len(weighted)), True))
        bad_groups = [g.name for g in o.vertex_groups if g.name not in arm.data.bones and not g.name.startswith("BC ")]
        if bad_groups:
            items.append(("groups:" + tag, 'info', "%s: %d vertex groups have no bone (%s)" % (tag, len(bad_groups),
                          ", ".join(bad_groups[:3])), True))
        if not o.data.vertices:
            items.append(("empty:" + tag, 'error', "%s has no vertices" % tag, False))
        if o.hide_viewport or o.hide_get():
            items.append(("hidden:" + tag, 'warn', "%s is hidden, so it will not be exported" % tag, True))
        if not is_source_copy(o) and not source_of(o):
            items.append(("source:" + tag, 'info', "%s: no untouched copy kept yet (made when you press Fix all)" % tag, True))
        elif source_of(o) and len(source_of(o).data.vertices) != len(o.data.vertices):
            items.append(("sourcecount:" + tag, 'warn', "%s: the mesh changed since the setup was made (vertex count "
                          "differs from the kept copy)" % tag, True))
    if arm.data.pose_position != 'POSE':
        items.append(("pose_position", 'info', "The armature is set to show the rest position", False))
    if abs(bpy.context.scene.unit_settings.scale_length - 1.0) > 1e-6:
        items.append(("units", 'warn', "Scene unit scale is not 1; lengths shown here will not be metres", False))
    s = arm.matrix_world.to_scale()
    if max(abs(s.x - s.y), abs(s.y - s.z)) > 1e-4:
        items.append(("armature_scale", 'error', "The armature object has a non-uniform scale", False))
    return items


# ---- fixes ----------------------------------------------------------------------------------------------------------

def fix_modifier(arm, obj):
    mods = armature_modifiers(obj)
    keep = None
    for m in mods:
        if m.object == arm and keep is None:
            keep = m
        elif keep is None and m is mods[0]:
            keep = m
    if keep is None:
        keep = obj.modifiers.new("Armature", 'ARMATURE')
    for m in list(obj.modifiers):
        if m.type == 'ARMATURE' and m is not keep:
            obj.modifiers.remove(m)
    keep.object = arm
    keep.use_vertex_groups = True
    keep.use_deform_preserve_volume = False
    idx = list(obj.modifiers).index(keep)
    if idx != 0:
        obj.modifiers.move(idx, 0)


def fix_transform(arm, obj):
    m = local_matrix(arm, obj)
    me = obj.data
    if me.users > 1:
        obj.data = me = me.copy()
    me.transform(m, shape_keys=True)
    if m.determinant() < 0:
        me.flip_normals()
    obj.parent = arm
    obj.parent_type = 'OBJECT'
    obj.matrix_parent_inverse = Matrix.Identity(4)
    obj.location = (0, 0, 0)
    obj.rotation_euler = (0, 0, 0)
    obj.rotation_quaternion = (1, 0, 0, 0)
    obj.scale = (1, 1, 1)
    me.update()
    bpy.context.view_layer.update()


def fix_weightless(obj):
    """Vertices without weights take the weights of the nearest weighted vertex."""
    me = obj.data
    weighted = [v.index for v in me.vertices if len(v.groups)]
    if not weighted:
        return 0
    tree = kdtree.KDTree(len(weighted))
    for k, i in enumerate(weighted):
        tree.insert(me.vertices[i].co, k)
    tree.balance()
    fixed = 0
    todo = [v for v in me.vertices if not len(v.groups)]
    plan = []
    for v in todo:
        _, k, _ = tree.find(v.co)
        src = me.vertices[weighted[k]]
        plan.append((v.index, [(g.group, g.weight) for g in src.groups]))
    for idx, groups in plan:
        for gi, w in groups:
            obj.vertex_groups[gi].add([idx], w, 'REPLACE')
        fixed += 1
    return fixed


def normalize_weights(obj):
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    dl = bm.verts.layers.deform.verify()
    for v in bm.verts:
        d = v[dl]
        total = sum(d.values())
        if total > 0 and abs(total - 1.0) > 1e-6:
            for k in list(d.keys()):
                d[k] = d[k] / total
    bm.to_mesh(obj.data)
    bm.free()


def remove_orphan_groups(arm, obj):
    for g in list(obj.vertex_groups):
        if g.name not in arm.data.bones and not g.name.startswith("BC "):
            obj.vertex_groups.remove(g)


def unhide(obj):
    obj.hide_viewport = False
    obj.hide_set(False)


def transfer_from_body(obj, body, arm):
    """Weights from the nearest points of a body mesh (all bone groups by name), for a mesh that has none."""
    me, bme = obj.data, body.data
    bvh_verts = [body.matrix_world @ v.co for v in bme.vertices]
    from mathutils.bvhtree import BVHTree
    polys = [tuple(p.vertices) for p in bme.polygons]
    tree = BVHTree.FromPolygons(bvh_verts, polys)
    names = {vg.index: vg.name for vg in body.vertex_groups}
    for g in names.values():
        if g in arm.data.bones and g not in obj.vertex_groups:
            obj.vertex_groups.new(name=g)
    for v in me.vertices:
        p = obj.matrix_world @ v.co
        loc, _, face, _ = tree.find_nearest(p)
        if face is None:
            continue
        verts = [bme.vertices[i] for i in bme.polygons[face].vertices]
        dist = [max((bvh_verts[v2.index] - loc).length, 1e-6) for v2 in verts]
        acc = {}
        for v2, d in zip(verts, dist):
            for g in v2.groups:
                name = names.get(g.group)
                if name in obj.vertex_groups:
                    acc[name] = acc.get(name, 0.0) + g.weight / d
        total = sum(acc.values())
        for name, w in acc.items():
            if w / total > 0.001:
                obj.vertex_groups[name].add([v.index], w / total, 'REPLACE')


# ---- source copy ----------------------------------------------------------------------------------------------------

def _source_collection():
    coll = bpy.data.collections.get(SOURCE_COLLECTION)
    if coll is None:
        coll = bpy.data.collections.new(SOURCE_COLLECTION)
        bpy.context.scene.collection.children.link(coll)
    for layer in bpy.context.scene.view_layers:
        lc = _find_layer_collection(layer.layer_collection, coll.name)
        if lc:
            lc.exclude = True
    return coll


def _find_layer_collection(lc, name):
    if lc.name == name:
        return lc
    for c in lc.children:
        r = _find_layer_collection(c, name)
        if r:
            return r
    return None


def ensure_source(obj, refresh=False):
    """The hidden, untouched copy of a cloth object (original positions, shape keys and weights)."""
    existing = source_of(obj)
    if existing and not refresh:
        return existing
    if existing:
        bpy.data.objects.remove(existing, do_unlink=True)
    copy_data = obj.data.copy()
    copy_data.name = obj.name + ".bc_source"
    src = bpy.data.objects.new(obj.name + ".bc_source", copy_data)
    # the group names travel with the mesh data (Blender 3+); only add them when the copy lacks them
    if len(src.vertex_groups) != len(obj.vertex_groups):
        for vg in obj.vertex_groups:
            if vg.name not in src.vertex_groups:
                src.vertex_groups.new(name=vg.name)
    src.parent = obj.parent
    src.matrix_parent_inverse = obj.matrix_parent_inverse.copy()
    src.matrix_basis = obj.matrix_basis.copy()
    src["bone_cloth_source_of"] = obj.name
    _source_collection().objects.link(src)
    obj["bc_source"] = src.name
    bpy.context.view_layer.update()
    return src


def _source_weights(src):
    """Per source vertex: {group name: weight}, from the untouched copy."""
    names = {vg.index: vg.name for vg in src.vertex_groups}
    return [{names[g.group]: g.weight for g in v.groups if g.group in names} for v in src.data.vertices]


def _write_weights(obj, per_vertex):
    """Replaces every bone weight of obj with per_vertex[i] ({group name: weight}); groups that are not bones (such as
    'BC amount') are left alone."""
    arm_bones = None
    for m in obj.modifiers:
        if m.type == 'ARMATURE' and m.object:
            arm_bones = set(b.name for b in m.object.data.bones)
    for d in per_vertex:
        for name in d:
            if name not in obj.vertex_groups:
                obj.vertex_groups.new(name=name)
    index = {vg.name: vg.index for vg in obj.vertex_groups}
    keep = {vg.index for vg in obj.vertex_groups if arm_bones is not None and vg.name not in arm_bones}
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    dl = bm.verts.layers.deform.verify()
    for v in bm.verts:
        d = v[dl]
        kept = {k: d[k] for k in list(d.keys()) if k in keep}
        d.clear()
        for name, w in per_vertex[v.index].items():
            d[index[name]] = w
        for k, w in kept.items():
            d[k] = w
    bm.to_mesh(obj.data)
    bm.free()
    obj.data.update()


def restore_weights_from_source(obj):
    """Puts the original body weights back (same vertex count only)."""
    src = source_of(obj)
    if not src or len(src.data.vertices) != len(obj.data.vertices):
        return False
    _write_weights(obj, _source_weights(src))
    return True


def transfer_weights_from_source(obj, tolerance=0.02):
    """For a mesh that was edited after the copy was made (vertices merged or removed): every vertex takes the original
    weights of the nearest vertex of the copy. Raises when a vertex has no copy vertex within `tolerance` (armature units
    are not assumed: the distance is measured in the mesh's own space, relative to its size)."""
    src = source_of(obj)
    if not src:
        raise RuntimeError("There is no kept copy of %s to take the weights from" % obj.name)
    sv = src.data.vertices
    tree = kdtree.KDTree(len(sv))
    for i, v in enumerate(sv):
        tree.insert(v.co, i)
    tree.balance()
    size = max((max(c) - min(c)) for c in zip(*[tuple(v.co) for v in sv]))
    per_src = _source_weights(src)
    out, worst = [], 0.0
    for v in obj.data.vertices:
        _, i, dist = tree.find(v.co)
        worst = max(worst, dist)
        out.append(per_src[i])
    if worst > tolerance * size:
        raise RuntimeError("%s was changed too much since the setup (a vertex moved %.1f%% of the mesh size); use Start over"
                           % (obj.name, 100.0 * worst / size))
    _write_weights(obj, out)
    return worst


def forget_source(obj):
    """Throws the hidden untouched copy away (and the link to it): the mesh is taken as it is now, the next setup makes a
    new copy of it."""
    src = source_of(obj)
    if src is not None:
        bpy.data.objects.remove(src, do_unlink=True)
    if "bc_source" in obj:
        del obj["bc_source"]


def restore_from_source(obj):
    """Puts the cloth object's geometry and weights back from its source copy."""
    src = source_of(obj)
    if not src or len(src.data.vertices) != len(obj.data.vertices):
        return False
    n = len(obj.data.vertices)
    buf = np.empty(n * 3, dtype=np.float32)
    src.data.vertices.foreach_get('co', buf)
    obj.data.vertices.foreach_set('co', buf)
    if obj.data.shape_keys and src.data.shape_keys:
        for kb in obj.data.shape_keys.key_blocks:
            sk = src.data.shape_keys.key_blocks.get(kb.name)
            if sk:
                b = np.empty(n * 3, dtype=np.float32)
                sk.data.foreach_get('co', b)
                kb.data.foreach_set('co', b)
    obj.data.update()
    return True


# ---- run -------------------------------------------------------------------------------------------------------------

def fix(setup, arm, fix_id, body=None):
    kind, _, name = fix_id.partition(":")
    obj = bpy.data.objects.get(name)
    if obj is None:
        return
    if kind == "modifier":
        fix_modifier(arm, obj)
    elif kind == "transform":
        fix_transform(arm, obj)
    elif kind == "weightless":
        if not any(len(v.groups) for v in obj.data.vertices) and body is not None:
            transfer_from_body(obj, body, arm)
        fix_weightless(obj)
        normalize_weights(obj)
    elif kind == "groups":
        remove_orphan_groups(arm, obj)
    elif kind == "hidden":
        unhide(obj)
    elif kind == "source":
        ensure_source(obj)
    elif kind == "sourcecount":
        # the mesh was edited after the setup was made: carry the original weights over, then keep the edited mesh
        transfer_weights_from_source(obj)
        ensure_source(obj, refresh=True)


def fix_all(setup, arm, body=None):
    done = []
    for item in check(setup, arm):
        if item[3] and not item[0].startswith(("sourcecount",)):
            fix(setup, arm, item[0], body)
            done.append(item[0])
    # the order matters: the transform is baked before the copy is made
    for o in cloth_objects(setup):
        ensure_source(o)
    return done
