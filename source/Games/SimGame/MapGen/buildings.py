"""Building prefabs (GameData/prefabs/buildings/*.map.json): origin at the middle of the footprint on the ground, front (door side) towards +Z, floor top at F = 0.12.
Enterable: house_open, bakery, mill, tavern (interiors in the same level, no separate levels). Closed shells (solid mass, the sim site has playable = 0): house_shell, church.
Every sim entity has a `targetname` (its sim id is the resolved name `<instances>|<targetname>`), `sim_door` / `sim_slot` / `sim_container` reference their `sim_site` (`e_site`) through `refs`.
Navigation: the engine's nav mesh uses an agent of radius 0.5 m, height 2.0 m, climb 0.6 m, so every passage is at least 1.3 m wide and 2.3 m high.
Facing of sim_door / sim_slot is the entity's yaw (forward = (sin yaw, cos yaw) in x, z): a door's yaw points out of the building, a slot's yaw is the way the occupant faces.
"""
import math
from kit import P, wall, gable, chimney
import props as pr

F = 0.12            # floor top above the ground
T = 0.35            # outer wall thickness
BRICK, PLASTER, FLOOR, CEIL = pr.BRICK, "lq_dev/wall_tan_a", "lq_wood/plank1", "lq_wood/med_wood2"
ROOF, STONE, PANE, FRAME = "lq_dev/wall_grey_c", pr.STONE, "lq_dev/dot_grey_c", "lq_wood/med_wood5"
CONTAINER_PROPS = {"shelf_bread", "till", "flour_bin", "grain_bin", "pantry", "chest"}
TAVERN_ROOM_SPAWN = (-3.6, 3.02 + 0.1, -3.2)   # local position of the player's start in the tavern's upper room (the world map places the sim_spawn)


# ---------------------------------------------------------------------------------------------------- helpers
def prop(p, name, inst, pos, yaw=0.0, y=F, access=None):
    ov = {}
    if name in CONTAINER_PROPS:
        ov["e_container"] = {"refs": {"site": "e_site"}}
        if access:
            ov["e_container"]["keyvalues"] = {"access": access}
    p.inst(f"prefabs/props/{name}.map.json", inst, (pos[0], y, pos[1]), yaw, ov)


def slot(p, eid, pos, yaw, kind, y=F):
    p.ent("sim_slot", "e_" + eid, (pos[0], y, pos[1]), yaw, refs={"site": "e_site"}, targetname=eid, slotType=kind)


def door(p, eid, pos, yaw, y=F):
    p.ent("sim_door", "e_" + eid, (pos[0], y, pos[1]), yaw, refs={"site": "e_site"}, targetname=eid)


def container_pt(p, eid, pos, ctype, access, y=F + 0.5):
    p.ent("sim_container", "e_" + eid, (pos[0], y, pos[1]), 0, refs={"site": "e_site"}, targetname=eid, containerType=ctype, access=access, capacity=0, visible=0)


def site(p, lo, hi, kind, name, playable, capacity):
    p.ent("sim_site", "e_site", brushes=[p.brush_box(lo, hi, "lq_dev/trigger")], targetname="site", kind=kind, displayName=name, playable=playable, capacity=capacity)


def window(a0, a1, y0=0.9, y1=2.2):
    return (a0, a1, F + y0, F + y1)


def outer_walls(p, w, d, h, holes, tex=BRICK, y0=-0.3):
    """The four outer walls up to y = h. holes: {'front': [...], 'back': [...], 'left': [...], 'right': [...]} as (a0, a1, y0, y1); a = x for front/back, z for left/right."""
    hw, hd = w / 2, d / 2
    wall(p, "x", hd - T / 2, -hw + T, hw - T, T, y0, h, tex, holes.get("front", ()))
    wall(p, "x", -hd + T / 2, -hw + T, hw - T, T, y0, h, tex, holes.get("back", ()))
    wall(p, "z", -hw + T / 2, -hd, hd, T, y0, h, tex, holes.get("left", ()))
    wall(p, "z", hw - T / 2, -hd, hd, T, y0, h, tex, holes.get("right", ()))


def facade_trim(p, w, d, door_x, y_top, windows_front=(), windows_side=(), levels=(F,), door=True):
    """Dark frames and panes on the outside of a closed shell (front +Z, sides), and the door."""
    hw, hd = w / 2, d / 2
    if door:
        p.box((door_x - 0.8, F, hd), (door_x + 0.8, F + 2.4, hd + 0.12), pr.DARK, True)
        p.box((door_x - 0.65, F, hd + 0.12), (door_x + 0.65, F + 2.3, hd + 0.16), "lq_wood/plank2", True)
    for y0 in levels:
        for x in windows_front:
            p.box((x - 0.5, y0 + 0.85, hd), (x + 0.5, y0 + 2.25, hd + 0.08), pr.DARK, True)
            p.box((x - 0.4, y0 + 0.95, hd + 0.08), (x + 0.4, y0 + 2.15, hd + 0.11), PANE, True)
        for z in windows_side:
            for sx in (-1, 1):
                xa, xb = (hw, hw + 0.08) if sx > 0 else (-hw - 0.08, -hw)
                xc, xd = (hw + 0.08, hw + 0.11) if sx > 0 else (-hw - 0.11, -hw - 0.08)
                p.box((xa, y0 + 0.85, z - 0.5), (xb, y0 + 2.25, z + 0.5), pr.DARK, True)
                p.box((xc, y0 + 0.95, z - 0.4), (xd, y0 + 2.15, z + 0.4), PANE, True)


def roof_block(p, w, d, y, rh, ridge="x", ov=0.4):
    gable(p, w / 2, d / 2, y, rh, ov, ROOF, ridge)


# ---------------------------------------------------------------------------------------------------- house (open and closed share one layout)
def _house_sim(p, playable):
    """Both variants: bed and table seats, pantry and purse at the same places. Front room (z > 0) kitchen / living, back room (z < 0) bedroom."""
    site(p, (-4.15, F - 0.1, -4.15), (4.15, F + 2.7, 4.15), "house", "House", playable, 6)
    door(p, "door", (0, 5.3), 0)
    slot(p, "bed1", (3.2, -3.1), 0, "bed")
    slot(p, "bed2", (1.4, -3.1), 0, "bed")
    slot(p, "seat1", (1.5, 1.2), 0, "seat")
    slot(p, "seat2", (1.5, 3.2), 180, "seat")
    slot(p, "seat3", (0.5, 2.2), 90, "seat")
    slot(p, "stand", (0, 3.9), 180, "stand")


def house_open():
    p = P()
    w = d = 9.0
    hw = w / 2
    p.box((-hw, -0.3, -hw), (hw, F, hw), FLOOR)
    holes = {"front": [(-0.65, 0.65, F, F + 2.3), window(-3.2, -2.0), window(2.0, 3.2)],
             "back": [window(-0.6, 0.6)], "left": [window(-2.5, -1.3), window(1.8, 3.0)], "right": [window(-2.5, -1.3), window(1.8, 3.0)]}
    outer_walls(p, w, d, F + 2.7 + 0.25, holes)
    wall(p, "x", 0.0, -hw + T, hw - T, 0.15, F, F + 2.7, PLASTER, [(-3.4, -2.1, F, F + 2.3)])      # bedroom partition, door at x -3.4..-2.1
    p.box((-hw, F + 2.7, -hw), (hw, F + 2.95, hw), CEIL)
    roof_block(p, w, d, F + 2.95, 3.9)
    chimney(p, 2.2, -1.0, F + 2.9, F + 7.2, BRICK)
    prop(p, "table", "table", (1.5, 2.2), 90)
    prop(p, "stool", "stool1", (1.5, 1.2), 0)
    prop(p, "stool", "stool2", (1.5, 3.2), 0)
    prop(p, "stool", "stool3", (0.5, 2.2), 0)
    prop(p, "pantry", "pantry", (-3.65, 3.2), 90, access="household")
    prop(p, "bed", "bed1", (3.2, -3.1), 0)
    prop(p, "bed", "bed2", (1.4, -3.1), 0)
    prop(p, "chest", "chest", (-3.4, -3.6), 0, access="household")
    p.light("e_lamp", (0, F + 2.3, 2.3), "1 0.72 0.42", 1.6, 6.5, style=4, source_size=0.1)
    p.light("e_lamp_bed", (2.3, F + 2.3, -2.3), "1 0.72 0.42", 1.0, 5.0, style=4, source_size=0.1)
    _house_sim(p, 1)
    return p


def house_shell():
    p = P()
    w = d = 9.0
    hw = w / 2
    p.box((-hw, -0.3, -hw), (hw, F + 5.4, hw), BRICK)
    p.box((-hw - 0.1, -0.3, -hw - 0.1), (hw + 0.1, F + 0.5, hw + 0.1), STONE, True)
    roof_block(p, w, d, F + 5.4, 3.9)
    chimney(p, 2.2, -1.0, F + 5.3, F + 9.6, BRICK)
    facade_trim(p, w, d, 0.0, F + 5.4, windows_front=(-3.0, 3.0), windows_side=(-1.9, 2.4), levels=(F, F + 2.75))
    _house_sim(p, 0)
    container_pt(p, "pantry", (-3.65, 3.2), "pantry", "household")
    container_pt(p, "chest", (-3.4, -3.6), "purse", "household")
    return p


# ---------------------------------------------------------------------------------------------------- bakery
def bakery():
    """12 x 11. Shop (z 0.4..5.5) with the counter across it and two bread shelves behind; bake room (z -5.5..0.4) with the oven, the kneading table and the flour bin. Staff pass the counter at its east end."""
    p = P()
    w, d = 12.0, 11.0
    hw, hd = w / 2, d / 2
    H = F + 3.0
    p.box((-hw, -0.3, -hd), (hw, F, hd), FLOOR)
    holes = {"front": [(-3.65, -2.35, F, F + 2.3), window(-1.4, 1.4), window(2.4, 5.2), window(-5.2, -4.0)],
             "back": [window(-2.4, -0.6), window(1.2, 3.0)], "left": [window(-3.6, -2.0), window(1.8, 3.4)], "right": [window(-3.6, -2.0), window(1.8, 3.4)]}
    outer_walls(p, w, d, H + 0.25, holes)
    wall(p, "x", 0.4, -hw + T, hw - T, 0.2, F, H, PLASTER, [(4.35, 5.65, F, F + 2.3)])     # partition with the staff opening
    p.box((-hw, H, -hd), (hw, H + 0.25, hd), CEIL)
    roof_block(p, w, d, H + 0.25, 4.1)
    chimney(p, -3.5, -4.6, H, H + 8.0, BRICK, 1.1)
    # shop
    for i, x in enumerate((-4.9, -3.4, -1.9, -0.4, 1.1, 2.6)):
        prop(p, "counter_seg", f"counter{i + 1}", (x, 2.5), 0)
    prop(p, "till", "till", (3.2, 2.5), 0, y=F + 1.06, access="staff")
    prop(p, "shelf_bread", "shelf1", (-3.9, 0.76), 0, access="public")
    prop(p, "shelf_bread", "shelf2", (-1.3, 0.76), 0, access="public")
    prop(p, "barrel", "barrel_shop", (5.3, 4.7), 0)
    prop(p, "crate", "crate_shop", (-5.2, 4.8), 0)
    slot(p, "clerk", (0.5, 1.6), 0, "counter")
    slot(p, "customer", (0.5, 3.4), 180, "customer")
    slot(p, "queue1", (-0.6, 4.2), 180, "queue")
    slot(p, "queue2", (-0.6, 5.0), 180, "queue")
    slot(p, "queue3", (1.6, 4.2), 180, "queue")
    # bake room
    prop(p, "oven", "oven", (-3.5, -4.25), 0)
    prop(p, "table", "table", (1.5, -2.6), 0)
    prop(p, "flour_bin", "flour_bin", (4.9, -4.7), 0, access="staff")
    prop(p, "sack", "sack1", (3.7, -4.9), 0)
    prop(p, "sack", "sack2", (3.3, -4.4), 30)
    prop(p, "crate", "crate_bake", (-5.2, -1.0), 0)
    slot(p, "oven_slot", (-3.5, -2.9), 180, "oven")
    slot(p, "work", (1.5, -1.6), 180, "work")
    container_pt(p, "stock", (0.0, -3.5), "pantry", "staff")
    p.light("e_lamp_shop", (0, F + 2.5, 3.1), "1 0.72 0.42", 1.8, 7.0, style=4, source_size=0.1)
    p.light("e_lamp_bake", (0, F + 2.5, -2.6), "1 0.72 0.42", 1.4, 7.0, style=4, source_size=0.1)
    p.light("e_glow", (-3.5, F + 0.8, -3.6), "1 0.45 0.15", 1.0, 4.0, style=4, source_size=0.1)
    site(p, (-hw + T, F - 0.1, -hd + T), (hw - T, H, hd - T), "bakery", "Bakery", 1, 14)
    door(p, "door", (-3.0, hd + 0.8), 0)
    return p


# ---------------------------------------------------------------------------------------------------- mill
def mill():
    """12 x 11, one tall room: the millstone in the middle, grain bin and flour bin against the north wall, a flour counter by the door. The water wheel is part of the prefab on the west side (x < -6)."""
    p = P()
    w, d = 12.0, 11.0
    hw, hd = w / 2, d / 2
    Hg = F + 3.4
    Ht = F + 7.6
    p.box((-hw, -0.3, -hd), (hw, F, hd), FLOOR)
    holes = {"front": [(-0.65, 0.65, F, F + 2.3), window(3.0, 4.4), window(-4.4, -3.0)],
             "back": [window(-1.0, 1.0, 1.2, 2.4)], "right": [window(-3.0, -1.8), window(2.0, 3.2)], "left": [window(-2.0, 0.4, 1.0, 2.2)]}
    outer_walls(p, w, d, Hg + 0.25, holes)
    p.box((-hw, Hg, -hd), (hw, Hg + 0.25, hd), CEIL)
    p.box((-hw, Hg + 0.25, -hd), (hw, Ht, hd), BRICK)
    for z in (-2.0, 2.0):
        p.box((-hw - 0.08, Hg + 1.4, z - 0.5), (-hw, Hg + 2.8, z + 0.5), pr.DARK, True)
        p.box((hw, Hg + 1.4, z - 0.5), (hw + 0.08, Hg + 2.8, z + 0.5), pr.DARK, True)
    for x in (-3.0, 3.0):
        p.box((x - 0.5, Hg + 1.4, hd), (x + 0.5, Hg + 2.8, hd + 0.08), pr.DARK, True)
    roof_block(p, w, d, Ht, 4.0)
    prop(p, "millstone", "millstone", (0.5, -1.2), 0)
    prop(p, "grain_bin", "grain_bin", (-4.4, -4.6), 0, access="staff")
    prop(p, "flour_bin", "flour_bin", (4.6, -4.8), 0, access="staff")
    prop(p, "sack", "sack1", (2.8, -4.9), 0)
    prop(p, "sack", "sack2", (3.2, -4.5), 40)
    prop(p, "sack", "sack3", (-2.6, -4.9), 0)
    prop(p, "crate", "crate", (-5.2, 0.6), 0)
    prop(p, "counter_seg", "counter1", (3.9, 2.4), 90)
    prop(p, "counter_seg", "counter2", (3.9, 3.9), 90)
    prop(p, "till", "till", (4.0, 3.15), 0, y=F + 1.06, access="staff")
    prop(p, "mill_wheel", "wheel", (-6.9, 0.2), 0, y=0)   # axle along X: yaw 0 keeps the wheel in the YZ plane
    slot(p, "miller", (0.5, 0.45), 180, "millstone")
    slot(p, "clerk", (5.0, 3.15), 270, "counter")
    slot(p, "customer", (2.6, 3.15), 90, "customer")
    slot(p, "queue1", (2.0, 4.4), 90, "queue")
    slot(p, "queue2", (1.2, 4.4), 90, "queue")
    container_pt(p, "stock", (-1.0, -4.0), "pantry", "staff")
    p.light("e_lamp", (0, F + 3.0, 1.0), "1 0.72 0.42", 1.8, 8.0, style=4, source_size=0.1)
    p.light("e_lamp_back", (0, F + 3.0, -3.5), "1 0.72 0.42", 1.2, 8.0, style=4, source_size=0.1)
    site(p, (-hw + T, F - 0.1, -hd + T), (hw - T, Hg, hd - T), "mill", "Mill", 1, 8)
    door(p, "door", (0.0, hd + 0.8), 0)
    return p


# ---------------------------------------------------------------------------------------------------- tavern
def tavern():
    """13 x 13. Taproom on the ground floor (bar along the north wall, three tables), a stair along the west wall up to the back half's upper floor: corridor and the guest room (player start). The front half of the taproom is open to the ceiling."""
    p = P()
    w = d = 13.0
    hw = hd = 6.5
    Fu = F + 2.9                      # upper floor top
    Hc = Fu + 2.6                     # ceiling underside
    Ht = Hc + 0.25
    p.box((-hw, -0.3, -hd), (hw, F, hd), FLOOR)
    holes = {"front": [(-0.65, 0.65, F, F + 2.3), window(-4.8, -3.0), window(3.0, 4.8), window(-4.8, -3.0, 3.6, 4.8), window(3.0, 4.8, 3.6, 4.8)],
             "back": [window(-1.0, 1.0, 1.0, 2.2), window(3.0, 4.4, 1.0, 2.2), window(-4.8, -3.0, 4.0, 5.2)],
             "left": [window(-3.0, -1.6, 1.0, 2.2), window(2.0, 3.4, 1.0, 2.2), window(-4.3, -2.7, 4.0, 5.2)],
             "right": [window(-3.0, -1.6, 1.0, 2.2), window(2.0, 3.4, 1.0, 2.2), window(-3.0, -1.6, 4.0, 5.2)]}
    outer_walls(p, w, d, Ht, holes)
    p.box((-hw, Hc, -hd), (hw, Ht, hd), CEIL)
    roof_block(p, w, d, Ht, 4.6)
    chimney(p, 4.5, -6.0, Ht - 0.3, Ht + 7.6, BRICK, 1.2)
    # stair along the west wall, climbing north (to -z) from z = 5.4 to z = 1.0
    n_steps, z0, run = 16, 5.4, 4.4
    rise, tread = (Fu - F) / n_steps, run / n_steps
    for i in range(n_steps):
        p.box((-6.15, F, z0 - (i + 1) * tread), (-4.65, F + (i + 1) * rise, z0 - i * tread), "lq_wood/plank2", True)
    # upper floor over the back half (z < 1.0) and its rooms
    p.box((-6.15, Fu - 0.27, -6.15), (6.15, Fu, 1.0), FLOOR)
    wall(p, "x", -1.6, -6.15, 1.0, 0.2, Fu, Hc, PLASTER, [(-3.65, -2.35, Fu, Fu + 2.3)])               # corridor wall, guest-room door
    p.box((-1.5, Fu, -6.15), (6.15, Hc, -1.7), BRICK, True)                                         # the other rooms: closed
    p.box((1.0, Fu, -1.7), (6.15, Hc, 1.0), BRICK, True)                                            # the far end of the corridor: closed
    wall(p, "x", 1.0 - 0.05, -4.65, 1.0, 0.1, Fu, Fu + 1.05, pr.WOOD, (), True)                      # balustrade over the taproom
    wall(p, "z", -1.5 + 0.1, -6.15, -1.6, 0.2, Fu, Hc, PLASTER)                                     # guest room east wall
    # taproom
    for i, x in enumerate((0.75, 2.25, 3.75, 5.25)):
        prop(p, "counter_seg", f"bar{i + 1}", (x, -3.0), 0)
    prop(p, "barrel", "barrel1", (1.2, -5.7), 0)
    prop(p, "barrel", "barrel2", (2.3, -5.7), 0)
    prop(p, "barrel", "barrel3", (4.8, -5.7), 0)
    prop(p, "crate", "crate", (5.6, -4.7), 0)
    prop(p, "till", "till", (3.0, -3.0), 0, y=F + 1.06, access="staff")
    tables = [("t1", -3.4, 0.2), ("t2", -3.4, 3.4), ("t3", 3.4, 2.4)]
    for tid, tx, tz in tables:
        prop(p, "table", tid, (tx, tz), 0)
        prop(p, "stool", tid + "_sa", (tx, tz - 0.85), 0)
        prop(p, "stool", tid + "_sb", (tx, tz + 0.85), 0)
        slot(p, tid + "_seat_a", (tx, tz - 0.85), 0, "seat")
        slot(p, tid + "_seat_b", (tx, tz + 0.85), 180, "seat")
    prop(p, "bench", "bench1", (-1.3, -5.7), 0)
    slot(p, "barkeep", (3.0, -4.1), 0, "counter")
    for i, x in enumerate((1.2, 3.0, 4.8)):
        slot(p, f"bar_stand{i + 1}", (x, -1.9), 180, "customer")
    slot(p, "queue1", (0.0, -0.4), 180, "queue")
    slot(p, "stand", (0.0, 4.2), 180, "stand")
    container_pt(p, "stock", (3.0, -5.6), "pantry", "staff")
    # guest room (upper floor, west back corner)
    prop(p, "bed", "guest_bed", (-5.2, -5.1), 0, y=Fu)
    prop(p, "chest", "guest_chest", (-3.3, -5.7), 0, y=Fu, access="owner")
    prop(p, "stool", "guest_stool", (-2.4, -3.0), 0, y=Fu)
    slot(p, "guest_bed", (-5.2, -5.1), 0, "bed", y=Fu)
    # lights
    p.light("e_lamp_a", (-2.0, F + 3.4, 1.0), "1 0.72 0.42", 2.2, 9.0, style=4, source_size=0.1)
    p.light("e_lamp_b", (3.5, F + 3.4, -1.0), "1 0.72 0.42", 2.2, 9.0, style=4, source_size=0.1)
    p.light("e_lamp_c", (0.0, F + 3.4, 4.5), "1 0.72 0.42", 1.6, 8.0, style=4, source_size=0.1)
    p.light("e_lamp_room", (-3.6, Fu + 2.0, -3.4), "1 0.72 0.42", 1.2, 5.5, style=4, source_size=0.1)
    p.light("e_lamp_corr", (-3.0, Fu + 2.1, -0.3), "1 0.72 0.42", 1.0, 6.0, style=4, source_size=0.1)
    site(p, (-hw + T, F - 0.1, -hd + T), (hw - T, Ht, hd - T), "pub", "Tavern", 1, 30)
    door(p, "door", (0.0, hd + 0.8), 0)
    return p


# ---------------------------------------------------------------------------------------------------- church (closed)
def church():
    """A closed shell: nave 9 x 16, a 5 x 5 tower with a spire at the front (+Z) holding the door, buttresses, pinnacled gable. Site playable = 0."""
    p = P()
    nw, nd = 9.0, 16.0
    hw, hd = nw / 2, nd / 2
    Hn = F + 8.0
    p.box((-hw, -0.3, -hd), (hw, Hn, hd), STONE)
    for z in (-6.0, -2.0, 2.0, 6.0):
        p.box((hw, 0, z - 0.3), (hw + 0.6, F + 6.0, z + 0.3), STONE, True)
        p.box((-hw - 0.6, 0, z - 0.3), (-hw, F + 6.0, z + 0.3), STONE, True)
    gable(p, hw, hd, Hn, 5.2, 0.5, ROOF, "z")
    # tower at the front
    tw = 5.0
    p.box((-tw / 2, -0.3, hd - 0.5), (tw / 2, F + 17.0, hd + tw - 0.5), STONE)
    p.hull([(-tw / 2 - 0.3, F + 17.0, hd - 0.8), (tw / 2 + 0.3, F + 17.0, hd - 0.8), (tw / 2 + 0.3, F + 17.0, hd + tw - 0.2), (-tw / 2 - 0.3, F + 17.0, hd + tw - 0.2),
            (0, F + 28.0, hd + tw / 2 - 0.5)], ROOF)
    fz = hd + tw - 0.5
    p.box((-0.9, F, fz), (0.9, F + 2.8, fz + 0.15), pr.DARK, True)
    p.box((-0.75, F, fz + 0.15), (0.75, F + 2.7, fz + 0.2), "lq_wood/plank2", True)
    for y in (F + 6.0, F + 11.0, F + 15.0):
        p.box((-0.45, y, fz), (0.45, y + 2.0, fz + 0.1), pr.DARK, True)
    for z in (-5.0, 0.0, 5.0):
        for sx in (-1, 1):
            xa, xb = (hw, hw + 0.1) if sx > 0 else (-hw - 0.1, -hw)
            p.box((xa, F + 2.5, z - 0.6), (xb, F + 6.5, z + 0.6), pr.DARK, True)
    site(p, (-hw, F - 0.1, -hd), (hw, Hn, hd), "church", "Church", 0, 40)
    door(p, "door", (0.0, hd + tw - 0.5 + 0.9), 0)
    slot(p, "stand1", (0, 0), 0, "stand", y=F)
    slot(p, "stand2", (2.0, 4.0), 180, "stand", y=F)
    return p


BUILDINGS = {"house_open": house_open, "house_shell": house_shell, "bakery": bakery, "mill": mill, "tavern": tavern, "church": church}
