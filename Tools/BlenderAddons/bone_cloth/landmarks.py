"""Finds the hips, chest, legs and shoulders of an armature (DESIGN.md section 8.1)."""
import numpy as np

from . import frames

# per rig family: key -> candidate names, first present wins
TABLES = {
    'UE': {
        'hips': ['pelvis'], 'chest': ['spine_03', 'spine_02'], 'neck': ['neck_01'],
        'thigh_l': ['thigh_l'], 'calf_l': ['calf_l'], 'foot_l': ['foot_l'], 'ball_l': ['ball_l'],
        'thigh_r': ['thigh_r'], 'calf_r': ['calf_r'], 'foot_r': ['foot_r'], 'ball_r': ['ball_r'],
        'clavicle_l': ['clavicle_l'], 'clavicle_r': ['clavicle_r'],
    },
    'MIXAMO': {
        'hips': ['mixamorig:Hips', 'mixamorig9:Hips'], 'chest': ['mixamorig:Spine2', 'mixamorig9:Spine2'],
        'neck': ['mixamorig:Neck', 'mixamorig9:Neck'],
        'thigh_l': ['mixamorig:LeftUpLeg', 'mixamorig9:LeftUpLeg'], 'calf_l': ['mixamorig:LeftLeg', 'mixamorig9:LeftLeg'],
        'foot_l': ['mixamorig:LeftFoot', 'mixamorig9:LeftFoot'], 'ball_l': ['mixamorig:LeftToeBase', 'mixamorig9:LeftToeBase'],
        'thigh_r': ['mixamorig:RightUpLeg', 'mixamorig9:RightUpLeg'], 'calf_r': ['mixamorig:RightLeg', 'mixamorig9:RightLeg'],
        'foot_r': ['mixamorig:RightFoot', 'mixamorig9:RightFoot'], 'ball_r': ['mixamorig:RightToeBase', 'mixamorig9:RightToeBase'],
        'clavicle_l': ['mixamorig:LeftShoulder', 'mixamorig9:LeftShoulder'],
        'clavicle_r': ['mixamorig:RightShoulder', 'mixamorig9:RightShoulder'],
    },
    'RIGIFY': {
        'hips': ['DEF-spine', 'spine'], 'chest': ['DEF-spine.003', 'DEF-spine.002'], 'neck': ['DEF-spine.004'],
        'thigh_l': ['DEF-thigh.L'], 'calf_l': ['DEF-shin.L'], 'foot_l': ['DEF-foot.L'], 'ball_l': ['DEF-toe.L'],
        'thigh_r': ['DEF-thigh.R'], 'calf_r': ['DEF-shin.R'], 'foot_r': ['DEF-foot.R'], 'ball_r': ['DEF-toe.R'],
        'clavicle_l': ['DEF-shoulder.L'], 'clavicle_r': ['DEF-shoulder.R'],
    },
    'METARIG': {
        'hips': ['spine'], 'chest': ['spine.003', 'spine.002'], 'neck': ['spine.004'],
        'thigh_l': ['thigh.L'], 'calf_l': ['shin.L'], 'foot_l': ['foot.L'], 'ball_l': ['toe.L'],
        'thigh_r': ['thigh.R'], 'calf_r': ['shin.R'], 'foot_r': ['foot.R'], 'ball_r': ['toe.R'],
        'clavicle_l': ['shoulder.L'], 'clavicle_r': ['shoulder.R'],
    },
}
KEYS = ['hips', 'chest', 'thigh_l', 'calf_l', 'foot_l', 'ball_l', 'thigh_r', 'calf_r', 'foot_r', 'ball_r',
        'clavicle_l', 'clavicle_r', 'neck']
REQUIRED = ['hips', 'chest', 'thigh_l', 'calf_l', 'foot_l', 'thigh_r', 'calf_r', 'foot_r']
LABELS = {
    'hips': "Hips", 'chest': "Chest", 'thigh_l': "Left thigh", 'calf_l': "Left calf", 'foot_l': "Left foot",
    'ball_l': "Left toe", 'thigh_r': "Right thigh", 'calf_r': "Right calf", 'foot_r': "Right foot", 'ball_r': "Right toe",
    'clavicle_l': "Left shoulder", 'clavicle_r': "Right shoulder", 'neck': "Neck",
}


def _family(names):
    best, best_hits = 'GENERIC', 0
    for fam, table in TABLES.items():
        hits = sum(1 for cands in table.values() if any(c in names for c in cands))
        if hits > best_hits:
            best, best_hits = fam, hits
    return best if best_hits >= 6 else 'GENERIC'


def _chain_down(bones, bone, depth=3):
    chain = [bone]
    while len(chain) < depth and chain[-1].children:
        chain.append(max(chain[-1].children, key=lambda b: b.length))
    return chain


def _structural(arm, found):
    """Fallback by structure: hips are the bone with two children that lead down into chains of three."""
    bones = arm.data.bones
    mw = arm.matrix_world

    def z(b):
        return (mw @ b.head_local).z

    if not found.get('hips'):
        best = None
        for b in bones:
            legs = [c for c in b.children if len(_chain_down(bones, c, 3)) >= 3
                    and z(_chain_down(bones, c, 3)[-1]) < z(c) - 0.2]
            if len(legs) >= 2:
                xs = [(mw @ c.head_local).x for c in legs]
                if max(xs) > 0 > min(xs) or abs(max(xs) - min(xs)) > 0.1:
                    if best is None or z(b) > z(best):
                        best = b
        if best:
            found['hips'] = best.name
    hips = bones.get(found.get('hips', ''))
    if hips and not found.get('thigh_l'):
        legs = [c for c in hips.children if len(_chain_down(bones, c, 3)) >= 3 and z(_chain_down(bones, c, 3)[-1]) < z(c) - 0.2]
        legs.sort(key=lambda c: -(mw @ c.head_local).x)    # left is +X for a character facing -Y
        for c, side in zip(legs[:2], ('l', 'r')):
            chain = _chain_down(bones, c, 4)
            found['thigh_' + side] = chain[0].name
            found['calf_' + side] = chain[1].name
            found['foot_' + side] = chain[2].name
            if len(chain) > 3:
                found['ball_' + side] = chain[3].name
    if hips and not found.get('chest'):
        # highest bone on the spine whose children include a left/right pair
        spine = [b for b in bones if b.parent is None or True]
        best = None
        for b in spine:
            if b.name == hips.name or hips not in b.parent_recursive:
                continue
            kids = [c for c in b.children if c.name not in (found.get('thigh_l'), found.get('thigh_r'))]
            xs = [(mw @ c.head_local).x for c in kids]
            if len(kids) >= 2 and max(xs) > 0.05 and min(xs) < -0.05:
                if best is None or z(b) > z(best):
                    best = b
        if best:
            found['chest'] = best.name


def detect(arm, character):
    """Fills `character` from the armature. Names are tried per rig family first, then the structure."""
    names = set(b.name for b in arm.data.bones)
    fam = _family(names)
    found = {}
    if fam != 'GENERIC':
        for key, cands in TABLES[fam].items():
            for c in cands:
                if c in names:
                    found[key] = c
                    break
    if any(k not in found for k in REQUIRED):
        _structural(arm, found)
    character.rig_family = fam
    for key in KEYS:
        if found.get(key):
            setattr(character, key, found[key])
    return [k for k in REQUIRED if not getattr(character, k)]


def exclude_bones(arm, character):
    """Bones whose weight keeps cloth off: the arms (below the shoulders), the neck and the head."""
    out = set()
    bones = arm.data.bones
    for key in ('clavicle_l', 'clavicle_r', 'neck'):
        name = getattr(character, key)
        if name in bones:
            out.add(name)
            out.update(b.name for b in bones[name].children_recursive)
    return out


def leg_capsules(character):
    """Default colliders: (bone, to) pairs."""
    pairs = []
    for side in ('l', 'r'):
        t, c, f = (getattr(character, k + '_' + side) for k in ('thigh', 'calf', 'foot'))
        if t and c:
            pairs.append((t, c))
        if c and f:
            pairs.append((c, f))
    return pairs


def waist_height(arm, character):
    """Height of the first spine bone above the hips (the bone just above the pelvis on the way to the chest)."""
    bones = arm.data.bones
    if character.hips not in bones or character.chest not in bones:
        return None
    node = bones[character.chest]
    while node.parent and node.parent.name != character.hips:
        node = node.parent
    if node.parent is None:
        return None
    return float(frames.bone_head(arm, node.name)[2])


def spine_path(arm, character):
    """Spine bones from the chest down to the hips, top first (the chest and the hips included)."""
    bones = arm.data.bones
    if character.chest not in bones or character.hips not in bones:
        return []
    out, node = [], bones[character.chest]
    while node is not None:
        out.append(node.name)
        if node.name == character.hips:
            return out
        node = node.parent
    return [character.chest, character.hips]
