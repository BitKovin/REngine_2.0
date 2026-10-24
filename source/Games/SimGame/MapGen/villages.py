"""The three village prefabs (GameData/prefabs/village_a|b|c.map.json), each about 100 x 100 m around its own origin (x east, z south), built from building and prop prefabs.
Every village: roads and a green, buildings facing them, a mill with its pond, a field, lamps; the sim sites that are not buildings (settlement, green or square, well, field).
`check` verifies that nothing overlaps and that every door can be reached from the village's gates on a 0.5 m grid (the engine's nav bake is the final word).
"""
import math
from kit import P, yaw_xz
import buildings as bl
import props as pr

COBBLE, DIRT, STONE, WATER = "lq_terra/med_cobstn1_1", "lq_terra/may_drt1_1", "lq_medieval/med_block_1a", "lq_dev/floor_blue_a"
SOIL, GRASS = "lq_terra/med_bigdirt", "Ground/grass"

# footprints in the prefab's own frame (x0, x1, z0, z1), margins included; door-out points (where agents stand outside the door) as the buildings place their sim_door
FOOT = {"house_open": (-4.8, 4.8, -4.8, 4.8), "house_shell": (-4.8, 4.8, -4.8, 4.8), "bakery": (-6.3, 6.3, -5.8, 5.8), "mill": (-13.6, 6.3, -5.8, 5.8),
        "tavern": (-7.0, 7.0, -7.0, 7.0), "church": (-5.2, 5.2, -8.6, 13.0)}
DOOR = {"house_open": (0, 5.3), "house_shell": (0, 5.3), "bakery": (-3.0, 6.3), "mill": (0, 6.3), "tavern": (0, 7.3), "church": (0, 13.4)}
POND = (-13.6, -7.0, -5.5, 5.5)      # local to the mill


def _rect_poly(x0, x1, z0, z1, yaw, cx, cz):
    return [(cx + yaw_xz(yaw, x, z)[0], cz + yaw_xz(yaw, x, z)[1]) for x, z in ((x0, z0), (x1, z0), (x1, z1), (x0, z1))]


def _hit(p, q):
    for poly in (p, q):
        for i in range(len(poly)):
            x1, z1 = poly[i]; x2, z2 = poly[(i + 1) % len(poly)]
            nx, nz = z1 - z2, x2 - x1
            a = [nx * x + nz * z for x, z in p]; b = [nx * x + nz * z for x, z in q]
            if max(a) <= min(b) + 1e-6 or max(b) <= min(a) + 1e-6:
                return False
    return True


class V:
    def __init__(self, key, name):
        self.key, self.name = key, name
        self.p = P()
        self.foot, self.roads, self.blocks, self.doors, self.gates, self.lamps_at = [], [], [], [], [], []
        self.n_house = 0
        self.problems = []

    # --- placing --------------------------------------------------------------------------------------------------
    def bld(self, kind, name, x, z, yaw, y=0.0, label=None):
        p = self.p
        ov = {}
        if label:
            ov["e_site"] = {"keyvalues": {"displayName": label}}
        p.inst(f"prefabs/buildings/{kind}.map.json", name, (x, y, z), yaw, ov)
        x0, x1, z0, z1 = FOOT[kind]
        self.foot.append((name, _rect_poly(x0, x1, z0, z1, yaw, x, z), True))
        if kind == "mill":
            px0, px1, pz0, pz1 = POND
            self.foot[-1] = (name, _rect_poly(-6.3, 6.3, -5.8, 5.8, yaw, x, z), True)
            self.foot.append((name + "_pond", _rect_poly(px0, px1, pz0, pz1, yaw, x, z), False))
            self._pond(x, z, yaw)
        dx, dz = DOOR[kind]
        ox, oz = yaw_xz(yaw, dx, dz)
        self.doors.append((name, (x + ox, z + oz)))

    def house(self, x, z, yaw, open_=False, label=None):
        self.n_house += 1
        self.bld("house_open" if open_ else "house_shell", f"house{self.n_house}", x, z, yaw, label=label)

    def road(self, x0, z0, x1, z1, tex=COBBLE, h=0.04):
        self.p.box((x0, -0.3, z0), (x1, h, z1), tex, True)
        self.roads.append((x0, x1, z0, z1))

    def lamp(self, x, z):
        self.p.inst("prefabs/props/lamp_post.map.json", f"lamp{len(self.lamps_at) + 1}", (x, 0, z), 0)
        self.lamps_at.append((x, z))

    def lamps_along(self, x0, z0, x1, z1, step=15.0, off=0.7, skip=()):
        """Lamps on both edges of a straight road, every `step` metres."""
        horiz = (x1 - x0) >= (z1 - z0)
        a0, a1 = (x0, x1) if horiz else (z0, z1)
        n = max(1, int((a1 - a0) // step))
        for i in range(n + 1):
            a = a0 + 2 + i * (a1 - a0 - 4) / max(1, n)
            for side in (0, 1):
                pos = (a, (z0 - off) if side == 0 else (z1 + off)) if horiz else ((x0 - off) if side == 0 else (x1 + off), a)
                if i % 2 == side and not any(abs(pos[0] - sx) < 6 and abs(pos[1] - sz) < 6 for sx, sz in skip):
                    self.lamp(*pos)

    def tree(self, x, z, kind="tree"):
        self.p.inst(f"prefabs/props/{kind}.map.json", f"tree{len(self.p.insts)}", (x, 0, z), (x * 7 + z * 13) % 360)
        self.blocks.append((x - 0.5, x + 0.5, z - 0.5, z + 0.5))

    def prop(self, name, x, z, yaw=0.0, y=0.0, tag=None):
        self.p.inst(f"prefabs/props/{name}.map.json", f"{tag or name}{len(self.p.insts)}", (x, y, z), yaw)

    def _pond(self, x, z, yaw):
        p = self.p
        px0, px1, pz0, pz1 = POND
        c = lambda lx, lz: tuple(round(v, 3) for v in (x + yaw_xz(yaw, lx, lz)[0], z + yaw_xz(yaw, lx, lz)[1]))
        # axis-aligned water and curbs in the world frame of the village, whichever way the mill faces: use the rotated rectangle's hull
        pts = [(*c(lx, lz), 0) for lx, lz in ((px0, pz0), (px1, pz0), (px1, pz1), (px0, pz1))]
        water = [(a, -0.6, b) for a, b, _ in pts] + [(a, 0.06, b) for a, b, _ in pts]
        p.hull(water, WATER, True)
        for (la, lb), (lc, ld) in (((px0, pz0), (px1, pz0)), ((px1, pz0), (px1, pz1)), ((px1, pz1), (px0, pz1)), ((px0, pz1), (px0, pz0))):
            t = 0.4
            ax, az = c(la, lb); bx, bz = c(lc, ld)
            dx, dz = bx - ax, bz - az
            L = math.hypot(dx, dz)
            nx, nz = -dz / L * t / 2, dx / L * t / 2
            quad = [(ax - nx, -0.3, az - nz), (bx - nx, -0.3, bz - nz), (bx + nx, -0.3, bz + nz), (ax + nx, -0.3, az + nz)]
            p.hull(quad + [(a, 0.7, b) for a, _, b in quad], STONE, True)

    def field(self, x0, z0, x1, z1, door_at, sid="field"):
        p = self.p
        z = z0 + 1.0
        while z < z1 - 0.5:
            p.box((x0 + 0.5, 0, z), (x1 - 0.5, 0.22, z + 1.0), SOIL, True)
            z += 2.2
        p.ent("sim_site", "e_" + sid, brushes=[p.brush_box((x0, -0.2, z0), (x1, 2.0, z1), "lq_dev/trigger")], targetname=sid, kind="field", displayName="Field", playable=1, capacity=12)
        p.ent("sim_door", f"e_{sid}_door", (door_at[0], 0, door_at[1]), 0, refs={"site": "e_" + sid}, targetname=sid + "_door")
        k = 0
        for i in range(4):
            for j in range(2):
                p.ent("sim_slot", f"e_{sid}_slot{k}", (x0 + (x1 - x0) * (i + 0.5) / 4, 0, z0 + (z1 - z0) * (j + 0.5) / 2), 0, refs={"site": "e_" + sid}, targetname=f"{sid}_slot{k}", slotType="field")
                k += 1
        self.prop("haystack", x1 - 2, z0 + 2, 0)

    def open_site(self, sid, lo, hi, kind, label, door_at, slots, capacity=12):
        """An outdoor site (green, square, well): a footprint, an entry point and `slots` stand slots on a ring or row."""
        p = self.p
        p.ent("sim_site", "e_" + sid, brushes=[p.brush_box(lo, hi, "lq_dev/trigger")], targetname=sid, kind=kind, displayName=label, playable=1, capacity=capacity)
        p.ent("sim_door", f"e_{sid}_door", (door_at[0], 0, door_at[1]), 0, refs={"site": "e_" + sid}, targetname=sid + "_door")
        for k, (sx, sz, syaw) in enumerate(slots):
            p.ent("sim_slot", f"e_{sid}_slot{k}", (sx, 0, sz), syaw, refs={"site": "e_" + sid}, targetname=f"{sid}_slot{k}", slotType="stand")

    def settlement(self, label):
        p = self.p
        p.ent("sim_settlement", "e_settlement", brushes=[p.brush_box((-50, -0.5, -50), (50, 0.5, 50), "lq_dev/trigger")], targetname="settlement", displayName=label)

    def churchyard(self, x0, z0, x1, z1, gate_side, gate_c, gate_w=3.0, y0=0.0, stones=6, wall_h=1.4):
        """Low stone wall around a churchyard with a gap (the gate) on one side ('N', 'S', 'E', 'W' centred at gate_c), and a few gravestones."""
        t, h = 0.5, y0 + wall_h
        sides = {"N": (x0, z0, x1, z0 + t), "S": (x0, z1 - t, x1, z1), "W": (x0, z0, x0 + t, z1), "E": (x1 - t, z0, x1, z1)}
        for s, (a, b, c, d) in sides.items():
            if s == gate_side:
                if s in "NS":
                    self.p.box((a, y0, b), (gate_c - gate_w / 2, h, d), STONE, True); self.p.box((gate_c + gate_w / 2, y0, b), (c, h, d), STONE, True)
                    self.blocks += [(a, gate_c - gate_w / 2, b, d), (gate_c + gate_w / 2, c, b, d)]
                else:
                    self.p.box((a, y0, b), (c, h, gate_c - gate_w / 2), STONE, True); self.p.box((a, y0, gate_c + gate_w / 2), (c, h, d), STONE, True)
                    self.blocks += [(a, c, b, gate_c - gate_w / 2), (a, c, gate_c + gate_w / 2, d)]
            else:
                self.p.box((a, y0, b), (c, h, d), STONE, True)
                self.blocks.append((a, c, b, d))
        for k in range(stones):
            gx = x0 + 2.5 + ((k * 37) % 10) / 10 * (x1 - x0 - 5)
            gz = z0 + 3 + ((k * 53) % 10) / 10 * 8
            self.prop("gravestone", gx, gz, (k * 40) % 30 - 15, y=y0)

    # --- checks ---------------------------------------------------------------------------------------------------
    def check(self, gates):
        """Overlaps (buildings against each other, against roads) and reachability of every door from the gates on a 0.5 m grid."""
        probs = []
        polys = [(n, poly, solid) for n, poly, solid in self.foot]
        for i, (n, a, _) in enumerate(polys):
            for m, b, _ in polys[i + 1:]:
                if n.split("_pond")[0] != m.split("_pond")[0] and _hit(a, b):
                    probs.append(f"{n} overlaps {m}")
            for x0, x1, z0, z1 in self.roads:
                if _hit(a, [(x0, z0), (x1, z0), (x1, z1), (x0, z1)]) and not n.endswith("_pond"):
                    probs.append(f"{n} overlaps a road {x0, z0, x1, z1}")
            for x, z in a:
                if abs(x) > 50 or abs(z) > 50:
                    probs.append(f"{n} outside the village")
        S, N = 0.5, 200
        blocked = [[False] * N for _ in range(N)]
        def mark(poly):
            xs = [p[0] for p in poly]; zs = [p[1] for p in poly]
            for i in range(int((min(xs) + 50) / S) - 1, int((max(xs) + 50) / S) + 2):
                for j in range(int((min(zs) + 50) / S) - 1, int((max(zs) + 50) / S) + 2):
                    if 0 <= i < N and 0 <= j < N and _hit(poly, [(i * S - 50, j * S - 50), (i * S - 50 + S, j * S - 50), (i * S - 50 + S, j * S - 50 + S), (i * S - 50, j * S - 50 + S)]):
                        blocked[i][j] = True
        for n, poly, solid in polys:
            mark(poly)
        for x0, x1, z0, z1 in self.blocks:
            mark([(x0, z0), (x1, z0), (x1, z1), (x0, z1)])
        seen = set()
        stack = []
        for gx, gz in gates:
            i, j = int((gx + 50) / S), int((gz + 50) / S)
            stack.append((min(max(i, 0), N - 1), min(max(j, 0), N - 1)))
        while stack:
            i, j = stack.pop()
            if (i, j) in seen or not (0 <= i < N and 0 <= j < N) or blocked[i][j]:
                continue
            seen.add((i, j))
            stack += [(i + 1, j), (i - 1, j), (i, j + 1), (i, j - 1)]
        for n, (dx, dz) in self.doors:
            if (int((dx + 50) / S), int((dz + 50) / S)) not in seen:
                probs.append(f"door of {n} at ({dx:.1f}, {dz:.1f}) is not reachable from the gates")
        return probs


# ======================================================================================================== A: ribbon street
def village_a():
    v = V("a", "Ashby")
    p = v.p
    v.settlement("Ashby")
    v.road(-50, -3, 50, 3)
    v.road(37, -10, 41, -3)                                   # church gate lane
    v.road(34, 3, 38, 9.0)                                    # mill lane
    v.road(21, 3, 25, 26, DIRT)                               # field path
    v.house(-41, -10.5, 0, open_=True)
    v.house(-28.5, -10.5, 0)
    v.bld("bakery", "bakery", -14, -11, 0, label="Bakery")
    v.bld("tavern", "tavern", 3, -12.5, 0, label="The Tavern")
    v.house(17.5, -10.5, 0)
    v.bld("church", "church", 39, -28, 0, label="Church")
    v.road(37.5, -14.6, 40.5, -10, DIRT)
    v.churchyard(28, -42, 50, -10, "S", 39)
    v.house(-41, 10.5, 180)
    v.house(-28.5, 10.5, 180, open_=True)
    v.house(0.5, 10.5, 180)
    v.house(12.5, 10.5, 180)
    v.bld("mill", "mill", 36, 15, 180, label="Mill")
    v.p.box((-21, -0.3, 6), (-7, 0.03, 20), "Ground/grass", True)
    v.prop("well", -14, 13)
    v.blocks.append((-15.1, -12.9, 11.9, 14.1))
    v.open_site("green", (-21, -0.3, 6), (-7, 2.0, 20), "green", "Green", (-14, 5), [(-17, 11, 90), (-11, 11, 270), (-14, 17, 180), (-18, 16, 0), (-10, 16, 0)])
    v.open_site("well", (-15.5, -0.3, 11.5), (-12.5, 2.0, 14.5), "well", "Well", (-14, 10.4), [(-14, 10.5, 0), (-12, 13, 270), (-16, 13, 90), (-14, 15.5, 180)], 4)
    v.field(15, 28, 48, 48, (23, 27))
    v.lamps_along(-50, -3, 50, 3, 15)
    for x, z in ((-12, 8), (-18, 18), (-9, 18), (-46, -6), (30, 38), (-4, 30), (-30, 30), (-44, 28)):
        v.tree(x, z)
    for x in (-32, -20, -8):
        v.prop("fence", x, 21.5, 0)
    return v, [(-49, 0), (49, 0)]


# ======================================================================================================== B: green and ring
def village_b():
    v = V("b", "Brook End")
    p = v.p
    v.settlement("Brook End")
    v.road(-22, -19, 22, -14); v.road(-22, 14, 22, 19)
    v.road(-22, -14, -17, 14); v.road(17, -14, 22, 14)        # the ring
    v.road(-3, 19, 3, 50)                                     # entry road, the south gate
    v.road(-3, -24, 3, -19)                                   # church approach
    v.road(-1.5, -26.3, 1.5, -24, DIRT)
    v.road(22, -24, 44, -19)                                  # mill track
    v.p.box((-17, -0.3, -14), (17, 0.03, 14), "Ground/grass", True)
    v.prop("well", 0, 0)
    v.blocks.append((-1.1, 1.1, -1.1, 1.1))
    v.open_site("green", (-17, -0.3, -14), (17, 2.0, 14), "green", "Village green", (-14, 11), [(-8, -6, 0), (8, 6, 180), (-8, 8, 0), (8, -8, 180), (0, 7, 180), (0, -7, 0)], 14)
    v.open_site("well", (-1.5, -0.3, -1.5), (1.5, 2.0, 1.5), "well", "Well", (0, 1.6), [(0, 1.6, 180), (1.6, 0, 270), (-1.6, 0, 90), (0, -1.6, 0)], 4)
    v.bld("church", "church", 0, -39.9, 0, label="Church")
    v.churchyard(-17, -50, 17, -24, "S", 0)
    v.house(-30.5, -14, 90, open_=True)
    v.bld("bakery", "bakery", -31.5, 0, 90, label="Bakery")
    v.house(-30.5, 14, 90)
    v.house(30.5, -13.5, 270)
    v.bld("tavern", "tavern", 32.5, 0, 270, label="The Tavern")
    v.house(30.5, 14, 270, open_=True)
    v.house(-9, 28.5, 90)
    v.house(9, 28.5, 270)
    v.house(-9, 40.5, 90)
    v.house(9, 40.5, 270)
    v.bld("mill", "mill", 41, -30, 0, label="Mill")
    v.field(-48, -48, -26, -22, (-25, -20), "field_n")
    v.field(-48, 24, -26, 48, (-25, 30), "field_s")
    v.lamps_along(-22, -19, 22, -14, 15); v.lamps_along(-22, 14, 22, 19, 15); v.lamps_along(-3, 19, 3, 50, 15)
    v.lamps_along(-22, -14, -17, 14, 14); v.lamps_along(17, -14, 22, 14, 14)
    for x, z in ((-8, -6), (8, 6), (-10, 9), (10, -9), (-40, 20), (38, 28), (-24, 36), (24, 44), (46, -6)):
        v.tree(x, z)
    return v, [(0, 49)]


# ======================================================================================================== C: crossroads, church on a terrace (home)
def village_c():
    v = V("c", "Crossways")
    p = v.p
    v.settlement("Crossways")
    v.road(0, 5, 50, 11)                                      # main road from the east
    v.road(-18, 0, 0, 16)                                     # market square
    v.road(-12, -22, -8, 0)                                   # lane to the church
    v.road(-12, 16, -8, 40)                                   # lane to the mill
    v.road(-50, 4, -18, 8)                                    # west lane
    v.road(-18, 30, -12, 34)                                  # mill door lane
    v.road(32.5, 11, 35.5, 32, DIRT)                          # field path
    th, rise, tread = 2.4, 0.2, 0.3
    steps = int(round(th / rise))
    zc = -22 - steps * tread                                  # the terrace front
    p.box((-30, -0.3, -50), (6, th, zc), STONE)
    for i in range(steps):
        p.box((-12, -0.3, -22 - (i + 1) * tread), (-8, (i + 1) * rise, -22 - i * tread), STONE, True)
    p.box((-12.6, -0.3, zc), (-12, th, -22), STONE, True); p.box((-8, -0.3, zc), (-7.4, th, -22), STONE, True)
    v.blocks += [(-12.6, -12, zc, -22), (-8, -7.4, zc, -22)]
    v.bld("church", "church", -10, -41, 0, y=th, label="Church")
    v.churchyard(-30, -50, 6, zc, "S", -10, gate_w=4.0, y0=th, stones=8, wall_h=1.0)
    v.prop("well", -9, 8)
    v.blocks.append((-10.1, -7.9, 6.9, 9.1))
    v.open_site("square", (-18, -0.3, 0), (0, 2.0, 16), "market", "Market square", (-1.5, 8), [(-5, 3, 180), (-13, 3, 180), (-4, 13, 0), (-14, 13, 0), (-16, 8, 90)], 16)
    v.open_site("well", (-10.6, -0.3, 6.4), (-7.4, 2.0, 9.6), "well", "Well", (-9, 5.8), [(-9, 5.9, 0), (-7.2, 8, 270), (-10.8, 8, 90), (-9, 10.2, 180)], 4)
    v.bld("bakery", "bakery", 9, -3, 0, label="Bakery")
    v.house(25.5, -3.5, 5, open_=True)
    v.house(38.5, -3.5, 0)
    v.bld("tavern", "tavern", 12.5, 20.5, 180, label="The Tavern")
    v.house(27, 19.5, 180)
    v.house(41, 19.5, 180)
    v.house(-27, -2, 0, open_=True)
    v.house(-40, -2, 0)
    v.house(-27, 14, 180)
    v.house(-0.5, 28, 270)
    v.bld("mill", "mill", -24, 32, 90, label="Mill")
    v.field(14, 34, 48, 48, (34, 33))
    v.lamps_along(0, 5, 50, 11, 15); v.lamps_along(-50, 4, -18, 8, 16)
    v.lamps_along(-12, -22, -8, 0, 12); v.lamps_along(-12, 16, -8, 40, 16)
    for x, z in ((-20, 22), (-14, 26), (-45, 20), (-5, -18), (20, -20), (42, -20), (6, 36), (44, 30)):
        v.tree(x, z)
    return v, [(49, 8), (-49, 6)]


VILLAGES = {"village_a": village_a, "village_b": village_b, "village_c": village_c}
