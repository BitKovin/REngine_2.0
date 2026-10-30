"""Property groups: the add-on's data model (DESIGN.md section 6.4). Stored on the armature object as Object.bone_cloth."""
import json
import uuid

import bpy
from bpy.props import (BoolProperty, CollectionProperty, EnumProperty, FloatProperty, FloatVectorProperty, IntProperty,
                       PointerProperty, StringProperty)
from bpy.types import PropertyGroup

STAGES = ("character", "mesh", "bones", "weights", "rest", "poses", "colliders", "motion")


def _redraw(self, context):
    from . import overlay
    overlay.tag_dirty()


class BCObjectRef(PropertyGroup):
    obj: PointerProperty(type=bpy.types.Object)


class BCColumn(PropertyGroup):
    name: StringProperty(name="Name")
    side: EnumProperty(name="Side", items=[('L', "Left", ""), ('R', "Right", ""), ('C', "Centre", "")], default='L')
    angle: FloatProperty(name="Angle", subtype='ANGLE', unit='ROTATION', description="Around the body: 0 is the front, "
                         "90 is the character's left", default=0.0, update=_redraw)
    top_offset: FloatProperty(name="Top offset", subtype='DISTANCE', unit='LENGTH', description="Where this panel starts, "
                              "relative to the top line (negative is lower)", default=0.0, update=_redraw)
    enabled: BoolProperty(name="Use", default=True, update=_redraw)
    open_mult: FloatProperty(name="Opens", description="How much of the Open pose this panel takes part in", default=1.0,
                             min=0.0, max=1.5)
    volume_mult: FloatProperty(name="Flares", description="How much of the Volume pose this panel takes part in", default=1.0,
                               min=0.0, max=1.5)
    rest_flare_mult: FloatProperty(name="Rest flare", default=1.0, min=0.0, max=1.5)


class BCPin(PropertyGroup):
    height: FloatProperty(name="Height", subtype='DISTANCE', unit='LENGTH', default=1.1, update=_redraw)
    half_width: FloatProperty(name="Half width", subtype='DISTANCE', unit='LENGTH', default=0.025, min=0.0, update=_redraw)
    softness: FloatProperty(name="Softness", subtype='DISTANCE', unit='LENGTH', default=0.01, min=0.0)
    strength: FloatProperty(name="Strength", default=1.0, min=0.0, max=1.0)


class BCCharacter(PropertyGroup):
    hips: StringProperty(name="Hips")
    chest: StringProperty(name="Chest")
    thigh_l: StringProperty(name="Thigh L")
    calf_l: StringProperty(name="Calf L")
    foot_l: StringProperty(name="Foot L")
    ball_l: StringProperty(name="Toe L")
    thigh_r: StringProperty(name="Thigh R")
    calf_r: StringProperty(name="Calf R")
    foot_r: StringProperty(name="Foot R")
    ball_r: StringProperty(name="Toe R")
    clavicle_l: StringProperty(name="Shoulder L")
    clavicle_r: StringProperty(name="Shoulder R")
    neck: StringProperty(name="Neck")
    rig_family: StringProperty(name="Rig", default="")
    facing: EnumProperty(name="Facing", items=[
        ('AUTO', "Automatic", "From the feet"), ('NEG_Y', "-Y", ""), ('POS_Y', "+Y", ""), ('NEG_X', "-X", ""),
        ('POS_X', "+X", "")], default='AUTO', update=_redraw)
    flip: BoolProperty(name="Flip", description="Turn the detected facing around", default=False, update=_redraw)


class BCGuides(PropertyGroup):
    layout: EnumProperty(name="Layout", items=[
        ('OPEN_FRONT', "Open front", "Coat or jacket with the front open"),
        ('CLOSED_RING', "Closed ring", "Skirt, closed jacket or robe: panels all the way around"),
        ('BACK_ONLY', "Back only (cape)", "A cape or cloak: panels over the back, hanging from the shoulders"),
        ('STRAND', "Single strand", "A tail, scarf end or flap: one chain of bones along the mesh, from a bone")],
        default='OPEN_FRONT')
    anchors: EnumProperty(name="Anchors", items=[
        ('CHEST_AND_HIPS', "Chest + hips", "An upper part rides the chest, the skirt turns with the hips"),
        ('HIPS', "Hips only", "Everything hangs from the hips"),
        ('CHEST', "Chest only", "A cape: everything hangs from the shoulders, the top line is the shoulder line"),
        ('SPINE', "Follow the spine", "Short jackets: every panel runs from the chest to the hem and each part hangs "
         "from the spine bone at its height, so the jacket bends with the spine")], default='CHEST_AND_HIPS')
    strand_anchor: StringProperty(name="Hangs from", description="Single strand: the bone the strand hangs from")
    opening_half_angle: FloatProperty(name="Front opening", subtype='ANGLE', unit='ROTATION', default=0.7, min=0.0,
                                      max=3.0, description="Half of the opening at the front", update=_redraw)
    columns_per_side: IntProperty(name="Panels per side", default=3, min=1, max=6, update=_redraw)
    center_back: BoolProperty(name="Centre back panel", default=False, update=_redraw)
    center_front: BoolProperty(name="Centre front panel", default=False, update=_redraw)
    back_vent: BoolProperty(name="Back vent", description="Hard split at the back", default=False)
    upper_part: BoolProperty(name="Upper part", description="Add the bone from the chest to the waist", default=True,
                             update=_redraw)
    rows: IntProperty(name="Rows below the waist", description="Bones in the lowest part (below the waist, or below the "
                      "last spine bone when following the spine)", default=3, min=1, max=8, update=_redraw)
    row_spacing: EnumProperty(name="Row spacing", items=[
        ('EVEN', "Even", ""), ('DENSER_AT_HEM', "More at the hem", ""), ('DENSER_AT_TOP', "More at the top", "")],
        default='EVEN', update=_redraw)
    row_bias: FloatProperty(name="Bias", default=0.5, min=0.0, max=1.0, update=_redraw)
    top_auto: BoolProperty(name="Auto", default=True, update=_redraw)
    top_line: FloatProperty(name="Top line", subtype='DISTANCE', unit='LENGTH', default=1.4, update=_redraw)
    waist_auto: BoolProperty(name="Auto", default=True, update=_redraw)
    waist_line: FloatProperty(name="Waist line", subtype='DISTANCE', unit='LENGTH', default=1.1, update=_redraw)
    hem_auto: BoolProperty(name="Auto", default=True, update=_redraw)
    hem_line: FloatProperty(name="Hem line", subtype='DISTANCE', unit='LENGTH', default=0.5, update=_redraw)
    placement: EnumProperty(name="Place bones", items=[
        ('OUTER', "On the outer surface", ""), ('MIDDLE', "In the middle of the cloth", ""),
        ('INNER', "On the inner surface", "")], default='OUTER', update=_redraw)
    surface_offset: FloatProperty(name="Offset", subtype='DISTANCE', unit='LENGTH', default=0.0, update=_redraw)
    path: EnumProperty(name="Path", items=[
        ('FOLLOW', "Follow the surface", ""), ('STRAIGHT', "Straight", "Straight from the waist to the hem")],
        default='FOLLOW', update=_redraw)
    max_bone_length: FloatProperty(name="Max bone length", subtype='DISTANCE', unit='LENGTH', description="Follow the spine: "
                                   "a section longer than this is split into equal bones, so the panel bends in smaller, even steps",
                                   default=0.0, min=0.0, max=1.0, update=_redraw)
    bone_inset: FloatProperty(name="Bones toward the spine", description="Follow the spine: how far the torso part of "
                              "the bones moves from the cloth surface to the spine's joints. The more, the closer the "
                              "jacket follows a bending spine; the panels stay apart as long as this is below 1",
                              default=0.0, min=0.0, max=0.9, update=_redraw)
    symmetric: BoolProperty(name="Symmetric", default=True)
    columns: CollectionProperty(type=BCColumn)
    columns_index: IntProperty()


class BCWeights(PropertyGroup):
    profile: EnumProperty(name="Where does the cloth leave the body?", items=[
        ('LOOSE_BELOW_WAIST', "Loose below the waist", "Skirts and jackets"),
        ('LOOSE_BELOW_CHEST', "Loose below the chest", "Robes"),
        ('BELTED_COAT', "Belted coat", "A coat held at the waist by a belt"),
        ('CUSTOM', "Custom", "")], default='BELTED_COAT')
    blend_auto: BoolProperty(name="Auto", default=True)
    start_from_top: FloatProperty(name="Blend start below the top", subtype='DISTANCE', unit='LENGTH', default=-0.10)
    full_from_waist: FloatProperty(name="Fully loose below the waist", subtype='DISTANCE', unit='LENGTH', default=-0.13)
    blend_start: FloatProperty(name="Blend starts", subtype='DISTANCE', unit='LENGTH', default=1.3, update=_redraw)
    blend_full: FloatProperty(name="Fully loose", subtype='DISTANCE', unit='LENGTH', default=0.95, update=_redraw)
    pins: CollectionProperty(type=BCPin)
    pins_index: IntProperty()
    exclude_arms: BoolProperty(name="Keep sleeves on the arms", default=True)
    exclude_threshold: FloatProperty(name="Arm weight limit", default=0.4, min=0.05, max=1.0)
    use_painted_amount: BoolProperty(name="Use my painted cloth amount", default=False)
    panel_sharpness: FloatProperty(name="Panel edges", description="Soft: panels blend into each other. Sharp: hard "
                                   "edges", default=0.5, min=0.0, max=0.95)
    joint_softness: FloatProperty(name="Joint blend", description="How far the blend between a panel's bones reaches, in "
                                  "bones", default=0.35, min=0.0, max=0.5)
    rigid_parts: BoolProperty(name="Small parts move rigidly", description="Buttons, buckles and other loose parts "
                              "follow the cloth under them without bending", default=True)
    rigid_max_size: FloatProperty(name="Up to", subtype='DISTANCE', unit='LENGTH', default=0.04, min=0.0)
    smooth_iterations: IntProperty(name="Smoothing passes", default=4, min=0, max=20)
    smooth_strength: FloatProperty(name="Smoothing strength", default=0.5, min=0.0, max=1.0)
    clean_below: FloatProperty(name="Drop weights below", default=0.01, min=0.0, max=0.2)


class BCRest(PropertyGroup):
    flare: FloatProperty(name="Flare at hem", subtype='DISTANCE', unit='LENGTH', default=0.0, min=0.0, max=1.0)
    open: FloatProperty(name="Open front at rest", subtype='DISTANCE', unit='LENGTH', default=0.0, min=0.0, max=1.0)
    bend: FloatProperty(name="Bend", description="0: bends evenly along the panel, 1: mostly near the hem", default=0.5,
                        min=0.0, max=1.0)
    clear_legs: BoolProperty(name="Keep clear of the legs", default=True)
    clearance: FloatProperty(name="Margin", subtype='DISTANCE', unit='LENGTH', default=0.01, min=0.0)
    applied: BoolProperty(name="Applied", default=False)
    snapshot: StringProperty(name="Snapshot", description="Bones as generated, as JSON", default="")


class BCPoseLayer(PropertyGroup):
    enabled: BoolProperty(name="Use", default=True)
    clip_name: StringProperty(name="Clip name")
    action: PointerProperty(type=bpy.types.Action)
    mode: EnumProperty(name="Mode", items=[('GENERATED', "Generated", ""), ('HAND', "Hand edited", "")], default='GENERATED')
    amount: FloatProperty(name="Hem moves", subtype='DISTANCE', unit='LENGTH', default=0.2, min=0.0, max=2.0)
    direction: FloatProperty(name="Direction", description="Open: 0 straight out, 1 sideways. Volume: 0 outward, 1 "
                             "lifted", default=0.6, min=0.0, max=1.0)
    max_turn: FloatProperty(name="Max turn", description="The most the whole panel may turn from hanging straight, in "
                            "degrees. A panel that is asked to move further than it can curls up at the hem instead",
                            subtype='ANGLE', unit='ROTATION', default=0.7, min=0.05, max=1.5)
    slide: FloatProperty(name="Slide out", subtype='DISTANCE', unit='LENGTH', description="Shifts the whole panel sideways "
                         "from its top (away from the front opening), so the chest and lapels open too, not only the hem",
                         default=0.0, min=0.0, max=0.5)
    backward: FloatProperty(name="Blown back", description="How much the panels are swept backwards, as in a headwind",
                            default=0.0, min=0.0, max=2.0)
    bend: FloatProperty(name="Bend", description="Where along the panel the turn happens: 0 spreads it evenly (an arc), "
                        "1 mostly near the hem, below 0 more near the top (a straighter line from the top)", default=0.5,
                        min=-1.0, max=1.0)
    upper_share: FloatProperty(name="Chest part", description="How much the chest-to-waist bone takes part", default=0.5,
                               min=0.0, max=1.0)


class BCClipSlot(PropertyGroup):
    """One wind or turn clip: frame 1 the rest shape, then the poses the cloth goes through as the wind from that side
    (or the turn) grows. The game plays it as far as the wind is strong."""
    action: PointerProperty(type=bpy.types.Action, name="Clip")
    mirrored: BoolProperty(name="Mirror of the other side", default=True, description="Written from the other side's "
                           "clip, mirrored (right from left), on export and with the Mirror button")


class BCClipGen(PropertyGroup):
    """How strong the generated clips' later frames are (clipgen.py)."""
    blow_back: FloatProperty(name="Blown back", description="Last frame of a headwind (and of the far side in a side "
                             "wind): how far the front panels swing round toward the back, as a share of the Open "
                             "pose's opening. Far past 1 the front panels cross the side seams and tear away from the "
                             "back ones", default=1.0, min=0.0, max=3.0)
    lift: FloatProperty(name="Hem lift", subtype='ANGLE', unit='ROTATION', description="Strong wind: how far the hems "
                        "lift away from the body", default=0.4363, min=0.0, max=1.5)
    sweep: FloatProperty(name="Sweep", subtype='ANGLE', unit='ROTATION', description="Side wind and turning: how far "
                         "the panels sweep round the body with the air", default=0.6109, min=0.0, max=1.5)
    turn_flare: FloatProperty(name="Turn flare", subtype='ANGLE', unit='ROTATION', description="Turning: how far the "
                              "hems fly out", default=0.5236, min=0.0, max=1.5)


class BCPoses(PropertyGroup):
    open: PointerProperty(type=BCPoseLayer)
    volume: PointerProperty(type=BCPoseLayer)
    preview_open: FloatProperty(name="Open", default=0.0, min=0.0, max=1.0, update=lambda s, c: _preview(c))
    preview_volume: FloatProperty(name="Volume", default=0.0, min=0.0, max=1.0, update=lambda s, c: _preview(c))
    # the wind and turn clips (BoneClothData windFront ... turnRight); they replace Open and Volume when any is set
    windFront: PointerProperty(type=BCClipSlot)
    windLeft: PointerProperty(type=BCClipSlot)
    windBack: PointerProperty(type=BCClipSlot)
    windRight: PointerProperty(type=BCClipSlot)
    turnLeft: PointerProperty(type=BCClipSlot)
    turnRight: PointerProperty(type=BCClipSlot)
    gen: PointerProperty(type=BCClipGen)
    preview_wind: FloatProperty(name="Wind", description="How far through the wind clips (0 rest, 1 their last frame)",
                                default=0.0, min=0.0, max=1.0, update=lambda s, c: _preview(c))
    preview_wind_from: FloatProperty(name="From", subtype='ANGLE', unit='ROTATION', description="Where the wind comes "
                                     "from, seen from the character: 0 the front, 90 the left, 180 behind", default=0.0,
                                     update=lambda s, c: _preview(c))
    preview_turn: FloatProperty(name="Turn", description="How far through the turn clips: positive turning left, "
                                "negative turning right", default=0.0, min=-1.0, max=1.0, update=lambda s, c: _preview(c))


def _preview(context):
    from . import poses
    poses.apply_preview(context)


class BCCollider(PropertyGroup):
    bone: StringProperty(name="From")
    to: StringProperty(name="To")
    enabled: BoolProperty(name="Use", default=True)
    auto_radius: BoolProperty(name="Auto", default=True)
    radius: FloatProperty(name="Radius", subtype='DISTANCE', unit='LENGTH', default=0.08, min=0.0)
    auto_end: BoolProperty(name="Auto", default=True)
    end_radius: FloatProperty(name="End radius", subtype='DISTANCE', unit='LENGTH', default=0.06, min=0.0)
    fitted_radius: FloatProperty(default=0.0)
    fitted_end: FloatProperty(default=0.0)
    surface: BoolProperty(name="Keeps the cloth out", default=False, description="surface: the cloth itself (a few "
                          "vertices each bone carries) stays out of it, not the bones' tips - for the torso, where the "
                          "bones run inside the body. Auto radii shrink until the resting cloth is just outside")
    covers: BoolProperty(name="Cloth covers it", default=False, description="covers: the cloth goes over this part (a "
                         "leg under a jacket) - with surface capsules, the cloth itself is kept on its outer side, even "
                         "when it comes up through the cloth (a thigh in a crouch). Not for parts outside the cloth (arms)")
    offset: FloatVectorProperty(name="Offset", size=3, subtype='TRANSLATION', unit='LENGTH', default=(0.0, 0.0, 0.0),
                                description="offset: moves both ends by this much as the character rests (a capsule "
                                "along the side of the torso); each end then moves with its bone")


class BCMotion(PropertyGroup):
    # BoneClothData fields, same names and defaults as Engine/Cloth/BoneClothData.h
    followRoot: FloatProperty(name="Top stiffness", description="followRoot: how fast a bone's tip catches up with where "
                              "it would hang at the top, 1/s", default=60.0, min=0.0)
    followTip: FloatProperty(name="Hem stiffness", description="followTip: the same at the free end (low: floatier)",
                             default=8.0, min=0.0)
    trail: FloatProperty(name="Lag behind movement", description="trail: share of the character's movement the cloth "
                         "trails behind", default=0.2, min=0.0, max=1.0)
    maxAngleRoot: FloatProperty(name="Swing limit at top", description="maxAngleRoot (degrees)", default=8.0, min=0.0,
                                max=180.0)
    maxAngleTip: FloatProperty(name="Swing limit at hem", description="maxAngleTip (degrees)", default=70.0, min=0.0,
                               max=180.0)
    radius: FloatProperty(name="Padding", subtype='DISTANCE', unit='LENGTH', description="radius: how far the joints "
                          "keep from colliders", default=0.02, min=0.0)
    teleportDistance: FloatProperty(name="Teleport distance", subtype='DISTANCE', unit='LENGTH', default=2.0, min=0.0)
    tipLength: FloatProperty(name="Hem bone length", subtype='DISTANCE', unit='LENGTH', description="tipLength: 0 is to "
                             "the farthest vertex", default=0.0, min=0.0)
    volume: FloatProperty(name="Always-on volume", description="volume: base weight of the Volume pose", default=0.0,
                          min=0.0, max=1.0)
    windStart: FloatProperty(name="Clips start at", description="windStart: apparent wind where the wind clips leave "
                             "their first frame (m/s)", default=2.0, min=0.0)
    windFull: FloatProperty(name="Clips end at", description="windFull: apparent wind where they reach their last frame "
                            "(m/s)", default=12.0, min=0.0)
    windStrengthFront: FloatProperty(name="Wind front", description="windStrengthFront: how far the wind-from-the-front "
                                     "clip plays for the same wind. 1 as made, below calmer, above reaches its end sooner",
                                     default=1.0, min=0.0, soft_max=2.0)
    windStrengthSides: FloatProperty(name="Wind sides", description="windStrengthSides: the same for the left and right "
                                     "wind clips", default=1.0, min=0.0, soft_max=2.0)
    windStrengthBack: FloatProperty(name="Wind back", description="windStrengthBack: the same for the wind-from-behind "
                                    "clip", default=1.0, min=0.0, soft_max=2.0)
    spinStrength: FloatProperty(name="Spin", description="spinStrength: how far the turn clips play for the same turn",
                                default=1.0, min=0.0, soft_max=2.0)
    swing: FloatProperty(name="Swing", description="swing: how much the cloth trails and swings behind the pose (0: it "
                         "holds the clips' pose exactly)", default=1.0, min=0.0, max=1.0)
    motionWindow: FloatProperty(name="Motion window", subtype='TIME', unit='TIME_ABSOLUTE', description="motionWindow: "
                                "the cloth trails the body's average movement over this many seconds, so a sway back and "
                                "forth within it cancels out (0: the classic trailing, by Lag behind movement)",
                                default=0.0, min=0.0, soft_max=2.0)
    lagRoot: FloatProperty(name="Trail at top", subtype='TIME', unit='TIME_ABSOLUTE', description="lagRoot: seconds of "
                           "that average movement the top trails behind", default=0.0, min=0.0, soft_max=0.5)
    lagTip: FloatProperty(name="Trail at hem", subtype='TIME', unit='TIME_ABSOLUTE', description="lagTip: the same at "
                          "the hem", default=0.02, min=0.0, soft_max=0.5)
    physicsFront: FloatProperty(name="Physics front", description="physicsFront: how much the cloth trails when the "
                                "body moves forward", default=1.0, min=0.0, soft_max=2.0)
    physicsBack: FloatProperty(name="Physics back", description="physicsBack: the same moving backward",
                               default=1.0, min=0.0, soft_max=2.0)
    physicsSides: FloatProperty(name="Physics sides", description="physicsSides: the same moving sideways",
                                default=1.0, min=0.0, soft_max=2.0)
    physicsVertical: FloatProperty(name="Physics vertical", description="physicsVertical: the same moving up or down",
                                   default=1.0, min=0.0, soft_max=2.0)
    physicsSpin: FloatProperty(name="Physics spin", description="physicsSpin: the same for the body turning",
                               default=1.0, min=0.0, soft_max=2.0)
    collision: FloatProperty(name="Collision", description="collision: how hard the leg and torso capsules push the "
                             "cloth (0: not at all). Swing and Collision at 0: the cloth is driven by the clips alone",
                             default=1.0, min=0.0, max=1.0)
    openWindStart: FloatProperty(name="Wind opens from", description="openWindStart (m/s)", default=2.0, min=0.0)
    openWindFull: FloatProperty(name="Fully open at", description="openWindFull (m/s)", default=8.0, min=0.0)
    windwardBias: FloatProperty(name="Windward side bias", default=0.5, min=0.0, max=1.0)
    volumeWindStart: FloatProperty(name="Wind fills from", description="volumeWindStart (m/s)", default=2.0, min=0.0)
    volumeWindFull: FloatProperty(name="Fully filled at", description="volumeWindFull (m/s)", default=10.0, min=0.0)
    layerSmoothing: FloatProperty(name="Wind response time", subtype='TIME', unit='TIME_ABSOLUTE', default=0.2, min=0.0)
    spinOpen: FloatProperty(name="Spin opens", description="spinOpen: how much a fast turn opens the front. With "
                            "the clips: a full spin counts as this share of 'Clips end at' of headwind", default=0.0,
                            min=0.0, max=1.0)
    spinVolume: FloatProperty(name="Spin fills", default=0.0, min=0.0, max=1.0)
    spinStart: FloatProperty(name="Spin from", description="spinStart (degrees/s)", default=180.0, min=0.0)
    spinFull: FloatProperty(name="Spin full at", description="spinFull (degrees/s)", default=720.0, min=0.0)
    spinRelease: FloatProperty(name="Spin settles in", subtype='TIME', unit='TIME_ABSOLUTE', default=0.5, min=0.0)
    forward_override: BoolProperty(name="Set forward by hand", default=False)
    forward: FloatVectorProperty(name="Forward", size=3, default=(0.0, 0.0, 1.0))
    feel: EnumProperty(name="Feel", items=[
        ('LEATHER', "Leather", ""), ('WOOL', "Heavy wool", ""), ('COTTON', "Cotton", ""), ('SILK', "Light silk", ""),
        ('CUSTOM', "Custom", "")], default='WOOL')
    weight: FloatProperty(name="Fabric weight", description="Heavy to light", default=0.3, min=0.0, max=1.0)
    response: FloatProperty(name="Response", description="Snappy to floaty", default=0.4, min=0.0, max=1.0)


class BCExportTarget(PropertyGroup):
    glb_path: StringProperty(name="Model", subtype='FILE_PATH')
    write_glb: BoolProperty(name="Model", default=True)
    write_skmm: BoolProperty(name="Settings", default=True)


class BCExport(PropertyGroup):
    targets: CollectionProperty(type=BCExportTarget)
    targets_index: IntProperty()
    deform_bones_only: BoolProperty(name="Export deform bones only", description="Drops helper bones from the model. "
                                    "Changes the exported skeleton", default=False)
    last_roots: StringProperty(default="")


class BCPreview(PropertyGroup):
    motion: EnumProperty(name="Test motion", items=[
        ('ANIMATION', "Animation only", "The armature is left alone (its own animation and root motion)"),
        ('STAND', "Stand still", "The character does not move: only the wind acts"),
        ('WALK', "Walk 1.5 m/s", ""), ('RUN', "Run 5 m/s", ""),
        ('CIRCLE', "Run in a circle", "5 m/s around a 3 m circle: turning all the time"),
        ('SPIN', "Spin 720 deg/s", "Standing, turning fast: the cloth should open"),
        ('STOP_GO', "Stop and go", "Run 1.5 s, stand 1.5 s, again"),
        ('STRAFE', "Strafe", "Sideways at 3 m/s, changing direction every second"),
        ('TELEPORT', "Teleport", "Stands, then jumps 5 m every 2 s: the cloth must snap, not stretch")], default='RUN')
    wind_speed: FloatProperty(name="Wind speed", description="m/s", default=0.0, min=0.0, max=40.0)
    wind_from: FloatProperty(name="Wind from", subtype='ANGLE', unit='ROTATION', description="Where the wind blows from, "
                             "seen from the character: 0 the front, 90 the left, 180 behind", default=0.0)
    gust: FloatProperty(name="Gusts", description="Share of the wind speed the gusts add and take away", default=0.3,
                        min=0.0, max=1.0)
    gust_hz: FloatProperty(name="Gust rate", description="Hz", default=0.4, min=0.0, max=5.0)
    debug_lines: BoolProperty(name="Engine debug lines", description="Draw the strips (yellow) and the leg capsules "
                              "(orange) the way cloth_debug does in the game", default=True)


class BCStage(PropertyGroup):
    name: StringProperty()
    input_hash: StringProperty()
    output_hash: StringProperty()
    note: StringProperty()
    edited: BoolProperty(default=False)


class BCSetup(PropertyGroup):
    id: StringProperty()
    name: StringProperty(name="Name", default="coat")
    prefix: StringProperty(name="Bone prefix", default="coat")
    preset: EnumProperty(name="Preset", items=[
        ('LONG_COAT', "Long coat", ""), ('JACKET', "Jacket", ""), ('SKIRT', "Skirt", ""), ('ROBE', "Robe", ""), ('CAPE', "Cape", ""), ('STRAND', "Strand", ""),
        ('CUSTOM', "Custom", "")], default='LONG_COAT')
    adopted: BoolProperty(default=False, description="The bones were made by hand and adopted: never rebuilt or deleted")
    preset_version: IntProperty(default=0)       # presets.VERSION when the preset was last applied
    cloth_objects: CollectionProperty(type=BCObjectRef)
    cloth_index: IntProperty()
    character: PointerProperty(type=BCCharacter)
    guides: PointerProperty(type=BCGuides)
    weights: PointerProperty(type=BCWeights)
    rest: PointerProperty(type=BCRest)
    poses: PointerProperty(type=BCPoses)
    colliders: CollectionProperty(type=BCCollider)
    colliders_index: IntProperty()
    motion: PointerProperty(type=BCMotion)
    export: PointerProperty(type=BCExport)
    preview: PointerProperty(type=BCPreview)
    stages: CollectionProperty(type=BCStage)
    generated_bones: StringProperty(default="")      # JSON list
    root_bones: StringProperty(default="")           # JSON list
    strips_json: StringProperty(default="[]")        # JSON list of {column, side, theta, bones}
    show_overlay: BoolProperty(name="Show guides", default=True, update=_redraw)
    show_help: BoolProperty(name="Show hints", default=True)


class BCArmature(PropertyGroup):
    setups: CollectionProperty(type=BCSetup)
    active: IntProperty(default=0)


CLASSES = (BCObjectRef, BCColumn, BCPin, BCCharacter, BCGuides, BCWeights, BCRest, BCPoseLayer, BCClipSlot, BCClipGen, BCPoses,
           BCCollider, BCMotion, BCExportTarget, BCExport, BCPreview, BCStage, BCSetup, BCArmature)


def register():
    for c in CLASSES:
        bpy.utils.register_class(c)
    bpy.types.Object.bone_cloth = PointerProperty(type=BCArmature)


def unregister():
    del bpy.types.Object.bone_cloth
    for c in reversed(CLASSES):
        bpy.utils.unregister_class(c)


# ---- helpers ------------------------------------------------------------------------------------------------------

def get_setup(obj):
    """The active setup of an armature object, or None."""
    if obj is None or obj.type != 'ARMATURE':
        return None
    data = obj.bone_cloth
    if not data.setups:
        return None
    return data.setups[min(data.active, len(data.setups) - 1)]


def new_setup(obj, name):
    data = obj.bone_cloth
    s = data.setups.add()
    s.id = uuid.uuid4().hex[:8]
    s.name = name
    s.prefix = name
    data.active = len(data.setups) - 1
    for key in STAGES:
        st = s.stages.add()
        st.name = key
    return s


def _plain(pg):
    out = {}
    for prop in pg.bl_rna.properties:
        pid = prop.identifier
        if pid in ('rna_type', 'name') and prop.type != 'STRING':
            continue
        if pid == 'rna_type':
            continue
        value = getattr(pg, pid)
        if prop.type == 'POINTER':
            if isinstance(value, bpy.types.ID):
                out[pid] = value.name
            elif value is not None:
                out[pid] = _plain(value)
        elif prop.type == 'COLLECTION':
            out[pid] = [_plain(v) for v in value]
        elif prop.type in ('FLOAT', 'INT', 'BOOLEAN') and getattr(prop, 'array_length', 0):
            out[pid] = [round(float(x), 5) for x in value]
        elif prop.type == 'FLOAT':
            out[pid] = round(float(value), 5)
        else:
            out[pid] = value if not hasattr(value, '__iter__') or isinstance(value, str) else sorted(value)
    return out


def settings_hash(pg):
    return json.dumps(_plain(pg), sort_keys=True, default=str)


def stage(setup, key):
    for st in setup.stages:
        if st.name == key:
            return st
    st = setup.stages.add()
    st.name = key
    return st
