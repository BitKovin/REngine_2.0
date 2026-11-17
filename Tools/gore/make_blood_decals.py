"""Draws the blood decal atlas for the gore system (an offline tool: run it, commit the png, nothing is generated while the game runs).

    python Tools/gore/make_blood_decals.py [output.png]

Default output: source/Games/StealthAction/GameData/textures/gore/blood_decals.png

The atlas is 4x5 cells of CELL texels, one row per kind of mark (GoreTypes.h DecalKind is the row):
    0 splashes (blasts, exits)                  1 bullet holes (a black hole, a dark torn ring, blood soaked around it)
    2 slashes (a thin cut with a bloody edge)   3 shotgun spreads (a cluster of small holes)   4 bruises (blunt blows)
Per texel:
    R = coverage (how much of the skin the blood covers, 0..1)
    G = how thick and dark it lies: low = thin fresh blood (drops, tips of the arms), high = the deep pool
    B = 0, A = 255 (no alpha: it is never premultiplied or dropped by a loader)
Everything stays inside a circle of radius 0.93 of the cell: the shader lays the cell on a sphere of the decal's size, and the
mip chain of the engine averages cells, so the border of every cell is empty.
"""
import sys
import numpy as np
from PIL import Image
from scipy import ndimage

CELL = 256
OUT = 'source/Games/StealthAction/GameData/textures/gore/blood_decals.png'

ys, xs = np.mgrid[0:CELL, 0:CELL]
U = (xs + 0.5) / CELL * 2.0 - 1.0
V = (ys + 0.5) / CELL * 2.0 - 1.0


def smoothstep(a, b, x):
    t = np.clip((x - a) / (b - a), 0.0, 1.0)
    return t * t * (3.0 - 2.0 * t)


class Splash:
    """A union of soft discs: the pool, the arms (chains of shrinking discs) and the drops."""

    def __init__(self, rng):
        self.rng = rng
        self.field = np.zeros((CELL, CELL), np.float32)

    def disc(self, cx, cy, r):
        d = np.sqrt((U - cx) ** 2 + (V - cy) ** 2)
        np.maximum(self.field, 1.0 - smoothstep(r * 0.8, r * 1.2, d), out=self.field)

    def blob(self, cx, cy, r, lumps, lump_r):
        """A pool with an uneven outline: a disc plus lumps around its rim."""
        self.disc(cx, cy, r)
        for k in range(lumps):
            a = self.rng.uniform(0, 2 * np.pi)
            rr = r * self.rng.uniform(0.75, 1.05)
            self.disc(cx + np.cos(a) * rr, cy + np.sin(a) * rr, r * self.rng.uniform(*lump_r))

    def arm(self, ox, oy, angle, length, base_r, tip_r, curve):
        """A splash arm: discs along a slightly curved line, thick at the root, tapering to a point."""
        steps = max(8, int(length / 0.012))
        phase = self.rng.uniform(0, 6.28)
        for i in range(steps + 1):
            t = i / steps
            a = angle + curve * np.sin(t * 2.4 + phase) * t
            d = t * length
            self.disc(ox + np.cos(a) * d, oy + np.sin(a) * d, base_r * (1.0 - t) ** 1.4 + tip_r)

    def drops(self, ox, oy, count, near, far, angle, spread, size):
        """Drops flung out around an angle (spread = how far from it): smaller the farther they went."""
        for _ in range(count):
            a = angle + self.rng.uniform(-spread, spread)
            d = self.rng.uniform(near, far)
            r = size * (1.15 - d) * self.rng.uniform(0.5, 1.1)
            self.disc(ox + np.cos(a) * d, oy + np.sin(a) * d, max(r, 0.009))

    def mask(self):
        """The union of the discs as a smooth 0..1 mask (no edge noise)."""
        return smoothstep(0.4, 0.6, ndimage.gaussian_filter(self.field, 1.2))

    def render(self):
        """(coverage, thickness) in 0..1."""
        rng = self.rng
        f = ndimage.gaussian_filter(self.field, 1.6)
        n = ndimage.gaussian_filter(rng.standard_normal((CELL, CELL)).astype(np.float32), 3.0)
        n = n / n.std() * 0.07 + ndimage.gaussian_filter(rng.standard_normal((CELL, CELL)).astype(np.float32), 1.0) * 0.05
        cov = smoothstep(0.38, 0.62, f + np.clip(n, -0.25, 0.25))
        cov *= 1.0 - smoothstep(0.88, 0.93, np.sqrt(U ** 2 + V ** 2))   # nothing at the border of the cell
        # Thick where there is a lot of blood around (the pool), thin on the arms and drops, with an uneven patchy tone.
        pool = ndimage.gaussian_filter(cov, CELL * 0.045)
        mottle = ndimage.gaussian_filter(rng.standard_normal((CELL, CELL)).astype(np.float32), 5.0)
        mottle = mottle / mottle.std() * 0.05
        thick = 0.30 + 0.45 * smoothstep(0.12, 0.7, pool) + mottle
        return cov.astype(np.float32), np.clip(thick, 0.0, 0.9).astype(np.float32)


def impact(rng):
    """A round impact: pool in the middle, arms all around, drops along the arms."""
    s = Splash(rng)
    s.blob(0, 0, 0.20, 9, (0.35, 0.6))
    arms = rng.integers(10, 14)
    base = rng.uniform(0, 6.28)
    for k in range(arms):
        a = base + (k + rng.uniform(-0.3, 0.3)) * 6.2832 / arms
        s.arm(0, 0, a, rng.uniform(0.35, 0.82), rng.uniform(0.045, 0.085), 0.010, rng.uniform(-0.35, 0.35))
        s.drops(0, 0, 2, 0.5, 0.9, a, 0.12, 0.05)
    s.drops(0, 0, 8, 0.3, 0.85, 0, 3.14, 0.04)
    return s.render()


def spray(rng):
    """Thrown out to one side: a smaller pool pushed back, long arms inside a cone, a trail of drops."""
    s = Splash(rng)
    s.blob(-0.22, 0, 0.14, 7, (0.4, 0.7))
    for k in range(9):
        a = rng.uniform(-0.75, 0.75)
        s.arm(-0.22, 0, a, rng.uniform(0.5, 1.05), rng.uniform(0.035, 0.07), 0.009, rng.uniform(-0.2, 0.2))
    s.drops(-0.22, 0, 16, 0.3, 0.95, 0.0, 0.55, 0.05)
    s.drops(-0.22, 0, 4, 0.1, 0.35, 3.14, 0.9, 0.04)
    return s.render()


def flecks(rng):
    """A light spatter: a small pool and fine drops all around, hardly any arms."""
    s = Splash(rng)
    s.blob(0, 0, 0.11, 6, (0.4, 0.75))
    for k in range(6):
        s.arm(0, 0, rng.uniform(0, 6.28), rng.uniform(0.22, 0.45), rng.uniform(0.025, 0.045), 0.008, 0.3)
    s.drops(0, 0, 34, 0.18, 0.88, 0, 3.14, 0.055)
    return s.render()


def splotch(rng):
    """A wide uneven pool with short stubby arms and a few big drops."""
    s = Splash(rng)
    s.blob(0, 0, 0.26, 8, (0.25, 0.45))
    s.blob(rng.uniform(-0.12, 0.12), rng.uniform(-0.12, 0.12), 0.16, 4, (0.3, 0.5))
    for k in range(6):
        s.arm(0, 0, rng.uniform(0, 6.28), rng.uniform(0.4, 0.7), rng.uniform(0.05, 0.08), 0.010, rng.uniform(-0.4, 0.4))
    s.drops(0, 0, 10, 0.5, 0.9, 0, 3.14, 0.06)
    return s.render()


def wobble(rng, ang, amp):
    """A smooth, closed wobble of an outline along the angle: about +-amp."""
    w = np.zeros_like(ang)
    for k in range(2, 8):
        w += rng.uniform(0.25, 1.0) / k * np.cos(k * ang + rng.uniform(0, 6.28))
    return w * amp * 1.6


def bullet_hole(rng, smear=0.0, trickle=False, droplets=0):
    """A bullet hole: the black hole, a thin dark torn ring around it, and blood soaked into the skin around that. The soaked patch is
    round and uneven (smear stretches it to one side), a trickle runs out of it, drops lie around it."""
    r0 = np.sqrt(U ** 2 + V ** 2)
    ang = np.arctan2(V, U)
    edge = ndimage.gaussian_filter(rng.standard_normal((CELL, CELL)).astype(np.float32), 3.0)
    edge = edge / edge.std() * 0.045

    # The soaked patch, centred on the hole, stretched along a direction by `smear`.
    phi = rng.uniform(0, 6.28)
    along = U * np.cos(phi) + V * np.sin(phi)
    across = -U * np.sin(phi) + V * np.cos(phi)
    d = np.sqrt((along / (1.0 + smear) - 0.12 * smear) ** 2 + across ** 2)
    rh = 0.40 * (1.0 + wobble(rng, np.arctan2(across, along), 0.14))
    halo = 1.0 - smoothstep(0.80, 1.0, d / rh + edge)

    # Hole and ring, each with its own uneven outline.
    r_hole = 0.115 * (1.0 + wobble(rng, ang, 0.12))
    r_ring = r_hole + 0.065 * (1.0 + wobble(rng, ang, 0.25))
    hole = 1.0 - smoothstep(r_hole - 0.012, r_hole, r0 + edge * 0.15)
    ring = 1.0 - smoothstep(r_ring - 0.014, r_ring, r0 + edge * 0.25)

    cov = np.maximum(halo, ring)
    mottle = ndimage.gaussian_filter(rng.standard_normal((CELL, CELL)).astype(np.float32), 5.0)
    mottle = mottle / mottle.std() * 0.05
    thick = 0.28 + 0.16 * np.clip(1.0 - d / rh, 0.0, 1.0) + mottle
    thick = thick * (1.0 - ring) + 0.88 * ring
    thick = thick * (1.0 - hole) + 1.0 * hole

    extra = Splash(rng)
    if trickle:
        a = rng.uniform(0, 6.28)
        extra.arm(0, 0, a, 0.88, 0.036, 0.020, rng.uniform(-0.25, 0.25))
        extra.disc(np.cos(a) * 0.88, np.sin(a) * 0.88, 0.04)
    extra.drops(0, 0, droplets, 0.5, 0.88, 0, 3.14, 0.05)
    if trickle or droplets:
        m = extra.mask() * (1.0 - smoothstep(0.88, 0.93, r0))
        cov = np.maximum(cov, m)
        thick = np.where(m > halo, 0.40, thick)
    cov *= 1.0 - smoothstep(0.88, 0.93, r0)
    return np.clip(cov, 0, 1).astype(np.float32), np.clip(thick, 0.0, 1.0).astype(np.float32)


def holes():
    return (lambda rng: bullet_hole(rng),
            lambda rng: bullet_hole(rng, smear=0.5),
            lambda rng: bullet_hole(rng, smear=0.15, trickle=True),
            lambda rng: bullet_hole(rng, smear=0.25, droplets=9))


def edge_noise(rng, sigma, amp):
    n = ndimage.gaussian_filter(rng.standard_normal((CELL, CELL)).astype(np.float32), sigma)
    return n / n.std() * amp


def finish(cov, thick):
    cov = cov * (1.0 - smoothstep(0.88, 0.93, np.sqrt(U ** 2 + V ** 2)))   # nothing at the border of the cell
    return np.clip(cov, 0, 1).astype(np.float32), np.clip(thick, 0.0, 1.0).astype(np.float32)


def slash(rng, trickles=0, double=False, droplets=0):
    """A cut along the horizontal axis (the shader turns the cell): a thin open lens, dark inside, with a narrow band of blood along
    its edges, thicker in the middle. Trickles run out of it sideways, drops lie beside it."""
    mottle = edge_noise(rng, 5.0, 0.05)
    edge = edge_noise(rng, 3.0, 0.010)
    cov = np.zeros((CELL, CELL), np.float32)
    thick = np.zeros((CELL, CELL), np.float32)
    lines = [(0.0, 1.0)] if not double else [(-0.20, 0.9), (0.18, 0.75)]
    for off, scale in lines:
        half = rng.uniform(0.62, 0.78) * scale
        bend = rng.uniform(-0.12, 0.12)
        tilt = rng.uniform(-0.08, 0.08)
        t = np.clip(U / half, -1.0, 1.0)
        centre = off + bend * (1.0 - t ** 2) + tilt * U
        d = np.abs(V - centre)
        along = np.sqrt(np.clip(1.0 - t ** 2, 0.0, 1.0))
        lens = 0.034 * along * (1.0 + 0.25 * np.sin(U * 17.0 + rng.uniform(0, 6.28)))   # the opening
        band = 0.030 + 0.050 * along ** 0.7                                            # the blood along it
        cut = 1.0 - smoothstep(lens - 0.012, lens + 0.004, d + edge * 0.25)
        halo = (1.0 - smoothstep(band * 0.55, band, d + edge)) * (1.0 - smoothstep(0.85, 1.05, np.abs(U) / half))
        c = np.maximum(halo, cut)
        th = (0.30 + 0.20 * along + mottle) * (1.0 - cut) + 1.0 * cut
        cov = np.maximum(cov, c)
        thick = np.maximum(thick, th * c)
    extra = Splash(rng)
    for _ in range(trickles):
        x = rng.uniform(-0.4, 0.4)
        down = 1.0 if rng.random() < 0.5 else -1.0
        extra.arm(x, 0.0, down * np.pi / 2 + rng.uniform(-0.25, 0.25), rng.uniform(0.35, 0.8), 0.030, 0.014, rng.uniform(-0.15, 0.15))
    extra.drops(0, 0, droplets, 0.2, 0.8, 0, 3.14, 0.05)
    if trickles or droplets:
        m = extra.mask()
        cov = np.maximum(cov, m)
        thick = np.where(m > 0.5, np.maximum(thick, 0.40 * m), thick)
    return finish(cov, np.where(cov > 0, thick / np.maximum(cov, 1e-3), 0))


def pellets(rng, count, spread, big=False):
    """The marks of a shotgun blast: small holes scattered over a disc (dense in the middle), each with a dark ring and a little blood
    around it, the blood of neighbours running together. big: one larger hole in the middle."""
    cov = np.zeros((CELL, CELL), np.float32)
    thick = np.zeros((CELL, CELL), np.float32)
    soak = np.zeros((CELL, CELL), np.float32)
    mottle = edge_noise(rng, 5.0, 0.05)
    spots = []
    if big:
        spots.append((0.0, 0.0, 0.060))
    for _ in range(count):
        a = rng.uniform(0, 6.28)
        d = spread * np.sqrt(rng.uniform(0, 1)) ** 1.3
        spots.append((np.cos(a) * d, np.sin(a) * d, rng.uniform(0.020, 0.036)))
    for cx, cy, r in spots:
        d = np.sqrt((U - cx) ** 2 + (V - cy) ** 2)
        hole = 1.0 - smoothstep(r - 0.008, r, d)
        ring = 1.0 - smoothstep(r + 0.020, r + 0.034, d)
        halo = 1.0 - smoothstep(r * 2.0, r * 3.6, d)
        soak = np.maximum(soak, halo)
        cov = np.maximum(cov, np.maximum(halo, ring))
        th = (0.30 + mottle) * (1.0 - ring) + 0.88 * ring
        th = th * (1.0 - hole) + 1.0 * hole
        thick = np.maximum(thick, th * np.maximum(halo, ring))
    # Blood that ran together between the holes.
    blend = ndimage.gaussian_filter(soak, CELL * 0.03)
    patch = smoothstep(0.35, 0.7, blend) * 0.8
    cov = np.maximum(cov, patch)
    thick = np.maximum(thick, (0.26 + mottle) * patch)
    return finish(cov, np.where(cov > 0, thick / np.maximum(cov, 1e-3), 0))


def bruise(rng, stretch=1.0, scrape=False, spatter=0):
    """A blunt blow: a wide, dull, uneven patch of blood-darkened skin, not a hole. Optional scrape (a short split) and fine spatter."""
    phi = rng.uniform(0, 6.28)
    along = U * np.cos(phi) + V * np.sin(phi)
    across = -U * np.sin(phi) + V * np.cos(phi)
    d = np.sqrt((along / stretch) ** 2 + across ** 2)
    rb = 0.55 * (1.0 + wobble(rng, np.arctan2(across, along), 0.18))
    patch = 1.0 - smoothstep(0.55, 1.0, d / rb + edge_noise(rng, 3.0, 0.06))
    mottle = edge_noise(rng, 6.0, 0.35) + edge_noise(rng, 2.0, 0.12)
    cov = np.clip(patch * (0.62 + mottle), 0.0, 0.85)
    thick = np.clip(0.50 + 0.25 * patch + 0.4 * mottle, 0.25, 0.8)
    if scrape:
        t = np.clip(along / 0.4, -1, 1)
        line = np.abs(across - 0.05 * np.sin(along * 9.0))
        cut = (1.0 - smoothstep(0.012 * np.sqrt(np.clip(1 - t ** 2, 0, 1)), 0.026, line)) * (np.abs(along) < 0.4)
        cov = np.maximum(cov, cut)
        thick = np.where(cut > 0.5, 1.0, thick)
    if spatter:
        extra = Splash(rng)
        extra.drops(0, 0, spatter, 0.35, 0.85, 0, 3.14, 0.045)
        m = extra.mask()
        cov = np.maximum(cov, m)
        thick = np.where(m > 0.5, 0.40, thick)
    return finish(cov, thick)


def slashes():
    return (lambda rng: slash(rng),
            lambda rng: slash(rng, trickles=2),
            lambda rng: slash(rng, double=True),
            lambda rng: slash(rng, trickles=1, droplets=8))


def spreads():
    return (lambda rng: pellets(rng, 9, 0.50),
            lambda rng: pellets(rng, 14, 0.62),
            lambda rng: pellets(rng, 7, 0.38, big=True),
            lambda rng: pellets(rng, 11, 0.55, big=True))


def bruises():
    return (lambda rng: bruise(rng),
            lambda rng: bruise(rng, stretch=1.7),
            lambda rng: bruise(rng, scrape=True),
            lambda rng: bruise(rng, stretch=1.2, spatter=10))


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else OUT
    rng = np.random.default_rng(20261115)
    atlas = np.zeros((CELL * 5, CELL * 4, 4), np.uint8)
    atlas[..., 3] = 255
    for row, makers in enumerate(((impact, spray, flecks, splotch), holes(), slashes(), spreads(), bruises())):
        for col, make in enumerate(makers):
            cov, thick = make(rng)
            y, x = row * CELL, col * CELL
            atlas[y:y + CELL, x:x + CELL, 0] = np.round(cov * 255)
            atlas[y:y + CELL, x:x + CELL, 1] = np.round(thick * cov * 255)   # thickness only where there is blood
    Image.fromarray(atlas).save(out)
    print('wrote', out, atlas.shape)


if __name__ == '__main__':
    main()
