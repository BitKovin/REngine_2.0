"""Compare two PNGs, or build an ignore-mask from two baseline runs.

compare:   python -I ui_image_diff.py a.png b.png [--tolerance 2] [--mask m.png] [--out diff.png]
make mask: python -I ui_image_diff.py --make-mask run1.png run2.png --out mask.png
Exit code 0 = equal (within tolerance, outside the mask), 1 = different, 2 = error.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ui_png import read_png, write_png  # noqa: E402


def main(argv):
    args = list(argv)
    tol, mask_path, out_path, make_mask = 2, None, None, False
    if "--make-mask" in args:
        make_mask = True
        args.remove("--make-mask")
    for flag in ("--tolerance", "--mask", "--out"):
        if flag in args:
            i = args.index(flag)
            value = args[i + 1]
            del args[i:i + 2]
            if flag == "--tolerance":
                tol = int(value)
            elif flag == "--mask":
                mask_path = value
            else:
                out_path = value
    if len(args) != 2:
        print(__doc__)
        return 2
    wa, ha, a = read_png(args[0])
    wb, hb, b = read_png(args[1])
    if (wa, ha) != (wb, hb):
        print(f"SIZE MISMATCH {wa}x{ha} vs {wb}x{hb}")
        return 1
    mask = None
    if mask_path:
        wm, hm, mask = read_png(mask_path)
        if (wm, hm) != (wa, ha):
            print("MASK SIZE MISMATCH")
            return 2
    n = wa * ha
    out = bytearray(n * 4)
    bad = 0
    max_diff = 0
    minx, miny, maxx, maxy = wa, ha, -1, -1
    for p in range(n):
        o = p * 4
        if mask is not None and mask[o + 3] > 0:
            continue
        d = max(abs(a[o] - b[o]), abs(a[o + 1] - b[o + 1]), abs(a[o + 2] - b[o + 2]), abs(a[o + 3] - b[o + 3]))
        if d > max_diff:
            max_diff = d
        if d > tol:
            bad += 1
            x, y = p % wa, p // wa
            minx, miny, maxx, maxy = min(minx, x), min(miny, y), max(maxx, x), max(maxy, y)
            out[o:o + 4] = b"\xff\x00\x00\xff"
    if make_mask:
        # dilate by one pixel so edge jitter is covered too
        dil = bytearray(n * 4)
        for p in range(n):
            if out[p * 4 + 3]:
                x, y = p % wa, p // wa
                for dy in (-1, 0, 1):
                    for dx in (-1, 0, 1):
                        xx, yy = x + dx, y + dy
                        if 0 <= xx < wa and 0 <= yy < ha:
                            dil[(yy * wa + xx) * 4:(yy * wa + xx) * 4 + 4] = b"\xff\xff\xff\xff"
        write_png(out_path or "mask.png", wa, ha, dil)
        print(f"MASK {bad} unstable pixels")
        return 0
    if out_path:
        write_png(out_path, wa, ha, out)
    if bad:
        print(f"DIFFERENT {bad} pixels over tolerance {tol}, max diff {max_diff}, box ({minx},{miny})-({maxx},{maxy})")
        return 1
    print(f"EQUAL max diff {max_diff}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
