"""How close the generated front-wind frames come to an artist's hand-made ones (positions of each strip's hem, cm).
BC_CLIPGEN=SIDE_CARRY=0.5,... overrides clipgen constants, BC_SET=blow_back=1,lift=0.524 the generator's settings.
Run on a copy of a scene whose windFront clip is the artist's: BC_NO_PREPARE=1 python tests/run_headless.py fit_hand.py <copy>"""
import os, sys
import bpy, numpy as np
sys.path.insert(0, r"F:\Git\SdlWasmEngine\Tools\BlenderAddons")
import bone_cloth
from bone_cloth import props, poses, clipgen, frames, bones as bones_mod
bone_cloth.register()
arm = bpy.data.objects["root"]
s = props.get_setup(arm)
act = s.poses.windFront.action
assert poses.clip_is_hand(act), "the scene's front-wind clip is not hand-made"
for item in filter(None, os.environ.get("BC_CLIPGEN", "").split(",")):
    k, v = item.split("=")
    setattr(clipgen, k, float(v))
for item in filter(None, os.environ.get("BC_SET", "").split(",")):
    k, v = item.split("=")
    setattr(s.poses.gen, k, float(v))
frame = frames.compute_frame(arm, s.character)
centre = np.array(arm.matrix_world @ arm.pose.bones[s.character.hips].head)


def hems(bases):
    out = {}
    with poses.held_pose(arm):
        poses.set_bases(arm, {n: q for n, (q, _) in bases.items()}, {n: l for n, (_, l) in bases.items()})
        bpy.context.view_layer.update()
        for info in bones_mod.strips(s, arm):
            out[info["column"] + info["side"]] = np.array([np.array(arm.matrix_world @ arm.pose.bones[n].tail) - centre
                                                           for n in info["bones"]])
    return out


gen = clipgen.clip_frames_for(s, arm, "windFront", frame)
line = []
for fi, g in zip((2, 3), gen):
    hand, mine = hems(poses.bases_at(act, fi)), hems(g)
    line.append("frame %d: " % fi + ", ".join("%s %.1f" % (k, 100 * float(np.linalg.norm(hand[k] - mine[k], axis=1).mean()))
                                              for k in hand if k.endswith("l")))
    if fi == 3:
        for k in hand:
            if k.endswith("l"):
                print("  hem %-6s hand (%.2f %.2f %.2f)  generated (%.2f %.2f %.2f)" % ((k,) + tuple(hand[k][-1]) + tuple(mine[k][-1])))
print("FIT mean hem error cm | " + " | ".join(line))
