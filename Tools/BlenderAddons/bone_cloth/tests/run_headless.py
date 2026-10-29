"""Runs a test script in a background Blender on a scratch copy of the coat (BC_NO_PREPARE=1: leave the scene as it is).
python tests/run_headless.py <script.py> [blend]     (BC_TEST_BLEND names the default blend)
Full output goes to tests/last_run.txt (exporter warnings removed); the tail is printed."""
import os
import subprocess
import sys

BLENDER = r"D:\SteamLibrary\steamapps\common\Blender\blender.exe"
here = os.path.dirname(os.path.abspath(__file__))
script = os.path.join(here, sys.argv[1])
blend = sys.argv[2] if len(sys.argv) > 2 else os.environ.get("BC_TEST_BLEND")
wrapper = os.path.join(os.environ.get("TEMP", "."), "bc_wrapper.py")
with open(wrapper, "w") as f:
    if os.environ.get("BC_NO_PREPARE"):      # scenes that already are what the test needs (the jacket scenes)
        f.write("exec(open(r'%s').read())\n" % script)
    else:
        f.write("exec(open(r'%s').read())\nexec(open(r'%s').read())\n" % (os.path.join(here, "prepare_coat.py"), script))
cmd = [BLENDER, "-b", blend, "--factory-startup", "--python", wrapper]
p = subprocess.run(cmd, capture_output=True, text=True, timeout=900)
out = p.stdout + ("\nSTDERR:\n" + p.stderr if p.stderr.strip() else "")
out = "\n".join(line for line in out.splitlines() if "Animation target" not in line)
with open(os.path.join(here, "last_run.txt"), "w", encoding="utf-8") as f:
    f.write(out)
print(out[-int(os.environ.get("BC_TAIL", "9000")):])
sys.exit(p.returncode)
