"""python tests/verify_glb.py model.glb: what the engine would build from an exported model (see glb_check.py)."""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
import glb_check  # noqa: E402

if __name__ == "__main__":
    r = glb_check.verify(sys.argv[1])
    print("strips:", len(r["strips"]))
    for s in r["strips"]:
        print("  ", s)
    print("colliders:", r["colliders"])
    print("clips (strips using each layer):", r["clips"])
