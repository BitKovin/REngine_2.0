"""Step 6 (and the rest shape): the swing solver, wind-pose actions, preview (DESIGN.md sections 8.5.2, 8.6)."""
import math
from contextlib import contextmanager

import bpy
import numpy as np
from mathutils import Matrix, Quaternion, Vector

from . import bones as bones_mod
from . import engine_contract as ec
from . import frames

MAX_ANGLE = math.radians(70.0)


def arm_rotation(arm):
    """The armature object's rotation as a pure rotation matrix (its scale removed)."""
    return arm.matrix_world.to_3x3().normalized()


def _rot(axis, angle):
    return Matrix.Rotation(angle, 3, Vector(axis))


class StripRig:
    """One strip's rest geometry in world space and the rest rotations of its bones in armature space."""

    def __init__(self, arm, info):
        self.info = info
        self.names = info["bones"]
        self.theta = info["theta"]
        self.heads = [frames.bone_head(arm, b) for b in self.names]
        self.tails = [frames.bone_tail(arm, b) for b in self.names]
        self.rest_r = [arm.data.bones[b].matrix_local.to_3x3().normalized() for b in self.names]
        self.parent_in_strip = [arm.data.bones[b].parent.name if arm.data.bones[b].parent
                                and arm.data.bones[b].parent.name in self.names else None for b in self.names]
        self.has_upper = "_00" in self.names[0]

    def fk(self, rots):
        """Joint heads and tails after the cumulative world rotations `rots`, as one chain from the strip's top: every
        joint starts where the one above ends. That is how the engine builds a strip (each head is the previous joint's
        tail), and in Blender the BC Follow constraint pins a bone that hangs from a different spine bone to the previous
        tail the same way, so the heads of the upper bones do NOT stay at their rest positions."""
        heads, tails = [], []
        head = self.heads[0]
        for k in range(len(self.names)):
            v = self.tails[k] - self.heads[k]
            tail = head + np.array(rots[k]) @ v
            heads.append(head)
            tails.append(tail)
            head = tail
        return heads, tails


def shares(rig, bend, upper_share):
    """How the panel's total turn is split between its bones: by length, so that bend 0 is an even arc whatever the
    bones' lengths (short bones near the hem must not turn as much as long ones); more bend puts more of the turn lower."""
    n = len(rig.names)
    length = np.array([np.linalg.norm(rig.tails[k] - rig.heads[k]) for k in range(n)])
    total = float(length.sum())
    if total < 1e-9:
        return np.full(n, 1.0 / n)
    mid = (np.cumsum(length) - 0.5 * length) / total
    w = length * np.power(np.clip(mid, 0.05, 1.0), 3.0 * bend)
    if rig.has_upper:
        w[0] *= upper_share
    s = w.sum()
    return w / s if s > 1e-9 else np.full(n, 1.0 / n)


REFERENCE_BEND = 0.25      # the shape that defines where the hem goes; the layer's bend only changes how the panel gets there


def solve_strip(arm, frame, rig, direction, amount, bend, upper_share, max_angle=MAX_ANGLE):
    """Cumulative world rotations (list of 3x3 numpy) that move the strip's hem `amount` toward `direction`.

    The hem's place is fixed first, from a neutral shape (REFERENCE_BEND); `bend` then only chooses how the turn is spread
    along the panel (negative: more near the top, a straighter line; positive: more near the hem) and the total turn is
    fitted so the hem lands as close to that same place as it can. So changing the bend keeps the end points."""
    n = len(rig.names)
    ident = [np.eye(3)] * n
    if amount <= 1e-6 or np.linalg.norm(direction) < 1e-9:
        return ident
    top, hem = rig.heads[0], rig.tails[-1]
    S = hem - top
    S /= max(np.linalg.norm(S), 1e-9)
    D = np.asarray(direction, float)
    D /= np.linalg.norm(D)
    axis = np.cross(S, D)
    if np.linalg.norm(axis) < 1e-6:
        return ident
    axis /= np.linalg.norm(axis)

    def profile(b):
        return np.cumsum(shares(rig, b, upper_share))

    def rots_for(cum, alpha):
        return [np.array(_rot(axis, alpha * c)) for c in cum]

    def hem_at(cum, alpha):
        return rig.fk(rots_for(cum, alpha))[1][-1]

    # 1. the place: the neutral shape turned until the hem has moved `amount` (or the turn limit is reached)
    ref = profile(REFERENCE_BEND)
    if float(np.linalg.norm(hem_at(ref, max_angle) - hem)) <= amount:
        a_ref = max_angle
    else:
        lo, hi = 0.0, max_angle
        for _ in range(40):
            mid = 0.5 * (lo + hi)
            if float(np.linalg.norm(hem_at(ref, mid) - hem)) < amount:
                lo = mid
            else:
                hi = mid
        a_ref = 0.5 * (lo + hi)
    target = hem_at(ref, a_ref)
    if abs(bend - REFERENCE_BEND) < 1e-9:
        return rots_for(ref, a_ref)

    # 2. the way: this bend's profile, turned (within the limit) to land nearest that place
    cum = profile(bend)
    alphas = np.linspace(0.0, max_angle, 71)
    best = min(alphas, key=lambda a: float(np.linalg.norm(hem_at(cum, a) - target)))
    step = alphas[1] - alphas[0]
    lo, hi = max(0.0, best - step), min(max_angle, best + step)
    for _ in range(30):                           # refine around the best sample (golden-section)
        m1, m2 = lo + (hi - lo) * 0.382, lo + (hi - lo) * 0.618
        if np.linalg.norm(hem_at(cum, m1) - target) < np.linalg.norm(hem_at(cum, m2) - target):
            hi = m2
        else:
            lo = m1
    return rots_for(cum, 0.5 * (lo + hi))


def to_basis(arm, rig, rots):
    """Pose-bone rotation quaternions (rest-local) that produce the cumulative world rotations `rots`."""
    A = arm_rotation(arm)
    out = {}
    index = {n: k for k, n in enumerate(rig.names)}
    for k, name in enumerate(rig.names):
        Rk = A.transposed() @ Matrix(rots[k].tolist()) @ A     # in armature space
        parent = rig.parent_in_strip[k]
        if parent is not None:
            Rp = A.transposed() @ Matrix(rots[index[parent]].tolist()) @ A
            delta = Rp.inverted() @ Rk
        else:
            delta = Rk
        rb = rig.rest_r[k]
        out[name] = (rb.inverted() @ delta @ rb).to_quaternion()
    return out


def column_mult(setup, theta, kind):
    """The column of a strip (by its angle) and its multiplier for 'open', 'volume' or 'rest'."""
    best, diff = None, 1e9
    for c in setup.guides.columns:
        for ang in ((c.angle, -c.angle) if c.side == 'L' else (c.angle,)):
            d = abs(ang - theta)
            if d < diff:
                best, diff = c, d
    if best is None:
        return 1.0
    return {"open": best.open_mult, "volume": best.volume_mult, "rest": best.rest_flare_mult}[kind]


def layer_directions(frame, theta, layer_kind, direction, backward=0.0):
    """Where the panel's hem is pushed: open = sideways away from the front opening, volume = outward and up; `backward`
    sweeps either one toward the back, the way a headwind blows a coat."""
    radial = frame.radial(theta)
    if layer_kind == "open":
        sign = 1.0 if theta > 1e-3 else (-1.0 if theta < -1e-3 else 0.0)
        away = sign * frame.around(theta)
        d = (1.0 - direction) * radial + direction * away
        if np.linalg.norm(d) < 1e-6:
            d = radial
    else:
        d = radial + direction * np.array([0.0, 0.0, 1.0])
    return d - backward * frame.forward


def beside_opening(theta):
    """1 for a panel beside the front opening, fading to 0 over the last 60 degrees toward the back: how much of the Open
    pose's sideways motion (slide along the ring, top bone's slide) the panel takes."""
    return min(1.0, max(0.0, (math.pi - abs(theta)) / math.radians(60.0)))


def open_target(frame, rig, theta, direction, backward, amount):
    """(direction, distance) that carry a panel's hem to its place in the open pose: the hem keeps its height and slides ALONG the ring around the body (away
    from the front opening, further with `backward`) while it moves out by `1 - direction`, so the hems of all panels stay on one smooth arc. A straight
    push would take the front hems off the ring in a line and leave a corner where they meet the back."""
    hem = rig.tails[-1]
    rel = hem - frame.origin
    h = float(rel @ frame.up)
    planar = rel - frame.up * h
    r = float(np.linalg.norm(planar))
    sign = 1.0 if theta > 1e-3 else (-1.0 if theta < -1e-3 else 0.0)
    if r < 1e-6 or sign == 0.0:
        return layer_directions(frame, theta, "open", direction, backward), amount
    radial = planar / r
    out, along = (1.0 - direction), (direction + backward)
    norm = math.hypot(out, along)
    if norm < 1e-9:
        return radial, amount
    out, along = out / norm * amount, along / norm * amount       # the hem moves `amount` in total
    # sliding along the ring is for the panels beside the opening; toward the back the ring runs the wrong way (the left and
    # right back panels would be pushed into each other), so there the slide turns into outward travel, same distance
    fade = beside_opening(theta)
    out, along = math.hypot(out, along * (1.0 - fade)), along * fade
    # slide along the ring: positive turns about the body axis go from the front toward the character's left (`left = up x forward`)
    phi = sign * along / r
    c, s_ = math.cos(phi), math.sin(phi)
    k = frame.up
    turned = planar * c + np.cross(k, planar) * s_ + k * float(k @ planar) * (1 - c)
    target = frame.origin + frame.up * h + turned + radial * out
    d = target - hem
    dist = float(np.linalg.norm(d))
    return (d / dist if dist > 1e-9 else radial), dist


def solve_layer(setup, arm, frame, kind):
    """{bone: quaternion} for one wind layer ('open' or 'volume'), strips whose column takes part only."""
    return solve_layer_full(setup, arm, frame, kind)[0]


def slide_location(arm, frame, rig, theta, kind, distance):
    """The top bone's pose location (its own rest-local axes) that carries it `distance` metres sideways away from the
    front opening (open) or outward (volume): the whole panel hangs from that bone, so the whole panel shifts."""
    if kind == "open":
        sign = 1.0 if theta > 1e-3 else (-1.0 if theta < -1e-3 else 0.0)
        world = sign * frame.left * distance * beside_opening(theta)
    else:
        world = frame.radial(theta) * distance
    in_arm = arm.matrix_world.inverted().to_3x3() @ Vector(tuple(float(x) for x in world))
    return rig.rest_r[0].inverted() @ in_arm


def solve_layer_full(setup, arm, frame, kind):
    """({bone: quaternion}, {bone: location}) for one wind layer. The locations are the panels' slide."""
    layer = getattr(setup.poses, kind)
    result, locs = {}, {}
    for info in bones_mod.strips(setup, arm):
        mult = column_mult(setup, info["theta"], kind)
        if mult <= 1e-4:
            continue
        rig = StripRig(arm, info)
        if kind == "open":
            d, amount = open_target(frame, rig, info["theta"], layer.direction, layer.backward, layer.amount * mult)
        else:
            d, amount = layer_directions(frame, info["theta"], kind, layer.direction, layer.backward), layer.amount * mult
        rots = solve_strip(arm, frame, rig, d, amount, layer.bend, layer.upper_share, layer.max_turn)
        result.update(to_basis(arm, rig, rots))
        if layer.slide > 1e-6:
            locs[rig.names[0]] = slide_location(arm, frame, rig, info["theta"], kind, layer.slide * mult)
    return result, locs


# ---- actions --------------------------------------------------------------------------------------------------------

def write_action(arm, layer, quats, locs=None):
    """Replaces the layer's action with a two-frame clip: frame 1 rest, frame 2 the given bases (and locations)."""
    locs = locs or {}
    name = layer.clip_name
    old = layer.action
    ad = arm.animation_data
    shown = ad is not None and old is not None and ad.action == old      # the user is looking at it in the Action Editor
    if old is not None:
        old.use_fake_user = False
        bpy.data.actions.remove(old)
    stale = bpy.data.actions.get(name)
    if stale is not None:
        if stale.get("bone_cloth"):
            stale.use_fake_user = False
            bpy.data.actions.remove(stale)
        else:
            stale.name = name + "_old"      # somebody else's clip of that name: keep it
            stale.use_fake_user = True
    act = bpy.data.actions.new(name)
    slot = act.slots.new('OBJECT', arm.name)
    strip = act.layers.new("Layer").strips.new(type='KEYFRAME')
    cb = strip.channelbag(slot, ensure=True)
    ident = Quaternion((1, 0, 0, 0))
    for bone in list(quats) + [b for b in locs if b not in quats]:
        q = quats.get(bone, ident)
        for path, values_a, values_b in (
                ("location", (0, 0, 0), tuple(locs.get(bone, (0, 0, 0)))),
                ("rotation_quaternion", tuple(ident), tuple(q)),
                ("scale", (1, 1, 1), (1, 1, 1))):
            for i, (a, b) in enumerate(zip(values_a, values_b)):
                fc = cb.fcurves.new('pose.bones["%s"].%s' % (bone, path), index=i, group_name=bone)
                for frame_no, v in ((1, a), (2, b)):
                    kp = fc.keyframe_points.insert(frame_no, v)
                    kp.interpolation = 'LINEAR'
    act.use_frame_range = True
    act.frame_start, act.frame_end = 1, 2
    act.use_fake_user = True
    act["bone_cloth"] = layer.id_data.name
    act["bc_sig"], act["bc_count"] = action_signature(act)
    stamp_layout(arm, act)
    layer.action = act
    if shown:
        ad.action = act
        ad.action_slot = slot
        # the pose on screen is the old clip's until the animation is evaluated again
        scene = bpy.context.scene
        scene.frame_set(scene.frame_current)
        arm.update_tag()
        for window in bpy.context.window_manager.windows:
            for area in window.screen.areas:
                area.tag_redraw()
    return act


def read_action(act):
    """{bone: Quaternion} of frame 2 of a clip written by write_action (or any two-frame clip)."""
    out = {}
    if act is None:
        return out
    for lyr in act.layers:
        for st in lyr.strips:
            for cb in st.channelbags:
                parts = {}
                for fc in cb.fcurves:
                    if fc.data_path.endswith(".rotation_quaternion"):
                        bone = fc.data_path.split('"')[1]
                        parts.setdefault(bone, [1, 0, 0, 0])[fc.array_index] = fc.evaluate(2.0)
                for bone, q in parts.items():
                    out[bone] = Quaternion(q)
    return out


def read_action_locs(act):
    """{bone: Vector} of frame 2 locations of a two-frame clip (only bones that move)."""
    out = {}
    if act is None:
        return out
    for lyr in act.layers:
        for st in lyr.strips:
            for cb in st.channelbags:
                parts = {}
                for fc in cb.fcurves:
                    if fc.data_path.endswith(".location"):
                        bone = fc.data_path.split('"')[1]
                        parts.setdefault(bone, [0.0, 0.0, 0.0])[fc.array_index] = fc.evaluate(2.0)
                for bone, v in parts.items():
                    if any(abs(x) > 1e-9 for x in v):
                        out[bone] = Vector(v)
    return out


def action_signature(act):
    """A fingerprint of what a two-frame clip does (frame 2 of every key), to notice keys added or edited by hand."""
    parts = []
    if act is not None:
        for lyr in act.layers:
            for st in lyr.strips:
                for cb in st.channelbags:
                    for fc in cb.fcurves:
                        parts.append("%s[%d]=%.4f" % (fc.data_path, fc.array_index, fc.evaluate(2.0)))
    import zlib
    return "%08x" % zlib.crc32("|".join(sorted(parts)).encode("utf-8")), len(parts)


def layout_signature(arm, names):
    """A fingerprint of the bones a pose is made for: their names and rest positions (to a millimetre)."""
    import zlib
    parts = []
    for n in sorted(names):
        b = arm.data.bones.get(n)
        if b is not None:
            parts.append("%s:%s" % (n, ",".join("%.1f" % (x * 10.0) for x in tuple(b.head_local) + tuple(b.tail_local))))
    return "%08x" % zlib.crc32("|".join(parts).encode("utf-8"))


def pose_bone_names(act):
    names = set()
    if act is not None:
        for lyr in act.layers:
            for st in lyr.strips:
                for cb in st.channelbags:
                    for fc in cb.fcurves:
                        if fc.data_path.startswith('pose.bones["'):
                            names.add(fc.data_path.split('"')[1])
    return names


def stamp_layout(arm, act):
    """Remembers which bones the clip was made for."""
    cloth = [n for n in pose_bone_names(act) if n in arm.pose.bones and "bone_cloth" in arm.pose.bones[n]]
    act["bc_layout"] = layout_signature(arm, cloth)
    act["bc_layout_bones"] = len(cloth)


def layout_changed(arm, layer):
    """True when the clip was made for other bones than these (the bones were rebuilt since)."""
    act = layer.action
    if act is None or "bc_layout" not in act:
        return False
    cloth = [n for n in pose_bone_names(act) if n in arm.pose.bones and "bone_cloth" in arm.pose.bones[n]]
    return len(cloth) != act.get("bc_layout_bones", len(cloth)) or layout_signature(arm, cloth) != act["bc_layout"]


def layer_is_hand(layer):
    """True for a pose that is the artist's: saved from the viewport, or edited (keys changed or added) since it was
    generated. Such a pose is never overwritten by Update."""
    if layer.action is None:
        return False
    if layer.mode == 'HAND':
        return True
    act = layer.action
    if "bc_sig" not in act:
        return False
    sig, count = action_signature(act)
    return act["bc_sig"] != sig or act.get("bc_count", count) != count


def update_layer(setup, arm, frame, kind, force=False):
    """Generates the layer's clip from the settings. A hand-made clip is kept unless `force`."""
    layer = getattr(setup.poses, kind)
    if not layer.enabled:
        return None
    if not force and layer_is_hand(layer):
        layer.mode = 'HAND'
        return layer.action
    if not layer.clip_name:
        layer.clip_name = "%s_%s" % (setup.prefix, kind if kind == "open" else "volume")
    quats, locs = solve_layer_full(setup, arm, frame, kind)
    layer.mode = 'GENERATED'
    return write_action(arm, layer, quats, locs)


def partner_name(name):
    """The bone on the other side: coat_front_01_l <-> coat_front_01_r."""
    if name.endswith("_l"):
        return name[:-2] + "_r"
    if name.endswith("_r"):
        return name[:-2] + "_l"
    return None


def _mirrored(arm, src_names, dst_names):
    """{bone: (rotation, location)} for dst_names: the current pose of src_names mirrored through the body's centre plane
    (rotations, and the top bone's slide)."""
    mir3 = Matrix(((-1, 0, 0), (0, 1, 0), (0, 0, 1)))
    deltas = []
    for n in src_names:
        pb = arm.pose.bones[n]
        rest = pb.bone.matrix_local.to_3x3().normalized()
        deltas.append(mir3 @ (pb.matrix.to_3x3().normalized() @ rest.inverted()) @ mir3)
    out = {}
    for k, n in enumerate(dst_names):
        bone = arm.data.bones[n]
        rb = bone.matrix_local.to_3x3().normalized()
        par = bone.parent.name if bone.parent else None
        if par in dst_names:
            delta = deltas[dst_names.index(par)].inverted() @ deltas[k]
        else:
            delta = deltas[k]
        out[n] = ((rb.inverted() @ delta @ rb).to_quaternion(), Vector((0.0, 0.0, 0.0)))
    # the slide of the top bone
    s_pb = arm.pose.bones[src_names[0]]
    t = Vector(s_pb.head) - Vector(s_pb.bone.head_local)
    d_bone = arm.data.bones[dst_names[0]]
    out[dst_names[0]] = (out[dst_names[0]][0],
                         d_bone.matrix_local.to_3x3().normalized().inverted() @ Vector((-t.x, t.y, t.z)))
    return out


def mirror_pose(setup, arm, source):
    """Copies the pose of one side's panels onto the other side's, mirrored through the body's centre plane. source is
    'R' or 'L'. Works on the current pose of the cloth bones (rotations and the top bones' slide). Returns the bone names
    that were set."""
    infos = bones_mod.strips(setup, arm)
    by_name = {i["bones"][0]: i for i in infos}
    suffix = "_r" if source == 'R' else "_l"
    result = {}
    for info in infos:
        first = info["bones"][0]
        if not first.endswith(suffix):
            continue
        twin = by_name.get(partner_name(first))
        if twin is None or len(info["bones"]) != len(twin["bones"]):
            continue
        result.update(_mirrored(arm, info["bones"], twin["bones"]))
    _pose_bones(arm, result)
    return list(result)


def swapped_pose(setup, arm):
    """The cloth bones' current pose mirrored through the body's centre plane, both sides at once: the left panels take
    the right ones' pose and the other way round, a middle panel mirrors onto itself. {bone: (rotation, location)}."""
    infos = bones_mod.strips(setup, arm)
    by_name = {i["bones"][0]: i for i in infos}
    out = {}
    for info in infos:
        name = partner_name(info["bones"][0])
        twin = by_name.get(name) if name else info
        if twin is None or len(info["bones"]) != len(twin["bones"]):
            continue
        out.update(_mirrored(arm, info["bones"], twin["bones"]))
    return out


def _pose_bones(arm, bases):
    for n, (q, loc) in bases.items():
        pb = arm.pose.bones[n]
        pb.rotation_mode = 'QUATERNION'
        pb.rotation_quaternion = q
        pb.location = loc
    bpy.context.view_layer.update()


def key_pose_into(act, arm, names):
    """Writes the current pose of the named bones into an existing two-frame clip at frame 2 (their rotation and
    location keys), leaving every other key of the clip as it is."""
    done = 0
    for lyr in act.layers:
        for st in lyr.strips:
            for cb in st.channelbags:
                for fc in cb.fcurves:
                    if '"' not in fc.data_path:
                        continue
                    bone = fc.data_path.split('"')[1]
                    if bone not in names:
                        continue
                    pb = arm.pose.bones[bone]
                    if fc.data_path.endswith(".rotation_quaternion"):
                        value = pb.rotation_quaternion[fc.array_index]
                    elif fc.data_path.endswith(".location"):
                        value = pb.location[fc.array_index]
                    else:
                        continue
                    fc.keyframe_points.insert(2, value).interpolation = 'LINEAR'
                    fc.update()
                    done += 1
    return done


def capture_current(setup, arm, kind, only_selected=True):
    """Saves the cloth bones' current pose (rotations and the top bones' slide) as the layer's pose, hand made. With
    cloth bones selected in Pose mode only those are saved; otherwise every cloth bone that is posed."""
    layer = getattr(setup.poses, kind)
    if not layer.clip_name:
        layer.clip_name = "%s_%s" % (setup.prefix, kind if kind == "open" else "volume")
    names = [b for info in bones_mod.strips(setup, arm) for b in info["bones"]]
    selected = [n for n in names if arm.pose.bones[n].select]
    if only_selected and selected:
        names = selected
    quats, locs = {}, {}
    for b in names:
        pb = arm.pose.bones[b]
        if pb.rotation_quaternion.angle > 1e-5:
            quats[b] = pb.rotation_quaternion.copy()
        if pb.location.length > 1e-6:
            locs[b] = pb.location.copy()
    if not quats and not locs:
        raise RuntimeError("Pose the cloth bones first (Pose mode), then save")
    layer.mode = 'HAND'
    return write_action(arm, layer, quats, locs)


# ---- preview --------------------------------------------------------------------------------------------------------

def set_bases(arm, bases, locs=None):
    locs = locs or {}
    for pb in arm.pose.bones:
        if "bone_cloth" in pb:
            pb.rotation_mode = 'QUATERNION'
            pb.rotation_quaternion = bases.get(pb.name, Quaternion((1, 0, 0, 0)))
            pb.location = locs.get(pb.name, (0.0, 0.0, 0.0))


def preview_bases(setup, wo, wv):
    q_open = read_action(setup.poses.open.action) if setup.poses.open.enabled else {}
    q_vol = read_action(setup.poses.volume.action) if setup.poses.volume.enabled else {}
    ident = Quaternion((1, 0, 0, 0))
    out = {}
    for b in set(q_open) | set(q_vol):
        o = ident.slerp(q_open.get(b, ident), wo)
        v = ident.slerp(q_vol.get(b, ident), wv)
        out[b] = o @ v
    return out


def preview_locs(setup, wo, wv):
    lo = read_action_locs(setup.poses.open.action) if setup.poses.open.enabled else {}
    lv = read_action_locs(setup.poses.volume.action) if setup.poses.volume.enabled else {}
    zero = Vector((0.0, 0.0, 0.0))
    return {b: lo.get(b, zero) * wo + lv.get(b, zero) * wv for b in set(lo) | set(lv)}


def apply_preview(context):
    arm = context.active_object
    from .props import get_setup
    setup = get_setup(arm)
    if setup is None:
        return
    if uses_clips(setup):
        pv = setup.poses
        # in clip positions (1 = the clips' end): the wind's, turned to headwind by the spin, and spinOpen of the way
        air = ec.spin_air(-math.cos(pv.preview_wind_from) * pv.preview_wind, -math.sin(pv.preview_wind_from) * pv.preview_wind,
                          setup.motion.spinOpen, abs(pv.preview_turn), 1.0, pv.preview_wind)
        layers = ec.clip_layers(air[0], air[1], min(math.hypot(*air), 1.0), pv.preview_turn, strengths(setup))
        pose = blend_layers(clip_sample_sets(setup), layers)
        set_bases(arm, {b: q for b, (q, _) in pose.items()}, {b: loc for b, (_, loc) in pose.items()})
        arm.update_tag()
        context.view_layer.update()
        return
    wo, wv = setup.poses.preview_open, setup.poses.preview_volume
    if wo > 0 or wv > 0:
        set_bases(arm, preview_bases(setup, wo, wv), preview_locs(setup, wo, wv))
    else:
        set_bases(arm, {})
    arm.update_tag()
    context.view_layer.update()


# ---- wind and turn clips --------------------------------------------------------------------------------------------
# Each clip is the cloth bones' poses from frame 1 (the rest shape) on, as the wind from one side - or the turn - grows;
# the game plays it as far as the wind is strong, blends the wind's sides and puts the turn's on top
# (engine_contract.clip_layers).

CLIP_TITLES = {"windFront": "Wind from the front", "windLeft": "Wind from the left", "windBack": "Wind from behind",
               "windRight": "Wind from the right", "turnLeft": "Turning left", "turnRight": "Turning right"}
MIRROR_OF = {"windRight": "windLeft", "turnRight": "turnLeft"}      # a clip that can be written from the other side's


def clip_slot(setup, key):
    return getattr(setup.poses, key)


def uses_clips(setup):
    """True when any wind or turn clip is set: they replace the Open and Volume layers."""
    return any(clip_slot(setup, k).action is not None for k in ec.CLIP_SLOTS)


def cloth_bone_names(setup, arm):
    return [b for info in bones_mod.strips(setup, arm) for b in info["bones"]]


def _channelbag(act):
    for lyr in act.layers:
        for st in lyr.strips:
            for cb in st.channelbags:
                return cb
    return None


def _rest_channels(cb, names, frames_at):
    """Every channel of the named bones (location, rotation, scale), keyed at rest on each of `frames_at`."""
    for bone in names:
        for path, values in (("location", (0.0, 0.0, 0.0)), ("rotation_quaternion", (1.0, 0.0, 0.0, 0.0)),
                             ("scale", (1.0, 1.0, 1.0))):
            for i, v in enumerate(values):
                fc = cb.fcurves.new('pose.bones["%s"].%s' % (bone, path), index=i, group_name=bone)
                for frame_no in frames_at:
                    fc.keyframe_points.insert(frame_no, v).interpolation = 'LINEAR'


def new_clip(setup, arm, key):
    """A clip of the cloth bones at rest on frames 1 and 2, set on the slot (pose and key the later frames)."""
    snake = "".join("_" + c.lower() if c.isupper() else c for c in key)
    act = bpy.data.actions.new("%s_%s" % (setup.prefix, snake))
    slot = act.slots.new('OBJECT', arm.name)
    cb = act.layers.new("Layer").strips.new(type='KEYFRAME').channelbag(slot, ensure=True)
    _rest_channels(cb, cloth_bone_names(setup, arm), (1, 2))
    act.use_frame_range = True
    act.frame_start, act.frame_end = 1, 2
    act.use_fake_user = True
    act["bone_cloth"] = arm.name
    clip_slot(setup, key).action = act
    return act


def write_clip(setup, arm, key, frame_bases):
    """The slot's clip made of `frame_bases` ([{bone: (rotation, location)}] for frames 2, 3, ...) after the rest shape
    on frame 1, replacing its keys (the action and its name are kept), marked as generated."""
    act = clip_slot(setup, key).action or new_clip(setup, arm, key)
    cb = _channelbag(act)
    for fc in list(cb.fcurves):
        cb.fcurves.remove(fc)
    _rest_channels(cb, cloth_bone_names(setup, arm), (1,))
    act.frame_start, act.frame_end = 1, 1
    for k, bases in enumerate(frame_bases):
        key_into_clip(act, 2 + k, bases)
    act["bc_clip_sig"] = "%s:%d" % action_signature(act)
    return act


def clip_is_hand(act):
    """True for a clip the artist made or changed: one the add-on did not write, or changed since it did (a clip that
    is still all rest counts as neither)."""
    if "bc_clip_sig" in act:
        return act["bc_clip_sig"] != "%s:%d" % action_signature(act)
    end = bases_at(act, clip_frames(act)[-1])
    return any(q.angle > 1e-4 or loc.length > 1e-6 for q, loc in end.values())


@contextmanager
def held_pose(arm):
    """The body at rest and no clip on the armature while posing for a measurement; everything put back after."""
    ad = arm.animation_data
    held_action = ad.action if ad else None
    held_slot = ad.action_slot if ad else None
    held = {pb.name: (pb.matrix_basis.copy(), pb.rotation_mode) for pb in arm.pose.bones}
    try:
        if ad:
            ad.action = None
        for pb in arm.pose.bones:
            pb.matrix_basis = Matrix.Identity(4)
        bpy.context.view_layer.update()
        yield
    finally:
        for pb in arm.pose.bones:
            matrix, mode = held[pb.name]
            pb.rotation_mode = mode
            pb.matrix_basis = matrix
        if ad:
            ad.action = held_action
            if held_slot is not None:
                ad.action_slot = held_slot
        bpy.context.view_layer.update()


def edit_clip(arm, act):
    """Shows the clip on the armature (its action), so it is posed and keyed with Blender's own tools."""
    if arm.animation_data is None:
        arm.animation_data_create()
    ad = arm.animation_data
    ad.action = act
    if act.slots:
        ad.action_slot = act.slots[0]
    scene = bpy.context.scene
    scene.frame_set(scene.frame_current)


def current_bases(setup, arm, names=None):
    """{bone: (rotation, location)} of the cloth bones as posed now."""
    names = names if names is not None else cloth_bone_names(setup, arm)
    return {n: (arm.pose.bones[n].rotation_quaternion.copy(), arm.pose.bones[n].location.copy()) for n in names}


def key_into_clip(act, frame, bases):
    """Writes `bases` ({bone: (rotation, location)}) into the clip at `frame`, replacing keys there, and widens the
    clip's range to it. A channel the clip did not have yet starts at rest on the clip's first frame."""
    cb = _channelbag(act)
    if cb is None:
        raise RuntimeError("The clip '%s' has no keys to add to" % act.name)
    first = int(round(act.frame_range[0]))
    for bone, (q, loc) in bases.items():
        for path, values, rest in (("rotation_quaternion", tuple(q), (1.0, 0.0, 0.0, 0.0)),
                                   ("location", tuple(loc), (0.0, 0.0, 0.0))):
            for i, v in enumerate(values):
                data_path = 'pose.bones["%s"].%s' % (bone, path)
                fc = cb.fcurves.find(data_path, index=i)
                if fc is None:
                    fc = cb.fcurves.new(data_path, index=i, group_name=bone)
                    if frame != first:
                        fc.keyframe_points.insert(first, rest[i]).interpolation = 'LINEAR'
                fc.keyframe_points.insert(frame, float(v)).interpolation = 'LINEAR'
                fc.update()
    act.use_frame_range = True
    act.frame_start = min(act.frame_start, frame)
    act.frame_end = max(act.frame_end, frame)


def clip_frames(act):
    """The whole frames of the clip, first to last (the exporter samples every one)."""
    a, b = int(round(act.frame_range[0])), int(round(act.frame_range[1]))
    return list(range(a, max(a, b) + 1))


def bases_at(act, frame):
    """{bone: (rotation, location)} the clip gives the bones at `frame`."""
    parts = {}
    for lyr in act.layers:
        for st in lyr.strips:
            for cb in st.channelbags:
                for fc in cb.fcurves:
                    if not fc.data_path.startswith('pose.bones["'):
                        continue
                    bone = fc.data_path.split('"')[1]
                    entry = parts.setdefault(bone, [[1.0, 0.0, 0.0, 0.0], [0.0, 0.0, 0.0]])
                    if fc.data_path.endswith(".rotation_quaternion"):
                        entry[0][fc.array_index] = fc.evaluate(frame)
                    elif fc.data_path.endswith(".location"):
                        entry[1][fc.array_index] = fc.evaluate(frame)
    return {b: (Quaternion(q).normalized(), Vector(loc)) for b, (q, loc) in parts.items()}


def clip_samples(act, count=ec.CLIP_SAMPLE_COUNT):
    """The clip as the game samples it: `count` even samples over its range, each {bone: (rotation, location)} as the
    change from the clip's first frame. The exported clip holds every frame and the game interpolates between them."""
    frames = clip_frames(act)
    per_frame = [bases_at(act, f) for f in frames]
    first = per_frame[0]
    out = []
    for s in range(count):
        at = (len(frames) - 1) * s / (count - 1)
        lo = min(int(at), max(len(frames) - 2, 0))
        hi = min(lo + 1, len(frames) - 1)
        t = at - lo
        sample = {}
        for bone, (q0, l0) in first.items():
            qa, la = per_frame[lo].get(bone, (q0, l0))
            qb, lb = per_frame[hi].get(bone, (q0, l0))
            sample[bone] = (qa.slerp(qb, t) @ q0.inverted(), la.lerp(lb, t) - l0)
        out.append(sample)
    return out


def clip_sample_sets(setup):
    """{slot key: samples} of every clip that is set."""
    return {k: clip_samples(clip_slot(setup, k).action) for k in ec.CLIP_SLOTS if clip_slot(setup, k).action is not None}


def strengths(setup):
    """(front, sides, back, spin) for engine_contract.clip_layers, from the Motion step's Strength box."""
    m = setup.motion
    return m.windStrengthFront, m.windStrengthSides, m.windStrengthBack, m.spinStrength


def blend_layers(sample_sets, layers):
    """{bone: (rotation, location)} of engine_contract.clip_layers' layers, each blended, the later on top of the
    earlier (the turn on the wind), as the game applies them."""
    out = {}
    for layer in layers:
        for bone, (q, loc) in blend_clips(sample_sets, layer).items():
            q0, l0 = out.get(bone, (Quaternion(), Vector((0.0, 0.0, 0.0))))
            out[bone] = (q @ q0, l0 + loc)
    return out


def blend_clips(sample_sets, shares):
    """{bone: (rotation, location)} the clips give the cloth bones for one layer [(slot, position, weight)], blended as
    the game does: a weighted sum of the samples' rotations, each the short way round, normalised.
    A slot without a clip, or a bone it does not key, counts as the rest shape."""
    total = sum(w for _, _, w in shares)
    sums, locs, weights = {}, {}, {}
    for slot, position, weight in shares:
        samples = sample_sets.get(ec.CLIP_SLOTS[slot])
        if not samples:
            continue
        lower, upper, t = ec.sample_pair(position, len(samples))
        for index, part in ((lower, 1.0 - t), (upper, t)):
            share = weight * part
            if share <= 0.0:
                continue
            for bone, (q, loc) in samples[index].items():
                v = np.array([q.w, q.x, q.y, q.z]) * (1.0 if q.w >= 0.0 else -1.0)
                sums[bone] = sums.get(bone, np.zeros(4)) + v * share
                locs[bone] = locs.get(bone, Vector((0.0, 0.0, 0.0))) + loc * share
                weights[bone] = weights.get(bone, 0.0) + share
    out = {}
    for bone, v in sums.items():
        v = v + np.array([1.0, 0.0, 0.0, 0.0]) * (total - weights[bone])      # the rest shape's share
        n = float(np.linalg.norm(v))
        out[bone] = (Quaternion(v / n if n > 1e-9 else (1.0, 0.0, 0.0, 0.0)), locs[bone])
    return out


def mirror_clip(setup, arm, key):
    """Writes the slot `key` (windRight, turnRight) from its other side's clip, mirrored frame by frame: both sides of
    the jacket swap. The body is held at rest while it is worked out; the armature's pose and clip are put back."""
    source = clip_slot(setup, MIRROR_OF[key]).action
    if source is None:
        raise RuntimeError("Make the %s clip first" % CLIP_TITLES[MIRROR_OF[key]].lower())
    target = clip_slot(setup, key).action
    if target is None or target == source:
        target = new_clip(setup, arm, key)
    cb = _channelbag(target)
    for fc in list(cb.fcurves):
        cb.fcurves.remove(fc)
    _rest_channels(cb, cloth_bone_names(setup, arm), (clip_frames(source)[0],))   # every channel starts at rest
    target.use_frame_range = True
    target.frame_start, target.frame_end = source.frame_range[0], source.frame_range[1]
    with held_pose(arm):
        for frame in clip_frames(source):
            pose = bases_at(source, frame)
            _pose_bones(arm, {b: v for b, v in pose.items() if b in arm.pose.bones})
            key_into_clip(target, frame, swapped_pose(setup, arm))
    target["bc_clip_sig"] = "%s:%d" % action_signature(target)
    return target
