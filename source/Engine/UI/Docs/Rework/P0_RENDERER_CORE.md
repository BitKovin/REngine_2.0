# Phase 0: renderer core

**Goal:** replace how `UiRenderer` submits work with a recorded, batched draw list and the base `ui2` shader (phase 3 adds dedicated effect programs). Every existing UI must look **exactly** the same, which is proven with golden screenshots. Phase 0 also adds the test harness every later phase uses, and a small `UiPainter` API that new elements will draw with.

**Prerequisites:** read `README.md` and `00_CONVENTIONS.md`.

**Out of scope for phase 0:**

- markup
- new elements
- SDF text
- gradients, shadows and filters
- the UI image atlas
- any change to legacy layout or input

---

## Overview of what you build

```
source/Engine/UI/Test/UiTestRunner.h/.cpp            -ui_test <script> runner
source/Engine/Screenshot.h/.cpp                      + RequestTarget() for UI-only screenshots with alpha
Tools/ui/ui_png.py, ui_image_diff.py, ui_image_probe.py, sync_ui_assets.py
source/Engine/UI/Test/Scripts/<Game>/legacy_menus.uitest
source/Engine/UI/Assets/GameData/shaders/source/ui2/{varying.def.sc, vs_ui2.sc, fs_ui2.sc}
source/Engine/UI/Render/UiRenderTypes.h              plain structs and constants
source/Engine/UI/Render/UiPrimBuffer.h/.cpp          primitive data texture
source/Engine/UI/Render/UiDrawList.h/.cpp            recorded geometry + external commands
source/Engine/UI/Render/UiGpu.h/.cpp                 shader, layouts, white texture, Submit()
source/Engine/UI/Render/UiPainter.h/.cpp             Box / Image / clip API for new code
source/Engine/UI/Render/UiPainterTestCard.h/.cpp     test drawing for checks
source/Engine/UI/UiRenderer.h/.cpp                   legacy API now records into the draw list
source/Engine/EngineMain.cpp                         pass begin/end, frame hook, test runner tick
source/Engine/UI/WorldSpace/UiBilboard.cpp           pass begin/end
source/Engine/LoadingScreen/LoadingScreenSystem.cpp  pass begin/end, EndFrame, frame hook
```

---

## Task 0.1: UI test runner

**Files:** create `source/Engine/UI/Test/UiTestRunner.h` and `.cpp`. Edit `source/Engine/EngineMain.cpp`. Register both new files (conventions §6).

### Script format

- Plain text, one command per line.
- `#` starts a comment, to the end of the line. Blank lines are ignored.
- Tokens are separated by spaces. The rest of the line after the command name is its argument string.

| Command | Arguments | Behaviour |
|---|---|---|
| `wait <seconds>` | float | Blocks until that much **real** time has passed (use `SDL_GetTicks64()`, not game time; game time stops while paused). |
| `wait_frames <n>` | int | Blocks for n frames. |
| `action <name>` | input action name | `Input::GetAction(name)->SimulatePressed()` this frame. If the action doesn't exist (`GetAction` returns `&InputAction::NullAction`), record a FAIL for this line and continue. Then wait 2 frames automatically. |
| `screenshot <name>` | file stem | `Screenshot::Request(dir + name + ".png")` (full window, alpha forced to 255). Then wait 3 frames. |
| `ui_screenshot <name>` | file stem | `Screenshot::RequestTarget(EngineMain::MainInstance->UiFrameBuffer->frameBufferHandle(), dir + name + ".png")` (task 0.2). Then wait 3 frames. |
| `console <text>` | rest of line | `Console::Get().Execute(text)`. |
| `log <text>` | rest of line | `Logger::Log("[UiTest] %s", text)`. |
| `quit` | none | Writes the result file, waits 3 frames, then pushes `SDL_QUIT` (`SDL_Event e{}; e.type = SDL_QUIT; SDL_PushEvent(&e);`). |

- `dir` is `SaveData/ui_test/<scriptStem>/`, where `<scriptStem>` is the script file name without folder and extension.
- An unknown command records `FAIL <line>: unknown command '<cmd>'` and continues.
- If the script ends without `quit`, behave as if `quit` were there.
- Safety timeout: if the whole script runs longer than **180 real seconds**, record `FAIL 0: timeout`, write the result and quit.

### Result file

Path: `SaveData/ui_test/<scriptStem>/result.txt`. Write it with `FileSystemEngine::WriteFile`. One line per executed command, then a final line:

```
OK 3 wait 6
OK 4 ui_screenshot hud
FAIL 9 action does_not_exist: unknown action
RESULT FAIL
```

`RESULT PASS` only if no line failed. Later phases add `expect…` commands that can fail.

### Class

```cpp
// UiTestRunner.h
#pragma once
#include <string>
#include <vector>

class UiTestRunner
{
public:
    // Reads -ui_test from EngineMain::MainInstance->Arguments. No-op when absent.
    static void InitFromArguments();
    // Call once per frame from EngineMain::MainLoop, right after Input::Update().
    static void Tick();
    static bool IsActive() { return s_active; }

    // Used by later phases to add commands. Return false and set `error` to fail the line.
    using CommandFn = bool(*)(const std::string& args, std::string& error);
    static void RegisterCommand(const std::string& name, CommandFn fn);

private:
    struct Line { int number; std::string command; std::string args; };
    static inline bool s_active = false;
    static inline std::vector<Line> s_lines;
    static inline size_t s_next = 0;
    static inline int s_waitFrames = 0;
    static inline uint64_t s_waitUntilMs = 0;
    static inline uint64_t s_startMs = 0;
    static inline bool s_failed = false;
    static inline std::string s_dir;
    static inline std::vector<std::string> s_resultLines;
    static inline int s_quitInFrames = -1;
};
```

`Tick()` logic, in this order:

1. If not active, return.
2. If `s_quitInFrames >= 0`: decrement it. When it reaches 0, push `SDL_QUIT` and set `s_active = false`. Return.
3. If more than 180 s have passed since `s_startMs`: fail with "timeout", write the result, set `s_quitInFrames = 3`, return.
4. If `s_waitFrames > 0`: decrement and return.
5. If `SDL_GetTicks64() < s_waitUntilMs`: return.
6. Execute exactly **one** line (`s_next++`). Commands that wait set `s_waitFrames` or `s_waitUntilMs` themselves.
7. If `s_next == s_lines.size()` and the last command wasn't `quit`, run the `quit` behaviour.

Reading the script: the `-ui_test` value is a path as given (absolute paths work). Read it with `FileSystemEngine::ReadFile`. If the file is empty or missing, log an error, write `RESULT FAIL` with `FAIL 0: cannot read script`, and quit.

### EngineMain integration

- In `EngineMain::Init` (or wherever `ConsoleDefaultCommands::RegisterAll()` is called), after the arguments are parsed, call `UiTestRunner::InitFromArguments();`.
- In `EngineMain::MainLoop`, find the line `Input::Update();` that sits just before `UiNavigation::Update(&Viewport);`. Insert `UiTestRunner::Tick();` directly after `Input::Update();`. Ticking there means simulated actions are visible to UI navigation and to the game update of the same frame. `InputAction::Update()` (called inside `Input::Update`) clears `pressed`, so the press must happen after it.

**Check 0.1:** write `source/Engine/UI/Test/Scripts/StealthAction/smoke.uitest`:

```
wait 5
log hello
action does_not_exist
quit
```

Run StealthAction headless with it. Expect `SaveData/ui_test/smoke/result.txt` (under `Build/StealthAction/windows/`) to contain `OK 1 wait 5`, `OK 2 log hello`, a `FAIL 3 …` line and `RESULT FAIL`, and the process to exit by itself within about 10 s.

---

## Task 0.2: UI-only screenshots with alpha

**Files:** `source/Engine/Screenshot.h`, `source/Engine/Screenshot.cpp`.

1. Add to the `Screenshot` namespace in the header:

   ```cpp
   // Asks for a PNG of a framebuffer's colour attachment at the end of the next frame, keeping its alpha.
   void RequestTarget(bgfx::FrameBufferHandle target, const std::string& name);
   ```

2. In the `.cpp`, next to `pendingPath`, add `pendingTargetPath` (`std::string`), `pendingTarget` (`bgfx::FrameBufferHandle`, initialised invalid) and `std::unordered_set<std::string> keepAlphaPaths`. All are guarded by the existing `requestMutex`.
3. `RequestTarget` stores `PathFor(name)` and the handle under the lock.
4. In `BeforePresent`, after the existing block that issues the window screenshot: swap out the pending target request under the lock. If there is one and the handle is valid, insert the path into `keepAlphaPaths` (under the lock), then call `bgfx::requestScreenShot(handle, path.c_str())`.
5. In the callback's `screenShot(...)`, before the pixel loop, check under the lock whether `filePath` is in `keepAlphaPaths`. If so, erase it and copy alpha from the source (`row[x * 4 + 3]`) instead of writing 255.

**Check 0.2:** add `ui_screenshot test` to a copy of the smoke script (before `quit`), and run it. Expect `SaveData/ui_test/<stem>/test.png` to exist, sized like the UI target (1080 tall at `UiScale` 1). Open it with `ui_image_probe.py` (task 0.3): a pixel with no UI must have alpha 0.

If no file appears, bgfx on this renderer may not support framebuffer screenshots. **STOP** and report.

---

## Task 0.3: PNG tools (pure Python, no Pillow)

**Files:** create `Tools/ui/ui_png.py`, `Tools/ui/ui_image_diff.py` and `Tools/ui/ui_image_probe.py`. Use them exactly as written. Run them with `python -I`.

`Tools/ui/ui_png.py`:

```python
"""Minimal PNG read/write for 8-bit RGB/RGBA, non-interlaced (what stb_image_write produces)."""
import struct
import zlib

_SIG = b"\x89PNG\r\n\x1a\n"


def read_png(path):
    data = open(path, "rb").read()
    if data[:8] != _SIG:
        raise ValueError(f"{path}: not a PNG file")
    pos = 8
    width = height = depth = ctype = interlace = None
    idat = []
    while pos < len(data):
        (length,) = struct.unpack(">I", data[pos:pos + 4])
        kind = data[pos + 4:pos + 8]
        chunk = data[pos + 8:pos + 8 + length]
        pos += 12 + length
        if kind == b"IHDR":
            width, height, depth, ctype, _, _, interlace = struct.unpack(">IIBBBBB", chunk)
        elif kind == b"IDAT":
            idat.append(chunk)
        elif kind == b"IEND":
            break
    if depth != 8 or ctype not in (2, 6) or interlace != 0:
        raise ValueError(f"{path}: unsupported PNG (depth {depth}, colour type {ctype}, interlace {interlace})")
    bpp = 4 if ctype == 6 else 3
    raw = zlib.decompress(b"".join(idat))
    stride = width * bpp
    out = bytearray(width * height * 4)
    prev = bytearray(stride)
    i = 0
    for y in range(height):
        f = raw[i]
        i += 1
        line = bytearray(raw[i:i + stride])
        i += stride
        if f == 1:
            for x in range(bpp, stride):
                line[x] = (line[x] + line[x - bpp]) & 255
        elif f == 2:
            for x in range(stride):
                line[x] = (line[x] + prev[x]) & 255
        elif f == 3:
            for x in range(stride):
                a = line[x - bpp] if x >= bpp else 0
                line[x] = (line[x] + ((a + prev[x]) >> 1)) & 255
        elif f == 4:
            for x in range(stride):
                a = line[x - bpp] if x >= bpp else 0
                b = prev[x]
                c = prev[x - bpp] if x >= bpp else 0
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pred = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                line[x] = (line[x] + pred) & 255
        elif f != 0:
            raise ValueError(f"{path}: bad filter type {f}")
        if bpp == 4:
            out[y * width * 4:(y + 1) * width * 4] = line
        else:
            for x in range(width):
                o = (y * width + x) * 4
                out[o:o + 4] = bytes((line[x * 3], line[x * 3 + 1], line[x * 3 + 2], 255))
        prev = line
    return width, height, out


def write_png(path, width, height, rgba):
    rows = bytearray()
    for y in range(height):
        rows.append(0)
        rows += rgba[y * width * 4:(y + 1) * width * 4]

    def chunk(kind, body):
        c = struct.pack(">I", len(body)) + kind + body
        return c + struct.pack(">I", zlib.crc32(kind + body) & 0xFFFFFFFF)

    png = _SIG + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(bytes(rows), 6)) + chunk(b"IEND", b"")
    open(path, "wb").write(png)
```

`Tools/ui/ui_image_diff.py`:

```python
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
```

`Tools/ui/ui_image_probe.py`:

```python
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
```

`Tools/ui/run_ui_tests.py` runs a script on **every rendering backend** and compares the results. Every check from here on uses it.

```python
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
TIMEOUT_S = 240


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
    return sorted(p.name for p in Path(folder).glob("*.png"))


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
```

**Check 0.3:** probe the PNG from check 0.2 at a corner. Then diff the PNG against itself: it must print `EQUAL max diff 0` and exit 0.

---

## Task 0.4: golden baselines (before touching the renderer)

Do this **before** task 0.6. The baselines must come from unchanged rendering code.

1. For each in-scope game, write `source/Engine/UI/Test/Scripts/<Game>/legacy_menus.uitest`.
   - Read the game's `GameStart.cpp` (pause flow) and `UI/Pause/UiPauseMenu.h` / `UiSettingsMenu.h` to learn button order. Navigation uses the `ui_up`, `ui_down`, `ui_left`, `ui_right`, `ui_confirm` and `ui_cancel` actions.
   - Required captures, in this order, each followed by `wait 0.5` before the next action:
     1. `ui_screenshot 01_hud`: after `wait 8`, so the start level is loaded
     2. `action pause`, `wait 1`, `ui_screenshot 02_pause`
     3. navigate to Settings, `action ui_confirm`, `wait 1`, `ui_screenshot 03_settings`
     4. each settings page reachable from the settings menu, one capture each: `04_<page>`, `05_<page>`, …
     5. `action ui_cancel` back to the pause menu, `ui_screenshot 09_pause_again`
     6. `quit`
   - Add a comment line above each capture saying what it should show.
2. Build each game with unchanged code (conventions §8).
3. For each game, run its script twice **on all four backends**:
   - `python -I Tools/ui/run_ui_tests.py run --game <G> --script <abs path>/legacy_menus.uitest --label base1`
   - the same with `--label base2`
4. Build masks of run-to-run noise: `python -I Tools/ui/run_ui_tests.py mask --game <G> --stem legacy_menus --a base1 --b base2 --out mask`.
5. Record how the **unchanged** engine already differs between backends: `python -I Tools/ui/run_ui_tests.py cross --game <G> --stem legacy_menus --label base1 --mask mask`. Write the output into "Implementation notes". Differences here already existed before phase 0. Legacy content is only held to "same as its own backend's baseline". **New** content (the painter card, and everything from phase 1 on) must be equal across backends (conventions §11).
6. Open each `base1/dx11` PNG with the Read tool and check it shows what its comment says. A capture of the wrong screen is useless as a baseline. Fix the script and redo the game.
7. Never `git add` anything under `Build/ui_runs`.

**Check 0.4:** for every game and backend, `compare --base base1 --new base2 --mask mask` prints only `EQUAL`. A capture with more than 2% unstable pixels (the mask step marks it) is too animated to compare. Report which capture, and keep going with the rest. A backend that can't start headless (timeout, no output) is a **STOP**.

---

## Task 0.5: canonical assets folder and sync script

1. Create `source/Engine/UI/Assets/GameData/shaders/source/ui2/` (empty for now; task 0.6 fills it).
2. Create `Tools/ui/sync_ui_assets.py` exactly as below:

```python
"""Mirror engine UI assets into every in-scope game's GameData (see Docs/Rework/00_CONVENTIONS.md §7)."""
import filecmp
import shutil
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
SRC = REPO / "source" / "Engine" / "UI" / "Assets" / "GameData"
GAMES = ["ShooterGame", "SimGame", "SlasherGame", "StealthAction"]   # never HorrorGame
MANAGED = ["shaders/source/ui2", "ui/kit", "fonts/ui"]


def mirror(src: Path, dst: Path) -> int:
    changed = 0
    dst.mkdir(parents=True, exist_ok=True)
    src_names = {p.name for p in src.iterdir()}
    for p in list(dst.iterdir()):
        if p.name not in src_names:
            if p.is_dir():
                shutil.rmtree(p)
            else:
                p.unlink()
            changed += 1
    for p in src.iterdir():
        q = dst / p.name
        if p.is_dir():
            changed += mirror(p, q)
        elif not q.exists() or not filecmp.cmp(p, q, shallow=False):
            shutil.copy2(p, q)
            changed += 1
    return changed


def main():
    total = 0
    for game in GAMES:
        root = REPO / "source" / "Games" / game / "GameData"
        if not root.is_dir():
            print(f"ERROR: missing {root}")
            return 1
        for rel in MANAGED:
            src = SRC / rel
            if not src.is_dir():
                print(f"{game}/{rel}: skipped (no canonical folder yet)")
                continue
            n = mirror(src, root / rel)
            total += n
            print(f"{game}/{rel}: {n} change(s)")
    print(f"done, {total} change(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
```

**Check 0.5:** put a temporary file `test.txt` in the canonical `ui2` folder and run the script. All four games get `GameData/shaders/source/ui2/test.txt`. Delete it from the canonical folder and run again. It disappears from all four. HorrorGame is unchanged.

---

## Task 0.6: the base shader

**Files:** create in `source/Engine/UI/Assets/GameData/shaders/source/ui2/`: `varying.def.sc`, `vs_ui2.sc`, `fs_ui2.sc`. Use exactly this content. Later phases extend `fs_ui2.sc` in marked places.

`varying.def.sc`:

```
vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);
vec2 v_local     : TEXCOORD1 = vec2(0.0, 0.0);
vec4 v_misc      : TEXCOORD2 = vec4(0.0, 0.0, 0.0, 0.0);

vec2 a_position  : POSITION;
vec2 a_texcoord0 : TEXCOORD0;
vec2 a_texcoord1 : TEXCOORD1;
vec2 a_texcoord2 : TEXCOORD2;
```

`vs_ui2.sc`:

```
$input a_position, a_texcoord0, a_texcoord1, a_texcoord2
$output v_texcoord0, v_local, v_misc

#include <bgfx_shader.sh>

uniform mat4 u_uiProjection;

void main()
{
    v_texcoord0 = a_texcoord0;
    v_local = a_texcoord1;
    // x = primitive index, yz = position in UI units (for clipping), w unused
    v_misc = vec4(a_texcoord2.x, a_position.x, a_position.y, 0.0);
    gl_Position = mul(u_uiProjection, vec4(a_position.xy, 0.0, 1.0));
}
```

`fs_ui2.sc`:

```
$input v_texcoord0, v_local, v_misc

#include <bgfx_shader.sh>

// Primitive data texture: 16 RGBA32F texels per primitive (layout in UiRenderTypes.h).
SAMPLER2D(s_uiData, 0);
// Texture of the current batch (white 1x1 when the batch has none).
SAMPLER2D(s_uiTex0, 1);
// [P1] SDF glyph atlas sampler goes here as slot 2.
// [P3] gradient ramp sampler goes here as slot 3.

// x = data texture width in texels, y = texels per primitive, z = time (seconds), w = unused
uniform vec4 u_uiParams;

vec4 uiFetch(int prim, int t)
{
    int width = int(u_uiParams.x);
    int texel = prim * int(u_uiParams.y) + t;
    int row = texel / width;
    return texelFetch(s_uiData, ivec2(texel - row * width, row), 0);
}

bool uiFlag(float flags, float bit)
{
    return mod(floor(flags / bit), 2.0) > 0.5;
}

float uiCoverage(float d, float px)
{
    return clamp(0.5 - d / px, 0.0, 1.0);
}

// p relative to the box centre, y down; r = (top-left, top-right, bottom-right, bottom-left)
float uiRoundBoxSdf(vec2 p, vec2 halfSize, vec4 r)
{
    float rad = (p.x < 0.0) ? ((p.y < 0.0) ? r.x : r.w) : ((p.y < 0.0) ? r.y : r.z);
    rad = min(rad, min(halfSize.x, halfSize.y));
    vec2 q = abs(p) - halfSize + vec2(rad, rad);
    return min(max(q.x, q.y), 0.0) + length(max(q, vec2(0.0, 0.0))) - rad;
}

vec4 uiPremul(vec4 c)
{
    return vec4(c.rgb * c.a, c.a);
}

vec4 uiShadeImage(int prim, vec4 tex)
{
    // Identical to the legacy fs_ui_textured: texture * colour, then premultiply.
    return uiPremul(tex * uiFetch(prim, 1));
}

vec4 uiShadeBox(int prim, vec2 local, vec4 tex, float flags, float px)
{
    vec4 fill   = uiFetch(prim, 1);
    vec2 size   = uiFetch(prim, 2).xy;
    vec4 radii  = uiFetch(prim, 3);
    vec4 bw     = uiFetch(prim, 4);   // top, right, bottom, left

    vec2 halfSize = size * 0.5;
    float outer = uiCoverage(uiRoundBoxSdf(local - halfSize, halfSize, radii), px);

    float inner = outer;
    if (bw.x + bw.y + bw.z + bw.w > 0.0)
    {
        vec2 innerMin = vec2(bw.w, bw.x);
        vec2 innerMax = size - vec2(bw.y, bw.z);
        vec2 innerHalf = max((innerMax - innerMin) * 0.5, vec2(0.0, 0.0));
        vec2 innerCentre = (innerMin + innerMax) * 0.5;
        vec4 innerRadii = max(radii - vec4(max(bw.w, bw.x), max(bw.x, bw.y), max(bw.y, bw.z), max(bw.z, bw.w)),
                              vec4(0.0, 0.0, 0.0, 0.0));
        inner = min(uiCoverage(uiRoundBoxSdf(local - innerCentre, innerHalf, innerRadii), px), outer);
    }

    vec4 fillColor = fill;
    if (uiFlag(flags, 2.0))
        fillColor *= tex;
    vec4 result = uiPremul(fillColor) * inner;

    if (outer - inner > 0.0)
    {
        // Pick the side by distance normalised by that side's width: this reproduces CSS's diagonal corner split.
        float dt = (bw.x > 0.0) ? local.y / bw.x : 1.0e9;
        float dr = (bw.y > 0.0) ? (size.x - local.x) / bw.y : 1.0e9;
        float db = (bw.z > 0.0) ? (size.y - local.y) / bw.z : 1.0e9;
        float dl = (bw.w > 0.0) ? local.x / bw.w : 1.0e9;
        vec4 bc = uiFetch(prim, 5);
        float m = dt;
        if (dr < m) { m = dr; bc = uiFetch(prim, 6); }
        if (db < m) { m = db; bc = uiFetch(prim, 7); }
        if (dl < m) { bc = uiFetch(prim, 8); }
        result += uiPremul(bc) * (outer - inner);
    }
    return result;
}

float uiClip(int prim, vec2 screenPos, float flags, float pxScreen)
{
    if (!uiFlag(flags, 1.0))
        return 1.0;
    vec4 rect  = uiFetch(prim, 9);
    vec4 radii = uiFetch(prim, 10);
    vec2 halfSize = (rect.zw - rect.xy) * 0.5;
    return uiCoverage(uiRoundBoxSdf(screenPos - (rect.xy + halfSize), halfSize, radii), pxScreen);
}

void main()
{
    int prim = int(v_misc.x + 0.5);
    vec2 screenPos = v_misc.yz;

    // Implicit-LOD sampling only here, in uniform control flow.
    vec4 tex = texture2D(s_uiTex0, v_texcoord0);
    // [P1] sample the glyph atlas here (texture2D at top level) when adding SDF text.

    vec4 header = uiFetch(prim, 0);
    float kind = header.x;
    float flags = header.y;
    // Anti-aliasing width comes from the CPU (local units per target pixel), not from dFdx/dFdy:
    // derivatives differ between backends, and every backend must produce the same pixels.
    float px = max(uiFetch(prim, 2).z, 0.0001);
    float pxScreen = 1.0;   // screen positions are in target pixels already

    vec4 color;
    if (kind < 0.5)
        color = uiShadeImage(prim, tex);
    else if (kind < 1.5)
        color = uiShadeBox(prim, v_local, tex, flags, px);
    // [P1] kind 2 = SDF glyph, [P3] kinds 3..5 go here.
    else
        color = vec4(1.0, 0.0, 1.0, 1.0);   // unknown kind: loud magenta

    color *= uiClip(prim, screenPos, flags, pxScreen);
    gl_FragColor = color;
}
```

Then:

1. Run `python Tools/ui/sync_ui_assets.py`, then the four `ShaderCompilationAutomation.exe` (conventions §7).
2. All `ui2` lines must say `[COMPILED]`, for every target (windows dx11/dx12/gl/spirv, linux, android, web).
3. Open `source/Games/StealthAction/GameData/shaders/compiled/web/gles/ui2/fs_ui2.bin` as text (it contains GLSL source after a binary header). Search for `precision`. If the only float precision declared is `mediump`, **STOP**: the SDF math needs `highp`. Report the line you found.

**Check 0.6:** no `[FAILED]`, and the precision check passes.

---

## Task 0.7: render types and the primitive data texture

**Files:** create `source/Engine/UI/Render/UiRenderTypes.h`, `UiPrimBuffer.h` and `UiPrimBuffer.cpp`.

`UiRenderTypes.h`, exactly:

```cpp
#pragma once
#include <glm.h>
#include <bgfx/bgfx.h>
#include <cstdint>

namespace UiRender
{
    // Primitive data texture: RGBA32F, kDataWidth x kDataHeight texels.
    constexpr int kDataWidth    = 1024;
    constexpr int kDataHeight   = 256;
    constexpr int kPrimTexels   = 16;
    constexpr int kPrimsPerRow  = kDataWidth / kPrimTexels;        // 64
    constexpr int kPrimRows     = 224;                              // rows 0..223 hold primitives
    constexpr int kMaxPrims     = kPrimRows * kPrimsPerRow;         // 14336 per frame
    constexpr int kAuxFirstRow  = 224;                              // rows 224..255: auxiliary data [P3]

    // Values of texel 0 .x ("kind"). Must match fs_ui2.sc.
    enum class PrimKind : int
    {
        Image     = 0,   // texture * colour (legacy textured quads and legacy text)
        Box       = 1,   // SDF rounded box with fill and per-side border
        SdfGlyph  = 2,   // [P1]
        BoxShadow = 3,   // [P3]
        Arc       = 4,   // [P3]
        Solid     = 5,   // [P3] plain colour, for tessellated paths
    };

    // Bits of texel 0 .y ("flags"). Must match fs_ui2.sc uiFlag() calls.
    constexpr int kFlagClip       = 1;   // texels 9/10 hold a clip rect and radii in UI units
    constexpr int kFlagHasTexture = 2;   // Box: multiply fill by the batch texture

    // One primitive's data. Texel meaning:
    //  T0  kind, flags, unused, aux index (-1 = none) [P3]
    //  T1  fill / tint colour, straight RGBA
    //  T2  rect width, rect height, AA width (local units per target pixel; 1 when unscaled), param
    //  T3  corner radii: top-left, top-right, bottom-right, bottom-left
    //  T4  border widths: top, right, bottom, left
    //  T5  border colour top      T6 right      T7 bottom      T8 left   (straight RGBA)
    //  T9  clip rect in UI units: x0, y0, x1, y1
    //  T10 clip radii: top-left, top-right, bottom-right, bottom-left
    //  T11 gradient header [P3]   T12 gradient geometry [P3]
    //  T13 shadow / glyph params [P1/P3]   T14 second colour [P1/P3]   T15 mask [P3]
    struct PrimBlock
    {
        glm::vec4 T[kPrimTexels] = {};

        PrimBlock() { T[0] = glm::vec4(0.f, 0.f, 0.f, -1.f); }
        void SetKind(PrimKind kind) { T[0].x = static_cast<float>(kind); }
        void AddFlag(int flag) { T[0].y = static_cast<float>(static_cast<int>(T[0].y) | flag); }
    };

    // Batch-relevant render state. Two items batch together only when every field is equal.
    struct BatchKey
    {
        uint8_t  Program = 0;                       // index into UiGpu's program table; 0 = ui2/fs_ui2 (base). [P3] adds effect programs
        uint64_t State = 0;                         // full bgfx state flags
        uint16_t TextureIdx = bgfx::kInvalidHandle; // kInvalidHandle = use the white texture
        uint8_t  StencilRef = 0;                    // 0 = no stencil test

        bool operator==(const BatchKey& o) const
        {
            return Program == o.Program && State == o.State && TextureIdx == o.TextureIdx && StencilRef == o.StencilRef;
        }
        bool operator!=(const BatchKey& o) const { return !(*this == o); }
    };

    struct Vertex
    {
        float X, Y;     // position in UI units (after the full transform)
        float U, V;     // texture coordinates
        float LX, LY;   // local position in the primitive's own rect space (for SDF), UI units
        float Prim;     // primitive index (frame-global), as float
        float Pad;      // keep 32 bytes
    };
    static_assert(sizeof(Vertex) == 32, "UiRender::Vertex must stay 32 bytes");
}
```

`UiPrimBuffer`, the frame-wide primitive storage:

```cpp
#pragma once
#include <UI/Render/UiRenderTypes.h>
#include <vector>

class UiPrimBuffer
{
public:
    void Init();      // creates the RGBA32F texture (point sampling, clamp); logs an error if unsupported
    void Shutdown();
    // Returns the frame-global primitive index, or -1 when the frame is full (logs once per frame).
    int  Add(const UiRender::PrimBlock& block);
    // Uploads every row touched since the last upload. Call right before submitting draws that use them.
    void UploadPending();
    // Starts a new frame. Only UiRenderer::OnBeforeBgfxFrame() may call this (conventions §3).
    void ResetFrame();
    bgfx::TextureHandle Texture() const { return m_texture; }
    int Count() const { return m_count; }

private:
    bgfx::TextureHandle m_texture = BGFX_INVALID_HANDLE;
    std::vector<glm::vec4> m_cpu;   // kDataWidth * kDataHeight texels, mirror of the whole texture
    int m_count = 0;                // primitives added this frame
    int m_uploadedCount = 0;        // primitives already covered by an upload this frame
    bool m_loggedFull = false;
};
```

Implementation rules:

- `Init`:
  - Check `bgfx::getCaps()->formats[bgfx::TextureFormat::RGBA32F] & BGFX_CAPS_FORMAT_TEXTURE_2D`. If that bit is missing, log `[UiRenderer] RGBA32F textures unsupported; new UI drawing disabled` and leave the texture invalid. `Add` then always returns -1.
  - Create the texture with `bgfx::createTexture2D(kDataWidth, kDataHeight, false, 1, bgfx::TextureFormat::RGBA32F, BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP)`.
  - Register it with `ResourceStatistics` as `"UiPrimData"`, the same way the font atlas does.
  - `m_cpu.assign(kDataWidth * kDataHeight, glm::vec4(0.f))`.
- `Add`: when `m_count >= kMaxPrims`, log once per frame and return -1. Otherwise copy the 16 texels to `m_cpu[m_count * kPrimTexels + i]` (texel index `m_count * 16 + i`, row = index / 1024), then return `m_count++`.
- `UploadPending`: if `m_uploadedCount == m_count`, return. Compute `firstRow = (m_uploadedCount * kPrimTexels) / kDataWidth` and `lastRow = ((m_count * kPrimTexels) - 1) / kDataWidth`. Call `bgfx::updateTexture2D(m_texture, 0, 0, 0, firstRow, kDataWidth, lastRow - firstRow + 1, bgfx::copy(&m_cpu[firstRow * kDataWidth], (lastRow - firstRow + 1) * kDataWidth * sizeof(glm::vec4)))`, then set `m_uploadedCount = m_count`. A row may be uploaded more than once per frame. That's correct, because each later upload contains all earlier primitives of that row.
- `ResetFrame`: `m_count = m_uploadedCount = 0; m_loggedFull = false;`. Don't clear `m_cpu`.

**Check 0.7:** it builds.

---

## Task 0.8: the draw list

**Files:** create `source/Engine/UI/Render/UiDrawList.h` and `.cpp`.

```cpp
#pragma once
#include <UI/Render/UiRenderTypes.h>
#include <functional>
#include <vector>

class UiDrawList
{
public:
    using ExternalFn = std::function<void(uint16_t viewId)>;

    struct Item
    {
        bool External = false;
        // geometry
        UiRender::BatchKey Key;
        uint32_t FirstVertex = 0, VertexCount = 0;
        uint32_t FirstIndex = 0, IndexCount = 0;
        // external
        uint32_t ExternalIndex = 0;
    };

    enum class BreakReason { Texture, State, Stencil, External, Capacity, Count };

    void Clear();
    bool Empty() const { return m_items.empty(); }

    // Appends triangles. `indices` are relative to `verts` (0..vertCount-1).
    void AddTriangles(const UiRender::BatchKey& key, const UiRender::Vertex* verts, uint32_t vertCount,
                      const uint16_t* indices, uint32_t indexCount);
    // Convenience: one quad, corners in order top-left, top-right, bottom-right, bottom-left.
    void AddQuad(const UiRender::BatchKey& key, const UiRender::Vertex corners[4]);
    // Runs `fn` at submit time, in order with the geometry around it.
    void AddExternal(ExternalFn fn);
    // Appends a copy of another list's items (used by UiElem chunks in phase 1).
    void AppendList(const UiDrawList& other);

    const std::vector<Item>& Items() const { return m_items; }
    const std::vector<UiRender::Vertex>& Vertices() const { return m_vertices; }
    const std::vector<uint16_t>& Indices() const { return m_indices; }
    const std::vector<ExternalFn>& Externals() const { return m_externals; }
    const uint32_t* BreakCounts() const { return m_breaks; }

private:
    std::vector<Item> m_items;
    std::vector<UiRender::Vertex> m_vertices;
    std::vector<uint16_t> m_indices;
    std::vector<ExternalFn> m_externals;
    uint32_t m_breaks[static_cast<int>(BreakReason::Count)] = {};
};
```

Rules for `AddTriangles`:

1. If `vertCount == 0` or `indexCount == 0`, return.
2. Look at the last item. It **extends** the last item when that item is geometry, its `Key == key`, and `VertexCount + vertCount <= 65535`. In that case the new indices are written as `index + last.VertexCount`, and `last.VertexCount` and `last.IndexCount` grow.
3. Otherwise start a new item with `FirstVertex = m_vertices.size()` and `FirstIndex = m_indices.size()`, and write the indices unchanged. If a previous item exists, count one break:
   - `External` if the last item is external
   - else `Capacity` if the key was equal (the vertex limit was hit)
   - else `Texture` if the texture differs
   - else `Stencil` if the stencil ref differs
   - else `State`
4. Append the vertices.

`AddQuad` calls `AddTriangles` with indices `{0, 1, 2, 0, 2, 3}`.

`AppendList` copies items, vertices, indices and externals. It re-bases `FirstVertex`, `FirstIndex` and `ExternalIndex`. It does **not** merge the first appended item into the current last one, which keeps the logic simple.

**Check 0.8:** it builds.

---

## Task 0.9: GPU submit

**Files:** create `source/Engine/UI/Render/UiGpu.h` and `.cpp`.

```cpp
#pragma once
#include <UI/Render/UiDrawList.h>
#include <UI/Render/UiPrimBuffer.h>

class Shader;

namespace UiGpu
{
    void Init();        // called from UiRenderer::Init()
    void Shutdown();    // called from UiRenderer::Shutdown()
    UiPrimBuffer& Prims();
    bgfx::TextureHandle WhiteTexture();

    // Exact bgfx state values the legacy code produces (computed once in Init, see below).
    uint64_t StateLegacyPremultiplied();   // SubmitQuad(): Premultiplied blend
    uint64_t StateLegacyAlpha();           // DrawTexturedRectRegion/9Slice: Alpha blend
    uint64_t StateLegacyText();            // DrawText: literal flags

    struct Projection { float Width = 0.f, Height = 0.f; };
    Projection CurrentProjection();        // same numbers SetShaderProjection uses (customViewport aware)

    // Submits the list into `viewId` and clears it.
    void Submit(UiDrawList& list, uint16_t viewId, const Projection& projection);

    struct Stats { uint32_t Passes, Prims, Vertices, GeometryDraws, Externals; uint32_t Breaks[6]; };
    const Stats& LastFrameStats();
    void OnBeforeBgfxFrame();              // copies running stats to "last frame", resets prims
}
```

### `Init`

1. Vertex layout:

   ```cpp
   layout.begin()
       .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
       .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
       .add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float)
       .add(bgfx::Attrib::TexCoord2, 2, bgfx::AttribType::Float)
       .end();
   ```

2. Program table: `std::vector<Shader*>`. Entry 0 = `ShaderManager::GetShaderProgram("ui2/vs_ui2", "ui2/fs_ui2")`. Later phases append effect programs through `uint8_t UiGpu::RegisterProgram(const char* vs, const char* fs)`. If entry 0 isn't `IsValid()`, log an error, and `Submit` then only runs externals. A geometry item whose program is invalid is skipped, with an error logged once per frame.
3. White texture: a 1×1 RGBA8 white, `bgfx::createTexture2D(1, 1, false, 1, RGBA8, 0, bgfx::copy(white, 4))`.
4. `Prims().Init()`.
5. Legacy states, computed **once**, exactly like the legacy code computes them:

   ```cpp
   const uint64_t saved = BgfxStateManager::GetState();
   BgfxStateManager::Reset();
   BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
   BgfxStateManager::SetBlend(BgfxStateManager::Blend::Premultiplied);
   s_statePremul = BgfxStateManager::GetState() | BGFX_STATE_MSAA;      // Apply() adds MSAA
   BgfxStateManager::Reset();
   BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
   BgfxStateManager::SetBlend(BgfxStateManager::Blend::Alpha);
   s_stateAlpha = BgfxStateManager::GetState() | BGFX_STATE_MSAA;
   BgfxStateManager::SetState(saved);
   s_stateText = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS |
                 BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
   ```

   Before writing this, open `BgfxStateManager.h` and confirm `Apply()` is exactly `AddFlags(BGFX_STATE_MSAA); bgfx::setState(s_state);`. If it does anything else, **STOP**.

### `CurrentProjection`

Copy the numbers from the legacy `SetShaderProjection` in `UiRenderer.cpp`:

- height = `UiManager::GetScaledUiHeight()`, width = height × `Camera::AspectRatio`
- when `UiRenderer::customViewport` is set, use `customViewportSize` instead

### `Submit(list, viewId, projection)`

1. `Prims().UploadPending()`.
2. Count the total vertices and indices in the list. If both are greater than 0:
   - check `bgfx::getAvailTransientVertexBuffer(total, layout) >= total` and `bgfx::getAvailTransientIndexBuffer(totalIdx) >= totalIdx`
   - if either fails, log once per frame and **skip geometry items** (still run the externals in order)
   - otherwise allocate one `TransientVertexBuffer` and one `TransientIndexBuffer` and memcpy the list's vectors into them
3. Build `proj = glm::ortho(0.f, projection.Width, projection.Height, 0.f, -1.f, 1.f)`. This is the same call as the legacy code.
4. For each item, in order:
   - **External:** `list.Externals()[item.ExternalIndex](viewId)`.
   - **Geometry:**

     ```cpp
     Shader* shader = s_programs[item.Key.Program];
     shader->UseProgram();
     shader->SetUniform("u_uiProjection", proj);
     shader->SetUniform("u_uiParams", glm::vec4(UiRender::kDataWidth, UiRender::kPrimTexels, (float)Time::GameTimeNoPause, 0.f));
     shader->SetTexture("s_uiData", Prims().Texture());
     bgfx::TextureHandle tex = { item.Key.TextureIdx };
     shader->SetTexture("s_uiTex0", bgfx::isValid(tex) ? tex : WhiteTexture());
     bgfx::setState(item.Key.State);
     if (item.Key.StencilRef > 0)
         bgfx::setStencil(BGFX_STENCIL_TEST_EQUAL | BGFX_STENCIL_FUNC_REF(item.Key.StencilRef) | BGFX_STENCIL_FUNC_RMASK(0xFF) |
                          BGFX_STENCIL_OP_FAIL_S_KEEP | BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_KEEP);
     bgfx::setVertexBuffer(0, &tvb, item.FirstVertex, item.VertexCount);
     bgfx::setIndexBuffer(&tib, item.FirstIndex, item.IndexCount);
     shader->Submit(viewId);
     ```

5. Add to the running stats: passes, geometry draws, externals, vertices, and the break counts from the list.
6. `list.Clear()`.

`OnBeforeBgfxFrame`: copy the running stats into "last frame" (prims = `Prims().Count()`), zero the running stats, then `Prims().ResetFrame()`.

**Check 0.9:** it builds. Nothing calls it yet.

---

## Task 0.10: route the legacy API through the draw list

**Files:** `source/Engine/UI/UiRenderer.h`, `UiRenderer.cpp`, `EngineMain.cpp`, `UI/WorldSpace/UiBilboard.cpp`, `LoadingScreen/LoadingScreenSystem.cpp`.

### 0.10a: pass API and call sites

1. Add to `UiRenderer.h`:

   ```cpp
   // A pass collects every UiRenderer draw between BeginPass and EndPass and submits it in order.
   void BeginPass();
   void EndPass(uint16_t viewId);
   bool InPass();
   UiDrawList& CurrentList();
   // Resets per-frame renderer data. Call only right before the two bgfx::frame() calls named in conventions §3.
   void OnBeforeBgfxFrame();
   ```

2. In `UiRenderer.cpp`, add `static UiDrawList s_list; static bool s_inPass = false;`.
   - `BeginPass`: if `s_inPass`, log an error. Clear `s_list` and set `s_inPass = true`.
   - `EndPass`: `UiGpu::Submit(s_list, viewId, UiGpu::CurrentProjection()); s_inPass = false;`.
   - `OnBeforeBgfxFrame` calls `UiGpu::OnBeforeBgfxFrame()`.
   - `Init` calls `UiGpu::Init()` at its end. `Shutdown` calls `UiGpu::Shutdown()` first.
3. Every legacy draw function starts with `if (!s_inPass) { LogOutsidePassOnce(); return; }`. `LogOutsidePassOnce` logs once per frame.
4. Call sites:
   - **`EngineMain.cpp`, UI pass:**
     - directly after `bgfx::touch(ViewIdManager::GetCurrentId());`, add `UiRenderer::BeginPass();`
     - after the late-draw block (after `UiElement::pendingLateDrawElements.clear();`) and **before** `UiRenderer::EndFrame();`, add `UiRenderer::EndPass(ViewIdManager::GetCurrentId());`
   - **`EngineMain.cpp`, main present:** immediately before the `bgfx::frame();` of the main loop (the one around line 1031; confirm by reading the surrounding code that it is the per-frame present), add `UiRenderer::OnBeforeBgfxFrame();`.
   - **`UiBilboard::DrawUi`:**
     - after `BgfxStateManager::SetBlend(BgfxStateManager::Blend::Premultiplied);`, add `UiRenderer::BeginPass();`
     - after `UiElement::pendingLateDrawElements.clear();`, add `UiRenderer::EndPass(ViewIdManager::GetCurrentId());`
     - this must happen **before** `UiRenderer::customViewport = false;`
   - **`LoadingScreenSystem::Draw`:**
     - wrap `viewport.Draw();` as `UiRenderer::BeginPass(); viewport.Draw(); UiRenderer::EndPass(viewId);`
     - then add `UiRenderer::EndFrame();` and `UiRenderer::OnBeforeBgfxFrame();` right before its `bgfx::frame();`

### 0.10b: every legacy draw becomes an external command (pure deferral)

Goal of this step: **no batching yet**. Each legacy draw records a closure that does exactly what the function did before, and the closure runs at `EndPass`. This proves the deferral is correct before anything else changes.

Rules for turning a function body into a closure:

1. Everything the old body read from **globals** at draw time must be read **at record time** and captured by value:
   - the projection: `UiGpu::CurrentProjection()`
   - the stencil depth: `(uint8_t)s_maskStack.size()`
   - `customViewport`
   - `currentTime`
2. Change the helpers to take those values as parameters:
   - `SetShaderProjection(Shader*, const UiGpu::Projection&)`
   - `ApplyStencilTest(uint8_t ref)`, which does nothing when `ref == 0`
   - `SubmitQuad(Shader*, uint8_t stencilRef, uint16_t viewId)`
   - `DrawStencilRect_Internal(const glm::mat4& model, uint8_t ref, const UiGpu::Projection&, uint16_t viewId)`
3. Replace `ViewIdManager::GetCurrentId()` inside closures with the `viewId` parameter.
4. Shaders, matrices, colours and textures (handles) are captured by value. Uniform maps are captured by value (`std::unordered_map` copies).
5. CPU work that must stay at record time stays outside the closure:
   - the glyph `EnsureGlyph` passes and the `FlushToGPU` call in `DrawText`
   - transient vertex buffer allocation and filling (the `TransientVertexBuffer` struct is captured by value; transient buffers are valid for the whole frame)
6. `PushMask`, `PopMask` and `ClearStencil` keep updating `s_maskStack` at **record** time. Only the stencil quad draw goes into the closure, with the model and ref captured.

Apply this to every function in `UiRenderer.cpp` that submits: both `DrawTexturedRect` overloads, both `DrawTexturedRectShader` overloads, both `DrawTexturedRectShaderParams` overloads, both `DrawBorderRect` overloads, `DrawText`, `DrawTexturedRectRegion`, `DrawTexturedRect9Slice`, both `PushMask` overloads, `PopMask` and `ClearStencil`.

**Check 0.10b** (the "golden regression" used by every later step):

1. Build all four games.
2. For each game: `run_ui_tests.py run --game <G> --script …/legacy_menus.uitest --label after`.
3. Then `run_ui_tests.py compare --game <G> --stem legacy_menus --base base1 --new after --mask mask`.
4. Every line, for every backend, must print `EQUAL`. Any `DIFFERENT`: open the `diff_X.png` the tool wrote next to the new capture, find which draw is wrong, fix it. **Don't continue** until all are equal.

### 0.10c: plain textured rectangles become geometry

Convert only the **no-custom-shader** paths: `DrawTexturedRect` (both overloads). Leave `DrawTexturedRectShader` and `DrawTexturedRectShaderParams` external.

New body (matrix overload; the legacy pos/rotation/pivot overload builds its `glm::mat4 model` with `BuildQuadModel` and then does the same):

```cpp
const glm::mat4 model = BuildQuadModelFromMat3(transform, size);   // same matrix the old code used
UiRender::PrimBlock block;
block.SetKind(UiRender::PrimKind::Image);
block.T[1] = color;
const int prim = UiGpu::Prims().Add(block);
if (prim < 0) return;
const glm::vec2 unit[4] = { {0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f}, {0.f, 1.f} };   // TL, TR, BR, BL
UiRender::Vertex v[4];
for (int i = 0; i < 4; ++i)
{
    const glm::vec4 p = model * glm::vec4(unit[i], 0.f, 1.f);
    v[i] = { p.x, p.y, unit[i].x, unit[i].y, unit[i].x * size.x, unit[i].y * size.y, (float)prim, 0.f };
}
UiRender::BatchKey key;
key.State = UiGpu::StateLegacyPremultiplied();
key.TextureIdx = texture.idx;
key.StencilRef = (uint8_t)s_maskStack.size();
s_list.AddQuad(key, v);
```

UVs equal the unit-quad corners, exactly like the legacy static quad (whose vertices are `(x, y, u = x, v = y)`). The legacy `bilboard` uniform does nothing in `vs_ui.sc` (the code is commented out), so dropping it changes nothing.

**Check 0.10c:** same as 0.10b, all `EQUAL`. Then run `ui_stats` (task 0.13 adds it; if it isn't there yet, skip this sub-check) and confirm geometry draws appear.

### 0.10d: legacy text without effects becomes geometry

In `DrawText`, keep passes 1–3 exactly. They fill the transient vertex array `v` with 6 vertices per glyph in **normalised** text-box space, plus `model = BuildQuadModelFromMat3(transform, drawSize)`. Change pass 3 to write into a local `std::vector<QuadVertex> glyphVerts(vertexCount)` instead of a transient buffer when `shader.empty()`. Then, in the `shader.empty()` branch, replace the submit with:

```cpp
UiRender::PrimBlock block;
block.SetKind(UiRender::PrimKind::Image);
block.T[1] = color;
const int prim = UiGpu::Prims().Add(block);
if (prim < 0) return;
std::vector<UiRender::Vertex> out(glyphVerts.size());
std::vector<uint16_t> idx(glyphVerts.size());
for (size_t i = 0; i < glyphVerts.size(); ++i)
{
    const glm::vec4 p = model * glm::vec4(glyphVerts[i].x, glyphVerts[i].y, 0.f, 1.f);
    out[i] = { p.x, p.y, glyphVerts[i].u, glyphVerts[i].v, glyphVerts[i].x * drawSize.x, glyphVerts[i].y * drawSize.y, (float)prim, 0.f };
    idx[i] = (uint16_t)i;
}
UiRender::BatchKey key;
key.State = UiGpu::StateLegacyText();
key.TextureIdx = atlas->texture.idx;
key.StencilRef = (uint8_t)s_maskStack.size();
// one call per <= 65535 vertices
s_list.AddTriangles(key, out.data(), (uint32_t)out.size(), idx.data(), (uint32_t)idx.size());
```

A string can have more than 10 922 glyphs (65 535 / 6). Then split `out` into chunks of at most 65 532 vertices (a multiple of 6) and call `AddTriangles` once per chunk.

The **effects** path (`!shader.empty()`) keeps its transient buffer and its per-glyph externals from 0.10b.

**Check 0.10d:** same as 0.10b, all `EQUAL`.

### 0.10e: region and 9-slice without a custom shader become geometry

`DrawTexturedRectRegion` and `DrawTexturedRect9Slice`, `shader.empty()` branch only:

- keep the legacy vertex computation, but write it into a local `std::vector<QuadVertex>`
- transform each vertex by the same `BuildQuadModelFromMat3(transform, size)` model, as in 0.10d
- local coordinates are `x * size.x`, `y * size.y`
- the state is `UiGpu::StateLegacyAlpha()` (the legacy code uses `Blend::Alpha` here; confirm by reading the function before converting it)
- the texture is the given texture, and the stencil ref is the current mask depth
- emit with `AddTriangles` and sequential indices

**Check 0.10e:** same as 0.10b, all `EQUAL`.

---

## Task 0.11: full golden regression

Run all four games' `legacy_menus.uitest` once more from a clean build, on all backends, and run the golden regression from check 0.10b. All must be `EQUAL`. Also play-test by script, not by hand: add `wait 3` and `ui_screenshot 10_after_resume` after resuming, to cover the HUD again after menus close.

**Check 0.11:** all `EQUAL` on every backend. Record the results table (game, backend, capture, result) under "Implementation notes" at the end of this file.

---

## Task 0.12: `UiPainter` (the API new elements draw with)

**Files:** create `source/Engine/UI/Render/UiPainter.h` and `.cpp`.

```cpp
#pragma once
#include <UI/Render/UiDrawList.h>
#include <glm.h>
#include <vector>

struct UiRect { float X = 0.f, Y = 0.f, W = 0.f, H = 0.f; };
struct UiCorners { float TopLeft = 0.f, TopRight = 0.f, BottomRight = 0.f, BottomLeft = 0.f; };
struct UiEdges { float Top = 0.f, Right = 0.f, Bottom = 0.f, Left = 0.f; };

struct UiBoxPaint
{
    glm::vec4 Fill = glm::vec4(0.f);                   // straight RGBA
    bgfx::TextureHandle FillTexture = BGFX_INVALID_HANDLE;
    glm::vec4 FillUv = glm::vec4(0.f, 0.f, 1.f, 1.f);  // u0, v0, u1, v1
    UiCorners Radii;
    UiEdges BorderWidth;
    glm::vec4 BorderColor[4] = { glm::vec4(0.f), glm::vec4(0.f), glm::vec4(0.f), glm::vec4(0.f) }; // top, right, bottom, left
};

class UiPainter
{
public:
    explicit UiPainter(UiDrawList& list) : m_list(list) {}

    // Element-local (origin top-left, y down) to UI units. Default identity.
    void SetTransform(const glm::mat3& m) { m_transform = m; }
    const glm::mat3& Transform() const { return m_transform; }

    void Box(const UiRect& rect, const UiBoxPaint& paint);
    void Image(const UiRect& rect, bgfx::TextureHandle texture, const glm::vec4& uv, const glm::vec4& tint);

    // Clip in element-local space. Axis-aligned transforms use the shader clip; others use the legacy stencil mask.
    void PushClip(const UiRect& rect, const UiCorners& radii);
    void PopClip();

private:
    struct Clip { glm::vec4 Rect; glm::vec4 Radii; bool Stencil; };
    UiDrawList& m_list;
    glm::mat3 m_transform = glm::mat3(1.f);
    std::vector<Clip> m_clips;
    bool IsTranslationOnly() const;
    void ApplyClip(UiRender::PrimBlock& block) const;
};
```

### Rules

1. **`IsTranslationOnly()`:** `m[0][0] == 1 && m[1][1] == 1 && m[0][1] == 0 && m[1][0] == 0` (exact float compares are intended).
2. **Thin-element rule (decision D10)**, applied in `Box` **before** snapping, on a local copy of rect and paint:
   - if `0 < rect.W < 1`: set `W = 1` and multiply `Fill.a` and every border colour's alpha by the original W
   - same for `H`
   - for each border side with `0 < width < 1`: set the width to 1 and multiply that side's colour alpha by the original width
3. **Snapping**, applied in `Box` and `Image` only when `IsTranslationOnly()`:
   - transform the rect to UI units (`x0 = rect.X + m[2][0]`, `y0 = rect.Y + m[2][1]`, and the same for x1/y1)
   - round x0, y0, x1 and y1 to the nearest integer with `std::round`
   - if a rounded size becomes 0 while the original was greater than 0, set x1 = x0 + 1 (or y1 = y0 + 1)
   - draw with identity transform and the snapped rect
4. **Box geometry:**
   - expand the rect by 1 UI unit on every side, so anti-aliasing isn't cut off (when not translation-only, expand by `1 / scale`, where `scale = length(m[0])`)
   - each vertex: `X, Y` = the transform applied to the expanded corner; `LX, LY` = the expanded corner relative to the **unexpanded** rect origin (so `(-1, -1)` at the top-left)
   - UVs map the unexpanded rect to `FillUv` linearly, so the expanded ring samples slightly outside; that is harmless because coverage is 0 there
   - block: `kind = Box`, `T[1] = Fill`, `T[2] = (W, H, aa, 0)` where `aa = 1 / scale` (`scale = sqrt(abs(determinant of the transform's 2x2 part))`, so 1 when unscaled), `T[3] = radii`, `T[4] = border widths (top, right, bottom, left)`, `T[5..8] = border colours`
   - `AddFlag(kFlagHasTexture)` when `FillTexture` is valid
   - key: `UiGpu::StateLegacyPremultiplied()`, texture = `FillTexture` (or `kInvalidHandle`), stencil = the current legacy mask depth plus this painter's stencil clips (see rule 6)
5. **`Image`:** kind `Image`, `T[1] = tint`, `T[2] = (W, H, aa, 0)`, the quad without expansion, UVs from `uv`.
   Legacy routing (task 0.10) leaves `T[2]` at zero for `Image` primitives. That is fine, since the `Image` shading path never reads it.
6. **Clips:**
   - `PushClip` with `IsTranslationOnly()`: compute the screen rect and intersect it with the current top clip's rect, if any. The intersection keeps the new clip's radii when the new rect lies fully inside the parent rect. Otherwise it keeps radii 0, which is a documented limitation. Push `{rect, radii, Stencil = false}`.
   - `PushClip` with a rotated or scaled transform: call `UiRenderer::PushMask(m_transform * Mat3Translate(rect.X, rect.Y), {rect.W, rect.H})`. That records the legacy stencil mask, and radii are ignored. Push `{…, Stencil = true}`.
   - `PopClip` pops, calling `UiRenderer::PopMask()` for stencil entries.
   - `ApplyClip`: when the top non-stencil clip exists, `AddFlag(kFlagClip)`, `T[9] = rect (x0, y0, x1, y1)`, `T[10] = radii`.
   - The stencil ref for keys is read from `UiRenderer`'s mask stack. Add `uint8_t UiRenderer::CurrentMaskDepth()` for that.

### Test card

`source/Engine/UI/Render/UiPainterTestCard.h/.cpp`: `void UiPainterTestCard::Draw();` draws the items below with a `UiPainter` on `UiRenderer::CurrentList()`. It is enabled by the console command `ui_painter_test 1` and disabled by `ui_painter_test 0` (register it with `REGISTER_CONSOLE_CMD` inside `ConsoleDefaultCommands::RegisterAll()`, next to `ui_reload`). It is drawn in the main UI pass right before `UiRenderer::EndPass`, only when enabled.

Items (UI units, identity transform unless stated):

| # | Draw | Probes (x y → expected RGBA in the UI target, premultiplied) |
|---|---|---|
| A | `Box({100,100,200,100})`, fill (1,0,0,1) | (150,150)→255,0,0,255; (100,150)→255,0,0,255; (299,150)→255,0,0,255; (99,150)→0,0,0,0; (300,150)→0,0,0,0 |
| B | `Box({350,100,200,100})`, fill (0,1,0,1), radii 40 all | (450,150)→0,255,0,255; (351,101)→0,0,0,0; (390,104)→0,255,0,255 |
| C | `Box({600,100,200,100})`, fill (0,0,0,0), border 4 all; colours top (1,0,0,1), right (0,1,0,1), bottom (0,0,1,1), left (1,1,1,1) | (700,101)→255,0,0,255; (797,150)→0,255,0,255; (700,198)→0,0,255,255; (602,150)→255,255,255,255; (700,150)→0,0,0,0 |
| D | `Box({850,100,200,0.5f})`, fill (1,1,1,1) | (950,100)→128,128,128,128 (tolerance 2) |
| E | `Box({850,150,200,40})`, fill (0,0,0,0), border 0.5 all, colours (1,1,1,1) | (950,150)→128,128,128,128 (tolerance 2); (950,170)→0,0,0,0 |
| F | `PushClip({100,250,200,100}, radii 30)`, `Box({50,200,400,250})` fill (0,0,1,1), `PopClip()` | (200,300)→0,0,255,255; (101,251)→0,0,0,0; (350,300)→0,0,0,0 |
| G | `SetTransform(T(700,350)·R(30°)·T(−50,−50))`, `Box({0,0,100,100})` fill (1,1,0,1), then reset the transform | (700,350)→255,255,0,255; (640,290)→0,0,0,0 |
| H | `Image({100,450,64,64}, white, uv (0,0,1,1), tint (1,0,1,1))` | (132,482)→255,0,255,255 |

Where `T(x,y)` = `UiElement::Mat3Translate` and `R(a)` = `UiElement::Mat3Rotate(glm::radians(a))`.

**Probe file:** write the table into `source/Engine/UI/Test/Scripts/StealthAction/painter_card.probes` (one line per probe: `X Y R G B A TOL LABEL`, TOL 1 unless stated). Write the script `painter_card.uitest`:

```
wait 6
console ui_painter_test 1
wait_frames 3
ui_screenshot card
console ui_painter_test 0
quit
```

The probes assume the UI target is 1080 tall (`UiManager::UiScale == 1`). If the capture's height isn't 1080, **STOP** and report.

**Check 0.12:**

1. `run_ui_tests.py run --game StealthAction --script …/painter_card.uitest --label card`, which covers all four backends.
2. For **each** backend: `python -I Tools/ui/ui_image_probe.py Build/ui_runs/StealthAction/painter_card/card/<backend>/card.png --probes source/Engine/UI/Test/Scripts/StealthAction/painter_card.probes`. Every line must print `OK`, with exit code 0.
3. Then `run_ui_tests.py cross --game StealthAction --stem painter_card --label card` must print `EQUAL` for every backend. New drawing must produce the same pixels everywhere (conventions §11). A backend that differs is a bug in the new code. Typical causes:
   - a y-flip of the render target on OpenGL
   - a texel-centre offset
   - precision
   Find and fix it. Don't raise the tolerance. Probe points are chosen in empty screen space. If the game's HUD covers one, move the whole card by the same offset and update the table, so the probes still check the same features.

---

## Task 0.13: `ui_stats`

Add the console command `ui_stats` (same registration place as `ui_reload`). It logs `UiGpu::LastFrameStats()` in one line:

```
[UiStats] passes 1 prims 37 vertices 412 draws 9 externals 4 breaks: texture 5 state 2 stencil 0 external 1 capacity 0
```

**Check 0.13:** run a script that does `wait 6`, `console ui_stats` and `quit`, and find the line in the game log.

---

## Task 0.14: phase wrap-up

1. All four games build.
2. All `legacy_menus.uitest` captures are `EQUAL` against their own backend's baseline on dx11, dx12, vk and gl (task 0.11).
3. `painter_card` probes are all `OK` on every backend, and `cross` reports `EQUAL` for every backend.
4. Shaders compile for all targets with no `[FAILED]`.
5. Fill in the README status row for phase 0.
6. Write "Implementation notes" below: anything you had to decide, every STOP you hit and how the owner resolved it, and the golden results table.

## Implementation notes

### Result (2026-11-06)

Phase 0 is implemented and verified on **StealthAction only** (the owner limited testing to that game; the other three games were not
built or run in this phase). All checks below ran on dx11, dx12, vk and gl.

| Check | Result |
|---|---|
| Shaders (`ui2/*`) compile for every target | 228 jobs, 0 `[FAILED]` (StealthAction); synced to all four games with `Tools/ui/sync_ui_assets.py` |
| 0.2/0.3 UI screenshots and PNG tools | smoke script writes a 1440x1080 PNG, corner alpha 0, diff of a file with itself `EQUAL` |
| 0.4 baselines | base1/base2 stable (0 unstable pixels) on all four backends |
| 0.10b, 0.10c-e and 0.11 golden regression (`legacy_menus`, 9 captures x 4 backends) | every capture `EQUAL` against the same backend's base1 |
| 0.12 `painter_card` probes | all 22 probes `OK` on dx11, dx12, vk, gl |
| 0.12 `cross` on `painter_card` | `EQUAL` for dx12, vk and gl against dx11 (GL max diff 2, tolerance 2), with the legacy-HUD mask described below |
| 0.13 `ui_stats` | `[UiStats] passes 1 prims 6 vertices 26 draws 5 externals 26 breaks: texture 1 state 0 stencil 0 external 3 capacity 0` |

### Decisions and deviations

- **Shader include:** `#include <../bgfx_shader.sh>` instead of `<bgfx_shader.sh>`, as every shader in a subfolder of this repo does.
- **Web GLES precision:** `compiled/web/gles/ui2/fs_ui2.bin` declares the default `precision mediump float` like every other engine shader, but it also has explicit `highp` on 68 declarations. Judged sufficient. Not run on a web build.
- **UI-only screenshots (task 0.2):** the plan's `bgfx::requestScreenShot(frameBufferHandle)` does not work: bgfx only supports screenshots of the swap chain (the D3D11 renderer returns early for other frame buffers). STOP was reported to the owner. Solution: `Screenshot::RequestTarget` blits the UI colour texture into a `BLIT_DST | READ_BACK` texture on view 254 and reads it back with `bgfx::read`; the PNG is written when `bgfx::frame()` reports that frame (`Screenshot::AfterPresent`). OpenGL returns the rows bottom-up, so rows are flipped when `caps->originBottomLeft` is set. `ui_screenshot` therefore waits 8 frames.
- **`texelFetch` removed from `fs_ui2.sc`:** shaderc drops a sampler that is only used through `texelFetch`, so the engine's `Shader` class never bound `s_uiData` and every primitive read as zeros. `uiFetch` now uses `texture2DLod` at the texel centre with the point-sampled data texture (exact). **Rule for later phases:** never rely on `texelFetch` alone for a sampler; use `texture2DLod`.
- **Border corner ties:** on the corner diagonals of a border the winning side was decided by interpolation rounding, so GL and dx11 disagreed. `fs_ui2.sc` now only lets a side win when it is closer by more than 0.001, with the order top, right, bottom, left.
- **`UiButton.hpp` default image path** was `GameData/texture/ui/white.png` (missing `s`), a file that does not exist. A missing texture draws differently per backend. Fixed to `textures/`. This changed the golden baselines, which were captured after the fix.
- **Legacy UI differs between backends (not caused by this work):** against dx11, vk differs by up to 12 (of 255) on 118 to 1,830 pixels (the 24x24 cursor icon: texture filtering), and gl on about 666 px (HUD progress-bar underlines drawn from `GameData/cat.png`, `UiProgressBar` default image: a different texel is sampled) up to about 12,900 px on the Input Settings page. These are baked into each backend's own baseline, so the golden regression is per backend. `painter_card` cross-checks use `Tools/ui/make_cross_mask.py` to mask the pixels that differ between backends in the legacy HUD alone (2,034 pixels), so only the new drawing is compared.
- **Legacy menu navigation:** the legacy menus are cursor-driven. `ui_down, ui_down, ui_up` reaches Settings in StealthAction's pause menu; the page names in the comments of `legacy_menus.uitest` for the later captures are therefore not exact (for example `06_video` shows Input Settings). The captures are deterministic, which is all the regression needs.
- **Timeouts:** the in-game runner stops a script after 75 s, `run_ui_tests.py` kills a run after 90 s (a run takes about 25 s; GL is slower than the others).
- **`UiDrawList::AddExternal`** does not count a break (the plan only counts breaks when a new geometry item starts); `External` breaks are counted when a geometry item follows an external one.
- **`uidb.json` (decision D13):** nothing in Phase 0 produces or consumes it; the model/serializer starts in Phase 1.
- **Windows `-renderapi gles`** was considered for a smaller feature set but windows has no compiled gles shaders; not added.
- **CRLF:** new engine files were converted to CRLF; `UiTestRunner.*`, `Screenshot.*` and `EngineMain.cpp` keep the line endings they already had.

### New files and tools

`UI/Render/{UiRenderTypes.h,UiPrimBuffer.*,UiDrawList.*,UiGpu.*,UiPainter.*,UiPainterTestCard.*}`, `UI/Test/UiTestRunner.*`,
`UI/Test/Scripts/StealthAction/{legacy_menus,smoke,painter_card,stats}.uitest` and `painter_card.probes`,
`Tools/ui/{ui_png,ui_image_diff,ui_image_probe,run_ui_tests,sync_ui_assets,make_cross_mask}.py`, `Build/ui_build.cmd`.
Console commands: `ui_painter_test 1|0`, `ui_stats`; launch argument `-ui_test <script>`.

