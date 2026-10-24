"""The world of the baker slice: GameData/maps/vale.map.json, one big flat level (600 x 400 m) with three villages (each a prefab instance), dirt roads between their gates, forests, a rock rim.
Run: python build_world.py. Sim entities stay unprefixed here (sim_spawn); everything inside the villages gets its id from the instance names (village_c|bakery|site ...).
"""
import math
import random
from kit import P, yaw_xz
import buildings
import villages

X0, X1, Z0, Z1 = -300.0, 300.0, -200.0, 200.0
POS = {"village_a": (-170.0, -60.0), "village_b": (170.0, -70.0), "village_c": (0.0, 100.0)}
ROAD_W = 5.0
DEV_VIEWS = {   # name: (x, y, z, yaw): yaw 0 faces +Z (south), 90 east, 180 north, 270 west
    "view_c_square": (-3.0, 0.1, 114.0, 225), "view_c_terrace": (-10.0, 2.5, 69.5, 0), "view_c_bakery": (6.0, 0.3, 101.5, 180),
    "view_c_mill": (-21.0, 0.3, 132.0, 270), "view_c_tavern": (12.5, 0.3, 117.5, 0), "view_a_street": (-215.0, 0.1, -60.0, 90), "view_b_green": (170.0, 0.1, -30.0, 180),
}
DIRT, ROCK, GRASS = "lq_terra/may_drt1_1", "lq_terra/darkrock", "Ground/grass"


def road(p, pts, rects):
    """A dirt road along a polyline of axis-aligned legs; rects collects the covered rectangles (for the forest exclusion)."""
    for (ax, az), (bx, bz) in zip(pts, pts[1:]):
        x0, x1 = sorted((ax, bx)); z0, z1 = sorted((az, bz))
        r = (x0 - ROAD_W / 2, z0 - ROAD_W / 2, x1 + ROAD_W / 2, z1 + ROAD_W / 2)
        p.box((r[0], -0.3, r[1]), (r[2], 0.04, r[3]), DIRT, True)
        rects.append(r)


def build(only=None, forest=True):
    """only: a village name, to build a world with that village alone (no roads, no forest) for lighting experiments."""
    p = P()
    keep = []                                                   # rectangles with no trees: (x0, z0, x1, z1)
    p.box((X0, -1.0, Z0), (X1, 0.0, Z1), GRASS)
    rim = 14.0
    for a, b in (((X0 - 12, -rim, Z0 - 12), (X0, 14.0, Z1 + 12)), ((X1, -rim, Z0 - 12), (X1 + 12, 14.0, Z1 + 12)),
                 ((X0, -rim, Z0 - 12), (X1, 14.0, Z0)), ((X0, -rim, Z1), (X1, 14.0, Z1 + 12))):
        p.box(a, b, ROCK)
    for name, (x, z) in POS.items():
        if only and name != only:
            continue
        p.inst(f"prefabs/{name}.map.json", name, (x, 0, z), 0)
        keep.append((x - 62, z - 62, x + 62, z + 62))
    # roads: C east gate (50, 108) -> east, up to B's south gate; C west gate -> A's east gate; stubs out of the map at the east and west ends
    cx, cz = POS["village_c"]; bx, bz = POS["village_b"]; ax, az = POS["village_a"]
    if only:
        forest = False
    else:
        road(p, [(cx + 49, cz + 8), (bx - 2.5, cz + 8), (bx - 2.5, bz + 49)], keep)
        road(p, [(bx - 2.5, cz + 8), (X1 - 12, cz + 8)], keep)
        road(p, [(cx - 49, cz + 6), (ax + 70, cz + 6), (ax + 70, az), (ax + 49, az)], keep)
        road(p, [(ax - 49, az), (X0 + 12, az)], keep)
    # the player's start: the tavern's guest room in village C
    tx, tz, tyaw = 12.5, 20.5, 180.0
    lx, ly, lz = buildings.TAVERN_ROOM_SPAWN
    ox, oz = yaw_xz(tyaw, lx, lz)
    sx, sz = cx + tx + ox, cz + tz + oz
    p.ent("sim_spawn", "e_start", (sx, ly, sz), 180, targetname="start", spawnflags=1)   # facing north, the way the guest room faces its door
    # development views (launch -sim_start <name>): places to take pictures from
    for nm, (wx, wy, wz, ang) in DEV_VIEWS.items():
        p.ent("sim_spawn", "e_" + nm, (wx, wy, wz), ang, targetname=nm)
    # the way out: the east road leaves the vale into the test road level (its north end)
    rx = X1 - 12 - 3.0
    br = p.brush_box((rx - 1.5, 0, cz + 8 - 2.5), (rx + 1.5, 3, cz + 8 + 2.5), "common/trigger")
    p.ent("sim_portal", "e_to_road", brushes=[br], targetname="to_road", targetLevel="sim_test_road.bsp", targetSpawn="from_vale")
    p.ent("sim_spawn", "e_from_road", (rx - 7.0, 0.1, cz + 8), 270, targetname="from_road")
    # light: a placeholder daylight until the style provider (SLICE_M4 section 4) bakes the four styles
    p.ent("light_directional", "e_sun", (0, 60, 0), 0, brightness=1.4, color="1 0.96 0.88", style=0)
    p.ents[-1]["angles"] = [55, 30, 0]
    # forests
    rng = random.Random(7)
    n = 0
    step = 9.0
    gz = Z0 + 4
    while forest and gz < Z1 - 4:
        gx = X0 + 4
        while gx < X1 - 4:
            x = gx + rng.uniform(-3.2, 3.2); z = gz + rng.uniform(-3.2, 3.2)
            m = math.sin(x / 70 + 1) * math.sin(z / 55 + 2) + 0.8 * math.sin(x / 37 + z / 53) + 0.5 * math.cos(z / 29 - x / 61)
            near_edge = x < X0 + 40 or x > X1 - 40 or z < Z0 + 30 or z > Z1 - 30
            if (m > -0.15 or near_edge) and not any(r[0] <= x <= r[2] and r[1] <= z <= r[3] for r in keep):
                kind = "tree" if rng.random() < 0.18 else "tree_far"
                p.inst(f"prefabs/props/{kind}.map.json", f"t{n}", (x, 0, z), rng.uniform(0, 360))
                n += 1
            gx += step
        gz += step
    return p, n


def save(name="vale", only=None):
    p, n = build(only, only is None)
    p.save(f"maps/{name}.map.json", root_kv={"_sky_auto": "box", "_sky_texture": "env/skybox1_cube", "message": "The Vale"})
    return n
