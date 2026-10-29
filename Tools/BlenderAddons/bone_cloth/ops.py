"""Operators. Every button in the panel is one undo step."""
import json

import bpy
from bpy.props import BoolProperty, EnumProperty, IntProperty, StringProperty

from . import bones as bones_mod
from . import (colliders, doctor, export, guides, mesh_prep, overlay, pipeline, poses, presets, props, rest_shape,
               stages, weights)


def find_armature(context):
    ob = context.active_object
    if ob is None:
        return None
    if ob.type == 'ARMATURE':
        return ob
    if ob.type == 'MESH':
        for m in ob.modifiers:
            if m.type == 'ARMATURE' and m.object:
                return m.object
        if ob.parent and ob.parent.type == 'ARMATURE':
            return ob.parent
    return None


class _Op(bpy.types.Operator):
    bl_options = {'REGISTER', 'UNDO'}
    needs_setup = True

    @classmethod
    def poll(cls, context):
        arm = find_armature(context)
        if arm is None:
            return False
        return props.get_setup(arm) is not None if cls.needs_setup else True

    def execute(self, context):
        arm = find_armature(context)
        setup = props.get_setup(arm)
        try:
            msg = self.run(context, arm, setup)
        except Exception as e:      # shown to the user in plain words; the details go to the console
            import traceback
            traceback.print_exc()
            self.report({'ERROR'}, str(e))
            return {'CANCELLED'}
        if msg:
            self.report({'INFO'}, msg)
        overlay.tag_dirty()
        return {'FINISHED'}


# ---- setup ---------------------------------------------------------------------------------------------------------

_ITEMS = []      # enum items must stay referenced


def _mesh_items(self, context):
    items = []
    for o in bpy.context.scene.objects:
        if o.type == 'MESH' and "bone_cloth_source_of" not in o:
            hint = any(w in o.name.lower() for w in ("coat", "jacket", "skirt", "cloak", "robe", "dress", "cape"))
            items.append((o.name, o.name + ("  (looks like cloth)" if hint else ""), ""))
    items.sort(key=lambda i: 0 if "looks like" in i[1] else 1)
    _ITEMS[:] = items or [("", "(no meshes)", "")]
    return _ITEMS


class BONECLOTH_OT_create(_Op):
    bl_idname = "bone_cloth.create"
    bl_label = "Set up cloth"
    bl_description = "Starts a cloth setup for this character: finds the body bones and prepares the steps"
    needs_setup = False
    name: StringProperty(name="Name", default="coat")
    preset: EnumProperty(name="What is it?", items=[
        ('LONG_COAT', "Long coat", "Open front, chest and skirt parts"), ('JACKET', "Jacket", "Short, open front"),
        ('SKIRT', "Skirt", "Closed ring from the hips"), ('ROBE', "Robe", "Closed ring from the chest")],
        default='LONG_COAT')
    mesh: EnumProperty(name="Which mesh is the cloth?", items=_mesh_items)

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self, width=360)

    def run(self, context, arm, setup):
        s = props.new_setup(arm, self.name or "coat")
        presets.apply(s, self.preset)
        if self.mesh and self.mesh in bpy.data.objects:
            s.cloth_objects.add().obj = bpy.data.objects[self.mesh]
        missing = pipeline.detect_character(s, arm)
        s.motion.feel = 'CUSTOM'
        if missing:
            return "Pick the bones that were not found in the Character step: " + ", ".join(missing)
        return "Ready. Press Auto Setup, or go through the steps."


class BONECLOTH_OT_remove_setup(_Op):
    bl_idname = "bone_cloth.remove_setup"
    bl_label = "Remove setup"
    bl_description = "Forgets this setup. Bones, weights and clips stay as they are"

    def run(self, context, arm, setup):
        arm.bone_cloth.setups.remove(arm.bone_cloth.active)
        arm.bone_cloth.active = max(0, arm.bone_cloth.active - 1)


class BONECLOTH_OT_start_over(_Op):
    bl_idname = "bone_cloth.start_over"
    bl_label = "Start over"
    bl_description = "Deletes the bones, clips and weights this setup made and puts the mesh back as it was"

    def invoke(self, context, event):
        return context.window_manager.invoke_confirm(self, event)

    def run(self, context, arm, setup):
        if setup.rest.applied:
            rest_shape.reset(setup, arm)
        bones_mod.remove_generated(setup, arm)
        for o in mesh_prep.cloth_objects(setup):
            mesh_prep.restore_from_source(o)
            mesh_prep.restore_weights_from_source(o)
        for layer in (setup.poses.open, setup.poses.volume):
            if layer.action:
                layer.action.use_fake_user = False
                bpy.data.actions.remove(layer.action)
            layer.mode = 'GENERATED'
        for st in setup.stages:
            st.input_hash = st.output_hash = ""
        setup.rest.applied = False
        return "Started over. The original mesh is back."


class BONECLOTH_OT_detect_character(_Op):
    bl_idname = "bone_cloth.detect_character"
    bl_label = "Find body bones"
    bl_description = "Looks for the hips, chest, legs and shoulders in this armature"

    def run(self, context, arm, setup):
        missing = pipeline.detect_character(setup, arm)
        return "Not found: " + ", ".join(missing) if missing else "All body bones found"


class BONECLOTH_OT_cloth_add(_Op):
    bl_idname = "bone_cloth.cloth_add"
    bl_label = "Add selected meshes"
    bl_description = "Uses the selected meshes as the cloth (outer shell, lining, buttons...)"

    def run(self, context, arm, setup):
        have = {r.obj for r in setup.cloth_objects}
        n = 0
        for o in context.selected_objects:
            if o.type == 'MESH' and o not in have and "bone_cloth_source_of" not in o:
                setup.cloth_objects.add().obj = o
                n += 1
        return "Added %d mesh(es)" % n if n else "Select the cloth mesh first (Ctrl+click it in the outliner)"


class BONECLOTH_OT_cloth_remove(_Op):
    bl_idname = "bone_cloth.cloth_remove"
    bl_label = "Remove"
    index: IntProperty()

    def run(self, context, arm, setup):
        setup.cloth_objects.remove(self.index)


# ---- mesh ----------------------------------------------------------------------------------------------------------

class BONECLOTH_OT_mesh_fix_all(_Op):
    bl_idname = "bone_cloth.mesh_fix_all"
    bl_label = "Fix all"
    bl_description = "Bakes odd transforms, sets up the armature modifier, tidies weights and keeps an untouched copy"

    def run(self, context, arm, setup):
        done = pipeline.fix_mesh(setup, arm)
        return "Fixed: " + ", ".join(d.split(":")[0] for d in done) if done else "Nothing to fix"


class BONECLOTH_OT_mesh_fix(_Op):
    bl_idname = "bone_cloth.mesh_fix"
    bl_label = "Fix"
    item: StringProperty()

    def run(self, context, arm, setup):
        mesh_prep.fix(setup, arm, self.item)
        stages.mark_done(setup, "mesh")


# ---- shape & bones -------------------------------------------------------------------------------------------------

class BONECLOTH_OT_measure(_Op):
    bl_idname = "bone_cloth.measure"
    bl_label = "Measure the cloth"
    bl_description = "Reads the mesh: finds the hem, the front opening and where each panel starts"

    def run(self, context, arm, setup):
        m, geom = pipeline.measure(setup, arm)
        return "Hem %.2f m, waist %.2f m, opening %.0f degrees" % (m['hem'], m['waist'], m['opening'] * 57.3)


class BONECLOTH_OT_rebuild_columns(_Op):
    bl_idname = "bone_cloth.rebuild_columns"
    bl_label = "Lay out panels"
    bl_description = "Spreads the panels around the body from the settings above"

    def run(self, context, arm, setup):
        frame = pipeline.frame_of(setup, arm)
        geom = guides.ClothGeom(setup, arm)
        guides.rebuild_columns(setup)
        guides.measure_column_tops(setup, frame, geom)


class BONECLOTH_OT_detect_belt(_Op):
    bl_idname = "bone_cloth.detect_belt"
    bl_label = "Find belt"
    bl_description = "Looks for a belt band near the waist and keeps it on the body"

    def run(self, context, arm, setup):
        belt = pipeline.add_belt_pin(setup, arm)
        return "Belt at %.2f m" % belt[0] if belt else "No belt found"


class BONECLOTH_OT_build_bones(_Op):
    bl_idname = "bone_cloth.build_bones"
    bl_label = "Build bones"
    bl_description = "Creates the root and panel bones from the layout. Replaces bones made earlier by this setup"

    def run(self, context, arm, setup):
        plan = pipeline.build_bones(setup, arm)
        return "%d panels built" % len(plan.strips) + (" (skipped: %s)" % "; ".join(plan.skipped) if plan.skipped else "")


class BONECLOTH_OT_remove_bones(_Op):
    bl_idname = "bone_cloth.remove_bones"
    bl_label = "Remove bones"

    def run(self, context, arm, setup):
        bones_mod.remove_generated(setup, arm)
        props.stage(setup, "bones").input_hash = ""


class BONECLOTH_OT_pin_add(_Op):
    bl_idname = "bone_cloth.pin_add"
    bl_label = "Add belt"

    def run(self, context, arm, setup):
        p = setup.weights.pins.add()
        p.height = setup.guides.waist_line + 0.05


class BONECLOTH_OT_pin_remove(_Op):
    bl_idname = "bone_cloth.pin_remove"
    bl_label = "Remove"
    index: IntProperty()

    def run(self, context, arm, setup):
        setup.weights.pins.remove(self.index)


# ---- weights -------------------------------------------------------------------------------------------------------

class BONECLOTH_OT_build_weights(_Op):
    bl_idname = "bone_cloth.build_weights"
    bl_label = "Build weights"
    bl_description = "Works out every vertex weight from the settings above"

    def run(self, context, arm, setup):
        stats = pipeline.build_weights(setup, arm)
        return "%d vertices: %d move with the panels, %d blend between panels" % (
            stats.get('vertices', 0), stats.get('moving', 0), stats.get('blended', 0))


class BONECLOTH_OT_amount_show(_Op):
    bl_idname = "bone_cloth.amount_show"
    bl_label = "Show cloth amount"
    bl_description = "Shows how much each vertex moves with the panels (red) or stays on the body (blue)"
    paint: BoolProperty(default=False)

    def run(self, context, arm, setup):
        objs = mesh_prep.cloth_objects(setup)
        if not objs:
            raise RuntimeError("Pick the cloth mesh first")
        obj = objs[0]
        presets.blend_heights(setup)
        weights.write_amount_group(setup, arm, pipeline.frame_of(setup, arm), obj)
        obj.vertex_groups.active = obj.vertex_groups[weights.AMOUNT_GROUP]
        if self.paint:
            setup.weights.use_painted_amount = True
        for o in context.view_layer.objects:
            o.select_set(False)
        context.view_layer.objects.active = obj
        obj.select_set(True)
        bpy.ops.object.mode_set(mode='WEIGHT_PAINT')
        return "Paint the group, then press Build weights" if self.paint else "Showing the cloth amount"


# ---- rest shape ----------------------------------------------------------------------------------------------------

class BONECLOTH_OT_rest_preview(_Op):
    bl_idname = "bone_cloth.rest_preview"
    bl_label = "Preview"

    def run(self, context, arm, setup):
        report = rest_shape.preview(setup, arm, pipeline.frame_of(setup, arm))
        return "Extra flare to clear the legs: " + ", ".join(report) if report else "Previewing the rest shape"


class BONECLOTH_OT_rest_clear_preview(_Op):
    bl_idname = "bone_cloth.rest_clear_preview"
    bl_label = "Stop preview"

    def run(self, context, arm, setup):
        rest_shape.clear_preview(arm)


class BONECLOTH_OT_rest_apply(_Op):
    bl_idname = "bone_cloth.rest_apply"
    bl_label = "Apply"
    bl_description = "Makes this the new rest shape: the bones and the mesh are bent into it"

    def run(self, context, arm, setup):
        rest_shape.clear_preview(arm)
        report = pipeline.apply_rest(setup, arm)
        colliders.fit_all(setup, arm)
        return "Applied. Extra flare: " + ", ".join(report) if report else "Applied"


class BONECLOTH_OT_rest_reset(_Op):
    bl_idname = "bone_cloth.rest_reset"
    bl_label = "Reset to original"

    def run(self, context, arm, setup):
        rest_shape.clear_preview(arm)
        pipeline.reset_rest(setup, arm)
        return "Back to the bones as built"


class BONECLOTH_OT_pose_cloth(_Op):
    bl_idname = "bone_cloth.pose_cloth"
    bl_label = "Edit by hand"
    bl_description = "Pose mode with only the cloth bones selected and X mirror on"

    def run(self, context, arm, setup):
        for o in context.view_layer.objects:
            o.select_set(False)
        context.view_layer.objects.active = arm
        arm.select_set(True)
        bpy.ops.object.mode_set(mode='POSE')
        arm.pose.use_mirror_x = True
        generated = set(bones_mod.generated(setup))
        for pb in arm.pose.bones:
            pb.select = pb.name in generated      # selection lives on the pose bones in Blender 5
        return "Rotate the cloth bones, then use the buttons in the panel"


# ---- wind poses ----------------------------------------------------------------------------------------------------

class BONECLOTH_OT_poses_update(_Op):
    bl_idname = "bone_cloth.poses_update"
    bl_label = "Update poses"
    bl_description = "Writes the Open and Volume clips from the settings above"

    def run(self, context, arm, setup):
        poses.set_bases(arm, {})
        kept = pipeline.update_poses(setup, arm)
        setup.poses.preview_open = setup.poses.preview_volume = 0.0
        return "Clips written" + (" (kept your hand-made %s pose)" % " and ".join(kept) if kept else "")


class BONECLOTH_OT_pose_capture(_Op):
    bl_idname = "bone_cloth.pose_capture"
    bl_label = "Save current pose"
    bl_description = ("Saves how the cloth bones are posed right now as this wind pose, including the top bones' slide. "
                      "With cloth bones selected only those are saved. Update never overwrites it")
    kind: StringProperty()
    mirror: EnumProperty(name="Mirror", items=[
        ('NONE', "As it is", ""), ('R_TO_L', "Right to left", "Mirror the right panels onto the left first"),
        ('L_TO_R', "Left to right", "Mirror the left panels onto the right first")], default='NONE')

    def run(self, context, arm, setup):
        if self.mirror != 'NONE':
            poses.mirror_pose(setup, arm, 'R' if self.mirror == 'R_TO_L' else 'L')
        poses.capture_current(setup, arm, self.kind)
        stages.mark_done(setup, "poses")
        return "Saved as the %s pose (hand made)%s" % (self.kind, ", mirrored" if self.mirror != 'NONE' else "")


class BONECLOTH_OT_pose_mirror(_Op):
    bl_idname = "bone_cloth.pose_mirror"
    bl_label = "Mirror pose"
    bl_description = "Copies the current pose of one side's panels onto the other side, mirrored (Pose mode)"
    source: EnumProperty(name="From", items=[('R', "Right", ""), ('L', "Left", "")], default='R')

    def run(self, context, arm, setup):
        names = poses.mirror_pose(setup, arm, self.source)
        return "Mirrored %d bones from the %s side" % (len(names), "right" if self.source == 'R' else "left")


class BONECLOTH_OT_pose_regenerate(_Op):
    bl_idname = "bone_cloth.pose_regenerate"
    bl_label = "Back to generated"
    bl_description = "Throws away the hand-made pose and generates it again from the settings"
    kind: StringProperty()

    def invoke(self, context, event):
        return context.window_manager.invoke_confirm(self, event)

    def run(self, context, arm, setup):
        poses.set_bases(arm, {})
        pipeline.update_poses(setup, arm, force=(self.kind,))
        return "The %s pose is generated again" % self.kind


class BONECLOTH_OT_pose_preview_reset(_Op):
    bl_idname = "bone_cloth.pose_preview_reset"
    bl_label = "Reset preview"

    def run(self, context, arm, setup):
        setup.poses.preview_open = setup.poses.preview_volume = 0.0
        poses.set_bases(arm, {})


# ---- colliders -----------------------------------------------------------------------------------------------------

class BONECLOTH_OT_colliders_defaults(_Op):
    bl_idname = "bone_cloth.colliders_defaults"
    bl_label = "Legs"
    bl_description = "Capsules on both thighs and calves"

    def run(self, context, arm, setup):
        pipeline.setup_colliders(setup, arm, defaults=True)


class BONECLOTH_OT_colliders_fit(_Op):
    bl_idname = "bone_cloth.colliders_fit"
    bl_label = "Refit"
    bl_description = "Measures the radii from the mesh, the way the game does"

    def run(self, context, arm, setup):
        pipeline.setup_colliders(setup, arm, defaults=False)


class BONECLOTH_OT_collider_add(_Op):
    bl_idname = "bone_cloth.collider_add"
    bl_label = "Add collider"

    def run(self, context, arm, setup):
        c = setup.colliders.add()
        pb = context.active_pose_bone
        c.bone = pb.name if pb else ""
        c.to = pb.children[0].name if pb and pb.children else ""


class BONECLOTH_OT_collider_remove(_Op):
    bl_idname = "bone_cloth.collider_remove"
    bl_label = "Remove"
    index: IntProperty()

    def run(self, context, arm, setup):
        setup.colliders.remove(self.index)


# ---- motion --------------------------------------------------------------------------------------------------------

class BONECLOTH_OT_feel(_Op):
    bl_idname = "bone_cloth.feel"
    bl_label = "Apply feel"
    bl_description = "Sets stiffness, swing limit and lag from the fabric sliders"

    def run(self, context, arm, setup):
        presets.apply_feel(setup.motion)


class BONECLOTH_OT_preset(_Op):
    bl_idname = "bone_cloth.preset"
    bl_label = "Apply preset"
    bl_description = "Replaces the settings with the starting values of this kind of cloth. Your meshes and bones stay"

    def invoke(self, context, event):
        return context.window_manager.invoke_confirm(self, event)

    def run(self, context, arm, setup):
        presets.apply(setup, setup.preset)
        return "Settings replaced. Press Update all."


# ---- check & export ------------------------------------------------------------------------------------------------

class BONECLOTH_OT_check(_Op):
    bl_idname = "bone_cloth.check"
    bl_label = "Check"
    bl_description = "Applies the game's own rules to the rig and lists what it would do"

    def run(self, context, arm, setup):
        issues = doctor.run(setup, arm)
        errors = len(doctor.blocking(issues))
        return "%d problems to fix, %d notes" % (errors, len(issues) - errors) if issues else "All good"


class BONECLOTH_OT_fix_issue(_Op):
    bl_idname = "bone_cloth.fix_issue"
    bl_label = "Fix"
    key: StringProperty()

    def run(self, context, arm, setup):
        msg = doctor.fix(setup, arm, self.key)
        doctor.run(setup, arm)
        return msg


class BONECLOTH_OT_update_all(_Op):
    bl_idname = "bone_cloth.update_all"
    bl_label = "Update all"
    bl_description = "Re-runs the steps that are out of date, in order"

    def run(self, context, arm, setup):
        ran = pipeline.update_all(setup, arm)
        return "Updated: " + ", ".join(stages.TITLES[k] for k in ran) if ran else "Everything is up to date"


class BONECLOTH_OT_auto_setup(_Op):
    bl_idname = "bone_cloth.auto_setup"
    bl_label = "Auto Setup"
    bl_description = "Runs every step with this kind of cloth's defaults. Each step can be tuned afterwards"

    def run(self, context, arm, setup):
        issues = pipeline.auto_setup(setup, arm, log=lambda m: print("[Bone Cloth]", m))
        errors = len(doctor.blocking(issues))
        return "Done. %d problem(s) to look at in Check & export" % errors if errors else "Done. Ready to export."


class BONECLOTH_OT_target_add(_Op):
    bl_idname = "bone_cloth.target_add"
    bl_label = "Add target"
    bl_description = "A .glb file the game loads. Its .skmm settings file is written beside it"
    filepath: StringProperty(subtype='FILE_PATH')
    filter_glob: StringProperty(default="*.glb", options={'HIDDEN'})

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}

    def run(self, context, arm, setup):
        t = setup.export.targets.add()
        t.glb_path = self.filepath if self.filepath.lower().endswith(".glb") else self.filepath + ".glb"


class BONECLOTH_OT_target_remove(_Op):
    bl_idname = "bone_cloth.target_remove"
    bl_label = "Remove"
    index: IntProperty()

    def run(self, context, arm, setup):
        setup.export.targets.remove(self.index)


class BONECLOTH_OT_export(_Op):
    bl_idname = "bone_cloth.export"
    bl_label = "Export"
    settings_only: BoolProperty(default=False)

    def run(self, context, arm, setup):
        msgs, issues = export.run(setup, arm, self.settings_only)
        for m in msgs:
            print("[Bone Cloth]", m)
        if msgs and msgs[0].startswith("Export stopped"):
            raise RuntimeError(msgs[0])
        problems = [m for m in msgs if m.startswith("Problem:")]
        if problems:
            self.report({'WARNING'}, "; ".join(problems))
        return msgs[-1]


class BONECLOTH_OT_adopt(_Op):
    bl_idname = "bone_cloth.adopt"
    bl_label = "Adopt an existing rig"
    bl_description = ("Takes over cloth bones that were made by hand: the strips are found the way the game finds them, the "
                      "settings, colliders and clip names come from the model's .skmm. Nothing is rebuilt or deleted")
    roots: StringProperty(name="Root bones", description="The bones the strips hang from, separated by commas "
                          "(empty: read them from the .skmm)", default="")
    skmm_path: StringProperty(name=".skmm file", subtype='FILE_PATH', default="")

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self, width=420)

    def run(self, context, arm, setup):
        from . import adopt
        entry = adopt.read_skmm(bpy.path.abspath(self.skmm_path)) if self.skmm_path else None
        roots = [r for r in self.roots.split(",") if r.strip()] or (entry["roots"] if entry else [])
        log = adopt.adopt(setup, arm, roots, entry)
        for line in log:
            print("[Bone Cloth]", line)
        return "Adopted: " + log[0]


class BONECLOTH_OT_sim_play(_Op):
    bl_idname = "bone_cloth.sim_play"
    bl_label = "Play with cloth"
    bl_description = ("Plays the timeline with the engine's cloth swing running on the cloth bones, with the test motion "
                      "and wind chosen below. Nothing is keyed; Stop puts the pose back")

    def run(self, context, arm, setup):
        from . import sim
        if setup.rest.applied is False and not bones_mod.generated(setup):
            raise RuntimeError("Build the bones first")
        sim.start(setup, arm, context.scene)
        if not context.screen.is_animation_playing:
            bpy.ops.screen.animation_play()
        return "Playing: %s" % setup.preview.motion.replace("_", " ").lower()


class BONECLOTH_OT_sim_stop(_Op):
    bl_idname = "bone_cloth.sim_stop"
    bl_label = "Stop"

    def run(self, context, arm, setup):
        from . import sim
        sim.stop(arm)
        if context.screen.is_animation_playing:
            bpy.ops.screen.animation_play()


class BONECLOTH_OT_sim_reset(_Op):
    bl_idname = "bone_cloth.sim_reset"
    bl_label = "Reset cloth"
    bl_description = "The strips snap back to the body's rest shape on the next frame"

    def run(self, context, arm, setup):
        from . import sim
        if arm.name in sim.RUNNING:
            sim.RUNNING[arm.name].reset()


class BONECLOTH_OT_toggle_overlay(_Op):
    bl_idname = "bone_cloth.toggle_overlay"
    bl_label = "Show guides"

    def run(self, context, arm, setup):
        setup.show_overlay = not setup.show_overlay


CLASSES = tuple(v for k, v in list(globals().items()) if k.startswith("BONECLOTH_OT_"))
