"""Phase 2: takes over a cloth rig that was made by hand (DESIGN.md sections 8.3.7 and 10).

The engine's own rules find the strips from the root bones, so an existing coat is recognised the way the game would
recognise it; the settings, colliders and clip names come from its .skmm entry. Nothing is rebuilt or deleted."""
import json
import math
import re

import bpy
import numpy as np
from mathutils import Vector

from . import bones as bones_mod
from . import frames, guides, landmarks, mesh_prep, model_data, pipeline, props, stages
from . import engine_contract as ec

MOTION_KEYS = ("followRoot", "followTip", "trail", "maxAngleRoot", "maxAngleTip", "radius", "teleportDistance", "tipLength",
               "volume", "openWindStart", "openWindFull", "windwardBias", "volumeWindStart", "volumeWindFull",
               "layerSmoothing", "spinOpen", "spinVolume", "spinStart", "spinFull", "spinRelease")


def read_skmm(path, index=0):
    """The cloth entry of a .skmm file (the index-th), or raises with a plain message."""
    with open(path, encoding="utf-8") as f:
        data = json.load(f)
    cloth = data.get("cloth", [])
    if not cloth:
        raise RuntimeError("%s has no cloth entry" % path)
    return cloth[min(index, len(cloth) - 1)]


def _column_name(bone, prefix):
    """coat_front_00_l -> 'front'; anything that does not fit gets None."""
    m = re.match(r"^%s_(.+?)_\d+(?:_[lr])?$" % re.escape(prefix), bone)
    return m.group(1) if m else None


def _common_prefix(names):
    first = names[0].split("_")[0]
    return first if all(n.startswith(first + "_") for n in names) else ""


def find_strips(setup, arm, roots):
    """The strips the game would build from these roots, with their positions: [(bone names, theta, side)]."""
    skel, verts = model_data.collect(arm)
    ids = []
    for r in roots:
        if r not in skel.index:
            raise RuntimeError("The armature has no bone '%s'" % r)
        ids.append(skel.index[r])
    frame = pipeline.frame_of(setup, arm)
    strips = ec.build_strips(skel, verts, ids, frame.flat_forward(), np.array([0.0, 0.0, 1.0]))
    out = []
    for s in strips:
        names = [skel.names[j.bone] for j in s.joints]
        theta = float(frame.angle(skel.head(s.joints[0].bone)))
        out.append((names, theta, s.side))
    return out, skel, verts, frame


def adopt(setup, arm, roots, skmm=None):
    """Makes `setup` describe an existing rig. Returns a list of messages."""
    log = []
    roots = [r.strip() for r in roots if r.strip()]
    if not roots:
        raise RuntimeError("Name the root bones the strips hang from (cloth_root, ...) or pick a .skmm file")
    if not setup.character.hips or not setup.character.chest:
        pipeline.detect_character(setup, arm)
    found, skel, verts, frame = find_strips(setup, arm, roots)
    if not found:
        raise RuntimeError("No strips found under those roots: no bone under them has vertices weighted to it")
    prefix = _common_prefix([n for names, _, _ in found for n in names])
    strips_json, all_bones = [], []
    for names, theta, side in found:
        col = _column_name(names[0], prefix) or "c%d" % len(strips_json)
        sfx = "l" if names[0].endswith("_l") else ("r" if names[0].endswith("_r") else "")
        strips_json.append({"column": col, "side": sfx, "theta": theta, "bones": names})
        all_bones.extend(names)
    setup.adopted = True
    setup.prefix = prefix or setup.prefix
    setup.generated_bones = json.dumps(all_bones)
    setup.root_bones = json.dumps(list(reversed(roots)))         # export lists them as the file did (see export.skmm_entry)
    setup.strips_json = json.dumps(strips_json)
    for n in all_bones + roots:
        if n in arm.pose.bones:
            arm.pose.bones[n]["bone_cloth"] = setup.id
    log.append("%d strips, %d bones under %s" % (len(found), len(all_bones), ", ".join(roots)))

    # the layout, read off the strips
    g = setup.guides
    angles = sorted(set(round(abs(t), 3) for _, t, _ in found))
    g.layout = 'CLOSED_RING' if any(a < math.radians(25) for a in angles) else 'OPEN_FRONT'
    parents = {arm.data.bones[r].parent.name if arm.data.bones[r].parent else "" for r in roots}
    g.anchors = 'SPINE' if len(roots) > 2 else ('CHEST_AND_HIPS' if len(roots) == 2 else 'HIPS')
    left = [(s["column"], s["theta"]) for s in strips_json if s["side"] != "r"]
    g.columns.clear()
    seen = set()
    for col, theta in sorted(left, key=lambda c: c[1]):
        if col in seen:
            continue
        seen.add(col)
        c = g.columns.add()
        c.name, c.angle = col, abs(theta)
        c.side = 'L' if any(s["column"] == col and s["side"] == "l" for s in strips_json) else 'C'
        c.open_mult = 1.0 if abs(theta) < math.radians(80) else (0.4 if abs(theta) < math.radians(150) else 0.8)
    g.columns_per_side = max(1, len([c for c in g.columns if c.side == 'L']))
    tops = [skel.head(skel.index[names[0]])[2] for names, _, _ in found]
    hems = [float(np.array(arm.matrix_world @ arm.data.bones[names[-1]].tail_local)[2]) for names, _, _ in found]
    g.top_auto = g.waist_auto = g.hem_auto = False
    g.top_line, g.hem_line = float(max(tops)), float(min(hems))
    waist = landmarks.waist_height(arm, setup.character)
    g.waist_line = waist if waist is not None else g.top_line - 0.3
    g.opening_half_angle = max(0.0, min(abs(theta) for _, theta, _ in found) - 0.2) if g.layout == 'OPEN_FRONT' else 0.0

    # which meshes carry the cloth
    if not setup.cloth_objects:
        cloth = set(all_bones)
        for o in model_data.skinned_meshes(arm):
            groups = {vg.name for vg in o.vertex_groups}
            if groups & cloth:
                setup.cloth_objects.add().obj = o
        log.append("cloth meshes: " + ", ".join(r.obj.name for r in setup.cloth_objects))

    # settings from the .skmm entry
    if skmm:
        m = setup.motion
        for k in MOTION_KEYS:
            if k in skmm:
                setattr(m, k, skmm[k])
        m.feel = 'CUSTOM'
        f = skmm.get("forward")
        if f:
            mine = frame.to_engine(frame.flat_forward())
            theirs = np.array([f["x"], f["y"], f["z"]])
            if np.linalg.norm(theirs) > 1e-6 and np.dot(mine, theirs / np.linalg.norm(theirs)) < math.cos(math.radians(5)):
                m.forward_override, m.forward = True, (f["x"], f["y"], f["z"])
                log.append("forward differs from the detected facing: kept the file's")
        setup.colliders.clear()
        for c in skmm.get("colliders", []):
            col = setup.colliders.add()
            col.bone, col.to = c["bone"], c["to"]
            col.auto_radius, col.radius = c.get("radius", 0.0) <= 0.0, c.get("radius", 0.0)
            col.auto_end, col.end_radius = c.get("endRadius", 0.0) <= 0.0, c.get("endRadius", 0.0)
        for kind, key in (("open", "openClip"), ("volume", "volumeClip")):
            layer = getattr(setup.poses, kind)
            name = skmm.get(key, "")
            layer.enabled = bool(name)
            if name:
                layer.clip_name = name
                act = bpy.data.actions.get(name)
                layer.action = act
                layer.mode = 'HAND'
                if act is None:
                    log.append("the %s clip '%s' is not in this file" % (kind, name))
        from . import colliders as colliders_mod
        colliders_mod.fit_all(setup, arm)
    elif not setup.colliders:
        from . import colliders as colliders_mod
        colliders_mod.add_defaults(setup, arm)
        colliders_mod.fit_all(setup, arm)

    for key in ("character", "bones", "weights", "rest", "poses", "colliders"):
        stages.mark_done(setup, key, "adopted" if key == "bones" else "")
    return log


def legacy_rig_bones(setup, arm):
    """Helper bones, constraints and drivers an older hand-made rig left that the game ignores: (bones, constraints, drivers)."""
    used = set(bones_mod.generated(setup)) | set(bones_mod.roots(setup))
    names = [b.name for b in arm.data.bones if b.name.startswith("MCH-") or b.name.endswith("_ctrl")]
    cons = sum(1 for pb in arm.pose.bones if pb.name in used for c in pb.constraints if c.name != "BC Follow")
    drivers = len(arm.animation_data.drivers) if arm.animation_data else 0
    return names, cons, drivers
