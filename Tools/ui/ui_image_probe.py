"""Read pixels of a PNG and optionally check them.

single:  python -I ui_image_probe.py img.png X Y
file:    python -I ui_image_probe.py img.png --probes probes.txt
probes.txt lines: X Y R G B A TOL LABEL   ('#' comments allowed). Exit 1 if any probe fails.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ui_png import read_png  # noqa: E402


def pixel(img, x, y):
    w, h, d = img
    if not (0 <= x < w and 0 <= y < h):
        raise ValueError(f"({x},{y}) outside {w}x{h}")
    o = (y * w + x) * 4
    return tuple(d[o:o + 4])


def main(argv):
    if len(argv) == 3 and argv[1] != "--probes":
        img = read_png(argv[0])
        print(pixel(img, int(argv[1]), int(argv[2])))
        return 0
    if not (len(argv) == 3 and argv[1] == "--probes"):
        print(__doc__)
        return 2
    img = read_png(argv[0])
    failed = 0
    for raw in open(argv[2]):
        line = raw.split("#", 1)[0].strip()
        if not line:
            continue
        parts = line.split()
        x, y, r, g, b, a, tol = (int(v) for v in parts[:7])
        label = " ".join(parts[7:])
        got = pixel(img, x, y)
        ok = all(abs(got[i] - want) <= tol for i, want in enumerate((r, g, b, a)))
        print(f"{'OK  ' if ok else 'FAIL'} {label}: at ({x},{y}) got {got} want {(r, g, b, a)} tol {tol}")
        failed += 0 if ok else 1
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
