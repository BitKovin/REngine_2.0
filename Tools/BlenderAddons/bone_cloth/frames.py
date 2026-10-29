"""Body frame and space conversions (DESIGN.md section 5). Geometry is worked in Blender world space, Z up, metres."""
import math

import numpy as np
from mathutils import Vector


def v3(v):
    return np.array([v[0], v[1], v[2]], dtype=float)


def bone_head(arm, name):
    return v3(arm.matrix_world @ arm.data.bones[name].head_local)


def bone_tail(arm, name):
    return v3(arm.matrix_world @ arm.data.bones[name].tail_local)


def arm_scale(arm):
    s = arm.matrix_world.to_scale()
    return (s.x + s.y + s.z) / 3.0


def world_to_arm(arm, p):
    return arm.matrix_world.inverted() @ Vector((float(p[0]), float(p[1]), float(p[2])))


def arm_to_world(arm, p):
    return v3(arm.matrix_world @ Vector((float(p[0]), float(p[1]), float(p[2]))))


class BodyFrame:
    def __init__(self, origin, up, forward):
        self.origin = np.asarray(origin, float)
        self.up = np.asarray(up, float)
        f = np.asarray(forward, float)
        f = f - self.up * np.dot(f, self.up)
        n = np.linalg.norm(f)
        self.forward = f / n if n > 1e-8 else np.array([0.0, -1.0, 0.0])
        self.left = np.cross(self.up, self.forward)

    def flat_forward(self):
        f = np.array([self.forward[0], self.forward[1], 0.0])
        n = np.linalg.norm(f)
        return f / n if n > 1e-8 else self.forward

    def radial(self, theta):
        """Horizontal direction at angle theta (radians): 0 front, +pi/2 the character's left."""
        return math.cos(theta) * self.forward + math.sin(theta) * self.left

    def around(self, theta):
        """Direction of increasing theta."""
        return -math.sin(theta) * self.forward + math.cos(theta) * self.left

    def angle(self, p):
        """Angle around the body axis of point(s) p (n,3) or (3,)."""
        d = np.asarray(p, float) - self.origin
        return np.arctan2(d @ self.left, d @ self.forward)

    def to_engine(self, p):
        """Blender world (x, y, z) -> engine model space (x, z, -y)."""
        p = np.asarray(p, float)
        return np.array([p[0], p[2], -p[1]])


FACING = {'NEG_Y': (0, -1, 0), 'POS_Y': (0, 1, 0), 'NEG_X': (-1, 0, 0), 'POS_X': (1, 0, 0)}


def compute_frame(arm, character):
    """Body frame from the detected landmarks."""
    hips = bone_head(arm, character.hips)
    chest = bone_head(arm, character.chest) if character.chest in arm.data.bones else hips + np.array([0, 0, 0.4])
    axis = chest - hips
    n = np.linalg.norm(axis)
    up = np.array([0.0, 0.0, 1.0])
    if n > 1e-6:
        up = 0.5 * (axis / n) + 0.5 * up
        up /= np.linalg.norm(up)

    if character.facing != 'AUTO':
        forward = np.array(FACING[character.facing], float)
    else:
        forward = np.zeros(3)
        for side in ('l', 'r'):
            foot, ball = getattr(character, 'foot_' + side), getattr(character, 'ball_' + side)
            if foot in arm.data.bones and ball in arm.data.bones:
                forward += bone_head(arm, ball) - bone_head(arm, foot)
        forward[2] = 0.0
        if np.linalg.norm(forward) < 1e-6:
            forward = np.array([0.0, -1.0, 0.0])
    if character.flip:
        forward = -forward
    return BodyFrame(hips, up, forward)
