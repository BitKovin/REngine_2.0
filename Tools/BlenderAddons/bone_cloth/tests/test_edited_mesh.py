"""Start over restores the original weights; a mesh edited after the setup (vertices merged) can be carried over.
Run on a saved jacket scene: BC_TEST_BLEND=<jacket_state.blend> BC_NO_PREPARE=1"""
import sys, os
import bpy, bmesh
import numpy as np
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import props, presets, pipeline, doctor, mesh_prep, bones
bone_cloth.register()
arm = bpy.data.objects["root"]
bpy.context.view_layer.objects.active = arm
s = props.get_setup(arm)
obj = mesh_prep.cloth_objects(s)[0]
src = mesh_prep.source_of(obj)

def stats(o):
    return len(o.data.vertices), sum(1 for v in o.data.vertices if len(v.groups))

print("source (verts, weighted):", stats(src))
print("before start over:", stats(obj))
bpy.ops.bone_cloth.start_over()
print("after start over :", stats(obj), "(must equal the source)")
a = {i: {src.vertex_groups[g.group].name: round(g.weight, 4) for g in v.groups} for i, v in enumerate(src.data.vertices)}
b = {i: {obj.vertex_groups[g.group].name: round(g.weight, 4) for g in v.groups} for i, v in enumerate(obj.data.vertices)}
print("weights identical to the source:", a == b)

# edit the mesh: merge vertices by distance
bm = bmesh.new(); bm.from_mesh(obj.data)
bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=0.5)
bm.to_mesh(obj.data); bm.free()
print("after merging:", stats(obj))
presets.apply(s, "JACKET")
try:
    pipeline.auto_setup(s, arm, log=lambda m: None)
    print("auto setup on edited mesh: NO ERROR (should have been refused)")
except RuntimeError as e:
    print("refused as expected:", str(e)[:110])
iss = doctor.run(s, arm)
print("issues:", [(i.id, i.fix) for i in iss if i.severity != "info"])
mesh_prep.fix(s, arm, "sourcecount:" + obj.name)
print("after fix:", stats(obj), "source", stats(mesh_prep.source_of(obj)))
iss = pipeline.auto_setup(s, arm, log=lambda m: print("  log:", m))
print("issues after auto setup:", [(i.severity, i.id) for i in iss if i.severity != "info"])
print("weightless:", sum(1 for v in obj.data.vertices if not len(v.groups)), "cloth groups:", len([g for g in obj.vertex_groups if g.name in bones.generated(s)]))
