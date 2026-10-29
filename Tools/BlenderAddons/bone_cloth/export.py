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
            cols.append({"bone": c.bone, "to": c.to, "radius": 0.0 if c.auto_radius else round(c.radius, 4),
                         "endRadius": 0.0 if c.auto_end else round(c.end_radius, 4)})
    open_clip = setup.poses.open.clip_name if setup.poses.open.enabled and setup.poses.open.action else ""
    vol_clip = setup.poses.volume.clip_name if setup.poses.volume.enabled and setup.poses.volume.action else ""
    return {
        "roots": roots, "forward": [float(v) for v in fwd], "tipLength": m.tipLength,
        "motion": [("followRoot", m.followRoot), ("followTip", m.followTip), ("trail", m.trail),
                   ("maxAngleRoot", m.maxAngleRoot), ("maxAngleTip", m.maxAngleTip), ("radius", m.radius),
                   ("teleportDistance", m.teleportDistance)],
        "colliders": cols,
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
        lines.append('%s    { "bone": %s, "to": %s, "radius": %s, "endRadius": %s }%s' % (
            i1, json.dumps(c["bone"]), json.dumps(c["to"]), _f(c["radius"]), _f(c["endRadius"]),
            "," if k + 1 < len(e["colliders"]) else ""))
    lines.append("%s]," % i1)
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


def run(setup, arm, settings_only=False):
    """Checks, then writes every target. Returns (messages, issues)."""
    issues = doctor.run(setup, arm)
    errors = doctor.blocking(issues)
    if errors:
        return ["Export stopped: " + errors[0].message], issues
    msgs = []
    for t in setup.export.targets:
        if not t.glb_path:
            continue
        path = bpy.path.abspath(t.glb_path)
        if t.write_glb and not settings_only:
            export_glb(setup, arm, path)
            msgs.append("Wrote " + path)
        if t.write_skmm:
            msgs.append("Wrote " + write_skmm(setup, arm, path))
    if not msgs:
        msgs.append("Add an export target first (the .glb path in the game's models folder)")
    return msgs, issues
