"""The sidebar: one panel per step (DESIGN.md section 7)."""
import math

import bpy

from . import bones as bones_mod
from . import colliders, doctor, landmarks, mesh_prep, ops, poses as poses_mod, presets, props, stages

ICONS = {"ok": 'CHECKMARK', "stale": 'FILE_REFRESH', "none": 'RADIOBUT_OFF', "warn": 'ERROR'}
SEV_ICON = {"error": 'CANCEL', "warning": 'ERROR', "info": 'INFO'}


def _ctx(context):
    arm = ops.find_armature(context)
    setup = props.get_setup(arm) if arm else None
    return arm, setup


def _hint(layout, setup, text):
    if setup.show_help:
        col = layout.column(align=True)
        for line in text.split("\n"):
            col.label(text=line)


class _Sub(bpy.types.Panel):
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = "Bone Cloth"
    bl_options = {'DEFAULT_CLOSED'}
    stage_key = ""

    @classmethod
    def poll(cls, context):
        arm, setup = _ctx(context)
        return setup is not None

    def draw_header(self, context):
        arm, setup = _ctx(context)
        if setup and self.stage_key:
            self.layout.label(icon=ICONS[stages.status(setup, arm, self.stage_key)])


# ---- main ----------------------------------------------------------------------------------------------------------

def next_step(setup, arm):
    """The first thing to do, in plain words."""
    if presets.outdated(setup):
        return "Press 'Apply the new preset' above (your settings are from an older one)."
    if not setup.cloth_objects:
        return "Pick the cloth mesh in the Character step."
    ch = setup.character
    if not ch.hips or not ch.chest:
        return "Press 'Find body bones' in the Character step."
    for key, text in (
            ("mesh", "Press 'Fix all' in the Mesh step."),
            ("bones", "Open Shape & bones: press Measure, then Build bones."),
            ("weights", "Press Build weights."),
            ("rest", "Shape the rest pose, or press Apply to keep it as it is."),
            ("poses", "Press Update poses in Wind poses."),
            ("colliders", "Check the leg capsules.")):
        s = stages.status(setup, arm, key)
        if s in ("none", "stale", "warn") and not (key == "rest" and s == "none" and
                                                     stages.status(setup, arm, "weights") == "ok" and not setup.rest.flare
                                                     and not setup.rest.open and not setup.rest.clear_legs):
            return text if s != "stale" else "'%s' is out of date: press Update all." % stages.TITLES[key]
    return "Open Check & export."


class BONECLOTH_PT_main(bpy.types.Panel):
    bl_label = "Bone Cloth"
    bl_idname = "BONECLOTH_PT_main"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = "Bone Cloth"

    @classmethod
    def poll(cls, context):
        return ops.find_armature(context) is not None

    def draw(self, context):
        layout = self.layout
        arm, setup = _ctx(context)
        if setup is None:
            col = layout.column()
            col.label(text="Cloth for the game: coats, jackets, skirts.")
            col.operator("bone_cloth.create", text="Set up cloth", icon='ADD')
            return
        data = arm.bone_cloth
        row = layout.row(align=True)
        if len(data.setups) > 1:
            row.prop(data, "active", text="Setup")
        else:
            row.label(text=setup.name, icon='MOD_CLOTH')
        row.operator("bone_cloth.create", text="", icon='ADD')
        row.operator("bone_cloth.remove_setup", text="", icon='REMOVE')

        if presets.outdated(setup):
            box = layout.box()
            box.alert = True
            box.label(text="The preset was improved.", icon='ERROR')
            box.label(text="Your settings are from an older one.")
            box.operator("bone_cloth.preset", text="Apply the new %s preset" % setup.preset.replace("_", " ").title())
        stale = stages.stale_list(setup, arm)
        errors = [i for i in doctor.LAST.get(setup.id, []) if i.severity == "error"]
        box = layout.box()
        if errors:
            box.label(text="%d problem(s) before export" % len(errors), icon='ERROR')
        elif stale:
            box.label(text="%d step(s) out of date" % len(stale), icon='FILE_REFRESH')
        else:
            box.label(text="Up to date", icon='CHECKMARK')
        row = layout.row(align=True)
        row.scale_y = 1.4
        row.operator("bone_cloth.auto_setup", icon='PLAY')
        row.operator("bone_cloth.update_all", icon='FILE_REFRESH')
        if setup.show_help:
            layout.label(text="Next: " + next_step(setup, arm))
        row = layout.row(align=True)
        row.prop(setup, "show_overlay", text="Guides", toggle=True, icon='OVERLAY')
        row.prop(setup, "show_help", text="Hints", toggle=True, icon='HELP')
        layout.operator("bone_cloth.start_over", icon='TRASH')


# ---- 1 character ---------------------------------------------------------------------------------------------------

class BONECLOTH_PT_character(_Sub):
    bl_label = "1  Character"
    bl_idname = "BONECLOTH_PT_character"
    bl_parent_id = "BONECLOTH_PT_main"
    stage_key = "character"

    def draw(self, context):
        arm, setup = _ctx(context)
        layout, ch = self.layout, setup.character
        _hint(layout, setup, "Which mesh is the cloth, and which bones\nare the hips, chest and legs?")
        row = layout.row(align=True)
        row.prop(setup, "preset", text="")
        row.operator("bone_cloth.preset", text="Apply preset")
        layout.label(text="Cloth meshes")
        for i, r in enumerate(setup.cloth_objects):
            row = layout.row(align=True)
            row.label(text=r.obj.name if r.obj else "(missing)", icon='MESH_DATA')
            op = row.operator("bone_cloth.cloth_remove", text="", icon='X')
            op.index = i
        layout.operator("bone_cloth.cloth_add", icon='ADD')
        layout.separator()
        row = layout.row()
        row.label(text="Body bones" + (" (%s)" % ch.rig_family if ch.rig_family else ""))
        row.operator("bone_cloth.detect_character", text="Find", icon='VIEWZOOM')
        col = layout.column(align=True)
        for key in landmarks.KEYS:
            row = col.row(align=True)
            ok = bool(getattr(ch, key)) and getattr(ch, key) in arm.data.bones
            row.label(text="", icon='CHECKMARK' if ok else ('ERROR' if key in landmarks.REQUIRED else 'BLANK1'))
            row.prop_search(ch, key, arm.data, "bones", text=landmarks.LABELS[key])
        row = layout.row(align=True)
        row.prop(ch, "facing", text="Facing")
        row.prop(ch, "flip", toggle=True)
        layout.separator()
        if setup.adopted:
            layout.label(text="Adopted rig: its bones are not rebuilt", icon='LINKED')
        else:
            layout.operator("bone_cloth.adopt", icon='IMPORT')


# ---- 2 mesh --------------------------------------------------------------------------------------------------------

class BONECLOTH_PT_mesh(_Sub):
    bl_label = "2  Mesh"
    bl_idname = "BONECLOTH_PT_mesh"
    bl_parent_id = "BONECLOTH_PT_main"
    stage_key = "mesh"

    def draw(self, context):
        arm, setup = _ctx(context)
        layout = self.layout
        _hint(layout, setup, "Gets the mesh ready: one armature modifier,\nno leftover scale or rotation, an untouched copy.")
        items = mesh_prep.check(setup, arm)
        if not items:
            layout.label(text="Nothing to fix", icon='CHECKMARK')
        for ident, sev, msg, fixable in items:
            row = layout.row(align=True)
            row.label(text=msg, icon=SEV_ICON.get({'warn': 'warning', 'error': 'error'}.get(sev, 'info'), 'INFO'))
            if fixable:
                op = row.operator("bone_cloth.mesh_fix", text="Fix")
                op.item = ident
        layout.operator("bone_cloth.mesh_fix_all", icon='MODIFIER')


# ---- 3 shape & bones -----------------------------------------------------------------------------------------------

class BONECLOTH_PT_shape(_Sub):
    bl_label = "3  Shape & bones"
    bl_idname = "BONECLOTH_PT_shape"
    bl_parent_id = "BONECLOTH_PT_main"
    stage_key = "bones"

    def draw(self, context):
        arm, setup = _ctx(context)
        layout, g = self.layout, setup.guides
        _hint(layout, setup, "Where the panels hang: lines for the top,\nwaist and hem, and how many panels around.")
        layout.operator("bone_cloth.measure", icon='DRIVER_DISTANCE')
        col = layout.column(align=True)
        col.prop(g, "layout")
        if g.layout == 'STRAND':
            col.prop_search(g, "strand_anchor", arm.data, "bones")
        else:
            col.prop(g, "anchors")
        if g.layout == 'OPEN_FRONT':
            col.prop(g, "opening_half_angle")
        if g.anchors == 'SPINE':
            col.prop(g, "bone_inset", slider=True)
        col.prop(g, "columns_per_side")
        row = col.row(align=True)
        row.prop(g, "center_back", toggle=True)
        row.prop(g, "center_front", toggle=True)
        if g.anchors == 'CHEST_AND_HIPS':
            col.prop(g, "upper_part")
        col.prop(g, "rows", text="Hem bones" if g.anchors == 'SPINE' else "Rows below the waist")
        col.prop(g, "row_spacing", text="Spacing")
        layout.separator()
        lines = [("Top", "top_auto", "top_line"), ("Waist", "waist_auto", "waist_line"), ("Hem", "hem_auto", "hem_line")]
        if g.anchors == 'SPINE':
            lines = [l for l in lines if l[0] != "Waist"]
        for label, auto, val in lines:
            row = layout.row(align=True)
            row.prop(g, auto, text="")
            sub = row.row(align=True)
            sub.enabled = not getattr(g, auto)
            sub.prop(g, val, text=label)
        col = layout.column(align=True)
        col.prop(g, "placement", text="Place bones")
        col.prop(g, "surface_offset")
        col.prop(g, "path")
        box = layout.box()
        row = box.row()
        row.label(text="Panels")
        row.operator("bone_cloth.rebuild_columns", text="Lay out", icon='MOD_ARRAY')
        for c in g.columns:
            r = box.row(align=True)
            r.prop(c, "enabled", text="")
            r.label(text=c.name)
            r.prop(c, "angle", text="")
            r.prop(c, "top_offset", text="")
        if g.columns:
            box.label(text="name   angle (0 front, 90 left)   start")
        layout.operator("bone_cloth.build_bones", icon='BONE_DATA')
        n = len(bones_mod.generated(setup))
        if n:
            layout.label(text="%d bones built" % n, icon='CHECKMARK')
            layout.operator("bone_cloth.remove_bones", icon='TRASH')
        note = props.stage(setup, "bones").note
        if note:
            layout.label(text=note, icon='ERROR')


# ---- 4 weights -----------------------------------------------------------------------------------------------------

class BONECLOTH_PT_weights(_Sub):
    bl_label = "4  Weights"
    bl_idname = "BONECLOTH_PT_weights"
    bl_parent_id = "BONECLOTH_PT_main"
    stage_key = "weights"

    def draw(self, context):
        arm, setup = _ctx(context)
        layout, w = self.layout, setup.weights
        _hint(layout, setup, "Where does the cloth leave the body, and how\nsmoothly? Red = moves with the panels.")
        layout.prop(w, "profile", text="")
        row = layout.row(align=True)
        row.prop(w, "blend_auto", text="")
        sub = row.column(align=True)
        sub.enabled = not w.blend_auto
        sub.prop(w, "blend_start")
        sub.prop(w, "blend_full")
        if w.blend_auto:
            col = layout.column(align=True)
            col.prop(w, "start_from_top", text="Starts below the top")
            col.prop(w, "full_from_waist", text="Loose below the waist")
        node = None
        from . import weights as weights_mod
        node = weights_mod.curve_node(setup, create=False)
        if node is not None:
            layout.label(text="How the blend goes")
            layout.template_curve_mapping(node, "mapping", type='NONE', levels=False, brush=False, show_tone=False)
        box = layout.box()
        row = box.row()
        row.label(text="Belts (stay on the body)")
        row.operator("bone_cloth.detect_belt", text="Find")
        row.operator("bone_cloth.pin_add", text="", icon='ADD')
        for i, pin in enumerate(w.pins):
            r = box.row(align=True)
            r.prop(pin, "height", text="")
            r.prop(pin, "half_width", text="±")
            r.prop(pin, "softness", text="soft")
            op = r.operator("bone_cloth.pin_remove", text="", icon='X')
            op.index = i
        col = layout.column(align=True)
        col.prop(w, "exclude_arms")
        col.prop(w, "use_painted_amount")
        col.prop(w, "panel_sharpness", slider=True)
        col.prop(w, "joint_softness", slider=True)
        row = col.row(align=True)
        row.prop(w, "rigid_parts")
        sub = row.row()
        sub.enabled = w.rigid_parts
        sub.prop(w, "rigid_max_size", text="")
        row = col.row(align=True)
        row.prop(w, "smooth_iterations", text="Smooth passes")
        row.prop(w, "smooth_strength", text="")
        row = layout.row(align=True)
        op = row.operator("bone_cloth.amount_show", text="Show amount")
        op.paint = False
        op = row.operator("bone_cloth.amount_show", text="Paint amount")
        op.paint = True
        layout.operator("bone_cloth.build_weights", icon='GROUP_VERTEX')


# ---- 5 rest shape --------------------------------------------------------------------------------------------------

class BONECLOTH_PT_rest(_Sub):
    bl_label = "5  Rest shape"
    bl_idname = "BONECLOTH_PT_rest"
    bl_parent_id = "BONECLOTH_PT_main"
    stage_key = "rest"

    def draw(self, context):
        arm, setup = _ctx(context)
        layout, r = self.layout, setup.rest
        _hint(layout, setup, "How the cloth hangs when nothing moves.\nThe game always swings back to this shape.")
        col = layout.column(align=True)
        col.prop(r, "flare")
        col.prop(r, "open")
        col.prop(r, "bend", slider=True)
        col.prop(r, "clear_legs")
        if r.clear_legs:
            col.prop(r, "clearance")
        row = layout.row(align=True)
        row.operator("bone_cloth.rest_preview")
        row.operator("bone_cloth.rest_clear_preview", text="", icon='X')
        row = layout.row(align=True)
        row.operator("bone_cloth.rest_apply", icon='CHECKMARK')
        row.operator("bone_cloth.rest_reset", icon='LOOP_BACK')
        layout.operator("bone_cloth.pose_cloth", icon='POSE_HLT')
        if r.applied:
            layout.label(text="Applied", icon='CHECKMARK')
        note = props.stage(setup, "rest").note
        if note:
            layout.label(text="Extra flare: " + note)


# ---- 6 wind poses --------------------------------------------------------------------------------------------------

class BONECLOTH_PT_poses(_Sub):
    bl_label = "6  Wind poses"
    bl_idname = "BONECLOTH_PT_poses"
    bl_parent_id = "BONECLOTH_PT_main"
    stage_key = "poses"

    def draw(self, context):
        arm, setup = _ctx(context)
        layout = self.layout
        _hint(layout, setup, "Open: front panels swing out like doors.\nVolume: every panel flares. The game blends\nfrom the rest shape toward each pose.")
        for kind, title in (("open", "Open"), ("volume", "Volume")):
            layer = getattr(setup.poses, kind)
            box = layout.box()
            row = box.row()
            row.prop(layer, "enabled", text=title)
            if poses_mod.layer_is_hand(layer):
                row.label(text="hand made (kept)", icon='GREASEPENCIL')
            if not layer.enabled:
                continue
            col = box.column(align=True)
            col.prop(layer, "clip_name", text="Clip")
            sub = col.column(align=True)
            sub.enabled = not poses_mod.layer_is_hand(layer)
            sub.prop(layer, "amount")
            sub.prop(layer, "direction", slider=True, text="Sideways" if kind == "open" else "Lift")
            sub.prop(layer, "backward", slider=True)
            sub.prop(layer, "max_turn")
            sub.prop(layer, "slide")
            sub.prop(layer, "bend", slider=True)
            sub.prop(layer, "upper_share", slider=True)
            row = box.row(align=True)
            op = row.operator("bone_cloth.pose_capture", text="Save my pose")
            op.kind = kind
            op = row.operator("bone_cloth.pose_capture", text="R → L")
            op.kind, op.mirror = kind, 'R_TO_L'
            op = row.operator("bone_cloth.pose_capture", text="L → R")
            op.kind, op.mirror = kind, 'L_TO_R'
            if poses_mod.layer_is_hand(layer):
                op = box.operator("bone_cloth.pose_regenerate", text="Back to generated")
                op.kind = kind
        layout.operator("bone_cloth.poses_update", icon='ACTION')
        col = layout.column(align=True)
        col.label(text="Preview")
        col.prop(setup.poses, "preview_open", slider=True)
        col.prop(setup.poses, "preview_volume", slider=True)
        col.operator("bone_cloth.pose_preview_reset")
        layout.label(text="Columns: how much each panel takes part")
        box = layout.box()
        for c in setup.guides.columns:
            row = box.row(align=True)
            row.label(text=c.name)
            row.prop(c, "open_mult", text="Opens")
            row.prop(c, "volume_mult", text="Flares")


# ---- 7 collisions --------------------------------------------------------------------------------------------------

class BONECLOTH_PT_colliders(_Sub):
    bl_label = "7  Collisions"
    bl_idname = "BONECLOTH_PT_colliders"
    bl_parent_id = "BONECLOTH_PT_main"
    stage_key = "colliders"

    def draw(self, context):
        arm, setup = _ctx(context)
        layout = self.layout
        _hint(layout, setup, "Capsules the cloth cannot enter (the legs).\nAuto radii are measured like the game does.")
        row = layout.row(align=True)
        row.operator("bone_cloth.colliders_defaults", icon='MOD_PHYSICS')
        row.operator("bone_cloth.colliders_fit", icon='FILE_REFRESH')
        row.operator("bone_cloth.collider_add", text="", icon='ADD')
        for i, c in enumerate(setup.colliders):
            box = layout.box()
            row = box.row(align=True)
            row.prop(c, "enabled", text="")
            row.prop_search(c, "bone", arm.data, "bones", text="")
            row.prop_search(c, "to", arm.data, "bones", text="")
            op = row.operator("bone_cloth.collider_remove", text="", icon='X')
            op.index = i
            r = box.row(align=True)
            r.prop(c, "auto_radius", text="Auto")
            if c.auto_radius:
                r.label(text="%.1f cm" % (c.fitted_radius * 100))
            else:
                r.prop(c, "radius", text="")
            r.prop(c, "auto_end", text="Auto")
            if c.auto_end:
                r.label(text="%.1f cm" % (c.fitted_end * 100))
            else:
                r.prop(c, "end_radius", text="")
        layout.prop(setup.motion, "radius", text="Padding")


# ---- 8 motion ------------------------------------------------------------------------------------------------------

class BONECLOTH_PT_motion(_Sub):
    bl_label = "8  Motion"
    bl_idname = "BONECLOTH_PT_motion"
    bl_parent_id = "BONECLOTH_PT_main"
    stage_key = "motion"

    def draw(self, context):
        arm, setup = _ctx(context)
        layout, m = self.layout, setup.motion
        _hint(layout, setup, "How the cloth swings. Pick a feel, or tune\nthe sliders. Everything is saved in the .skmm file.")
        row = layout.row(align=True)
        row.prop(m, "feel", text="")
        row.operator("bone_cloth.feel", text="Apply")
        col = layout.column(align=True)
        col.prop(m, "weight", slider=True)
        col.prop(m, "response", slider=True)
        col = layout.column(align=True)
        col.label(text="Wind")
        col.prop(m, "openWindStart")
        col.prop(m, "openWindFull")
        col.prop(m, "volumeWindStart")
        col.prop(m, "volumeWindFull")
        col.prop(m, "volume", slider=True)
        col = layout.column(align=True)
        col.label(text="Spinning")
        col.prop(m, "spinOpen", slider=True)
        col.prop(m, "spinVolume", slider=True)
        col.prop(m, "spinStart")
        col.prop(m, "spinFull")
        col.prop(m, "spinRelease")
        box = layout.box()
        box.label(text="Advanced (engine names in tooltips)")
        col = box.column(align=True)
        for k in ("followRoot", "followTip", "trail", "maxAngleRoot", "maxAngleTip", "layerSmoothing", "windwardBias",
                  "teleportDistance", "tipLength"):
            col.prop(m, k)
        col.prop(m, "forward_override")
        if m.forward_override:
            col.prop(m, "forward")


# ---- 9 preview -----------------------------------------------------------------------------------------------------

class BONECLOTH_PT_preview(_Sub):
    bl_label = "9  Preview"
    bl_idname = "BONECLOTH_PT_preview"
    bl_parent_id = "BONECLOTH_PT_main"

    def draw(self, context):
        from . import sim
        arm, setup = _ctx(context)
        layout, pv = self.layout, setup.preview
        _hint(layout, setup, "Plays the engine's cloth swing in the viewport:\nrun, spin, stop and go, with wind.")
        running = arm.name in sim.RUNNING
        row = layout.row(align=True)
        row.scale_y = 1.3
        if running:
            row.operator("bone_cloth.sim_stop", icon='PAUSE')
            row.operator("bone_cloth.sim_reset", icon='LOOP_BACK')
        else:
            row.operator("bone_cloth.sim_play", icon='PLAY')
        col = layout.column(align=True)
        col.prop(pv, "motion", text="")
        col.prop(pv, "wind_speed")
        sub = col.column(align=True)
        sub.enabled = pv.wind_speed > 0
        sub.prop(pv, "wind_from")
        sub.prop(pv, "gust", slider=True)
        sub.prop(pv, "gust_hz")
        layout.prop(pv, "debug_lines")
        if setup.show_help:
            layout.label(text="Tune in Motion (feel, wind ranges, spin), then Play again.")


# ---- 10 check & export ---------------------------------------------------------------------------------------------

class BONECLOTH_PT_export(_Sub):
    bl_label = "Check & export"
    bl_idname = "BONECLOTH_PT_export"
    bl_parent_id = "BONECLOTH_PT_main"

    def draw_header(self, context):
        arm, setup = _ctx(context)
        issues = doctor.LAST.get(setup.id)
        if issues is None:
            self.layout.label(icon='RADIOBUT_OFF')
        else:
            self.layout.label(icon='CANCEL' if doctor.blocking(issues) else ('ERROR' if issues else 'CHECKMARK'))

    def draw(self, context):
        arm, setup = _ctx(context)
        layout = self.layout
        _hint(layout, setup, "Checks the rig the way the game reads it, then\nwrites the .glb model and the .skmm settings.")
        layout.operator("bone_cloth.check", icon='VIEWZOOM')
        for i in doctor.LAST.get(setup.id, []):
            row = layout.row(align=True)
            row.label(text=i.message[:90], icon=SEV_ICON[i.severity])
            if i.fix:
                op = row.operator("bone_cloth.fix_issue", text="Fix")
                op.key = i.fix
        e = setup.export
        box = layout.box()
        row = box.row()
        row.label(text="Targets")
        row.operator("bone_cloth.target_add", text="", icon='ADD')
        for i, t in enumerate(e.targets):
            r = box.row(align=True)
            r.prop(t, "write_glb", text="glb")
            r.prop(t, "write_skmm", text="skmm")
            r.prop(t, "glb_path", text="")
            op = r.operator("bone_cloth.target_remove", text="", icon='X')
            op.index = i
        box.prop(e, "deform_bones_only")
        row = layout.row(align=True)
        row.scale_y = 1.3
        op = row.operator("bone_cloth.export", text="Export model + settings", icon='EXPORT')
        op.settings_only = False
        op = row.operator("bone_cloth.export", text="Settings only")
        op.settings_only = True
        if setup.show_help:
            layout.label(text="In game: cloth_reload re-reads settings, cloth_debug 1 draws strips")


CLASSES = (BONECLOTH_PT_main, BONECLOTH_PT_character, BONECLOTH_PT_mesh, BONECLOTH_PT_shape, BONECLOTH_PT_weights,
           BONECLOTH_PT_rest, BONECLOTH_PT_poses, BONECLOTH_PT_colliders, BONECLOTH_PT_motion, BONECLOTH_PT_preview, BONECLOTH_PT_export)
