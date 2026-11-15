"""Draws the blood decal atlas for the gore system (an offline tool: run it, commit the png, nothing is generated while the game runs).

    python Tools/gore/make_blood_decals.py [output.png]

Default output: source/Games/StealthAction/GameData/textures/gore/blood_decals.png

The atlas is 4x2 cells of CELL texels. The top row holds four blood splashes (slashes, blasts, exits), the bottom row four bullet
holes (a black hole, a dark torn ring and the blood soaked around it, not star-shaped). Per texel:
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


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else OUT
    rng = np.random.default_rng(20261115)
    atlas = np.zeros((CELL * 2, CELL * 4, 4), np.uint8)
    atlas[..., 3] = 255
    for row, makers in enumerate(((impact, spray, flecks, splotch), holes())):
        for col, make in enumerate(makers):
            cov, thick = make(rng)
            y, x = row * CELL, col * CELL
            atlas[y:y + CELL, x:x + CELL, 0] = np.round(cov * 255)
            atlas[y:y + CELL, x:x + CELL, 1] = np.round(thick * cov * 255)   # thickness only where there is blood
    Image.fromarray(atlas).save(out)
    print('wrote', out, atlas.shape)


if __name__ == '__main__':
    main()
