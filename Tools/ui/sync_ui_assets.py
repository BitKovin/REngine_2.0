"""Mirror engine UI assets into every in-scope game's GameData (see Docs/UI/Rework/00_CONVENTIONS.md §7)."""
import filecmp
import shutil
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
SRC = REPO / "Tools" / "ui" / "assets" / "GameData"
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
