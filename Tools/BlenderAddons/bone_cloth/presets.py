"""Starting values for each kind of cloth (DESIGN.md section 9) and the plain-language motion macros."""
import math

import numpy as np

from . import weights as weights_mod

VERSION = 10      # raise when a preset's values change; setups on an older one are told to apply it again

_COMMON_MOTION = dict(teleportDistance=2.0, tipLength=0.0, windwardBias=0.5)

PRESETS = {
    'LONG_COAT': {
        'guides.layout': 'OPEN_FRONT', 'guides.anchors': 'CHEST_AND_HIPS', 'guides.columns_per_side': 3,
        'guides.center_back': False, 'guides.center_front': False, 'guides.upper_part': True, 'guides.rows': 3,
        'weights.profile': 'BELTED_COAT', 'weights.start_from_top': -0.10, 'weights.full_from_waist': -0.13,
        'rest.clear_legs': True, 'rest.flare': 0.0,
        'motion.followRoot': 60.0, 'motion.followTip': 5.0, 'motion.trail': 0.5, 'motion.maxAngleRoot': 8.0,
        'motion.maxAngleTip': 50.0, 'motion.radius': 0.05, 'motion.volume': 0.5, 'motion.openWindStart': 1.0,
        'motion.openWindFull': 13.0, 'motion.volumeWindStart': 1.0, 'motion.volumeWindFull': 15.0,
        'motion.layerSmoothing': 0.2, 'motion.spinOpen': 0.8, 'motion.spinVolume': 0.6, 'motion.spinStart': 180.0,
        'motion.spinFull': 720.0, 'motion.spinRelease': 0.5,
        'poses.open.amount': 0.24, 'poses.open.direction': 0.6, 'poses.open.upper_share': 0.5,
        'poses.volume.amount': 0.11, 'poses.volume.direction': 0.0, 'poses.volume.upper_share': 0.3,
    },
    'JACKET': {
        'guides.layout': 'OPEN_FRONT', 'guides.anchors': 'SPINE', 'guides.columns_per_side': 3,
        'guides.center_back': False, 'guides.upper_part': False, 'guides.rows': 2, 'guides.bone_inset': 0.75,
        'weights.profile': 'LOOSE_BELOW_CHEST', 'weights.start_from_top': 0.15, 'weights.full_from_waist': 0.40,
        'rest.clear_legs': True, 'rest.flare': 0.0,
        'motion.followRoot': 80.0, 'motion.followTip': 12.0, 'motion.trail': 0.3, 'motion.maxAngleRoot': 5.0,
        'motion.maxAngleTip': 40.0, 'motion.radius': 0.03, 'motion.volume': 0.3, 'motion.openWindStart': 2.0,
        'motion.openWindFull': 12.0, 'motion.volumeWindStart': 2.0, 'motion.volumeWindFull': 14.0,
        'motion.layerSmoothing': 0.2, 'motion.spinOpen': 0.5, 'motion.spinVolume': 0.4, 'motion.spinStart': 180.0,
        'motion.spinFull': 720.0, 'motion.spinRelease': 0.4,
        # the poses are the full-wind shapes (the game blends toward them with the headwind): front panels open wide and
        # sweep back, every panel is blown back in one smooth arc. The Open values are fitted to a hand-made pose
        # (tests/data/jacket_open_target.json, rms error 1.9 cm); a jacket's opening is better authored by hand: save it
        # with 'Save my pose' and Update keeps it
        'poses.open.amount': 0.29, 'poses.open.direction': 0.54, 'poses.open.backward': 1.12, 'poses.open.bend': 0.0,
        'poses.open.upper_share': 1.0, 'poses.open.max_turn': 0.77, 'poses.open.slide': 0.08,
        'poses.volume.amount': 0.40, 'poses.volume.direction': 0.0, 'poses.volume.backward': 1.0, 'poses.volume.bend': 0.25,
        'poses.volume.upper_share': 1.0, 'poses.volume.max_turn': 0.95,
    },
    'SKIRT': {
        'guides.layout': 'CLOSED_RING', 'guides.anchors': 'HIPS', 'guides.columns_per_side': 3,
        'guides.center_front': True, 'guides.center_back': True, 'guides.upper_part': False, 'guides.rows': 3,
        'weights.profile': 'LOOSE_BELOW_WAIST', 'weights.start_from_top': 0.0, 'weights.full_from_waist': -0.12,
        'rest.clear_legs': True, 'rest.flare': 0.01,
        'motion.followRoot': 50.0, 'motion.followTip': 6.0, 'motion.trail': 0.4, 'motion.maxAngleRoot': 8.0,
        'motion.maxAngleTip': 55.0, 'motion.radius': 0.04, 'motion.volume': 0.4, 'motion.openWindStart': 2.0,
        'motion.openWindFull': 8.0, 'motion.volumeWindStart': 1.0, 'motion.volumeWindFull': 12.0,
        'motion.layerSmoothing': 0.2, 'motion.spinOpen': 0.0, 'motion.spinVolume': 0.7, 'motion.spinStart': 180.0,
        'motion.spinFull': 720.0, 'motion.spinRelease': 0.5,
        'poses.open.enabled': False, 'poses.open.amount': 0.0,
        'poses.volume.amount': 0.08, 'poses.volume.direction': 0.0, 'poses.volume.upper_share': 0.3,
    },
    'CAPE': {
        'guides.layout': 'BACK_ONLY', 'guides.anchors': 'CHEST', 'guides.columns_per_side': 2,
        'guides.center_back': True, 'guides.center_front': False, 'guides.upper_part': False, 'guides.rows': 4,
        'weights.profile': 'LOOSE_BELOW_CHEST', 'weights.start_from_top': 0.04, 'weights.full_from_waist': -0.12,
        'rest.clear_legs': True, 'rest.flare': 0.0,
        'motion.followRoot': 40.0, 'motion.followTip': 4.0, 'motion.trail': 0.6, 'motion.maxAngleRoot': 10.0,
        'motion.maxAngleTip': 80.0, 'motion.radius': 0.04, 'motion.volume': 0.3, 'motion.openWindStart': 1.0,
        'motion.openWindFull': 10.0, 'motion.volumeWindStart': 1.0, 'motion.volumeWindFull': 12.0,
        'motion.layerSmoothing': 0.3, 'motion.spinOpen': 0.6, 'motion.spinVolume': 0.8, 'motion.spinStart': 120.0,
        'motion.spinFull': 600.0, 'motion.spinRelease': 0.7,
        # the wind blows it back and out: one pose does it, there is no front to open
        'poses.open.enabled': False, 'poses.open.amount': 0.0,
        'poses.volume.amount': 0.35, 'poses.volume.direction': 0.0, 'poses.volume.backward': 0.6, 'poses.volume.bend': 0.4,
        'poses.volume.upper_share': 0.5, 'poses.volume.max_turn': 1.0,
    },
    'STRAND': {
        'guides.layout': 'STRAND', 'guides.anchors': 'HIPS', 'guides.columns_per_side': 1, 'guides.center_back': False,
        'guides.upper_part': False, 'guides.rows': 5,
        'weights.profile': 'CUSTOM', 'weights.start_from_top': 0.0, 'weights.full_from_waist': 0.0,
        'weights.exclude_arms': False, 'weights.rigid_parts': False,
        'rest.clear_legs': False, 'rest.flare': 0.0,
        'motion.followRoot': 50.0, 'motion.followTip': 6.0, 'motion.trail': 0.4, 'motion.maxAngleRoot': 10.0,
        'motion.maxAngleTip': 70.0, 'motion.radius': 0.02, 'motion.volume': 0.0, 'motion.openWindStart': 2.0,
        'motion.openWindFull': 10.0, 'motion.volumeWindStart': 2.0, 'motion.volumeWindFull': 12.0,
        'motion.layerSmoothing': 0.2, 'motion.spinOpen': 0.0, 'motion.spinVolume': 0.0, 'motion.spinStart': 180.0,
        'motion.spinFull': 720.0, 'motion.spinRelease': 0.5,
        'poses.open.enabled': False, 'poses.open.amount': 0.0,
        'poses.volume.amount': 0.15, 'poses.volume.direction': 0.0, 'poses.volume.backward': 0.6, 'poses.volume.bend': 0.4,
        'poses.volume.upper_share': 1.0, 'poses.volume.max_turn': 0.9,
    },
    'ROBE': {
        'guides.layout': 'CLOSED_RING', 'guides.anchors': 'CHEST_AND_HIPS', 'guides.columns_per_side': 3,
        'guides.center_front': False, 'guides.center_back': True, 'guides.upper_part': True, 'guides.rows': 4,
        'weights.profile': 'LOOSE_BELOW_CHEST', 'weights.start_from_top': -0.10, 'weights.full_from_waist': -0.10,
        'rest.clear_legs': True, 'rest.flare': 0.0,
        'motion.followRoot': 55.0, 'motion.followTip': 4.0, 'motion.trail': 0.5, 'motion.maxAngleRoot': 8.0,
        'motion.maxAngleTip': 45.0, 'motion.radius': 0.05, 'motion.volume': 0.5, 'motion.openWindStart': 1.0,
        'motion.openWindFull': 13.0, 'motion.volumeWindStart': 1.0, 'motion.volumeWindFull': 15.0,
        'motion.layerSmoothing': 0.25, 'motion.spinOpen': 0.7, 'motion.spinVolume': 0.6, 'motion.spinStart': 180.0,
        'motion.spinFull': 720.0, 'motion.spinRelease': 0.6,
        'poses.open.amount': 0.15, 'poses.open.direction': 0.6, 'poses.open.upper_share': 0.5,
        'poses.volume.amount': 0.10, 'poses.volume.direction': 0.0, 'poses.volume.upper_share': 0.3,
    },
}

FEELS = {'LEATHER': (0.0, 0.2), 'WOOL': (0.3, 0.4), 'COTTON': (0.6, 0.5), 'SILK': (0.9, 0.7)}


def _set(root, path, value):
    parts = path.split(".")
    obj = root
    for p in parts[:-1]:
        obj = getattr(obj, p)
    setattr(obj, parts[-1], value)


def apply(setup, name):
    """Writes a preset's settings (not data) into the setup. Stages that depend on them turn stale."""
    table = PRESETS.get(name)
    if not table:
        return
    setup.preset = name
    setup.preset_version = VERSION
    for path, value in table.items():
        _set(setup, path, value)
    setup.poses.open.enabled = table.get('poses.open.enabled', True)
    setup.motion.feel = 'CUSTOM'
    node = weights_mod.curve_node(setup)
    weights_mod.set_curve(node, weights_mod.BELT_PTS if setup.weights.profile == 'BELTED_COAT' else weights_mod.SMOOTH_PTS)
    setup.poses.open.clip_name = setup.poses.open.clip_name or "%s_open" % setup.prefix
    setup.poses.volume.clip_name = setup.poses.volume.clip_name or "%s_volume" % setup.prefix


def apply_feel(motion):
    """Fabric weight and response sliders -> the engine fields they drive."""
    if motion.feel in FEELS:
        motion.weight, motion.response = FEELS[motion.feel]
    w, r = motion.weight, motion.response
    motion.followTip = 10.0 + (3.0 - 10.0) * w
    motion.maxAngleTip = 35.0 + (75.0 - 35.0) * w
    motion.trail = 0.3 + (0.7 - 0.3) * w
    motion.followRoot = 90.0 + (40.0 - 90.0) * r
    motion.layerSmoothing = 0.1 + (0.4 - 0.1) * r


def blend_heights(setup):
    """Writes the automatic blend start / full heights from the lines and the preset's offsets."""
    w, g = setup.weights, setup.guides
    if w.blend_auto:
        w.blend_start = g.top_line + w.start_from_top
        # a cape has no waist: its 'fully loose' offset counts from the shoulder line
        w.blend_full = (g.top_line if g.anchors == 'CHEST' else g.waist_line) + w.full_from_waist


def outdated(setup):
    """True when the setup's preset has been improved since it was applied."""
    return setup.preset != 'CUSTOM' and setup.preset_version != VERSION
