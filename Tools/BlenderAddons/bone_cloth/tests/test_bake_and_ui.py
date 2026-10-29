import sys, math
import bpy
import numpy as np
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import props, presets, pipeline, rest_shape, bones, mesh_prep, frames, ui
bone_cloth.register()

arm = bpy.data.objects['root']
bpy.context.view_layer.objects.active = arm
coat = bpy.data.objects['coat']

# a shape key on the coat before anything happens
coat.shape_key_add(name="Basis")
sk = coat.shape_key_add(name="Puff")
for i, d in enumerate(sk.data):
    d.co = d.co + __import__("mathutils").Vector((0, 0, 0.5 if i % 50 == 0 else 0.0))
sk.value = 0.0

setup = props.new_setup(arm, "coat")
setup.cloth_objects.add().obj = coat
presets.apply(setup, 'LONG_COAT')
setup.rest.flare = 0.12          # a visible rest flare
setup.rest.clear_legs = False
pipeline.auto_setup(setup, arm, log=lambda m: None)
print("rest applied:", setup.rest.applied)

# 1. T5: baked result == what the armature modifier showed with the rest rotations posed (before the bake)
def evaluated(obj):
    dg = bpy.context.evaluated_depsgraph_get()
    ev = obj.evaluated_get(dg)
    me = ev.to_mesh()
    n = len(me.vertices)
    buf = np.empty(n * 3, dtype=np.float32)
    me.vertices.foreach_get('co', buf)
    ev.to_mesh_clear()
    return buf.reshape(n, 3)

after_apply = evaluated(coat)
src = mesh_prep.source_of(coat)
# reconstruct "expected": reset, pose, evaluate
rest_shape.reset(setup, arm)
bases, _ = rest_shape.solve(setup, arm, pipeline.frame_of(setup, arm))
from bone_cloth import poses
poses.set_bases(arm, bases)
bpy.context.view_layer.update()
expected = evaluated(coat)
poses.set_bases(arm, {})
bpy.context.view_layer.update()
print("max |baked - posed|:", float(np.abs(after_apply - expected).max()), "(units are cm; mesh size ~150)")

# 2. shape key delta survives (re-apply and compare the key's relative offset on a displaced vertex)
rest_shape.apply(setup, arm, pipeline.frame_of(setup, arm))
b = np.empty(len(coat.data.vertices) * 3, dtype=np.float32)
coat.data.shape_keys.key_blocks["Basis"].data.foreach_get('co', b)
base = b.reshape(-1, 3).copy()
coat.data.shape_keys.key_blocks["Puff"].data.foreach_get('co', b)
puff = b.reshape(-1, 3)
delta = (puff - base)[::50]
print("puff delta lengths (expect ~0.5 scaled by local skinning):", np.round(np.linalg.norm(delta, axis=1)[:6], 3))

# 3. reset restores the exact original geometry
rest_shape.reset(setup, arm)
orig = np.empty(len(src.data.vertices) * 3, dtype=np.float32)
src.data.vertices.foreach_get('co', orig)
cur = np.empty_like(orig)
coat.data.vertices.foreach_get('co', cur)
print("reset == source:", float(np.abs(orig - cur).max()))

# 4. every panel draws without touching a missing property or operator
class Mock:
    def __init__(self, log): self.log = log; self.scale_y = 1; self.enabled = True; self.alert = False
    def __getattr__(self, name):
        def call(*a, **k):
            self.log.append((name, a, k))
            return Mock(self.log) if name in ("row", "column", "box", "split", "grid_flow") else Mock(self.log)
        return call
log = []
errors = []
bpy.context.view_layer.objects.active = arm
class FakePanel:
    pass
for cls in ui.CLASSES:
    inst = FakePanel()
    inst.layout = Mock(log)
    inst.stage_key = getattr(cls, "stage_key", "")
    log.clear()
    try:
        cls.draw(inst, bpy.context)
        if hasattr(cls, "draw_header"):
            cls.draw_header(inst, bpy.context)
    except Exception as e:
        import traceback
        errors.append((cls.__name__, traceback.format_exc()))
    for name, a, k in log:
        if name == "prop" and len(a) >= 2 and hasattr(a[0], "bl_rna"):
            if a[1] not in a[0].bl_rna.properties:
                errors.append((cls.__name__, "missing property %s on %s" % (a[1], a[0].bl_rna.identifier)))
        if name == "prop_search" and len(a) >= 2 and hasattr(a[0], "bl_rna"):
            if a[1] not in a[0].bl_rna.properties:
                errors.append((cls.__name__, "missing property %s" % a[1]))
        if name == "operator":
            idn = a[0]
            mod, _, op = idn.partition(".")
            if not hasattr(getattr(bpy.ops, mod), op):
                errors.append((cls.__name__, "missing operator " + idn))
print("panels checked:", len(ui.CLASSES), "errors:", len(errors))
for e in errors:
    print(e)
