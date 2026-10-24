"""SimGame map kit: writes level-editor prefabs and maps (schema 0.2: metres, Y up, north = -Z, east = +X) from Python.

Real human scale (door 1.0 x 2.1, storey 2.8, street 5). A prefab is an ordinary map file: its local frame has the origin at the middle of its footprint on the floor and its front
(the side with the door, or the side a user stands on) towards +Z. A prefab instance with yaw 90 turns +Z to +X (the engine / editor convention, forward = Ry(yaw) * (0,0,1)).

Brush rules of the editor: func_group / worldspawn brushes are closed convex solids (boxes, hulls); func_detail and other brush entities may be anything. Everything here is built
additively (no subtractive brushes), so a prefab never depends on carving across files.

The editor's own kit (mapgen.Map, mansion.hull) is used, not copied: set LEVEL_KIT to its scripts folder if it is not at the default place.
"""
import json
import math
import os
import sys

KIT = os.environ.get("LEVEL_KIT", r"F:\Git\mapeditor\.claude\skills\level-blockout\scripts")
sys.path.insert(0, KIT)
os.environ.setdefault("LEVEL_OUT", ".")
from mapgen import Map          # noqa: E402  box brushes, doc skeleton
from mansion import hull as _hull   # noqa: E402  convex brush from points

GAME_DATA = os.environ.get("SIM_GAMEDATA", r"F:\Git\SdlWasmEngine\source\Games\SimGame\GameData")


class _Tmp:
    def __init__(self, n):
        self.world, self.detail, self.n = [], [], n


def yaw_xz(yaw, lx, lz):
    """Where the local point (lx, lz) lands for an instance with that yaw (degrees), relative to the instance origin."""
    r = math.radians(yaw)
    return (lx * math.cos(r) + lz * math.sin(r), -lx * math.sin(r) + lz * math.cos(r))


class P(Map):
    """One prefab (or map) file under construction."""

    def __init__(self):
        super().__init__()
        self.ents, self.insts = [], []

    # --- brushes -------------------------------------------------------------------------------------------------
    def box(self, mn, mx, tex, detail=False):
        (self.det if detail else self.mass)(tuple(mn), tuple(mx), tex)

    def hull(self, pts, tex, detail=False):
        t = _Tmp(self.n)
        _hull(t, pts, tex, detail)
        self.n = t.n
        (self.detail if detail else self.world).extend(t.detail if detail else t.world)

    def prism(self, cx, cz, r, y0, y1, sides, tex, detail=False, rot=0.0):
        ring = lambda y: [(cx + r * math.cos(rot + 2 * math.pi * i / sides), y, cz + r * math.sin(rot + 2 * math.pi * i / sides)) for i in range(sides)]
        self.hull(ring(y0) + ring(y1), tex, detail)

    def cone(self, cx, cz, r, y0, y1, sides, tex, detail=False):
        ring = [(cx + r * math.cos(2 * math.pi * i / sides), y0, cz + r * math.sin(2 * math.pi * i / sides)) for i in range(sides)]
        self.hull(ring + [(cx, y1, cz)], tex, detail)

    def brush_box(self, mn, mx, tex):
        """A box brush for a brush entity (not added to the prefab's own geometry)."""
        return self._brush(tuple(mn), tuple(mx), tex)

    def brush_hull(self, pts, tex):
        t = _Tmp(self.n)
        _hull(t, pts, tex, False)
        self.n = t.n
        return t.world[0]

    # --- entities ------------------------------------------------------------------------------------------------
    def ent(self, cls, eid, origin=(0, 0, 0), yaw=0.0, brushes=None, refs=None, **kv):
        """A point entity (brushes None) or a brush entity. Orientation lives in `angles` (the editor composes it with the instance); the legacy `angle` key is never written."""
        e = {"id": eid, "classname": cls, "origin": [round(v, 4) for v in origin], "angles": [0, yaw, 0],
             "keyvalues": {k: str(v) for k, v in kv.items()}}
        if brushes is not None:
            e["brushes"] = brushes
        if refs:
            e["refs"] = dict(refs)
        self.ents.append(e)
        return eid

    def light(self, eid, origin, color, brightness, radius, style=0, **kv):
        return self.ent("light_point", eid, origin, color=color, brightness=brightness, radius=radius, style=style, **kv)

    # --- instances -----------------------------------------------------------------------------------------------
    def inst(self, file, name, origin=(0, 0, 0), yaw=0.0, overrides=None, scale=None):
        i = {"id": "i_" + name, "name": name, "file": file, "origin": [round(v, 4) for v in origin], "angles": [0, yaw, 0], "overrides": overrides or {}}
        if scale:
            i["scale"] = list(scale)
        self.insts.append(i)
        return name

    # --- output --------------------------------------------------------------------------------------------------
    def doc(self, root_kv=None):
        """root_kv set: this is a level, its structural brushes go to `worldspawn` with those keys; otherwise a prefab, they go to a `func_group`."""
        ents = []
        if self.world or root_kv is not None:
            ents.append({"id": "e_struct", "classname": "worldspawn" if root_kv is not None else "func_group", "keyvalues": dict(root_kv or {}), "origin": [0, 0, 0], "angles": [0, 0, 0], "brushes": self.world})
        if self.detail:
            ents.append({"id": "e_detail", "classname": "func_detail", "keyvalues": {}, "origin": [0, 0, 0], "angles": [0, 0, 0], "brushes": self.detail})
        return {"schemaVersion": "0.2", "entities": ents + self.ents + self.points, "prefabInstances": self.insts}

    def save(self, rel, root_kv=None):
        path = os.path.join(GAME_DATA, rel.replace("/", os.sep))
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            json.dump(self.doc(root_kv), f, indent=1, sort_keys=True)
        return path


# ---------------------------------------------------------------------------------------------------- walls
def wall(p, axis, c, a0, a1, t, y0, y1, tex, holes=(), detail=False):
    """A wall along `axis` ('x' or 'z') centred on the other coordinate c, thickness t, from a0 to a1, y0..y1, with rectangular holes (ha0, ha1, hy0, hy1) in the wall's own (along, up) coordinates."""
    def put(b0, b1, ya, yb):
        if b1 - b0 < 1e-4 or yb - ya < 1e-4:
            return
        if axis == "x":
            p.box((b0, ya, c - t / 2), (b1, yb, c + t / 2), tex, detail)
        else:
            p.box((c - t / 2, ya, b0), (c + t / 2, yb, b1), tex, detail)
    cur = a0
    for h0, h1, hy0, hy1 in sorted(holes):
        put(cur, h0, y0, y1)
        put(h0, h1, y0, hy0)
        put(h0, h1, hy1, y1)
        cur = h1
    put(cur, a1, y0, y1)


def gable(p, hw, hd, y, rh, ov, tex, ridge_along="x", cx=0.0, cz=0.0, detail=False):
    """A gable roof (a convex hull) on the rectangle half-sizes hw x hd at height y, ridge rh above it, eaves overhang ov."""
    base = [(cx - hw - ov, y, cz - hd - ov), (cx + hw + ov, y, cz - hd - ov), (cx + hw + ov, y, cz + hd + ov), (cx - hw - ov, y, cz + hd + ov)]
    ridge = [(cx - hw - ov, y + rh, cz), (cx + hw + ov, y + rh, cz)] if ridge_along == "x" else [(cx, y + rh, cz - hd - ov), (cx, y + rh, cz + hd + ov)]
    p.hull(base + ridge, tex, detail)


def chimney(p, x, z, y0, y1, tex, w=0.9):
    p.box((x - w / 2, y0, z - w / 2), (x + w / 2, y1, z + w / 2), tex)
