"""Start over asks what Set up cloth asked (name, what it is, which mesh is the cloth) and sets up again: the bones and generated
clips go, the old mesh is back as it was (and, when another one is picked, forgotten), a fresh setup with the chosen preset
and mesh replaces the old one, hand-made clips move to it.
BC_NO_PREPARE=1 python tests/run_headless.py test_start_over.py <copy of a jacket scene>"""
import sys
import bpy
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import props, poses, bones as bones_mod, mesh_prep
bone_cloth.register()

arm = bpy.data.objects["root"]
s = props.get_setup(arm)
bpy.context.view_layer.objects.active = arm
old = mesh_prep.cloth_objects(s)[0]
old_id = s.id
assert len(bones_mod.generated(s)) > 0 and len(s.cloth_objects) == 1
bpy.ops.mesh.primitive_cube_add()
new = bpy.context.active_object
new.name = "other_jacket"
bpy.context.view_layer.objects.active = arm
hand = poses.clip_slot(s, "turnLeft").action
hand["bc_clip_sig"] = "edited by hand"                         # no longer what the generator wrote
assert poses.clip_is_hand(hand)
slots = bone_cloth.ops.ec.CLIP_SLOTS
n_clips = sum(poses.clip_slot(s, k).action is not None for k in slots)

# the same mesh, another preset
assert bpy.ops.bone_cloth.start_over(name="coat2", preset='SKIRT', mesh=old.name) == {'FINISHED'}
s = props.get_setup(arm)
print("same mesh: setups", len(arm.bone_cloth.setups), "name", s.name, "preset", s.preset, "bones", len(bones_mod.generated(s)),
      "cloth", [r.obj.name for r in s.cloth_objects])
assert len(arm.bone_cloth.setups) == 1 and s.id != old_id and s.name == "coat2" and s.preset == 'SKIRT'
assert [r.obj for r in s.cloth_objects] == [old] and len(bones_mod.generated(s)) == 0

# another mesh
assert bpy.ops.bone_cloth.start_over(name="jacket2", preset='JACKET', mesh="other_jacket") == {'FINISHED'}
s = props.get_setup(arm)
print("other mesh: cloth", [r.obj.name for r in s.cloth_objects], "preset", s.preset, "hidden copy of the old:", mesh_prep.source_of(old))
assert [r.obj for r in s.cloth_objects] == [new] and s.preset == 'JACKET' and s.name == "jacket2"
assert mesh_prep.source_of(old) is None and "bc_source" not in old
left = [k for k in slots if poses.clip_slot(s, k).action is not None]
print("clips left:", left, "of", n_clips)
assert left == ["turnLeft"]

# a mesh with a kept copy from an earlier setup that no longer matches it (edited since: other vertex count)
import bmesh
bpy.ops.mesh.primitive_cube_add()
stale = bpy.context.active_object
stale.name = "stale_jacket"
mesh_prep.ensure_source(stale)
bm = bmesh.new()
bm.from_mesh(stale.data)
bmesh.ops.subdivide_edges(bm, edges=bm.edges[:], cuts=1, use_grid_fill=True)
bm.to_mesh(stale.data)
bm.free()
assert mesh_prep.source_of(stale) is not None and len(mesh_prep.source_of(stale).data.vertices) != len(stale.data.vertices)
bpy.context.view_layer.objects.active = arm
assert bpy.ops.bone_cloth.start_over(name="stale", preset='JACKET', mesh="stale_jacket") == {'FINISHED'}
s = props.get_setup(arm)
print("stale copy:", mesh_prep.source_of(stale), [i[0] for i in mesh_prep.check(s, arm) if i[0].startswith("sourcecount")])
assert mesh_prep.source_of(stale) is None
assert not [i for i in mesh_prep.check(s, arm) if i[0].startswith("sourcecount")]
print("ALL OK")
