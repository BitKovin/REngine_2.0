"""Builds a mask of the pixels where the LEGACY UI already differs between two backends.

usage: python -I make_cross_mask.py --game StealthAction --legacy-stem legacy_menus --legacy-label base1 --capture 01_hud.png
                                    --stem painter_card --out hudmask [--ref dx11] [--tolerance 2]

Writes Build/ui_runs/<game>/<stem>/<out>/<ref>/<name>.png for every capture name of the target stem (all use the same mask),
which `run_ui_tests.py cross --mask <out>` then skips. The mask is the union over all other backends of the pixels that differ
between the legacy capture on that backend and on the reference backend, grown by one pixel.
"""
import argparse
import os
import sys
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ui_png import read_png, write_png  # noqa: E402

REPO = Path(__file__).resolve().parents[1].parent


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--game", required=True)
    ap.add_argument("--legacy-stem", required=True)
    ap.add_argument("--legacy-label", required=True)
    ap.add_argument("--capture", required=True)
    ap.add_argument("--stem", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--names", default="card.png")
    ap.add_argument("--ref", default="dx11")
    ap.add_argument("--tolerance", type=int, default=2)
    a = ap.parse_args()

    legacy = REPO / "Build" / "ui_runs" / a.game / a.legacy_stem / a.legacy_label
    w, h, ref = read_png(legacy / a.ref / a.capture)
    mask = bytearray(w * h * 4)
    marked = 0
    for backend in sorted(os.listdir(legacy)):
        if backend == a.ref:
            continue
        _, _, other = read_png(legacy / backend / a.capture)
        for p in range(w * h):
            o = p * 4
            if max(abs(ref[o + i] - other[o + i]) for i in range(4)) > a.tolerance:
                px, py = p % w, p // w
                for dy in (-1, 0, 1):
                    for dx in (-1, 0, 1):
                        xx, yy = px + dx, py + dy
                        if 0 <= xx < w and 0 <= yy < h:
                            q = (yy * w + xx) * 4
                            if mask[q + 3] == 0:
                                marked += 1
                            mask[q:q + 4] = b"\xff\xff\xff\xff"
    out = REPO / "Build" / "ui_runs" / a.game / a.stem / a.out / a.ref
    out.mkdir(parents=True, exist_ok=True)
    for name in a.names.split(","):
        write_png(out / name, w, h, mask)
    print(f"mask written: {marked} pixels masked -> {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
