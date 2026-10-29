"""Run: python -I tests/test_engine_contract.py   (plain asserts, needs numpy)"""
import os
import sys

import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
import engine_contract as ec  # noqa: E402


def mat(head, rot=None):
    m = np.eye(4)
    if rot is not None:
        m[:3, :3] = rot
    m[:3, 3] = head
    return m


def down_rot():
    """Bone whose local +Y points world -Z (hanging down), like a Blender bone laid out downwards."""
    # local X -> world X, local Y -> world -Z, local Z -> world +Y
    return np.array([[1, 0, 0], [0, 0, 1], [0, -1, 0]], float)


def coat(top2_x=0.0):
    """root (index 0) -> upper (1) -> ; root2 (2) -> lower (3) -> lower2 (4). upper ends at (0,0,1.0); lower head at (top2_x,0,1.0)."""
    R = down_rot()
    names = ["root_up", "upper", "root_low", "lower", "lower2"]
    parent = [-1, 0, -1, 2, 3]
    rest = [mat((0, 0, 1.4)), mat((0, 0, 1.4), R), mat((0, 0, 1.0)), mat((top2_x, 0, 1.0), R), mat((top2_x, 0, 0.7), R)]
    return ec.Skeleton(names, parent, rest)


def verts(bones_weights):
    pos, b, w = [], [], []
    for p, bw in bones_weights:
        pos.append(p)
        bb = [x for x, _ in bw] + [-1] * (4 - len(bw))
        ww = [y for _, y in bw] + [0.0] * (4 - len(bw))
        b.append(bb)
        w.append(ww)
    return ec.Vertices(pos, b, w)


def test_join_exact_and_tip():
    sk = coat()
    v = verts([((0, 0, 1.2), [(1, 1.0)]), ((0, 0, 0.8), [(3, 1.0)]), ((0, 0, 0.5), [(4, 1.0)])])
    strips = ec.build_strips(sk, v, roots=[0, 2], forward=(0, -1, 0), up=(0, 0, 1))
    assert len(strips) == 1, len(strips)
    assert strips[0].bones == [1, 3, 4]
    # tip: last bone head at z=0.7, vertex at 0.5 -> 0.2 below
    assert abs(strips[0].tip_length - 0.2) < 1e-9, strips[0].tip_length


def test_join_tolerance():
    # offset 4 % of the 0.0.. distance joins, 6 % does not; the upper bone is 0.4 long
    for offset, joins in ((0.4 * 0.04, True), (0.4 * 0.06, False)):
        sk = coat(top2_x=offset)
        v = verts([((0, 0, 1.2), [(1, 1.0)]), ((offset, 0, 0.8), [(3, 1.0)]), ((offset, 0, 0.5), [(4, 1.0)])])
        strips = ec.build_strips(sk, v, roots=[0, 2], forward=(0, -1, 0), up=(0, 0, 1))
        assert (len(strips) == 1) == joins, (offset, len(strips))


def test_empty_bone_truncates():
    sk = coat()
    v = verts([((0, 0, 1.2), [(1, 1.0)]), ((0, 0, 0.8), [(3, 1.0)])])   # lower2 has no vertices
    strips = ec.build_strips(sk, v, roots=[0, 2], forward=(0, -1, 0), up=(0, 0, 1))
    assert strips[0].bones == [1, 3]


def test_weight_thresholds():
    sk = coat()
    v = verts([((0, 0, 1.2), [(1, 0.04)]), ((0, 0, 0.8), [(3, 1.0)])])   # 0.04 < 0.05: bone 1 carries nothing
    chains = ec.find_chains(sk, v, [0, 2])
    assert chains == [[3]], chains


def test_tip_fallback_and_side():
    sk = coat()
    # last bone has weight 0.2 (< 0.25) vertices only: tip falls back to the previous segment (0.3)
    v = verts([((0, 0, 1.2), [(1, 1.0)]), ((0, 0, 0.8), [(3, 1.0)]), ((0, 0, 0.4), [(4, 0.2), (3, 0.8)])])
    s = ec.build_strips(sk, v, [0, 2], (0, -1, 0), (0, 0, 1))[0]
    assert abs(s.tip_length - 0.3) < 1e-9, s.tip_length
    assert s.side == 0


def test_side_tolerance():
    R = down_rot()
    names = ["r", "a", "b"]
    sk = ec.Skeleton(names, [-1, 0, 0], [mat((0, 0, 1)), mat((0.10, 0, 1), R), mat((-0.10, 0, 1), R)])
    v = verts([((0.1, 0, 0.8), [(1, 1.0)]), ((-0.1, 0, 0.8), [(2, 1.0)])])
    strips = ec.build_strips(sk, v, [0], (0, -1, 0), (0, 0, 1))
    sides = sorted(s.side for s in strips)
    # forward -Y, up +Z -> left = Z x -Y = +X
    assert sides == [-1, 1], sides


def test_collider_fit_and_fallback():
    R = down_rot()
    names = ["hips", "thigh", "calf", "foot", "twist"]
    parent = [-1, 0, 1, 2, 1]
    rest = [mat((0, 0, 1)), mat((0, 0, 0.9), R), mat((0, 0, 0.5), R), mat((0, 0, 0.1), R), mat((0, 0, 0.7), R)]
    sk = ec.Skeleton(names, parent, rest)
    pts = []
    for z in (0.85, 0.8, 0.75):
        pts.append(((0.08, 0, z), [(1, 1.0)]))        # near the start: thigh dominant
    for z in (0.55, 0.52, 0.6):
        pts.append(((0.05, 0, z), [(1, 1.0)]))        # near the end
    pts.append(((0.07, 0, 0.7), [(4, 1.0)]))          # twist bone (childless leaf under thigh)
    v = verts(pts)
    r0, r1 = ec.fit_collider(sk, v, 1, 2)
    assert abs(r0 - 0.08) < 1e-9 and abs(r1 - 0.05) < 1e-9, (r0, r1)
    # `to` not below `bone`: no vertices qualify -> 0.06 fallback
    r0, r1 = ec.fit_collider(sk, v, 2, 1)
    assert r0 == ec.FALLBACK_COLLIDER_RADIUS and r1 == ec.FALLBACK_COLLIDER_RADIUS
    # given radii are kept
    assert ec.fit_collider(sk, v, 1, 2, 0.1, 0.2) == (0.1, 0.2)


def test_limit_influences():
    b, w = ec.limit_influences([1, 2, 3, 4, 5], [0.1, 0.4, 0.2, 0.05, 0.25])
    assert b == [2, 5, 3, 1] and abs(sum(w) - 1.0) < 1e-12


def test_layer_threshold():
    sk = coat()
    v = verts([((0, 0, 1.2), [(1, 1.0)]), ((0, 0, 0.8), [(3, 1.0)]), ((0, 0, 0.5), [(4, 1.0)])])
    s = ec.build_strips(sk, v, [0, 2], (0, -1, 0), (0, 0, 1))[0]
    assert not ec.layer_uses_strip(s, {1: np.radians(0.4)})
    assert ec.layer_uses_strip(s, {4: np.radians(0.6)})


if __name__ == "__main__":
    n = 0
    for name, fn in list(globals().items()):
        if name.startswith("test_") and callable(fn):
            fn()
            n += 1
            print("ok", name)
    print(n, "tests passed")
