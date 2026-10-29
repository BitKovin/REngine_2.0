"""The steps as plain functions: the operators, Auto Setup, Update all and the tests all call these."""
import bpy
import numpy as np

from . import bones as bones_mod
from . import (colliders, doctor, frames, guides, landmarks, mesh_prep, poses, presets, props, rest_shape, stages,
               weights)


def frame_of(setup, arm):
    return frames.compute_frame(arm, setup.character)


def detect_character(setup, arm):
    missing = landmarks.detect(arm, setup.character)
    stages.mark_done(setup, "character")
    return missing


def fix_mesh(setup, arm, body=None):
    done = mesh_prep.fix_all(setup, arm, body)
    stages.mark_done(setup, "mesh")
    return done


def measure(setup, arm):
    """Reads the cloth and fills the lines, the opening, the panel angles and where each panel starts."""
    frame = frame_of(setup, arm)
    geom = guides.ClothGeom(setup, arm)
    m = guides.measure(setup, arm, frame, geom)
    g = setup.guides
    if g.top_auto:
        g.top_line = m['top']
    if g.waist_auto:
        g.waist_line = m['waist']
    if g.anchors == 'HIPS':
        g.top_line = g.waist_line
    if g.hem_auto:
        g.hem_line = m['hem']
    if g.layout == 'OPEN_FRONT':
        g.opening_half_angle = m['opening'] if m['opening'] > 0.1 else g.opening_half_angle
    guides.rebuild_columns(setup)
    guides.measure_column_tops(setup, frame, geom)
    presets.blend_heights(setup)
    return m, geom


def add_belt_pin(setup, arm, geom=None):
    geom = geom or guides.ClothGeom(setup, arm)
    belt = guides.detect_belt(geom, setup.guides.waist_line)
    if belt is None:
        return None
    for p in setup.weights.pins:
        if abs(p.height - belt[0]) < 0.02:
            return belt
    p = setup.weights.pins.add()
    p.height, p.half_width = belt
    return belt


def preview_plan(setup, arm):
    frame = frame_of(setup, arm)
    geom = guides.ClothGeom(setup, arm)
    return guides.make_plan(setup, arm, frame, geom), frame


def build_bones(setup, arm):
    if setup.rest.applied:
        rest_shape.reset(setup, arm)
    plan, frame = preview_plan(setup, arm)
    if not plan.strips:
        raise RuntimeError("No panels could be placed on the cloth: " + "; ".join(plan.skipped[:3]))
    bones_mod.build(setup, arm, frame, plan)
    stages.mark_done(setup, "bones", "; ".join(plan.skipped))
    return plan


def build_weights(setup, arm):
    frame = frame_of(setup, arm)
    reapply = setup.rest.applied
    if reapply:
        rest_shape.reset(setup, arm, frame)
    presets.blend_heights(setup)
    stats = weights.build(setup, arm, frame)
    stages.mark_done(setup, "weights")
    # the rest shape was built on the old weights
    if reapply:
        rest_shape.apply(setup, arm, frame)
        stages.mark_done(setup, "rest")
    return stats


def apply_rest(setup, arm):
    frame = frame_of(setup, arm)
    report = rest_shape.apply(setup, arm, frame)
    stages.mark_done(setup, "rest", "; ".join(report))
    return report


def reset_rest(setup, arm):
    rest_shape.reset(setup, arm)
    stages.mark_done(setup, "rest")


def rest_needed(setup, arm):
    bases, _ = rest_shape.solve(setup, arm, frame_of(setup, arm))
    return any(q.angle > 1e-4 for q in bases.values())


def update_poses(setup, arm, force=()):
    """Writes the generated wind clips. Hand-made poses are kept (named in the returned list) unless forced."""
    frame = frame_of(setup, arm)
    kept = []
    for kind in ("open", "volume"):
        layer = getattr(setup.poses, kind)
        if layer.enabled and kind not in force and poses.layer_is_hand(layer):
            kept.append(kind)
        poses.update_layer(setup, arm, frame, kind, force=kind in force)
    stages.mark_done(setup, "poses")
    return kept


def setup_colliders(setup, arm, defaults=True):
    if defaults:
        colliders.add_defaults(setup, arm)
    out = colliders.fit_all(setup, arm)
    stages.mark_done(setup, "colliders")
    return out


def update_all(setup, arm):
    """Re-runs what is out of date, in order."""
    ran = []
    for key in stages.ORDER:
        if stages.status(setup, arm, key) != "stale":
            continue
        if key == "bones":
            build_bones(setup, arm)
            # a new set of bones needs new weights and poses
        elif key == "weights":
            settle_weights(setup, arm, build_weights(setup, arm), lambda m: print("[Bone Cloth]", m))
        elif key == "rest":
            if setup.rest.applied or rest_needed(setup, arm):
                apply_rest(setup, arm)
            else:
                stages.mark_done(setup, "rest")
        elif key == "poses":
            update_poses(setup, arm)
        elif key == "colliders":
            setup_colliders(setup, arm, defaults=not setup.colliders)
        elif key == "character":
            detect_character(setup, arm)
        ran.append(key)
    return ran


def empty_upper_bones(setup, stats):
    """Chest-to-waist bones that no vertex follows (the cloth has nothing above the waist to move)."""
    counts = stats.get('bones', {})
    return [b for b, (n, _) in counts.items() if "_00" in b and n == 0]


def drop_empty_upper(setup, arm, stats, log=print):
    """A chest part with no cloth under it only makes the game cut panels: leave it out and rebuild the bones."""
    empty = empty_upper_bones(setup, stats)
    if not empty or not setup.guides.upper_part or setup.guides.anchors != 'CHEST_AND_HIPS':
        return False
    setup.guides.upper_part = False
    log("No cloth above the waist moves with the panels, so the chest part was left out (%d bones)" % len(empty))
    build_bones(setup, arm)
    return True


def empty_bones(stats):
    return [b for b, (n, _) in stats.get('bones', {}).items() if n == 0]


def fewer_panels(setup, arm, stats, log=print):
    """Panels with no vertices under them mean the mesh is too sparse for this many panels: use one fewer per side."""
    empty = empty_bones(stats)
    g = setup.guides
    if not empty or g.columns_per_side <= 1:
        return False
    g.columns_per_side -= 1
    log("The mesh is too sparse for that many panels (%d bones had no vertices): now %d per side"
        % (len(empty), g.columns_per_side))
    measure(setup, arm)
    build_bones(setup, arm)
    return True


def settle_weights(setup, arm, stats, log=print):
    """Rebuilds the bones and weights until every panel bone carries vertices: a chest part with no cloth is left out,
    and a mesh too sparse for the panel count gets fewer panels."""
    for _ in range(6):
        if drop_empty_upper(setup, arm, stats, log) or fewer_panels(setup, arm, stats, log):
            stats = build_weights(setup, arm)
        else:
            break
    return stats


def auto_setup(setup, arm, body=None, log=print):
    """Everything with the preset's defaults. Returns the list of issues."""
    if not setup.character.hips or not setup.character.chest:
        missing = detect_character(setup, arm)
        if missing:
            raise RuntimeError("Could not find: " + ", ".join(landmarks.LABELS[m] for m in missing))
    stages.mark_done(setup, "character")
    log("Mesh: " + ", ".join(fix_mesh(setup, arm, body)))
    m, geom = measure(setup, arm)
    if setup.weights.profile == 'BELTED_COAT':
        belt = add_belt_pin(setup, arm, geom)
        log("Belt: %s" % (belt,))
    plan = build_bones(setup, arm)
    log("Bones: %d strips (%s)" % (len(plan.strips), "; ".join(plan.skipped) or "all placed"))
    colliders.add_defaults(setup, arm)
    stages.mark_done(setup, "colliders")
    stats = settle_weights(setup, arm, build_weights(setup, arm), log)
    log("Weights: %s" % {k: v for k, v in stats.items() if k != 'bones'})
    colliders.fit_all(setup, arm)
    if rest_needed(setup, arm):
        log("Rest shape: %s" % apply_rest(setup, arm))
        colliders.fit_all(setup, arm)
    else:
        stages.mark_done(setup, "rest")
    update_poses(setup, arm)
    stages.mark_done(setup, "colliders")
    return doctor.run(setup, arm)
