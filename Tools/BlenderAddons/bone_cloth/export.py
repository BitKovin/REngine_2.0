"""Step 10b: the model (.glb) and the cloth settings (.skmm) (DESIGN.md section 8.11)."""
import json
import os

import bpy
import numpy as np

from . import bones as bones_mod
from . import colliders, doctor, frames, pipeline, poses


def _f(x):
    s = ("%.4f" % float(x)).rstrip("0")
    return s + "0" if s.endswith(".") else s


def skmm_entry(setup, arm):
    """BoneClothData as an ordered list of (group, [(key, value)]) so the file keeps its readable layout."""
    m = setup.motion
    frame = frames.compute_frame(arm, setup.character)
    fwd = frame.to_engine(frame.flat_forward()) if not m.forward_override else np.array(m.forward)
    roots = list(reversed(bones_mod.roots(setup)))        # upper first, as the hand-made file lists them
    cols = []
    for c in setup.colliders:
        if c.enabled and c.bone in arm.data.bones and c.to in arm.data.bones:
            if c.surface:      # the game's own fit is for limbs: a surface capsule always gets its radii
                r0, r1 = colliders.radii(c)
                col = {"bone": c.bone, "to": c.to, "radius": round(r0, 4), "endRadius": round(r1, 4), "surface": True}
            else:
                col = {"bone": c.bone, "to": c.to, "radius": 0.0 if c.auto_radius else round(c.radius, 4),
                       "endRadius": 0.0 if c.auto_end else round(c.end_radius, 4)}
            if any(abs(v) > 1e-6 for v in c.offset):
                col["offset"] = [round(float(v), 4) for v in frame.to_engine(np.array(c.offset))]
            cols.append(col)
    clips = poses.uses_clips(setup)        # the clips replace the two older layers
    open_clip = setup.poses.open.clip_name if not clips and setup.poses.open.enabled and setup.poses.open.action else ""
    vol_clip = setup.poses.volume.clip_name if not clips and setup.poses.volume.enabled and setup.poses.volume.action else ""
    clip_names = [(k, poses.clip_slot(setup, k).action.name if poses.clip_slot(setup, k).action else "")
                  for k in ("windFront", "windBack", "windLeft", "windRight")]
    turn_names = [(k, poses.clip_slot(setup, k).action.name if poses.clip_slot(setup, k).action else "")
                  for k in ("turnLeft", "turnRight")]
    return {
        "roots": roots, "forward": [float(v) for v in fwd], "tipLength": m.tipLength,
        "motion": [("followRoot", m.followRoot), ("followTip", m.followTip), ("trail", m.trail),
                   ("maxAngleRoot", m.maxAngleRoot), ("maxAngleTip", m.maxAngleTip), ("radius", m.radius),
                   ("teleportDistance", m.teleportDistance)],
        "colliders": cols,
        "clips": (clip_names + [("windStart", m.windStart), ("windFull", m.windFull)] + turn_names
                  + [("windStrengthFront", m.windStrengthFront), ("windStrengthSides", m.windStrengthSides),
                     ("windStrengthBack", m.windStrengthBack), ("spinStrength", m.spinStrength)]) if clips else [],
        "wind": [("openClip", open_clip), ("volumeClip", vol_clip), ("volume", m.volume),
                 ("openWindStart", m.openWindStart), ("openWindFull", m.openWindFull), ("windwardBias", m.windwardBias),
                 ("volumeWindStart", m.volumeWindStart), ("volumeWindFull", m.volumeWindFull),
                 ("layerSmoothing", m.layerSmoothing)],
        "spin": [("spinOpen", m.spinOpen), ("spinVolume", m.spinVolume), ("spinStart", m.spinStart),
                 ("spinFull", m.spinFull), ("spinRelease", m.spinRelease)],
    }


def _scalar(v):
    if isinstance(v, str):
        return json.dumps(v)
    return _f(v)


def entry_text(e, indent="        "):
    i1 = indent + "    "
    lines = [indent + "{"]
    lines.append('%s"roots": [ %s ],' % (i1, ", ".join(json.dumps(r) for r in e["roots"])))
    f = e["forward"]
    lines.append('%s"forward": { "x": %s, "y": %s, "z": %s },' % (i1, _f(f[0]), _f(f[1]), _f(f[2])))
    lines.append('%s"tipLength": %s,' % (i1, _f(e["tipLength"])))
    lines.append("")
    for key, v in e["motion"]:
        lines.append('%s"%s": %s,' % (i1, key, _scalar(v)))
    lines.append("")
    lines.append('%s"colliders": [' % i1)
    for k, c in enumerate(e["colliders"]):
        extra = ', "surface": true' if c.get("surface") else ""
        if "offset" in c:
            o = c["offset"]
            extra += ', "offset": { "x": %s, "y": %s, "z": %s }' % (_f(o[0]), _f(o[1]), _f(o[2]))
        lines.append('%s    { "bone": %s, "to": %s, "radius": %s, "endRadius": %s%s }%s' % (
            i1, json.dumps(c["bone"]), json.dumps(c["to"]), _f(c["radius"]), _f(c["endRadius"]), extra,
            "," if k + 1 < len(e["colliders"]) else ""))
    lines.append("%s]," % i1)
    lines.append("")
    for key, v in e.get("clips", []):
        lines.append('%s"%s": %s,' % (i1, key, _scalar(v)))
    if e.get("clips"):
        lines.append("")
    for key, v in e["wind"]:
        lines.append('%s"%s": %s,' % (i1, key, _scalar(v)))
    lines.append("")
    spin = e["spin"]
    for k, (key, v) in enumerate(spin):
        lines.append('%s"%s": %s%s' % (i1, key, _scalar(v), "," if k + 1 < len(spin) else ""))
    lines.append(indent + "}")
    return "\n".join(lines)


def merge_skmm(existing_text, entry, roots_before):
    """New file text: every key and every other cloth entry of the existing file kept, ours replaced or appended."""
    data = json.loads(existing_text) if existing_text and existing_text.strip() else {}
    cloth = data.get("cloth", [])
    ours = sorted(entry["roots"])
    slot = None
    for i, c in enumerate(cloth):
        if sorted(c.get("roots", [])) == ours or (roots_before and sorted(c.get("roots", [])) == sorted(roots_before)):
            slot = i
            break
    texts = []
    for i, c in enumerate(cloth):
        if i == slot:
            continue
        texts.append(json.dumps(c, indent=4).replace("\n", "\n        ").join(["        ", ""]))
    new = entry_text(entry)
    if slot is None:
        texts.append(new)
    else:
        texts.insert(slot, new)
    cloth_text = "[\n" + ",\n".join(texts) + "\n    ]"
    out = ["{"]
    keys = [k for k in data if k != "cloth"]
    items = []
    for k in keys:
        v = json.dumps(data[k], indent=4).replace("\n", "\n    ")
        items.append('    %s: %s' % (json.dumps(k), v))
    items.append('    "cloth": ' + cloth_text)
    out.append(",\n".join(items))
    out.append("}")
    return "\n".join(out) + "\n"


def write_skmm(setup, arm, glb_path):
    path = glb_path + ".skmm"
    existing = ""
    if os.path.exists(path):
        with open(path, encoding="utf-8") as f:
            existing = f.read()
    entry = skmm_entry(setup, arm)
    before = json.loads(setup.export.last_roots) if setup.export.last_roots else []
    text = merge_skmm(existing, entry, before)
    json.loads(text)      # must stay valid
    tmp = path + ".tmp"
    with open(tmp, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)
    os.replace(tmp, path)
    setup.export.last_roots = json.dumps(entry["roots"])
    return path


def export_glb(setup, arm, path):
    scene = bpy.context.scene
    stored = scene.get("glTF2ExportSettings")
    kwargs = dict(stored.to_dict()) if stored else {}
    op = bpy.ops.export_scene.gltf
    valid = set(op.get_rna_type().properties.keys())
    kwargs = {k: v for k, v in kwargs.items() if k in valid}
    kwargs.update(export_format='GLB', export_animations=True, export_skins=True, export_yup=True,
                  export_influence_nb=4, export_all_influences=False)
    for k, v in (("export_animation_mode", 'ACTIONS'), ("export_force_sampling", True),
                 ("export_rest_position_armature", True), ("use_visible", True), ("use_selection", False),
                 ("export_def_bones", setup.export.deform_bones_only)):
        if k in valid:
            kwargs[k] = v
    os.makedirs(os.path.dirname(path), exist_ok=True)
    bones_mod_mode = arm.mode
    if bones_mod_mode != 'OBJECT':
        bpy.ops.object.mode_set(mode='OBJECT')
    poses.set_bases(arm, {})
    bpy.ops.export_scene.gltf(filepath=path, **kwargs)


def verify_written(setup, arm, path):
    """Reads the written model back the way the game does and compares it with the panel's plan (DESIGN.md 8.11.4)."""
    from . import glb_check
    frame = frames.compute_frame(arm, setup.character)
    try:
        skel, verts, strips = doctor.engine_strips(setup, arm, frame)
        expected = [([skel.names[j.bone] for j in s.joints], s.side) for s in strips]
        cols = []
        for c in setup.colliders:
            if c.enabled and c.bone in arm.data.bones and c.to in arm.data.bones:
                cols.append((c.bone, c.to) + tuple(colliders.radii(c)))
        problems = glb_check.compare(path, expected, cols, roots=list(reversed(bones_mod.roots(setup))))
    except Exception as e:           # the check must never hide a successful export
        return ["Could not read the written model back: %s" % e]
    if problems:
        return ["Problem: " + p for p in problems]
    return ["Read the model back like the game: %d panels, as planned" % len(expected)]


def run(setup, arm, settings_only=False):
    """Checks, then writes every target. Returns (messages, issues)."""
    mirrored = []
    for key, source in poses.MIRROR_OF.items():      # mirrored clips follow their other side
        if poses.clip_slot(setup, key).mirrored and poses.clip_slot(setup, source).action is not None:
            poses.mirror_clip(setup, arm, key)
            mirrored.append("Wrote %s from %s, mirrored" % (poses.CLIP_TITLES[key].lower(),
                                                             poses.CLIP_TITLES[source].lower()))
    issues = doctor.run(setup, arm)
    errors = doctor.blocking(issues)
    if errors:
        return ["Export stopped: " + errors[0].message], issues
    msgs = mirrored
    for t in setup.export.targets:
        if not t.glb_path:
            continue
        path = bpy.path.abspath(t.glb_path)
        if t.write_glb and not settings_only:
            export_glb(setup, arm, path)
            msgs.append("Wrote " + path)
        if t.write_skmm:
            msgs.append("Wrote " + write_skmm(setup, arm, path))
        if t.write_glb and t.write_skmm and not settings_only:
            msgs.extend(verify_written(setup, arm, path))
    if not msgs:
        msgs.append("Add an export target first (the .glb path in the game's models folder)")
    return msgs, issues
