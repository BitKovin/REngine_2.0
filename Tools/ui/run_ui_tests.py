"""Run a UI test script on every rendering backend and compare results.

run:     python -I run_ui_tests.py run --game StealthAction --script <abs path .uitest> --label base1 [--backends dx11,dx12,vk,gl]
mask:    python -I run_ui_tests.py mask --game StealthAction --stem legacy_menus --a base1 --b base2 --out mask
compare: python -I run_ui_tests.py compare --game StealthAction --stem legacy_menus --base base1 --new after [--mask mask] [--tolerance 2]
cross:   python -I run_ui_tests.py cross --game StealthAction --stem legacy_menus --label after [--ref dx11] [--tolerance 2] [--mask mask]
Runs live in Build/ui_runs/<game>/<stem>/<label>/<backend>/. Exit code 0 = all good, 1 = difference or failed run, 2 = usage error.
"""
import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ui_png import read_png, write_png  # noqa: E402

REPO = Path(__file__).resolve().parents[2]
BACKENDS = "dx11,dx12,vk,gl"
TIMEOUT_S = 90


def run_dir(game, stem, label):
    return REPO / "Build" / "ui_runs" / game / stem / label


def diff(a_path, b_path, tol, mask_path=None, out_path=None):
    wa, ha, a = read_png(a_path)
    wb, hb, b = read_png(b_path)
    if (wa, ha) != (wb, hb):
        return -1, 0
    mask = read_png(mask_path)[2] if mask_path and Path(mask_path).exists() else None
    out = bytearray(wa * ha * 4) if out_path else None
    bad, worst = 0, 0
    for p in range(wa * ha):
        o = p * 4
        if mask is not None and mask[o + 3] > 0:
            continue
        d = max(abs(a[o + i] - b[o + i]) for i in range(4))
        worst = max(worst, d)
        if d > tol:
            bad += 1
            if out is not None:
                out[o:o + 4] = b"\xff\x00\x00\xff"
    if out is not None and bad:
        write_png(out_path, wa, ha, out)
    return bad, worst


def cmd_run(a):
    script = Path(a.script).resolve()
    exe_dir = REPO / "Build" / a.game / "windows"
    exe = exe_dir / "bin" / f"{a.game}.exe"
    if not exe.is_file():
        print(f"ERROR: {exe} not found")
        return 2
    ok = True
    for backend in a.backends.split(","):
        produced = exe_dir / "SaveData" / "ui_test" / script.stem
        if produced.exists():
            shutil.rmtree(produced)
        print(f"[{backend}] running {script.stem}", flush=True)
        try:
            subprocess.run([str(exe), "-headless", "-renderapi", backend, "-ui_test", str(script)],
                           cwd=str(exe_dir), timeout=TIMEOUT_S)
        except subprocess.TimeoutExpired:
            print(f"[{backend}] TIMEOUT after {TIMEOUT_S}s - look for CrashDump_*.dmp in {exe_dir}")
            ok = False
            continue
        dst = run_dir(a.game, script.stem, a.label) / backend
        if dst.exists():
            shutil.rmtree(dst)
        if not produced.exists():
            print(f"[{backend}] FAILED: the run produced no output folder")
            ok = False
            continue
        shutil.copytree(produced, dst)
        result = dst / "result.txt"
        lines = result.read_text().strip().splitlines() if result.exists() else []
        last = lines[-1] if lines else "RESULT MISSING"
        print(f"[{backend}] {last}")
        ok = ok and last == "RESULT PASS"
    return 0 if ok else 1


def pngs(folder):
    return sorted(p.name for p in Path(folder).glob("*.png") if not p.name.startswith(("xdiff_", "diff_")))


def cmd_mask(a):
    base = run_dir(a.game, a.stem, "")
    for backend in sorted(os.listdir(base / a.a)):
        out = base / a.out / backend
        out.mkdir(parents=True, exist_ok=True)
        for name in pngs(base / a.a / backend):
            w, h, x = read_png(base / a.a / backend / name)
            _, _, y = read_png(base / a.b / backend / name)
            m = bytearray(w * h * 4)
            unstable = 0
            for p in range(w * h):
                o = p * 4
                if max(abs(x[o + i] - y[o + i]) for i in range(4)) > 0:
                    unstable += 1
                    px, py = p % w, p // w
                    for dy in (-1, 0, 1):
                        for dx in (-1, 0, 1):
                            xx, yy = px + dx, py + dy
                            if 0 <= xx < w and 0 <= yy < h:
                                q = (yy * w + xx) * 4
                                m[q:q + 4] = b"\xff\xff\xff\xff"
            write_png(out / name, w, h, m)
            share = 100.0 * unstable / (w * h)
            print(f"[{backend}] {name}: {unstable} unstable pixels ({share:.2f}%)" + ("  <-- over 2%" if share > 2 else ""))
    return 0


def cmd_compare(a):
    base = run_dir(a.game, a.stem, "")
    ok = True
    for backend in sorted(os.listdir(base / a.base)):
        for name in pngs(base / a.base / backend):
            new = base / a.new / backend / name
            if not new.exists():
                print(f"[{backend}] {name}: MISSING in {a.new}")
                ok = False
                continue
            mask = base / a.mask / backend / name if a.mask else None
            bad, worst = diff(base / a.base / backend / name, new, a.tolerance, mask,
                              base / a.new / backend / ("diff_" + name))
            print(f"[{backend}] {name}: " + ("SIZE MISMATCH" if bad < 0 else ("EQUAL" if bad == 0 else f"DIFFERENT {bad} px")) + f" (max {worst})")
            ok = ok and bad == 0
    return 0 if ok else 1


def cmd_cross(a):
    base = run_dir(a.game, a.stem, a.label)
    ok = True
    for backend in sorted(os.listdir(base)):
        if backend == a.ref:
            continue
        for name in pngs(base / a.ref):
            other = base / backend / name
            if not other.exists():
                print(f"[{backend} vs {a.ref}] {name}: MISSING")
                ok = False
                continue
            mask = run_dir(a.game, a.stem, a.mask) / a.ref / name if a.mask else None
            bad, worst = diff(base / a.ref / name, other, a.tolerance, mask, base / backend / ("xdiff_" + name))
            print(f"[{backend} vs {a.ref}] {name}: " + ("SIZE MISMATCH" if bad < 0 else ("EQUAL" if bad == 0 else f"DIFFERENT {bad} px")) + f" (max {worst})")
            ok = ok and bad == 0
    return 0 if ok else 1


def main():
    ap = argparse.ArgumentParser()
    sp = ap.add_subparsers(dest="cmd", required=True)
    r = sp.add_parser("run")
    r.add_argument("--game", required=True)
    r.add_argument("--script", required=True)
    r.add_argument("--label", required=True)
    r.add_argument("--backends", default=BACKENDS)
    m = sp.add_parser("mask")
    for k in ("--game", "--stem", "--a", "--b", "--out"):
        m.add_argument(k, required=True)
    c = sp.add_parser("compare")
    for k in ("--game", "--stem", "--base", "--new"):
        c.add_argument(k, required=True)
    c.add_argument("--mask", default=None)
    c.add_argument("--tolerance", type=int, default=2)
    x = sp.add_parser("cross")
    for k in ("--game", "--stem", "--label"):
        x.add_argument(k, required=True)
    x.add_argument("--ref", default="dx11")
    x.add_argument("--mask", default=None)
    x.add_argument("--tolerance", type=int, default=2)
    a = ap.parse_args()
    return {"run": cmd_run, "mask": cmd_mask, "compare": cmd_compare, "cross": cmd_cross}[a.cmd](a)


if __name__ == "__main__":
    sys.exit(main())
