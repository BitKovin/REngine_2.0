"""Viewport overlays: lines for the top/waist/hem, ghost bones of the planned layout, leg capsules (DESIGN.md 8.3.1).

The planned layout is recomputed a moment after a setting changes (a timer), never inside the draw callback."""
import math

import bpy
import gpu
import numpy as np
from gpu_extras.batch import batch_for_shader

_dirty = True
_timer_set = False
_handle = None
_plan = {}          # setup id -> ghost segments [(a, b)] and roots
_geom = {}          # setup id -> (key, ClothGeom)


def tag_dirty():
    global _dirty, _timer_set
    _dirty = True
    if not _timer_set and bpy.app.timers is not None:
        _timer_set = True
        bpy.app.timers.register(_recompute, first_interval=0.3)


def _active_setup():
    from . import ops, props
    ctx = bpy.context
    try:
        arm = ops.find_armature(ctx)
    except Exception:
        return None, None
    return arm, props.get_setup(arm) if arm else None


def _recompute():
    global _dirty, _timer_set
    _timer_set = False
    arm, setup = _active_setup()
    if setup is None or not setup.show_overlay or not _dirty:
        return None
    _dirty = False
    try:
        from . import guides, pipeline, stages
        status = stages.status(setup, arm, "bones")
        if status == "ok" or not setup.cloth_objects:
            _plan.pop(setup.id, None)
        else:
            frame = pipeline.frame_of(setup, arm)
            key = (setup.id, tuple((r.obj.name, len(r.obj.data.vertices)) for r in setup.cloth_objects if r.obj),
                   setup.weights.exclude_arms, round(setup.weights.exclude_threshold, 3))
            cached = _geom.get(setup.id)
            if cached is None or cached[0] != key:
                _geom[setup.id] = (key, guides.ClothGeom(setup, arm))
            geom = _geom[setup.id][1]
            plan = guides.make_plan(setup, arm, frame, geom)
            segs = [(b.head, b.tail) for _, _, bl in plan.strips for b in bl]
            segs += [(b.head, b.tail) for b in plan.roots]
            _plan[setup.id] = segs
    except Exception as e:      # an overlay must never break the session
        print("[Bone Cloth] overlay:", e)
    for window in bpy.context.window_manager.windows:
        for area in window.screen.areas:
            if area.type == 'VIEW_3D':
                area.tag_redraw()
    return None


def _lines(points_pairs, color, width, region):
    if not points_pairs:
        return
    shader = gpu.shader.from_builtin('POLYLINE_UNIFORM_COLOR')
    coords = []
    for a, b in points_pairs:
        coords.append(tuple(float(x) for x in a))
        coords.append(tuple(float(x) for x in b))
    batch = batch_for_shader(shader, 'LINES', {"pos": coords})
    shader.bind()
    shader.uniform_float("viewportSize", (region.width, region.height))
    shader.uniform_float("lineWidth", width)
    shader.uniform_float("color", color)
    batch.draw(shader)


def _ring(centre, radius, z, n=48):
    pts = []
    for k in range(n):
        a0, a1 = 2 * math.pi * k / n, 2 * math.pi * (k + 1) / n
        pts.append(((centre[0] + radius * math.cos(a0), centre[1] + radius * math.sin(a0), z),
                    (centre[0] + radius * math.cos(a1), centre[1] + radius * math.sin(a1), z)))
    return pts


def _capsule(a, b, r0, r1, n=16):
    ab = b - a
    length = np.linalg.norm(ab)
    if length < 1e-6:
        return []
    d = ab / length
    u = np.cross(d, [1.0, 0.0, 0.0])
    if np.linalg.norm(u) < 1e-3:
        u = np.cross(d, [0.0, 1.0, 0.0])
    u /= np.linalg.norm(u)
    v = np.cross(d, u)
    out = []
    for centre, r in ((a, r0), (b, r1)):
        for k in range(n):
            a0, a1 = 2 * math.pi * k / n, 2 * math.pi * (k + 1) / n
            out.append((centre + r * (math.cos(a0) * u + math.sin(a0) * v), centre + r * (math.cos(a1) * u + math.sin(a1) * v)))
    for k in range(0, n, n // 4):
        ang = 2 * math.pi * k / n
        out.append((a + r0 * (math.cos(ang) * u + math.sin(ang) * v), b + r1 * (math.cos(ang) * u + math.sin(ang) * v)))
    return out


def _draw():
    try:
        arm, setup = _active_setup()
        if setup is None or not setup.show_overlay:
            return
        from . import colliders, frames, pipeline
        region = bpy.context.region
        if region is None:
            return
        gpu.state.blend_set('ALPHA')
        gpu.state.depth_test_set('NONE')
        frame = pipeline.frame_of(setup, arm)
        g = setup.guides
        c = frame.origin
        for z, color in ((g.top_line, (0.3, 0.8, 1.0, 0.8)), (g.waist_line, (1.0, 0.8, 0.2, 0.8)),
                         (g.hem_line, (1.0, 0.4, 0.4, 0.8))):
            _lines(_ring(c, 0.32, z), color, 1.5, region)
        ghost = _plan.get(setup.id)
        if ghost:
            _lines(ghost, (0.2, 1.0, 0.9, 0.9), 3.0, region)
        caps = []
        for a, b, r0, r1 in colliders.capsules(setup, arm):
            caps.extend(_capsule(a, b, r0, r1))
        _lines(caps, (1.0, 0.55, 0.1, 0.8), 1.5, region)
        gpu.state.blend_set('NONE')
    except Exception:
        pass


def register():
    global _handle
    _handle = bpy.types.SpaceView3D.draw_handler_add(_draw, (), 'WINDOW', 'POST_VIEW')


def unregister():
    global _handle
    if _handle is not None:
        bpy.types.SpaceView3D.draw_handler_remove(_handle, 'WINDOW')
        _handle = None
