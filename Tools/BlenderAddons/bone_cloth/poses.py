"""Step 6 (and the rest shape): the swing solver, wind-pose actions, preview (DESIGN.md sections 8.5.2, 8.6)."""
import math

import bpy
import numpy as np
from mathutils import Matrix, Quaternion, Vector

from . import bones as bones_mod
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
        world = sign * frame.left * distance
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


def mirror_pose(setup, arm, source):
    """Copies the pose of one side's panels onto the other side's, mirrored through the body's centre plane. source is
    'R' or 'L'. Works on the current pose of the cloth bones (rotations and the top bones' slide). Returns the bone names
    that were set."""
    infos = bones_mod.strips(setup, arm)
    by_name = {i["bones"][0]: i for i in infos}
    mir3 = Matrix(((-1, 0, 0), (0, 1, 0), (0, 0, 1)))
    suffix = "_r" if source == 'R' else "_l"
    changed = []
    for info in infos:
        first = info["bones"][0]
        if not first.endswith(suffix):
            continue
        twin = by_name.get(partner_name(first))
        if twin is None:
            continue
        src_names, dst_names = info["bones"], twin["bones"]
        if len(src_names) != len(dst_names):
            continue
        deltas = []
        for n in src_names:
            pb = arm.pose.bones[n]
            rest = pb.bone.matrix_local.to_3x3().normalized()
            deltas.append(mir3 @ (pb.matrix.to_3x3().normalized() @ rest.inverted()) @ mir3)
        for k, n in enumerate(dst_names):
            bone = arm.data.bones[n]
            rb = bone.matrix_local.to_3x3().normalized()
            par = bone.parent.name if bone.parent else None
            if par in dst_names:
                delta = deltas[dst_names.index(par)].inverted() @ deltas[k]
            else:
                delta = deltas[k]
            pb = arm.pose.bones[n]
            pb.rotation_mode = 'QUATERNION'
            pb.rotation_quaternion = (rb.inverted() @ delta @ rb).to_quaternion()
            changed.append(n)
        # the slide of the top bone
        s_pb = arm.pose.bones[src_names[0]]
        t = Vector(s_pb.head) - Vector(s_pb.bone.head_local)
        d_bone = arm.data.bones[dst_names[0]]
        arm.pose.bones[dst_names[0]].location = d_bone.matrix_local.to_3x3().normalized().inverted() @ Vector((-t.x, t.y, t.z))
    bpy.context.view_layer.update()
    return changed


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
    wo, wv = setup.poses.preview_open, setup.poses.preview_volume
    if wo > 0 or wv > 0:
        set_bases(arm, preview_bases(setup, wo, wv), preview_locs(setup, wo, wv))
    else:
        set_bases(arm, {})
    arm.update_tag()
    context.view_layer.update()
