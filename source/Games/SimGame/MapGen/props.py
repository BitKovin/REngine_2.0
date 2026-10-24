"""Prop prefabs (GameData/prefabs/props/*.map.json). Origin on the floor in the middle, front (+Z) towards the user.
Sim props carry their own `sim_container` and `sim_item_prop` entities; the instance (a building) sets the container's `site` reference and `access` with an override.
A container with items on show has one `sim_item_prop` brush entity per place a thing can lie; `slot` is the index of the place, or the fill stage (1 = a third, 2 = two thirds, 3 = full) of a bulk lot.
"""
import math
from kit import P

WOOD, PLANK, DARK, CRATE = "lq_wood/med_wood1", "lq_wood/plank1", "lq_wood/med_wood5", "lq_wood/crate4"
STONE, BRICK, IRON = "lq_medieval/med_block_1a", "wall/brickWall1", "lq_dev/wall_grey_b"
CLOTH, SACK, LOAF = "lq_dev/wall_orange_c", "lq_dev/wall_tan_b", "lq_dev/wall_brown_c"
FLOUR, GRAIN, SOOT = "lq_dev/floor_tan_a", "lq_dev/floor_yellow_b", "lq_dev/dot_grey_c"
LEAF, BARK, HEDGE = "lq_dev/floor_green_c", "lq_wood/med_wood2", "lq_dev/floor_green_b"


def crate():
    p = P()
    p.box((-0.4, 0, -0.4), (0.4, 0.7, 0.4), CRATE)
    p.box((-0.43, 0.62, -0.43), (0.43, 0.7, 0.43), WOOD, True)
    return p


def barrel():
    p = P()
    p.prism(0, 0, 0.36, 0, 0.95, 10, WOOD)
    for y in (0.18, 0.72):
        p.prism(0, 0, 0.385, y, y + 0.06, 10, IRON, True)
    return p


def sack():
    p = P()
    p.hull([(-0.25, 0, -0.17), (0.25, 0, -0.17), (0.25, 0, 0.17), (-0.25, 0, 0.17),
            (-0.30, 0.28, -0.21), (0.30, 0.28, -0.21), (0.30, 0.28, 0.21), (-0.30, 0.28, 0.21),
            (-0.13, 0.7, -0.1), (0.13, 0.7, -0.1), (0.13, 0.7, 0.1), (-0.13, 0.7, 0.1)], SACK)
    return p


def stool():
    p = P()
    p.box((-0.2, 0.42, -0.2), (0.2, 0.46, 0.2), WOOD)
    for sx in (-1, 1):
        for sz in (-1, 1):
            p.box((sx * 0.16 - 0.025, 0, sz * 0.16 - 0.025), (sx * 0.16 + 0.025, 0.42, sz * 0.16 + 0.025), WOOD)
    return p


def table():
    p = P()
    p.box((-0.8, 0.74, -0.45), (0.8, 0.78, 0.45), PLANK)
    for sx in (-1, 1):
        for sz in (-1, 1):
            p.box((sx * 0.72 - 0.04, 0, sz * 0.38 - 0.04), (sx * 0.72 + 0.04, 0.74, sz * 0.38 + 0.04), WOOD)
    return p


def bench():
    p = P()
    p.box((-0.9, 0.42, -0.18), (0.9, 0.46, 0.18), PLANK)
    for sx in (-1, 1):
        p.box((sx * 0.75 - 0.04, 0, -0.14), (sx * 0.75 + 0.04, 0.42, 0.14), WOOD)
    return p


def bed():
    """Head at -Z, 2.0 x 1.0 m. The sleeper's slot is placed by the building."""
    p = P()
    p.box((-0.5, 0.18, -1.0), (0.5, 0.4, 1.0), PLANK)
    p.box((-0.47, 0.4, -0.95), (0.47, 0.56, 0.95), CLOTH, True)
    p.box((-0.3, 0.56, -0.9), (0.3, 0.66, -0.5), "lq_dev/wall_tan_a", True)
    p.box((-0.52, 0.0, -1.04), (0.52, 1.1, -0.96), WOOD)
    p.box((-0.52, 0.0, 0.96), (0.52, 0.62, 1.04), WOOD)
    return p


def counter_seg():
    """1.5 m of shop counter (customer side +Z), 0.7 deep, 1.05 high."""
    p = P()
    p.box((-0.75, 0, -0.3), (0.75, 1.0, 0.3), PLANK)
    p.box((-0.75, 1.0, -0.38), (0.75, 1.06, 0.42), WOOD)
    return p


def shelf_bread():
    """Display shelf 2.4 m wide, three boards, twelve loaf places (container `shelf`, items `bread_loaf`)."""
    p = P()
    p.box((-1.2, 0, -0.24), (1.2, 0.12, 0.2), WOOD)
    p.box((-1.2, 0.12, -0.24), (1.2, 1.95, -0.2), PLANK)
    for x in (-1.2, 0.0, 1.2):
        p.box((x - 0.03, 0.12, -0.24), (x + 0.03, 1.95, 0.2), WOOD)
    boards = (0.5, 1.0, 1.5)
    for y in boards:
        p.box((-1.2, y, -0.24), (1.2, y + 0.04, 0.2), WOOD)
    p.ent("sim_container", "e_container", (0, 1.0, 0.0), containerType="shelf", access="public", capacity=12, visible=1, targetname="shelf")
    k = 0
    for y in boards:
        for x in (-0.9, -0.3, 0.3, 0.9):
            loaf = p.brush_hull([(x - 0.15, y + 0.04, -0.08), (x + 0.15, y + 0.04, -0.08), (x + 0.15, y + 0.04, 0.1), (x - 0.15, y + 0.04, 0.1),
                                 (x - 0.12, y + 0.17, -0.05), (x + 0.12, y + 0.17, -0.05), (x + 0.12, y + 0.17, 0.07), (x - 0.12, y + 0.17, 0.07)], LOAF)
            p.ent("sim_item_prop", f"e_loaf{k}", brushes=[loaf], refs={"container": "e_container"}, slot=k, itemType="bread_loaf")
            k += 1
    return p


def till():
    p = P()
    p.box((-0.22, 0, -0.18), (0.22, 0.22, 0.18), DARK)
    p.box((-0.2, 0.22, -0.16), (0.2, 0.3, 0.14), IRON, True)
    p.ent("sim_container", "e_container", (0, 0.15, 0), containerType="till", access="staff", capacity=0, visible=0, targetname="till")
    return p


def oven():
    """Brick bread oven 2.4 x 1.8, mouth at the front (+Z); the baker's slot is placed in front of it."""
    p = P()
    p.box((-1.2, 0, -0.9), (1.2, 1.0, 0.9), BRICK)
    p.hull([(-1.2, 1.0, -0.9), (1.2, 1.0, -0.9), (1.2, 1.0, 0.9), (-1.2, 1.0, 0.9), (-0.7, 1.75, -0.5), (0.7, 1.75, -0.5), (0.7, 1.75, 0.5), (-0.7, 1.75, 0.5)], BRICK)
    p.box((-0.4, 0.45, 0.9), (0.4, 0.95, 0.96), SOOT, True)
    p.box((-0.25, 1.75, -0.35), (0.25, 3.0, 0.15), BRICK, True)
    return p


def _bin(w, d, h, fill_tex, item, name, stages):
    p = P()
    t = 0.05
    p.box((-w / 2, 0, -d / 2), (w / 2, 0.08, d / 2), PLANK)
    p.box((-w / 2, 0, -d / 2), (w / 2, h, -d / 2 + t), WOOD)
    p.box((-w / 2, 0, d / 2 - t), (w / 2, h, d / 2), WOOD)
    p.box((-w / 2, 0, -d / 2), (-w / 2 + t, h, d / 2), WOOD)
    p.box((w / 2 - t, 0, -d / 2), (w / 2, h, d / 2), WOOD)
    p.ent("sim_container", "e_container", (0, h / 2, 0), containerType="bin", access="staff", capacity=0, visible=1, targetname=name)
    for i, top in enumerate(stages):
        b = p.brush_box((-w / 2 + t, 0.08, -d / 2 + t), (w / 2 - t, top, d / 2 - t), fill_tex)
        p.ent("sim_item_prop", f"e_fill{i + 1}", brushes=[b], refs={"container": "e_container"}, slot=i + 1, itemType=item)
    return p


def flour_bin():
    return _bin(1.0, 0.7, 0.8, FLOUR, "flour", "bin", (0.3, 0.52, 0.72))


def grain_bin():
    return _bin(1.5, 1.0, 1.1, GRAIN, "grain", "bin", (0.4, 0.72, 1.0))


def millstone():
    """Stone drum with a runner stone and a hopper; the miller's slot is placed beside it."""
    p = P()
    p.prism(0, 0, 1.15, 0, 0.7, 14, STONE)
    p.prism(0, 0, 1.0, 0.7, 0.98, 14, "lq_medieval/med_block_1b")
    p.hull([(-0.55, 1.9, -0.55), (0.55, 1.9, -0.55), (0.55, 1.9, 0.55), (-0.55, 1.9, 0.55), (-0.15, 1.3, -0.15), (0.15, 1.3, -0.15), (0.15, 1.3, 0.15), (-0.15, 1.3, 0.15)], WOOD, True)
    for sx in (-1, 1):
        for sz in (-1, 1):
            p.box((sx * 0.5 - 0.04, 0.98, sz * 0.5 - 0.04), (sx * 0.5 + 0.04, 1.9, sz * 0.5 + 0.04), WOOD, True)
    return p


def pantry():
    p = P()
    p.box((-0.5, 0, -0.25), (0.5, 1.85, 0.25), WOOD)
    p.box((-0.02, 0.1, 0.25), (0.02, 1.75, 0.27), DARK, True)
    p.ent("sim_container", "e_container", (0, 0.9, 0), containerType="pantry", access="household", capacity=0, visible=0, targetname="pantry")
    return p


def chest():
    p = P()
    p.box((-0.4, 0, -0.22), (0.4, 0.4, 0.22), DARK)
    p.hull([(-0.4, 0.4, -0.22), (0.4, 0.4, -0.22), (0.4, 0.4, 0.22), (-0.4, 0.4, 0.22), (-0.38, 0.52, -0.14), (0.38, 0.52, -0.14), (0.38, 0.52, 0.14), (-0.38, 0.52, 0.14)], WOOD, True)
    p.ent("sim_container", "e_container", (0, 0.25, 0), containerType="purse", access="owner", capacity=0, visible=0, targetname="chest")
    return p


def lamp_post():
    """Street lamp, 4 m: a style 4 light (on from dusk to dawn, baked into its own lightmap layer)."""
    p = P()
    p.prism(0, 0, 0.2, 0, 0.5, 8, IRON)
    p.prism(0, 0, 0.07, 0.5, 3.3, 8, IRON, True)
    p.box((-0.17, 3.3, -0.17), (0.17, 3.38, 0.17), IRON, True)
    p.box((-0.14, 3.38, -0.14), (0.14, 3.85, 0.14), "lq_dev/light_fbr", True)
    p.cone(0, 0, 0.26, 3.85, 4.2, 8, IRON, True)
    p.light("e_lamp", (0, 3.6, 0), "1 0.72 0.42", 4.0, 13.0, style=4, source_size=0.12)
    return p


def fence():
    """3 m of wooden fence along X."""
    p = P()
    for x in (-1.45, 0.0, 1.45):
        p.box((x - 0.05, 0, -0.05), (x + 0.05, 1.15, 0.05), WOOD, True)
    for y in (0.35, 0.8):
        p.box((-1.5, y, -0.03), (1.5, y + 0.1, 0.03), PLANK, True)
    return p


def hedge():
    p = P()
    p.box((-1.5, 0, -0.45), (1.5, 1.4, 0.45), HEDGE)
    return p


def well():
    p = P()
    p.prism(0, 0, 0.95, 0, 0.95, 12, STONE)
    for sx in (-1, 1):
        p.box((sx * 0.9 - 0.06, 0.95, -0.06), (sx * 0.9 + 0.06, 2.3, 0.06), WOOD, True)
    p.box((-0.95, 2.3, -0.07), (0.95, 2.42, 0.07), WOOD, True)
    p.hull([(-1.25, 2.4, -0.8), (1.25, 2.4, -0.8), (1.25, 2.4, 0.8), (-1.25, 2.4, 0.8), (-1.25, 3.1, 0), (1.25, 3.1, 0)], "lq_dev/wall_grey_c", True)
    return p


def mill_wheel():
    """Water wheel in the YZ plane (axle along X, 2.9 m up), the lowest paddles at y = 0; origin below the axle."""
    p = P()
    r_out, r_in, ax, n = 2.8, 2.3, 2.9, 12
    pt = lambda r, i: (ax + r * math.cos(2 * math.pi * i / n), r * math.sin(2 * math.pi * i / n))
    for i in range(n):
        pts = []
        for x in (-0.4, 0.4):
            for r, k in ((r_out, i), (r_out, i + 1), (r_in, i), (r_in, i + 1)):
                y, z = pt(r, k)
                pts.append((x, y, z))
        p.hull(pts, WOOD, True)
    p.box((-0.15, ax - r_in, -0.08), (0.15, ax + r_in, 0.08), DARK, True)
    p.box((-0.15, ax - 0.08, -r_in), (0.15, ax + 0.08, r_in), DARK, True)
    p.box((-0.7, ax - 0.2, -0.2), (0.7, ax + 0.2, 0.2), DARK, True)
    return p


def tree():
    """A pine, 9 m: 0.3 m trunk, three stacked crowns above 2.6 m (the trunk blocks, the crown is above head height)."""
    p = P()
    p.prism(0, 0, 0.28, 0, 3.0, 6, BARK)
    for y0, y1, r in ((2.6, 5.4, 2.5), (4.4, 7.2, 1.9), (6.2, 9.0, 1.2)):
        p.cone(0, 0, r, y0, y1, 7, LEAF)
    return p


def sign_post():
    """A shop sign on a bracket over a door is not needed; a road signpost: 2.4 m post, two boards."""
    p = P()
    p.box((-0.06, 0, -0.06), (0.06, 2.4, 0.06), WOOD)
    p.box((-0.6, 1.9, 0.06), (0.4, 2.2, 0.09), PLANK, True)
    p.box((-0.4, 1.5, 0.06), (0.6, 1.8, 0.09), PLANK, True)
    return p


def haystack():
    p = P()
    p.hull([(-1.2, 0, -1.2), (1.2, 0, -1.2), (1.2, 0, 1.2), (-1.2, 0, 1.2), (-0.5, 2.0, -0.5), (0.5, 2.0, -0.5), (0.5, 2.0, 0.5), (-0.5, 2.0, 0.5)], "lq_dev/floor_yellow_c")
    return p


def gravestone():
    p = P()
    p.box((-0.3, 0, -0.07), (0.3, 0.75, 0.07), STONE)
    p.hull([(-0.3, 0.75, -0.07), (0.3, 0.75, -0.07), (0.3, 0.75, 0.07), (-0.3, 0.75, 0.07), (-0.18, 0.95, -0.07), (0.18, 0.95, -0.07), (0.18, 0.95, 0.07), (-0.18, 0.95, 0.07)], STONE)
    return p


def tree_far():
    """A cheap forest tree: a 0.5 m trunk and one tall crown above head height (two brushes)."""
    p = P()
    p.prism(0, 0, 0.3, 0, 3.2, 5, BARK)
    p.cone(0, 0, 2.3, 2.6, 10.0, 6, LEAF)
    return p


def trough():
    p = P()
    p.box((-1.0, 0, -0.3), (1.0, 0.55, 0.3), WOOD)
    return p


PROPS = {n: f for n, f in globals().items() if callable(f) and n not in ("P",) and not n.startswith("_") and getattr(f, "__module__", "") == __name__}
