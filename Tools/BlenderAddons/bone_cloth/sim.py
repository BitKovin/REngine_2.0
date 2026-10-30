"""Preview simulator: a port of Engine/Cloth/BoneCloth.cpp (Step, Swing, wind and spin weights) running on the armature in
Blender, so the cloth swings in the viewport the way it will in the game (DESIGN.md section 8.9).

Everything is worked in Blender world space (Z up). The engine's `world` matrix is the armature object's matrix; its model
space matrices are the pose matrices, which include the armature object's scale, so bone-local lengths convert to metres
exactly as they do in the engine. ENGINE_COMMIT in engine_contract.py names the version this follows."""
import math

import bpy
import numpy as np
from mathutils import Matrix, Quaternion, Vector

from . import bones as bones_mod
from . import colliders as colliders_mod
from . import engine_contract as ec
from . import frames, model_data, pipeline, poses

MAX_DELTA_TIME = 1.0 / 15.0
UP = np.array([0.0, 0.0, 1.0])
RUNNING = {}          # armature name -> ClothSim (the frame handler drives these)


# ---- the engine's small helpers ---------------------------------------------------------------------------------------

def smoothstep(a, b, x):
    t = min(max((x - a) / (b - a), 0.0), 1.0)
    return t * t * (3.0 - 2.0 * t)


def ramp(start, full, x):
    """smoothstep that also takes start == full (a step)."""
    return smoothstep(start, full, x) if full > start else (1.0 if x >= start else 0.0)


def perpendicular(v):
    other = np.array([1.0, 0.0, 0.0]) if abs(v[0]) < 0.9 else np.array([0.0, 1.0, 0.0])
    c = np.cross(v, other)
    return c / np.linalg.norm(c)


def unit(v, fallback):
    n = np.linalg.norm(v)
    return v / n if n > 1e-9 else fallback


def axis_angle(axis, angle):
    axis = axis / np.linalg.norm(axis)
    K = np.array([[0, -axis[2], axis[1]], [axis[2], 0, -axis[0]], [-axis[1], axis[0], 0]])
    return np.eye(3) + math.sin(angle) * K + (1.0 - math.cos(angle)) * (K @ K)


def rotation_between(a, b):
    """glm::rotation: the shortest turn taking unit vector a to unit vector b, as a 3x3."""
    d = float(np.dot(a, b))
    if d > 1.0 - 1e-9:
        return np.eye(3)
    if d < -1.0 + 1e-9:
        return axis_angle(perpendicular(a), math.pi)
    return axis_angle(np.cross(a, b), math.acos(d))


def constrain(head, tail, rest_tail, max_angle):
    """The tail kept at the joint's length from its head, within max_angle of its rest direction."""
    rest = rest_tail - head
    length = np.linalg.norm(rest)
    if length < 1e-6:
        return rest_tail
    rest_dir = rest / length
    direction = tail - head
    current = np.linalg.norm(direction)
    direction = direction / current if current > 1e-6 else rest_dir
    if np.dot(direction, rest_dir) < math.cos(max_angle):
        axis = np.cross(rest_dir, direction)
        n = np.linalg.norm(axis)
        axis = axis / n if n > 1e-6 else perpendicular(rest_dir)
        direction = axis_angle(axis, max_angle) @ rest_dir
    return head + direction * length


def axis_parameter(p, a, b):
    axis = b - a
    l2 = float(np.dot(axis, axis))
    return float(np.clip(np.dot(p - a, axis) / l2, 0.0, 1.0)) if l2 > 1e-12 else 0.0


def wind_sample(velocity, gust_strength, gust_hz, position, time):
    """World/WindField.cpp: one base wind with gusts travelling along it."""
    speed = float(np.linalg.norm(velocity))
    if speed < 1e-4 or gust_strength <= 0.0:
        return velocity
    direction = velocity / speed
    travel = float(np.dot(position, direction)) / speed
    phase = (time - travel) * gust_hz * 2.0 * math.pi
    gust = 0.65 * math.sin(phase) + 0.35 * math.sin(phase * 2.37 + 1.3)
    return velocity * (1.0 + gust_strength * gust)


def mat3(m):
    return np.array(m)[:3, :3]


# ---- test motions -----------------------------------------------------------------------------------------------------

MOTIONS = [
    ('ANIMATION', "Animation only", "The armature is left alone (its own animation and root motion)"),
    ('STAND', "Stand still", "The character does not move: only the wind acts"),
    ('WALK', "Walk 1.5 m/s", ""), ('RUN', "Run 5 m/s", ""),
    ('CIRCLE', "Run in a circle", "5 m/s around a 3 m circle: turning all the time"),
    ('SPIN', "Spin 720 deg/s", "Standing, turning fast: the cloth should open"),
    ('STOP_GO', "Stop and go", "Run 1.5 s, stand 1.5 s, again"),
    ('STRAFE', "Strafe", "Sideways at 3 m/s, changing direction every second"),
    ('TELEPORT', "Teleport", "Stands, then jumps 5 m every 2 s: the cloth must snap, not stretch"),
]


def motion_state(kind, t, forward):
    """(displacement vector, yaw radians) of the test motion at time t, from the character's start."""
    fwd = np.array([forward[0], forward[1], 0.0])
    fwd = fwd / max(np.linalg.norm(fwd), 1e-9)
    left = np.cross(UP, fwd)
    if kind == 'WALK':
        return fwd * 1.5 * t, 0.0
    if kind == 'RUN':
        return fwd * 5.0 * t, 0.0
    if kind == 'CIRCLE':
        r, v = 3.0, 5.0
        a = v * t / r
        return fwd * (r * math.sin(a)) + left * (r * (1.0 - math.cos(a))), a
    if kind == 'SPIN':
        return np.zeros(3), math.radians(720.0) * t
    if kind == 'STOP_GO':
        cycle = 3.0
        full, rest = divmod(t, cycle)
        return fwd * 5.0 * (full * 1.5 + min(rest, 1.5)), 0.0
    if kind == 'STRAFE':
        # 1 s legs, alternating: a triangle wave of the position
        phase = (t % 2.0)
        return left * 3.0 * (phase if phase < 1.0 else 2.0 - phase), 0.0
    if kind == 'TELEPORT':
        return fwd * 5.0 * math.floor(t / 2.0), 0.0
    return np.zeros(3), 0.0


# ---- the simulation -----------------------------------------------------------------------------------------------------

class Joint:
    def __init__(self, bone_name, bone, above, tip_local, follow, max_angle):
        self.name, self.bone, self.above = bone_name, bone, above
        self.tip_local = np.asarray(tip_local, float)
        self.follow, self.max_angle = follow, max_angle
        self.probes = []          # (position in the bone's frame, ignore mask)
        self.leg_probes = []      # (position in the bone's frame, per collider how deep it may go into that leg)
        self.along = 0.0
        self.anchors = []         # windowed trailing: (time, where the tail would hang)
        self.lag = np.zeros(3)
        self.head = np.zeros(3)
        self.tail = np.zeros(3)
        self.basis = np.eye(3)


class SimStrip:
    def __init__(self, joints, side):
        self.joints, self.side = joints, side
        self.opens = self.flares = False
        self.open_weight = 0.0


class ClothSim:
    """One BoneCloth (one .skmm cloth entry) on an armature."""

    def __init__(self, setup, arm):
        self.setup, self.arm = setup, arm
        m = setup.motion
        self.data = {k: getattr(m, k) for k in (
            "followRoot", "followTip", "trail", "maxAngleRoot", "maxAngleTip", "radius", "teleportDistance", "tipLength",
            "volume", "openWindStart", "openWindFull", "windwardBias", "volumeWindStart", "volumeWindFull",
            "layerSmoothing", "spinOpen", "spinVolume", "spinStart", "spinFull", "spinRelease", "windStart", "windFull",
            "swing", "collision", "motionWindow", "lagRoot", "lagTip", "physicsFront", "physicsBack", "physicsSides",
            "physicsVertical", "physicsSpin")}
        self.frame = pipeline.frame_of(setup, arm)
        skel, verts = model_data.collect(arm)
        self.skel = skel
        root_ids = [skel.index[r] for r in bones_mod.roots(setup) if r in skel.index]
        fwd = self.frame.flat_forward()
        ec_strips = ec.build_strips(skel, verts, root_ids, fwd, UP, self.data["tipLength"])
        self.roots = root_ids
        self.strips = []
        for s in ec_strips:
            joints = []
            for j in s.joints:
                follow = max(self.data["followRoot"] + (self.data["followTip"] - self.data["followRoot"]) * j.along, 0.0)
                ang = math.radians(min(max(self.data["maxAngleRoot"] + (self.data["maxAngleTip"] - self.data["maxAngleRoot"])
                                           * j.along, 0.0), 180.0))
                joints.append(Joint(skel.names[j.bone], j.bone, j.above, j.tip_local, follow, ang))
                joints[-1].along = j.along
            self.strips.append(SimStrip(joints, s.side))
        # the wind and turn clips; then the older wind layers
        self.clips = poses.clip_sample_sets(setup)
        self.wind_air = np.zeros(2)       # along the facing, along the left
        self.wind_speed = 0.0
        self.turn_amount = 0.0
        layers = not self.clips          # the export leaves the layers out when there are clips
        self.q_open = poses.read_action(setup.poses.open.action) if layers and setup.poses.open.enabled else {}
        self.q_vol = poses.read_action(setup.poses.volume.action) if layers and setup.poses.volume.enabled else {}
        self.l_open = poses.read_action_locs(setup.poses.open.action) if layers and setup.poses.open.enabled else {}
        self.l_vol = poses.read_action_locs(setup.poses.volume.action) if layers and setup.poses.volume.enabled else {}
        for st in self.strips:
            ao = {skel.index[b]: v.angle for b, v in self.q_open.items() if b in skel.index}
            av = {skel.index[b]: v.angle for b, v in self.q_vol.items() if b in skel.index}
            st.opens = any(ao.get(j.bone, 0.0) >= ec.LAYER_MIN_ANGLE for j in st.joints)
            st.flares = any(av.get(j.bone, 0.0) >= ec.LAYER_MIN_ANGLE for j in st.joints)
        # colliders (bone indices, radii as the engine will fit them, surface, the offset ends in their bones' frames)
        self.colliders = []
        colliders_mod.fit_all(setup, arm, (skel, verts))
        rest_caps = []
        for c in setup.colliders:
            if c.enabled and c.bone in skel.index and c.to in skel.index:
                a, b = skel.index[c.bone], skel.index[c.to]
                r0, r1 = colliders_mod.radii(c)
                start, end = skel.head(a) + np.array(c.offset), skel.head(b) + np.array(c.offset)
                self.colliders.append((a, b, r0, r1, c.surface, (np.linalg.inv(skel.rest_world[a]) @ np.append(start, 1.0))[:3],
                                       (np.linalg.inv(skel.rest_world[b]) @ np.append(end, 1.0))[:3]))
                rest_caps.append((start, end, r0, r1, c.surface))
        if any(c[4] for c in self.colliders):
            heads = [skel.head(r) for r in self.roots]
            low, high = min(heads, key=lambda h: h[2]), max(heads, key=lambda h: h[2])
            axis_dir = (high - low) / np.linalg.norm(high - low) if np.linalg.norm(high - low) > 1e-4 else np.array([0.0, 0.0, 1.0])
            for st in self.strips:
                for j in st.joints:
                    to_bone = np.linalg.inv(skel.rest_world[j.bone])
                    picked = ec.pick_probes(skel, verts, j.bone)
                    j.probes = [((to_bone @ np.append(q, 1.0))[:3], ec.probe_ignore(q, rest_caps)) for q in picked]
                    # the legs push the same probes and the joint's tail (the joint below turns about it)
                    tail = (skel.rest_world[j.bone] @ np.append(j.tip_local, 1.0))[:3]
                    j.leg_probes = [((to_bone @ np.append(q, 1.0))[:3], ec.leg_allowance(q, rest_caps, low, axis_dir))
                                    for q in list(picked) + [tail]]
        # the body bones the roots hang from and their ancestors: the spin is read from them
        carry = set()
        for root in self.roots:
            p = skel.parent[root]
            while p >= 0:
                carry.add(p)
                p = skel.parent[p]
        self.spin_bones = sorted(carry)
        self.forward_local = mat3(arm.matrix_world).T @ np.array(fwd) / 1.0   # facing in the object's own axes
        n = np.linalg.norm(self.forward_local)
        self.forward_local = self.forward_local / n if n > 1e-9 else np.array([0.0, -1.0, 0.0])
        self.left_axis_local = None
        self.wind = np.zeros(3)
        self.gust, self.gust_hz = 0.0, 0.4
        self.needs_reset = True
        self.volume_weight = 0.0
        self.spin_weight = 0.0
        self.previous_origin = np.zeros(3)
        self.clock = 0.0
        self.body_track = []      # windowed trailing: (time, the body axis's lowest point)
        self.previous_spin = {}
        self.debug_lines = []
        self.names_needed = set(skel.names[b] for st in self.strips for j in st.joints for b in (j.bone, j.above)
                                if b >= 0) | set(skel.names[r] for r in self.roots)
        self.names_needed |= set(skel.names[b] for b in self.spin_bones)
        for c in self.colliders:
            self.names_needed |= {skel.names[c[0]], skel.names[c[1]]}
        self.cloth_pose_names = [pb.name for pb in arm.pose.bones if "bone_cloth" in pb]
        self.muted = []
        self.start_transform = arm.matrix_basis.copy()

    # ---- control ----
    def reset(self):
        self.needs_reset = True

    def set_wind(self, velocity, gust, gust_hz):
        self.wind = np.asarray(velocity, float)
        self.gust, self.gust_hz = gust, gust_hz

    def mute_follow(self, on):
        """BC Follow constraints keep the strips joined while posing by hand; the simulation does that itself."""
        if on:
            self.muted = []
            for pb in self.arm.pose.bones:
                for c in pb.constraints:
                    if c.name == "BC Follow" and not c.mute:
                        c.mute = True
                        self.muted.append(c)
        else:
            for c in self.muted:
                c.mute = False
            self.muted = []

    # ---- model space ----
    def _read(self):
        """World matrices of the bones the simulation needs, from the evaluated armature."""
        dg = bpy.context.evaluated_depsgraph_get()
        ev = self.arm.evaluated_get(dg)
        mw = np.array(ev.matrix_world)
        self.mw = mw
        self.M = {}
        for name in self.names_needed:
            pb = ev.pose.bones.get(name)
            if pb is not None:
                self.M[self.skel.index[name]] = mw @ np.array(pb.matrix)

    def _rest_shape(self):
        """Strips and roots at their rest transforms, posed by the wind and turn clips, then the wind layers turned by
        their weights."""
        ident = Quaternion((1, 0, 0, 0))
        bases, locs = {}, {}
        clip_pose = {}
        if self.clips:
            air = ec.spin_air(self.wind_air[0], self.wind_air[1], self.data["spinOpen"], self.spin_weight,
                              self.data["windFull"], self.wind_speed)
            wind_position = ramp(self.data["windStart"], self.data["windFull"], float(np.hypot(*air)))
            clip_pose = poses.blend_layers(self.clips, ec.clip_layers(air[0], air[1], wind_position, self.turn_amount,
                                                                      poses.strengths(self.setup)))
        vol_w = self.data["volume"] + self.volume_weight + self.data["spinVolume"] * self.spin_weight
        for st in self.strips:
            ow = st.open_weight + self.data["spinOpen"] * self.spin_weight
            for j in st.joints:
                q, loc = clip_pose.get(j.name, (ident, Vector((0.0, 0.0, 0.0))))
                loc = loc.copy()
                if st.flares and j.name in self.q_vol:
                    wv = min(max(vol_w, 0.0), 1.0)
                    q = ident.slerp(self.q_vol[j.name], wv) @ q
                    loc = loc + self.l_vol.get(j.name, Vector((0, 0, 0))) * wv
                if st.opens and j.name in self.q_open:
                    wo = min(max(ow, 0.0), 1.0)
                    q = ident.slerp(self.q_open[j.name], wo) @ q
                    loc = loc + self.l_open.get(j.name, Vector((0, 0, 0))) * wo
                bases[j.name] = q
                if loc.length > 0:
                    locs[j.name] = loc
        poses.set_bases(self.arm, bases, locs)
        bpy.context.view_layer.update()

    # ---- wind and spin weights ----
    def _spin_rate(self, dt):
        fastest = 0.0
        for b in self.spin_bones:
            m = self.M.get(b)
            if m is None:
                continue
            r = m[:3, :3].copy()
            for k in range(3):
                n = np.linalg.norm(r[:, k])
                if n > 1e-8:
                    r[:, k] /= n
            q = Matrix(r.tolist()).to_quaternion()
            q.normalize()
            if not self.needs_reset and dt > 0.0 and b in self.previous_spin:
                change = q @ self.previous_spin[b].inverted()
                if change.w < 0.0:
                    change = Quaternion((-change.w, -change.x, -change.y, -change.z))
                angle = 2.0 * math.acos(min(change.w, 1.0))
                axis = np.array([change.x, change.y, change.z])
                n = np.linalg.norm(axis)
                rate = angle * axis[2] / n / dt if n > 1e-6 else 0.0          # the yaw about the world's up, > 0 left
                if abs(rate) > abs(fastest):
                    fastest = rate
            self.previous_spin[b] = q
        return fastest

    def _wind_weights(self, dt, time, origin, rotation):
        d = self.data
        layers = bool(self.q_open or self.q_vol)
        if not layers and not self.clips:
            return
        blend = 1.0 - math.exp(-dt / max(d["layerSmoothing"], 1e-3)) if dt > 0.0 else 0.0
        turn_clips = "turnLeft" in self.clips or "turnRight" in self.clips
        if d["spinOpen"] > 0.0 or d["spinVolume"] > 0.0 or turn_clips:
            rate = self._spin_rate(dt)
            if self.needs_reset:
                self.spin_weight = 0.0
                self.turn_amount = 0.0
            elif dt > 0.0:
                wanted = ramp(math.radians(d["spinStart"]), math.radians(d["spinFull"]), abs(rate))
                release = 1.0 - math.exp(-dt / max(d["spinRelease"], 1e-3))
                self.spin_weight += (wanted - self.spin_weight) * (blend if wanted > self.spin_weight else release)
                turn = wanted if rate >= 0.0 else -wanted
                growing = abs(turn) > abs(self.turn_amount) and turn * self.turn_amount >= 0.0
                self.turn_amount += (turn - self.turn_amount) * (blend if growing else release)
        velocity = (origin - self.previous_origin) / dt if dt > 0.0 else np.zeros(3)
        rel = wind_sample(self.wind, self.gust, self.gust_hz, origin, time) - velocity
        fwd = rotation @ self.forward_local
        left = np.cross(UP, fwd)
        headwind = max(0.0, -float(np.dot(rel, fwd)))
        leftward = float(np.dot(rel, left))
        if self.clips:
            air = np.array([float(np.dot(rel, fwd)), leftward])
            self.wind_air = air if self.needs_reset else self.wind_air + (air - self.wind_air) * blend
            speed = float(np.linalg.norm(air))
            self.wind_speed = speed if self.needs_reset else self.wind_speed + (speed - self.wind_speed) * blend
        if not layers:
            return
        self.volume_weight += (ramp(d["volumeWindStart"], d["volumeWindFull"], float(np.linalg.norm(rel))) - self.volume_weight) * blend
        for st in self.strips:
            windward = max(0.0, -st.side * leftward)
            wanted = ramp(d["openWindStart"], d["openWindFull"], headwind + d["windwardBias"] * windward)
            st.open_weight += (wanted - st.open_weight) * blend

    # ---- the swing ----
    def _swing(self, dt, snap, origin):
        d = self.data
        moved = origin - self.previous_origin
        carry = moved * ((1.0 - min(max(d["trail"], 0.0), 1.0)) if dt > 0.0 else 1.0)
        swing = min(max(d["swing"], 0.0), 1.0)
        collision = min(max(d["collision"], 0.0), 1.0)
        window = d["motionWindow"]
        self.clock += dt
        # the body's axis (through the roots' heads) and its facing, for the windowed trail
        heads = [self.M[r][:3, 3] for r in self.roots if r in self.M]
        axis_low = min(heads, key=lambda h: h[2]) if heads else np.zeros(3)
        axis_high = max(heads, key=lambda h: h[2]) if heads else np.zeros(3)
        facing = self.mw[:3, :3].copy()
        for k in range(3):
            facing[:, k] /= max(np.linalg.norm(facing[:, k]), 1e-12)
        fwd_l = self.forward_local
        up_l = np.array([0.0, 0.0, 1.0]) @ facing          # world up in the object's axes
        left_l = np.cross(up_l, fwd_l)
        axis = axis_high - axis_low
        axis_dir = axis / np.linalg.norm(axis) if float(axis @ axis) > 1e-8 else np.array([0.0, 0.0, 1.0])
        body_velocity = np.zeros(3)                         # the body's own movement over the window
        if window > 0.0:
            if snap:
                self.body_track = []
            if dt > 0.0 or not self.body_track:
                self.body_track.append((self.clock, axis_low.copy()))
            while len(self.body_track) > 2 and self.body_track[1][0] <= self.clock - window:
                self.body_track.pop(0)
            span = self.clock - self.body_track[0][0]
            body_velocity = (axis_low - self.body_track[0][1]) / span if span > 1e-4 else np.zeros(3)
        cs = [((self.M[a] @ np.append(sl, 1.0))[:3], (self.M[b] @ np.append(el, 1.0))[:3], r0, r1, surface)
              for a, b, r0, r1, surface, sl, el in self.colliders if a in self.M and b in self.M]
        self.debug_lines = []
        for st in self.strips:
            head = self.M[st.joints[0].bone][:3, 3].copy()
            above_rest = np.eye(3)
            above_now = np.eye(3)
            for k, j in enumerate(st.joints):
                rest_model = self.M[j.bone][:3, :3]
                rest_basis = rest_model if k == 0 else above_now @ np.linalg.inv(above_rest) @ rest_model
                rest_tail = head + rest_basis @ j.tip_local
                if window > 0.0:
                    # windowed trailing: the average speed of the spot the tail hangs from over the window
                    anchor = (self.M[j.bone] @ np.append(j.tip_local, 1.0))[:3]
                    if snap:
                        j.anchors, j.lag = [], np.zeros(3)
                    if dt > 0.0 or not j.anchors:
                        j.anchors.append((self.clock, anchor))
                    while len(j.anchors) > 2 and j.anchors[1][0] <= self.clock - window:
                        j.anchors.pop(0)
                    t0, p0 = j.anchors[0]
                    span = self.clock - t0
                    velocity = (anchor - p0) / span if span > 1e-4 else np.zeros(3)
                    out = rest_tail - axis_low                      # out from the body's axis, and around it
                    out = out - axis_dir * float(out @ axis_dir)
                    out = out / np.linalg.norm(out) if np.linalg.norm(out) > 1e-6 else np.zeros(3)
                    around = np.cross(axis_dir, out)
                    spin = around * float((velocity - body_velocity) @ around)
                    local = facing.T @ (velocity - spin)
                    along, side, up = float(local @ fwd_l), float(local @ left_l), float(local @ up_l)
                    scaled = (fwd_l * along * (d["physicsFront"] if along >= 0.0 else d["physicsBack"])
                              + left_l * side * d["physicsSides"] + up_l * up * d["physicsVertical"])
                    wanted = -(facing @ scaled + spin * d["physicsSpin"]) * (d["lagRoot"] + (d["lagTip"] - d["lagRoot"]) * j.along)
                    wanted = wanted - out * min(float(wanted @ out), 0.0)     # never toward the body's axis
                    if dt > 0.0:
                        j.lag = j.lag + (wanted - j.lag) * (1.0 - math.exp(-j.follow * dt))
                    tail = rest_tail + j.lag * swing
                else:
                    if snap:
                        tail = rest_tail
                    else:
                        tail = (j.tail + carry) * math.exp(-j.follow * dt) + rest_tail * (1.0 - math.exp(-j.follow * dt))
                    tail = rest_tail + (tail - rest_tail) * swing      # swing 0: the pose exactly
                tail = constrain(head, tail, rest_tail, j.max_angle)
                for a, b, r0, r1, surface in cs:
                    if surface or collision <= 0.0:
                        continue
                    t = axis_parameter(tail, a, b)
                    nearest = a + (b - a) * t
                    radius = r0 + (r1 - r0) * t + d["radius"]
                    off = tail - nearest
                    dist = float(np.linalg.norm(off))
                    if dist < radius and dist > 1e-6:
                        tail = tail + (nearest + off * (radius / dist) - tail) * collision
                j.tail = constrain(head, tail, rest_tail, j.max_angle)
                frm, to = rest_tail - head, j.tail - head
                j.basis = rest_basis
                if np.dot(frm, frm) > 1e-12 and np.dot(to, to) > 1e-12:
                    j.basis = rotation_between(frm / np.linalg.norm(frm), to / np.linalg.norm(to)) @ rest_basis
                # the legs: the cloth of this joint and of those below it (hanging off it as posed) kept on the legs'
                # outer side, turning this joint by its share of each push - the joints from here to the probe's own
                # split it, so the strip bends over a thigh along its length
                legs = [c for c, cap in enumerate(cs) if not cap[4]]
                if legs and collision > 0.0 and any(m.leg_probes for m in st.joints[k:]):
                    to_k = np.linalg.inv(self.M[j.bone])
                    pts = [((to_k @ (self.M[m.bone] @ np.append(local, 1.0)))[:3], allowance, n + 1)
                           for n, m in enumerate(st.joints[k:]) for local, allowance in m.leg_probes]
                    for _ in range(ec.LEG_ITERATIONS):
                        deepest, probe_from, probe_to, share = 0.0, None, None, 1.0
                        for local, allowance, joints in pts:
                            point = head + j.basis @ local
                            for c in legs:
                                if allowance[c] is None:
                                    continue
                                a, b, r0, r1, _ = cs[c]
                                to = ec.leg_push(point, a, b, r0, r1, axis_low, axis_dir, allowance[c])
                                if to is not None and float(np.linalg.norm(to - point)) > deepest:
                                    deepest, probe_from, probe_to = float(np.linalg.norm(to - point)), point - head, to - head
                                    share = collision / joints
                        if probe_from is None or np.dot(probe_from, probe_from) < 1e-12 or np.dot(probe_to, probe_to) < 1e-12:
                            break
                        a_, b_ = probe_from / np.linalg.norm(probe_from), probe_to / np.linalg.norm(probe_to)
                        axis = np.cross(a_, b_)
                        turn = (axis_angle(axis, math.acos(min(max(float(a_ @ b_), -1.0), 1.0)) * share)
                                if np.linalg.norm(axis) > 1e-9 else np.eye(3))
                        j.basis = turn @ j.basis
                        j.tail = head + turn @ (j.tail - head)
                # the cloth it carries kept out of the surface colliders, turning the joint about its head
                for _ in range(ec.PROBE_ITERATIONS if j.probes and collision > 0.0 else 0):
                    deepest, probe_from, probe_to = 0.0, None, None
                    for local, ignore in j.probes:
                        point = head + j.basis @ local
                        for c, (a, b, r0, r1, surface) in enumerate(cs):
                            if not surface or ignore & (1 << c):
                                continue
                            depth, nearest = ec.capsule_depth(point, a, b, r0, r1)
                            off = point - nearest
                            dist = float(np.linalg.norm(off))
                            if depth > deepest and dist > 1e-6:
                                deepest, probe_from = depth, point - head
                                probe_to = nearest + off * ((depth + dist) / dist) - head
                    if probe_from is None or np.dot(probe_from, probe_from) < 1e-12 or np.dot(probe_to, probe_to) < 1e-12:
                        break
                    a_, b_ = probe_from / np.linalg.norm(probe_from), probe_to / np.linalg.norm(probe_to)
                    axis = np.cross(a_, b_)
                    turn = (axis_angle(axis, math.acos(min(max(float(a_ @ b_), -1.0), 1.0)) * collision)
                            if np.linalg.norm(axis) > 1e-9 else np.eye(3))
                    j.basis = turn @ j.basis
                    j.tail = head + turn @ (j.tail - head)
                j.head = head
                self.debug_lines.append((head.copy(), j.tail.copy()))
                above_rest, above_now = rest_model, j.basis
                head = j.tail
        self.capsules = [c[:4] for c in cs]

    def _write(self):
        """Each joint's swung transform as a pose basis (parents first), so the armature modifier shows it."""
        arm = self.arm
        mwi = np.linalg.inv(self.mw)
        done = {}
        for st in self.strips:
            for j in st.joints:
                t_world = np.eye(4)
                t_world[:3, :3] = j.basis
                t_world[:3, 3] = j.head
                t_arm = mwi @ t_world
                bone = arm.data.bones[j.name]
                parent = bone.parent
                if parent is None:
                    p_pose = np.eye(4)
                    rest_rel = np.array(bone.matrix_local)
                else:
                    pidx = self.skel.index[parent.name]
                    p_pose = done.get(parent.name)
                    if p_pose is None:
                        p_pose = mwi @ self.M[pidx]
                    rest_rel = np.linalg.inv(np.array(parent.matrix_local)) @ np.array(bone.matrix_local)
                basis = np.linalg.inv(rest_rel) @ np.linalg.inv(p_pose) @ t_arm
                done[j.name] = t_arm
                arm.pose.bones[j.name].matrix_basis = Matrix(basis.tolist())

    # ---- one frame ----
    def step(self, dt, time=0.0):
        """Advances the cloth by dt seconds. Returns nothing; the pose bones hold the result."""
        dt = min(max(dt, 0.0), MAX_DELTA_TIME)
        self._read()
        origin = self.mw[:3, 3].copy()
        rotation = self.mw[:3, :3].copy()
        for k in range(3):
            n = np.linalg.norm(rotation[:, k])
            if n > 1e-8:
                rotation[:, k] /= n
        if self.needs_reset:
            self.previous_origin = origin
        snap = self.needs_reset or float(np.linalg.norm(origin - self.previous_origin)) > self.data["teleportDistance"]
        self._wind_weights(dt, time, origin, rotation)
        self._rest_shape()
        self._read()
        self._swing(dt, snap, origin)
        self._write()
        self.previous_origin = origin
        self.needs_reset = False


# ---- driving it from the timeline -----------------------------------------------------------------------------------

def start(setup, arm, scene):
    stop(arm)
    sim = ClothSim(setup, arm)
    sim.mute_follow(True)
    sim.reset()
    pv = setup.preview
    sim.preview = pv
    sim.scene_start = scene.frame_current
    sim.last_frame = None
    sim.start_transform = arm.matrix_basis.copy()
    sim.start_forward = sim.frame.flat_forward().copy()
    RUNNING[arm.name] = sim
    if _handler not in bpy.app.handlers.frame_change_post:
        bpy.app.handlers.frame_change_post.append(_handler)
    return sim


def stop(arm):
    sim = RUNNING.pop(arm.name, None)
    if sim is None:
        return
    sim.mute_follow(False)
    for pb in arm.pose.bones:
        if "bone_cloth" in pb:
            pb.matrix_basis = Matrix.Identity(4)
    if sim.preview.motion not in ('ANIMATION',):
        arm.matrix_basis = sim.start_transform
    if not RUNNING and _handler in bpy.app.handlers.frame_change_post:
        bpy.app.handlers.frame_change_post.remove(_handler)
    bpy.context.view_layer.update()


def apply_motion(sim, arm, t):
    pv = sim.preview
    if pv.motion == 'ANIMATION':
        return
    disp, yaw = motion_state(pv.motion, t, sim.start_forward)
    m0 = sim.start_transform
    loc = Vector(m0.to_translation()) + Vector(tuple(float(x) for x in disp))
    rot = Matrix.Rotation(yaw, 4, 'Z') @ m0.to_quaternion().to_matrix().to_4x4()       # rotation only: the scale is added below
    scale = m0.to_scale()
    arm.matrix_basis = Matrix.Translation(loc) @ rot @ Matrix.Diagonal(Vector((scale.x, scale.y, scale.z, 1.0)))


def wind_world(sim, arm):
    pv = sim.preview
    fwd = sim.start_forward
    ang = pv.wind_from                        # radians (an angle property): 0 from the front, pi/2 from the left
    from_dir = math.cos(ang) * fwd + math.sin(ang) * np.cross(UP, fwd)
    return -from_dir * pv.wind_speed


def advance(sim, arm, scene):
    """One timeline frame of the simulation (frame handler, or called directly by tests)."""
    frame = scene.frame_current
    fps = scene.render.fps / scene.render.fps_base
    if sim.last_frame is None or frame < sim.last_frame or frame - sim.last_frame > 3:
        sim.reset()
        dt = 0.0
    else:
        dt = (frame - sim.last_frame) / fps
    sim.last_frame = frame
    t = (frame - sim.scene_start) / fps
    apply_motion(sim, arm, t)
    bpy.context.view_layer.update()
    sim.set_wind(wind_world(sim, arm), sim.preview.gust, sim.preview.gust_hz)
    sim.step(dt, t)


@bpy.app.handlers.persistent
def _handler(scene, depsgraph=None):
    for name, sim in list(RUNNING.items()):
        arm = bpy.data.objects.get(name)
        if arm is None:
            RUNNING.pop(name, None)
            continue
        try:
            advance(sim, arm, scene)
        except Exception as e:          # a preview must never break the timeline
            print("[Bone Cloth] preview stopped:", e)
            stop(arm)
