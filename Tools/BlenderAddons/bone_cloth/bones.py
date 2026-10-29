"""Step 3: creates the root and strip bones from a plan (DESIGN.md section 8.3.5)."""
import json
from contextlib import contextmanager

import bpy
from mathutils import Vector

from . import frames

COLLECTION_STRIPS = "Cloth"
COLLECTION_ROOTS = "Cloth Roots"


@contextmanager
def mode(arm, target):
    """Switches the armature into a mode and back."""
    view = bpy.context.view_layer
    prev_active = view.objects.active
    prev_mode = prev_active.mode if prev_active else 'OBJECT'
    if prev_active and prev_active != arm and prev_mode != 'OBJECT':
        bpy.ops.object.mode_set(mode='OBJECT')
    view.objects.active = arm
    if arm.mode != target:
        bpy.ops.object.mode_set(mode=target)
    try:
        yield
    finally:
        if arm.mode != 'OBJECT':
            bpy.ops.object.mode_set(mode='OBJECT')
        if prev_active and prev_active != arm:
            view.objects.active = prev_active
            if prev_mode != 'OBJECT':
                bpy.ops.object.mode_set(mode=prev_mode)


def generated(setup):
    return json.loads(setup.generated_bones or "[]")


def roots(setup):
    return json.loads(setup.root_bones or "[]")


def remove_generated(setup, arm):
    """Deletes the bones this setup made, their vertex groups on every mesh, and nothing else. A rig that was adopted is
    not the add-on's to delete: the setup just forgets it."""
    if setup.adopted:
        setup.generated_bones = setup.root_bones = setup.strips_json = "[]"
        setup.adopted = False
        return
    names = generated(setup) + roots(setup)
    names = [n for n in names if n in arm.data.bones]
    if names:
        with mode(arm, 'EDIT'):
            eb = arm.data.edit_bones
            for n in names:
                if n in eb:
                    eb.remove(eb[n])
        for o in bpy.data.objects:
            if o.type == 'MESH':
                for n in names:
                    vg = o.vertex_groups.get(n)
                    if vg:
                        o.vertex_groups.remove(vg)
    setup.generated_bones = "[]"
    setup.root_bones = "[]"
    setup.strips_json = "[]"


def _collection(arm, name):
    coll = arm.data.collections_all.get(name)
    return coll or arm.data.collections.new(name)


def build(setup, arm, frame, plan):
    """Creates the bones of `plan`. Existing generated bones are removed first."""
    remove_generated(setup, arm)
    inv3 = arm.matrix_world.to_3x3().inverted()
    created_roots, created = [], []
    with mode(arm, 'EDIT'):
        eb = arm.data.edit_bones
        for pb in plan.roots + [b for _, _, bones in plan.strips for b in bones]:
            if pb.name in eb:
                eb.remove(eb[pb.name])
        for pb in plan.roots:
            b = eb.new(pb.name)
            b.head = frames.world_to_arm(arm, pb.head)
            b.tail = frames.world_to_arm(arm, pb.tail)
            if pb.parent in eb:
                b.parent = eb[pb.parent]
            created_roots.append(pb.name)
        for _, _, bones in plan.strips:
            for pb in bones:
                if (pb.tail - pb.head).dot(pb.tail - pb.head) < 1e-8:
                    continue
                b = eb.new(pb.name)
                b.head = frames.world_to_arm(arm, pb.head)
                b.tail = frames.world_to_arm(arm, pb.tail)
                b.parent = eb[pb.parent] if pb.parent in eb else None
                if pb.connected:
                    b.use_connect = True
                outward = inv3 @ Vector(frame.radial(pb.theta))
                b.align_roll(outward)
                created.append(pb.name)

    strips_c, roots_c = _collection(arm, COLLECTION_STRIPS), _collection(arm, COLLECTION_ROOTS)
    for n in created_roots:
        roots_c.assign(arm.data.bones[n])
        arm.pose.bones[n]["bone_cloth"] = setup.id
    for n in created:
        strips_c.assign(arm.data.bones[n])
        arm.pose.bones[n]["bone_cloth"] = setup.id
    for upper, lower in plan.joined:
        if upper in arm.pose.bones and lower in arm.pose.bones:
            c = arm.pose.bones[lower].constraints.new('COPY_LOCATION')
            c.name = "BC Follow"
            c.target = arm
            c.subtarget = upper
            c.head_tail = 1.0
            c.target_space = c.owner_space = 'WORLD'
    setup.generated_bones = json.dumps(created)
    setup.root_bones = json.dumps(created_roots)
    setup.strips_json = json.dumps([
        {"column": col, "side": side, "theta": bones[0].theta, "bones": [b.name for b in bones if b.name in created],
         "strand": col == "strand"}
        for col, side, bones in plan.strips])
    return created_roots, created


def strips(setup, arm):
    """The generated strips: dicts with 'bones' (top first, upper part included), 'theta' (radians), 'column', 'side'."""
    out = []
    for s in json.loads(setup.strips_json or "[]"):
        bones = [b for b in s["bones"] if b in arm.data.bones]
        if bones:
            out.append(dict(s, bones=bones))
    return out
