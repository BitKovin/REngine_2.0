"""Generated wind and turn clips (DESIGN.md section 15.4).

Every clip is built from one reference Open pose - the hand-made one when there is one, else a hinge the add-on makes -
taken apart per bone into a turn around the body's up axis (yaw: a front panel swinging round the spine like a door),
a tilt out from the body (flare: the hem lifting away) and a roll, and put back together with each frame's amounts:
a headwind opens the jacket on frame 2 and blows it back on frame 3, wind from behind presses the back and lifts the
front hems, a side wind opens the far side and sweeps everything along with the air, a turn flares the cloth and lets it
trail. The right-hand clips are the left ones mirrored."""
import math

import bpy
import numpy as np
from mathutils import Matrix, Vector

from . import bones as bones_mod
from . import engine_contract as ec
from . import frames, poses

OPEN_YAW = math.radians(55.0)     # without a hand-made Open pose: how far a front panel swings round the body...
OPEN_FLARE = math.radians(35.0)   # ...and how far a back panel's hem flares out
OPEN_TOP = 0.6                    # ...reaching the full swing this far down the panel
MIN_PROFILE = math.radians(5.0)   # a panel that turns less than this in the reference takes its profile from its length
BEYOND_OPEN = 0.3                 # a front panel blown further than the Open pose: how evenly the extra swing spreads up
                                  # the panel (the reference's own spread is 1; lower takes the chest along)
FLARE_FROM = 0.2                  # a front panel's hem flare starts this far down the panel and grows to the hem


def _rotation(axis, angle):
    """3x3 turn by `angle` about the unit vector `axis` (Rodrigues)."""
    k = np.array([[0.0, -axis[2], axis[1]], [axis[2], 0.0, -axis[0]], [-axis[1], axis[0], 0.0]])
    return np.eye(3) + math.sin(angle) * k + (1.0 - math.cos(angle)) * (k @ k)


def _from_vector(v):
    angle = float(np.linalg.norm(v))
    return _rotation(v / angle, angle) if angle > 1e-9 else np.eye(3)


def _side(theta):
    return 1.0 if theta > 1e-3 else (-1.0 if theta < -1e-3 else 0.0)


def _lengths(rig):
    length = np.array([np.linalg.norm(rig.tails[k] - rig.heads[k]) for k in range(len(rig.names))])
    return np.cumsum(length) / max(float(length.sum()), 1e-9)


def _profile(values, rig):
    """How far down the panel each bone has its share of a motion, 0..1, never falling: from the reference motion
    itself, or from the bones' lengths when the panel hardly moves in it."""
    values = np.abs(np.asarray(values, float))
    if values.max() < MIN_PROFILE:
        return _lengths(rig)
    return np.maximum.accumulate(values) / values.max()


def _measure(setup, arm, frame, bases, locs):
    """The reference from a pose: per strip (by its first bone) the parts of each bone's turn, the profile and the
    top bones' slide."""
    out = {}
    A = arm.matrix_world.to_3x3().normalized()
    up = frame.up
    with poses.held_pose(arm):
        poses.set_bases(arm, bases, locs)
        bpy.context.view_layer.update()
        for info in bones_mod.strips(setup, arm):
            theta = info["theta"]
            radial, tangent = frame.radial(theta), frame.around(theta)
            parts = []
            for n in info["bones"]:
                pb = arm.pose.bones[n]
                turn = (A @ pb.matrix.to_3x3().normalized()) @ (A @ pb.bone.matrix_local.to_3x3().normalized()).inverted()
                q = turn.to_quaternion()
                if q.w < 0.0:
                    q.negate()
                twist_axis = up * float(np.array([q.x, q.y, q.z]) @ up)      # swing-twist about the body's up
                twist = np.array([q.w, *twist_axis])
                twist /= max(float(np.linalg.norm(twist)), 1e-12)
                yaw = 2.0 * math.atan2(float(np.array(twist[1:]) @ up), float(twist[0]))
                swing = np.array(turn) @ _rotation(up, yaw).T
                sq = Matrix(swing.tolist()).to_quaternion()
                angle = sq.angle
                axis = np.array(sq.axis) if angle > 1e-9 else np.zeros(3)
                parts.append((yaw, -angle * float(axis @ tangent), angle * float(axis @ radial)))   # yaw, out, roll
            rig = poses.StripRig(arm, info)
            yaws = [p[0] for p in parts]
            flares = [p[1] for p in parts]
            profile = _profile(yaws, rig) if np.abs(yaws).max() >= MIN_PROFILE else _profile(flares, rig)
            slide = {n: locs[n].copy() for n in info["bones"] if n in locs}
            out[info["bones"][0]] = {"parts": parts, "profile": profile, "lift_profile": _lengths(rig), "slide": slide}
    return out


def _parametric(setup, arm, frame):
    """The reference without a hand-made Open pose: front panels swing round the body (a hinge on the spine), back
    panels flare out, by each column's Opens share."""
    out = {}
    layer = setup.poses.open
    for info in bones_mod.strips(setup, arm):
        theta = info["theta"]
        rig = poses.StripRig(arm, info)
        cum = _lengths(rig)
        opens = poses.column_mult(setup, theta, "open")
        swing = np.minimum(1.0, cum / OPEN_TOP)
        parts = [(_side(theta) * OPEN_YAW * opens * c, OPEN_FLARE * (1.0 - opens) * k, 0.0) for c, k in zip(swing, cum)]
        slide = {}
        if layer.slide > 1e-6:
            slide[rig.names[0]] = poses.slide_location(arm, frame, rig, theta, "open", layer.slide * opens)
        out[info["bones"][0]] = {"parts": parts, "profile": swing if opens > 0.5 else cum, "lift_profile": cum,
                                 "slide": slide}
    return out


def reference(setup, arm, frame):
    layer = setup.poses.open
    if layer.action is not None and poses.layer_is_hand(layer):
        return _measure(setup, arm, frame, poses.read_action(layer.action), poses.read_action_locs(layer.action))
    return _parametric(setup, arm, frame)


class Frame:
    """One generated frame's amounts for the panels on one side. open / back: the front panels' opening and the back
    panels' flare as shares of the reference Open pose. lift_front / lift_back: radians the hems lift, toward `lift_to`
    (forward, left) in the body - where the air goes - or straight out from the body when it is None (a turn). push:
    radians every panel moves round the body toward the left (the air's way); spin: radians they all turn round it
    (the cloth trailing a turn). flare: radians the front panels' hems tilt out from the body, about the panel's own
    outward axis as it stands once opened (the lower panel fanning out wide instead of tucking in at the sides)."""

    def __init__(self, open=1.0, back=1.0, lift_front=0.0, lift_back=0.0, lift_to=None, push=0.0, spin=0.0,
                 blow_back=1.0, roll=None, flare=0.0):
        self.open, self.back, self.lift_front, self.lift_back = open, back, lift_front, lift_back
        self.lift_to, self.push, self.spin = lift_to, push, spin
        self.blow_back = blow_back      # the furthest a front panel may swing round, as a share of the Open pose
        self.roll = roll                # the share of the Open pose's roll; None: as the opening (at most 1)
        self.flare = flare

    def mirrored(self):
        to = None if self.lift_to is None else (self.lift_to[0], -self.lift_to[1])
        return Frame(self.open, self.back, self.lift_front, self.lift_back, to, -self.push, -self.spin, self.blow_back,
                     self.roll, self.flare)


BACKWARD, FORWARD, RIGHTWARD = (-1.0, 0.0), (1.0, 0.0), (0.0, -1.0)


def frame_params(gen, key, step, side):
    """The Frame for frame step + 1 of clip `key`, for the panels on one side (+1 left, -1 right, 0 middle)."""
    lift, sweep, turn = gen.lift, gen.sweep, gen.turn_flare
    full = step == 2
    if key in ("windRight", "turnRight"):           # the left ones mirrored
        return frame_params(gen, key.replace("Right", "Left"), step, -side).mirrored()
    if key == "windFront":                          # opens, then the whole jacket lifts back as one: front and back alike
        return (Frame(gen.blow_back, 1.0, 1.6 * lift, 1.6 * lift, BACKWARD, flare=lift) if full
                else Frame(1.0, 1.0))
    if key == "windBack":                           # subtle (after a hand-made one): the back is pressed on, the front
        k = 1.0 if full else 0.5                    # panels close a little and their hems tilt out
        return Frame(-0.15 * k, 0.0, 0.42 * lift * k, 0.0, None, roll=-0.34 * k)
    if key == "windLeft":                           # the air goes right: the left front is pressed shut, the rest goes
        if side > 0:                                # right and the right front opens
            return Frame(-0.1, 0.3)
        return (Frame(gen.blow_back, 1.4, lift, 0.5 * lift, RIGHTWARD, -sweep) if full
                else Frame(1.0, 1.0, 0.4 * lift, 0.2 * lift, RIGHTWARD, -0.5 * sweep))
    if key == "turnLeft":                           # moderate (after a hand-made turn): the whole jacket turns a little
        k = 1.0 if full else 0.5                    # with the body, the right front panel opens and flies out
        if side > 0:
            return Frame(0.0, 0.4 * k, 0.0, 0.0, None, 0.0, 0.45 * sweep * k)
        return Frame(0.8 * k, 0.4 * k, 1.3 * turn * k, 0.0, None, 0.0, 0.45 * sweep * k)
    raise KeyError(key)


def _limited(gen, frame_params_of):
    f = frame_params_of
    f.blow_back = gen.blow_back
    return f


def gen_blow_back(f):
    return f.blow_back


def strip_turns(ref, frame, theta, f):
    """Each bone's world turn for one strip with a Frame's amounts: the reference parts scaled (as far as the reference
    goes), the panel so shaped carried further round the body as one piece, then the hem lifted the way the air goes.
    The reference's tilt and roll are about the panel's own axes where it opens to; turning round further first would
    leave them about the wrong axes, a different twist on every bone, and fold the panel."""
    front = abs(theta) < math.pi / 2.0
    shape, lift = (f.open, f.lift_front) if front else (f.back, f.lift_back)
    # a push is a turn round the body as seen at this panel: front and back panels turn opposite ways to move alike;
    # the windward front panel (on the side the air comes from) is only pressed shut, it would cross the opening
    windward_front = front and f.push * math.sin(theta) < 0.0
    around = f.spin + (0.0 if windward_front else f.push * math.cos(theta))
    up, radial, tangent = frame.up, frame.radial(theta), frame.around(theta)
    if f.lift_to is None:
        lift_axis = -tangent                        # straight out from the body
    else:
        to = f.lift_to[0] * frame.forward + f.lift_to[1] * frame.left
        lift_axis = np.cross(-up, to)               # turns a hanging bone's tip toward `to`
        lift_axis /= max(float(np.linalg.norm(lift_axis)), 1e-9)
    yaws = [part[0] for part in ref["parts"]]
    peak = max(yaws, key=abs) if yaws else 0.0
    turns = []
    for (yaw, out, roll), c, h in zip(ref["parts"], ref["profile"], ref["lift_profile"]):
        rolled = max(-1.0, min(1.0, shape)) if f.roll is None or not front else f.roll
        swing = -shape * out * tangent + rolled * roll * radial
        further = max(shape - 1.0, 0.0) * peak * c ** BEYOND_OPEN + around * c
        if front and abs(peak) > 1e-6:
            # a front panel never swings round further than Blown back allows: past that it crosses the side seam,
            # where the skin mixes front and back panels, and the hem corner collapses
            room = max(gen_blow_back(f), 1.0) * abs(peak) * c ** BEYOND_OPEN - min(shape, 1.0) * abs(yaw)
            sign = 1.0 if peak > 0.0 else -1.0
            further = sign * min(sign * further, max(room, 0.0))
        # the lift spreads down every panel alike (h: how far down the panel), so panels lift together at their seams
        turn = (_rotation(lift_axis, lift * h) @ _rotation(up, further) @ _from_vector(swing)
                @ _rotation(up, min(shape, 1.0) * yaw))
        if front and f.flare:
            # the hem fans out: tilted away from the body about the panel's own outward axis once it has swung open
            # (its rest axis would tuck the hem in or push it forward, the panel stands turned by now)
            axis = _rotation(up, min(shape, 1.0) * yaw) @ -tangent
            turn = _rotation(axis, f.flare * max(0.0, (h - FLARE_FROM) / (1.0 - FLARE_FROM))) @ turn
        turns.append(turn)
    return turns, shape


def clip_frames_for(setup, arm, key, frame=None, ref=None):
    """[{bone: (rotation, location)}] for frames 2 and 3 of clip `key`."""
    frame = frame or frames.compute_frame(arm, setup.character)
    ref = ref or reference(setup, arm, frame)
    out = []
    for step in (1, 2):
        bases = {}
        for info in bones_mod.strips(setup, arm):
            r = ref.get(info["bones"][0])
            if r is None:
                continue
            theta = info["theta"]
            f = _limited(setup.poses.gen, frame_params(setup.poses.gen, key, step, _side(theta)))
            turns, shape = strip_turns(r, frame, theta, f)
            rig = poses.StripRig(arm, info)
            quats = poses.to_basis(arm, rig, turns)
            slide = max(-1.0, min(1.5, shape))
            for n in info["bones"]:
                bases[n] = (quats[n], r["slide"].get(n, Vector((0.0, 0.0, 0.0))) * slide)
        out.append(bases)
    return out


def generate(setup, arm, force=False):
    """Writes every wind and turn clip. A clip the artist made or changed is kept unless `force`; a right-hand clip set
    to mirror its left one is mirrored from it when that one is the artist's. Returns (written, kept) slot keys."""
    frame = frames.compute_frame(arm, setup.character)
    ref = reference(setup, arm, frame)
    written, kept = [], []
    hand = set()
    for key in ec.CLIP_SLOTS:
        act = poses.clip_slot(setup, key).action
        if act is not None and poses.clip_is_hand(act):
            hand.add(key)
        mirror_of = poses.MIRROR_OF.get(key)
        if mirror_of in hand and poses.clip_slot(setup, key).mirrored and not (key in hand and not force):
            poses.mirror_clip(setup, arm, key)
            written.append(key)
            continue
        if key in hand and not force:
            kept.append(key)
            continue
        poses.write_clip(setup, arm, key, clip_frames_for(setup, arm, key, frame, ref))
        written.append(key)
    return written, kept
