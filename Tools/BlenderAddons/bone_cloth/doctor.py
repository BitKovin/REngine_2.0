"""Step 10a: checks that apply the engine's own rules to the rig (DESIGN.md section 8.10)."""
import json
import math

import bpy
import numpy as np

from . import bones as bones_mod
from . import colliders, frames, mesh_prep, model_data, poses, stages
from . import engine_contract as ec

LAST = {}     # setup id -> list of Issue
POSE_DEPTH = 0.01       # cloth that rests outside the capsules and a clip pose takes this far in is reported
POSE_STRETCH = 1.3      # ... and cloth a clip pose stretches this much


class Issue:
    def __init__(self, id, severity, message, fix=None, bones=()):
        self.id, self.severity, self.message, self.fix, self.bones = id, severity, message, fix, list(bones)


def engine_strips(setup, arm, frame, data=None):
    """What the engine would build from the current rig: (skeleton, vertices, strips)."""
    skel, verts = data if data is not None else model_data.collect(arm)
    root_ids = [skel.index[r] for r in bones_mod.roots(setup) if r in skel.index]
    strips = ec.build_strips(skel, verts, root_ids, frame.flat_forward(), np.array([0.0, 0.0, 1.0]),
                             setup.motion.tipLength)
    return skel, verts, strips


def run(setup, arm):
    issues = []
    add = issues.append
    frame = frames.compute_frame(arm, setup.character)
    generated = bones_mod.generated(setup)

    # ---- the mesh and its modifiers ----
    for item in mesh_prep.check(setup, arm):
        kind = item[0].split(":")[0]
        sev = {'ok': 'info', 'info': 'info', 'warn': 'warning', 'error': 'error'}[item[1]]
        if kind in ("modifier", "transform", "weightless", "hidden", "groups", "no_objects", "units", "armature_scale",
                    "sourcecount"):
            ident = {"modifier": "PRESERVE-VOLUME", "transform": "MESH-TRANSFORM", "weightless": "WEIGHTLESS",
                     "hidden": "HIDDEN-CLOTH", "groups": "EMPTY-GROUPS", "no_objects": "NO-CLOTH", "units": "UNITS",
                     "armature_scale": "UNITS", "sourcecount": "SOURCE-CHANGED"}[kind]
            if kind == "hidden":
                sev = "error"
            add(Issue(ident, sev, item[2], ("mesh:" + item[0]) if item[3] else None))

    if not generated:
        add(Issue("NO-BONES", "error", "No panel bones yet. Open the Shape step and press Build bones."))
        LAST[setup.id] = issues
        return issues

    missing = [b for b in generated + bones_mod.roots(setup) if b not in arm.data.bones]
    if missing:
        add(Issue("ROOT-MISSING", "error", "Bones the setup made are gone: %s. Press Build bones." % ", ".join(missing[:3]),
                  bones=missing))

    # ---- a hand-made pose belongs to the bones it was made for ----
    for kind in ("open", "volume"):
        layer = getattr(setup.poses, kind)
        if layer.enabled and poses.layout_changed(arm, layer):
            add(Issue("HAND-POSE-BONES", "error", "The %s pose was made for different bones than these (they were rebuilt "
                      "since), so it would look wrong. Put the bone settings back, or pose again and save it." % kind))

    # ---- name clashes ----
    for b in generated + bones_mod.roots(setup):
        if b in bpy.data.objects:
            add(Issue("NAME-CLASH", "error", "An object is named like the bone '%s'; the game looks both up by name" % b))

    # ---- the engine's view of the strips ----
    try:
        data = model_data.collect(arm)
        skel, verts, strips = engine_strips(setup, arm, frame, data)
    except ValueError as e:
        add(Issue("NO-STRIPS", "error", "The game finds no panels: %s. Rebuild the weights." % e, "weights"))
        LAST[setup.id] = issues
        return issues

    used = set()
    for s in strips:
        used.update(skel.names[j.bone] for j in s.joints)
    cut = [b for b in generated if b not in used]
    if cut and all("_00" in b for b in cut) and setup.guides.upper_part and setup.guides.anchors == 'CHEST_AND_HIPS':
        add(Issue("STRIP-EMPTY", "error", "The chest part (%d bones) has no cloth to move on this mesh, so the game would "
                  "skip it. Leave it out." % len(cut), "drop_upper", cut))
    elif cut:
        add(Issue("STRIP-EMPTY", "error", "The game will cut panels short: %d bones carry no vertices (%s). The mesh may be "
                  "too sparse for %d panels per side." % (len(cut), ", ".join(cut[:3]), setup.guides.columns_per_side),
                  "fewer_panels" if setup.guides.columns_per_side > 1 else "weights", cut))
    expected = len(bones_mod.strips(setup, arm))
    if len(strips) != expected:
        add(Issue("STRIP-COUNT", "warning", "The game sees %d panels, the setup made %d" % (len(strips), expected)))

    chains = [[j.bone for j in s.joints] for s in strips]
    for a, b, ratio in ec.join_candidates(skel, chains):
        add(Issue("JOIN-NEAR-MISS", "warning", "%s points near %s (%.0f%% off) but will not join: the game needs under 5%%"
                  % (skel.names[chains[a][-1]], skel.names[chains[b][0]], ratio * 100)))

    weak = 0
    for s in strips:
        last = s.joints[-1].bone
        sel = (verts.bones == last) & (verts.weights >= ec.TIP_WEIGHT)
        if not sel.any():
            weak += 1
            add(Issue("TIP-SHORT", "warning", "%s has no vertex weighted 25%% or more, so the game ends the panel at the "
                      "previous bone's length" % skel.names[last], "weights", [skel.names[last]]))
    if verts.position.shape[0] and strips:
        centre = np.mean([skel.head(s.joints[0].bone) for s in strips], axis=0)
        _, left = ec.side_axes(frame.flat_forward(), np.array([0.0, 0.0, 1.0]))
        for s in strips:
            d = abs(float(np.dot(skel.head(s.joints[0].bone) - centre, left)))
            if 0.01 < d < 0.035 and s.side != 0:
                add(Issue("SIDE-AMBIGUOUS", "info", "%s is only %.0f mm from the middle; the game treats under 20 mm as "
                          "centre" % (skel.names[s.joints[0].bone], d * 1000)))

    # ---- weights ----
    raw = 0
    for o in mesh_prep.cloth_objects(setup):
        for v in o.data.vertices:
            if sum(1 for g in v.groups if g.weight > 0) > ec.MAX_BONE_INFLUENCE:
                raw += 1
    if raw:
        add(Issue("INFLUENCES", "warning", "%d vertices have more than 4 bones; the exporter drops the smallest" % raw,
                  "weights"))

    # ---- skeleton size ----
    count = len(arm.data.bones)
    if setup.export.deform_bones_only:
        count = sum(1 for b in arm.data.bones if b.use_deform)
    if count > ec.MAX_SKINNED_BONES:
        add(Issue("JOINTS-256", "error", "%d bones will be exported; the game allows %d" % (count, ec.MAX_SKINNED_BONES)))

    # ---- wind clips ----
    clips = poses.uses_clips(setup)
    for kind in ("open", "volume"):
        layer = getattr(setup.poses, kind)
        if not layer.enabled or clips:
            continue
        if layer.action is None:
            add(Issue("CLIP-MISSING", "warning", "The %s pose has not been made. Open Wind poses and press Update." % kind,
                      "poses"))
            continue
        q = poses.read_action(layer.action)
        angles = {skel.index[b]: v.angle for b, v in q.items() if b in skel.index}
        if not any(ec.layer_uses_strip(s, angles) for s in strips):
            add(Issue("CLIP-FLAT", "warning", "No panel turns in the %s pose, so the game leaves that layer off." % kind,
                      "poses"))

    if clips:
        cloth = set(used)
        for key in ec.CLIP_SLOTS:
            act = poses.clip_slot(setup, key).action
            if act is None:
                continue
            last = poses.clip_samples(act)[-1]
            if not any(b in cloth and (q.angle >= ec.LAYER_MIN_ANGLE or loc.length > 1e-4) for b, (q, loc) in last.items()):
                add(Issue("CLIP-FLAT", "warning", "The %s clip ends where it starts: its last frame is the rest shape, "
                          "so it does nothing in the game" % poses.CLIP_TITLES[key].lower()))
        for key, source in poses.MIRROR_OF.items():
            if poses.clip_slot(setup, key).action is None and poses.clip_slot(setup, source).action is not None:
                add(Issue("CLIP-NO-MIRROR", "info", "%s has no clip: the cloth keeps its rest shape there. Mirror it "
                          "from %s." % (poses.CLIP_TITLES[key], poses.CLIP_TITLES[source].lower())))
        if not sim_running(arm):
            for label, depth, depth_bone, stretch, stretch_bone in clip_body_check(setup, arm, cloth):
                if depth > POSE_DEPTH:
                    add(Issue("POSE-IN-BODY", "warning", "%s: the cloth goes %.1f cm into a collider near %s. The game "
                              "pushes it out, so it will not look as posed." % (label, depth * 100, depth_bone),
                              bones=[depth_bone]))
                if stretch > POSE_STRETCH:
                    add(Issue("POSE-STRETCH", "warning", "%s: the cloth stretches to %.1fx near %s (skinning cannot slide "
                              "one panel over another; turn the panels less apart there)" % (label, stretch, stretch_bone),
                              bones=[stretch_bone]))

    for kind in ("open", "volume"):
        layer = getattr(setup.poses, kind)
        if clips:
            break
        if layer.enabled and poses.layer_is_hand(layer) and not poses.layout_changed(arm, layer):
            add(Issue("HAND-POSE", "info", "The %s pose is hand made: Update keeps it as it is (Back to generated in Wind "
                      "poses throws it away)" % kind))

    # ---- colliders ----
    for c in setup.colliders:
        if c.bone not in arm.data.bones or c.to not in arm.data.bones:
            add(Issue("COLLIDER-BONE", "error", "Collision %s - %s names a bone the model lacks" % (c.bone, c.to)))
            continue
        if (c.auto_radius or c.auto_end):
            anc = arm.data.bones[c.to].parent_recursive
            if arm.data.bones[c.bone] not in anc:
                add(Issue("COLLIDER-NOT-DESCENDANT", "warning", "Collision %s - %s: '%s' is not below '%s', so the game uses "
                          "0.06 m" % (c.bone, c.to, c.to, c.bone)))
        r0, r1 = colliders.radii(c)
        if min(r0, r1) < 0.02 or max(r0, r1) > 0.30:
            add(Issue("COLLIDER-ODD-RADIUS", "warning", "Collision %s - %s has a radius of %.1f / %.1f cm; check it"
                      % (c.bone, c.to, r0 * 100, r1 * 100)))

    # ---- rest shape against the legs ----
    caps = colliders.capsules(setup, arm, surface=False)
    if caps:
        pad = setup.motion.radius
        for info in bones_mod.strips(setup, arm):
            rig = poses.StripRig(arm, info)
            depth = max(colliders.penetration(t, caps, pad) for t in rig.tails[len(rig.tails) // 2:])
            if depth > 0.0:
                add(Issue("REST-IN-LEGS", "warning", "%s rests %.1f cm inside the leg capsule; the game pushes it out every "
                          "frame. Use 'Keep clear of the legs' in Rest shape." % (info["bones"][-1], depth * 100), "rest"))
                break

    # ---- leftovers from older rigs ----
    legacy = [pb.name for pb in arm.pose.bones if pb.name in generated for c in pb.constraints if c.name != "BC Follow"]
    drivers = 0
    if arm.animation_data:
        drivers = sum(1 for d in arm.animation_data.drivers if any(('"%s"' % b) in d.data_path for b in generated))
    if legacy or drivers:
        add(Issue("LEGACY-RIG", "info", "Constraints or drivers on the cloth bones (%d) only act inside Blender; the game "
                  "ignores them" % (len(legacy) + drivers)))
    other_keys = 0
    ours = {layer.action.name for layer in (setup.poses.open, setup.poses.volume) if layer.action}
    gen_set = set(generated) | set(bones_mod.roots(setup))
    for act in bpy.data.actions:
        if act.name in ours:
            continue
        for lyr in act.layers:
            for st in lyr.strips:
                for cb in st.channelbags:
                    if any(fc.data_path.startswith('pose.bones["') and fc.data_path.split('"')[1] in gen_set
                           for fc in cb.fcurves):
                        other_keys += 1
                        break
    if other_keys:
        add(Issue("CLOTH-KEYS", "info", "%d other clips key the cloth bones; the game ignores those keys (the cloth hangs "
                  "from the body)" % other_keys, "cloth_keys"))

    # ---- motion values ----
    m = setup.motion
    if m.openWindStart >= m.openWindFull or m.volumeWindStart >= m.volumeWindFull:
        add(Issue("PARAMS", "info", "A wind range starts at or above its full value, so it acts as a step"))
    if m.followTip > m.followRoot:
        add(Issue("PARAMS", "info", "The hem catches up faster than the top; that is unusual"))
    if m.spinStart >= m.spinFull:
        add(Issue("PARAMS", "info", "Spin starts at or above its full value, so it acts as a step"))

    # ---- stale steps ----
    for k in stages.stale_list(setup, arm):
        add(Issue("STALE", "warning", "'%s' is out of date after a change above it. Press Update all." % stages.TITLES[k],
                  "update"))
    LAST[setup.id] = issues
    return issues


def sim_running(arm):
    from . import sim
    return arm.name in sim.RUNNING


def _posed_capsules(setup, arm):
    """[(start, end, r0, r1)] of every enabled collider on the body as it is posed now."""
    out = []
    for c in setup.colliders:
        if c.enabled and c.bone in arm.pose.bones and c.to in arm.pose.bones:
            off = np.array(c.offset)
            r0, r1 = colliders.radii(c)
            out.append((frames.v3(arm.matrix_world @ arm.pose.bones[c.bone].head) + off,
                        frames.v3(arm.matrix_world @ arm.pose.bones[c.to].head) + off, r0, r1))
    return out


def _cloth_mesh_state(setup, cloth):
    """Per cloth object: (world positions of the evaluated mesh, edges, mask of the vertices the strip bones carry most)."""
    dg = bpy.context.evaluated_depsgraph_get()
    out = []
    for o in mesh_prep.cloth_objects(setup):
        ev = o.evaluated_get(dg)
        me = ev.to_mesh()
        n = len(me.vertices)
        pos = np.empty(n * 3)
        me.vertices.foreach_get('co', pos)
        mw = np.array(o.matrix_world)
        pos = pos.reshape(n, 3) @ mw[:3, :3].T + mw[:3, 3]
        edges = np.empty(len(me.edges) * 2, dtype=np.int64)
        me.edges.foreach_get('vertices', edges)
        ev.to_mesh_clear()
        names = {g.index: g.name for g in o.vertex_groups}
        mask = np.array([bool(v.groups) and names.get(max(v.groups, key=lambda g: g.weight).group) in cloth
                         for v in o.data.vertices])
        out.append((pos, edges.reshape(-1, 2), mask))
    return out


def clip_body_check(setup, arm, cloth):
    """The clips' frames, and the blends halfway between two neighbouring wind sides, posed in Blender (the BC Follow
    constraints join the strips as the game does): [(label, deepest new depth into a collider, bone near it, largest
    stretch of an edge, bone near it)]. The armature's pose is put back afterwards."""
    sets = poses.clip_sample_sets(setup)
    cases = []
    for key in ec.CLIP_SLOTS:
        act = poses.clip_slot(setup, key).action
        if act is None:
            continue
        fr = poses.clip_frames(act)
        for f in fr[1:]:
            position = (f - fr[0]) / (fr[-1] - fr[0])
            cases.append(("%s, frame %d" % (poses.CLIP_TITLES[key], f), [(ec.CLIP_SLOTS.index(key), position, 1.0)]))
    for side in range(4):
        a, b = ec.CLIP_SLOTS[side], ec.CLIP_SLOTS[(side + 1) % 4]
        if a in sets and b in sets:
            for position in (0.5, 1.0):
                cases.append(("%s and %s %d%%" % (poses.CLIP_TITLES[a], poses.CLIP_TITLES[b].lower().replace("wind ", ""),
                                                  position * 100), [(side, position, 0.5), ((side + 1) % 4, position, 0.5)]))
    if not cases:
        return []

    held = {pb.name: (pb.matrix_basis.copy(), pb.rotation_mode) for pb in arm.pose.bones}
    results = []
    try:
        poses.set_bases(arm, {})
        bpy.context.view_layer.update()
        caps = _posed_capsules(setup, arm)
        rest = _cloth_mesh_state(setup, cloth)
        rest_depth = [np.max([[ec.capsule_depth(p, *c)[0] for c in caps] for p in pos], axis=1) if caps else np.zeros(len(pos))
                      for pos, _, _ in rest]
        for label, shares in cases:
            pose = poses.blend_clips(sets, shares)
            poses.set_bases(arm, {b: q for b, (q, _) in pose.items()}, {b: loc for b, (_, loc) in pose.items()})
            bpy.context.view_layer.update()
            depth, depth_at, stretch, stretch_at = 0.0, None, 1.0, None
            for (pos, edges, mask), (rpos, _, _), rd in zip(_cloth_mesh_state(setup, cloth), rest, rest_depth):
                for i in np.where(mask & (rd <= 0.0))[0]:
                    for c in caps:
                        d = ec.capsule_depth(pos[i], *c)[0]
                        if d > depth:
                            depth, depth_at = d, (rpos, i)
                both = mask[edges[:, 0]] & mask[edges[:, 1]]
                if both.any():
                    e = edges[both]
                    ratio = np.linalg.norm(pos[e[:, 0]] - pos[e[:, 1]], axis=1) / np.maximum(
                        np.linalg.norm(rpos[e[:, 0]] - rpos[e[:, 1]], axis=1), 1e-6)
                    k = int(np.argmax(ratio))
                    if ratio[k] > stretch:
                        stretch, stretch_at = float(ratio[k]), (rpos, int(e[k, 0]))
            results.append((label, depth, _nearest_bone(arm, cloth, depth_at), stretch, _nearest_bone(arm, cloth, stretch_at)))
    finally:
        for pb in arm.pose.bones:
            matrix, mode = held[pb.name]
            pb.rotation_mode = mode
            pb.matrix_basis = matrix
        bpy.context.view_layer.update()
    return results


def _nearest_bone(arm, cloth, at):
    """The cloth bone whose head is nearest a vertex at rest (to name where a problem is)."""
    if at is None:
        return ""
    rpos, i = at
    p = rpos[i]
    return min(cloth, key=lambda b: float(np.linalg.norm(frames.bone_head(arm, b) - p)) if b in arm.data.bones else 1e9)


def blocking(issues):
    return [i for i in issues if i.severity == "error"]


def fix(setup, arm, key):
    """Runs the fix named by an Issue's `fix`. Returns a message."""
    from . import pipeline
    if key.startswith("mesh:"):
        mesh_prep.fix(setup, arm, key[5:])
        return "Fixed"
    if key == "weights":
        pipeline.settle_weights(setup, arm, pipeline.build_weights(setup, arm))
        return "Rebuilt the weights"
    if key == "poses":
        pipeline.update_poses(setup, arm)
        return "Updated the wind poses"
    if key == "rest":
        setup.rest.clear_legs = True
        pipeline.apply_rest(setup, arm)
        return "Rest shape applied"
    if key == "drop_upper":
        setup.guides.upper_part = False
        pipeline.update_all(setup, arm)
        return "Left out the chest part and rebuilt"
    if key == "fewer_panels":
        setup.guides.columns_per_side = max(1, setup.guides.columns_per_side - 1)
        pipeline.measure(setup, arm)
        pipeline.update_all(setup, arm)
        return "Now %d panels per side" % setup.guides.columns_per_side
    if key == "update":
        pipeline.update_all(setup, arm)
        return "Updated"
    if key == "cloth_keys":
        gen = set(bones_mod.generated(setup)) | set(bones_mod.roots(setup))
        ours = {layer.action.name for layer in (setup.poses.open, setup.poses.volume) if layer.action}
        removed = 0
        for act in bpy.data.actions:
            if act.name in ours:
                continue
            for lyr in act.layers:
                for st in lyr.strips:
                    for cb in st.channelbags:
                        for fc in list(cb.fcurves):
                            if fc.data_path.startswith('pose.bones["') and fc.data_path.split('"')[1] in gen:
                                cb.fcurves.remove(fc)
                                removed += 1
        return "Removed %d keys" % removed
    return "Nothing to do"
