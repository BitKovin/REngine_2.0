# Bone Cloth (Blender add-on)

Sets up coats, jackets, skirts and robes for the engine's bone cloth (`source/Engine/Cloth/`): the panel bones, the weights,
the rest shape, the two wind poses, the leg capsules and the `.skmm` settings. Blender 5.1 or newer.
The design is in [DESIGN.md](DESIGN.md) (section 15 says what is built and where it differs); this file is how to use it.

## Install

1. Build the package (once, and after every change to the add-on):

   ```
   blender --command extension build --source-dir Tools/BlenderAddons/bone_cloth --output-dir <anywhere>
   ```

2. Blender: Edit > Preferences > Get Extensions > the arrow at the top right > **Install from Disk**, pick
   `bone_cloth-0.1.0.zip`, and enable it.

## Use it in ten minutes

Open the character's blend file, select the **armature** (or the cloth mesh), press **N** in the 3D viewport and open the
**Bone Cloth** tab.

1. Press **Set up cloth**. Pick what it is (Long coat, Jacket, Skirt, Robe) and which mesh is the cloth. The body bones are
   found for you.
2. Press **Auto Setup**. That runs every step with the preset's defaults. In the end the **Check & export** list tells you
   what is left.
3. Look at it. The blue, yellow and red rings in the viewport are the top / waist / hem lines. Use the sliders under **Wind poses >
   Preview** to see the Open and Volume poses.
4. Tune any step (each one has a short hint at the top, switch hints off with the **Hints** button), then press
   **Update all**. Only what is out of date is redone.
5. In **Check & export**, add a target (the `.glb` the game loads), press **Check**, fix what is listed (most items have a
   **Fix** button), then **Export model + settings**.
6. In the game: `cloth_reload` re-reads settings, `cloth_debug 1` draws the strips (yellow) and capsules (orange).
   **Export > Settings only** is the quick loop when you only tune the feel.

### What each step does

| Step | In one line |
|------|-------------|
| 1 Character | Which mesh is the cloth; finds hips, chest, legs, shoulders and which way the character faces |
| 2 Mesh | Fixes odd scale/rotation, the armature modifier, stray weights; keeps an untouched copy (`<name>.bc_source`, hidden) |
| 3 Shape & bones | Lines for top, waist, hem; how many panels around the body; **Build bones**. Anchors: chest + hips (coats), hips (skirts), **Follow the spine** (short jackets) |
| 4 Weights | Where the cloth leaves the body (two heights and a curve), belts that stay put, smooth blending between panels |
| 5 Rest shape | How the cloth hangs when nothing moves: a little flare, open front, kept clear of the legs |
| 6 Wind poses | Open (front swings out) and Volume (everything flares) as "cm at the hem" per panel |
| 7 Collisions | Leg capsules, radii measured the way the game measures them |
| 8 Motion | Fabric weight and response sliders, wind and spin ranges; every engine field under Advanced |
| 9 Preview | Plays the engine's cloth swing in the viewport: run, circle, spin, stop and go, strafe, teleport, with wind |
| Check & export | The game's rules applied to your rig, then the `.glb` and `.skmm`; the written model is read back like the game does and compared with the plan |

### Kinds of cloth

Pick one when you press **Set up cloth** (or later in the Character step's preset):
**Long coat** (chest part + skirt), **Jacket** (follows the spine, so it bends with it), **Skirt** and **Robe**
(closed ring), **Cape** (back only, hangs from the shoulders) and **Strand** (a tail, scarf end or flap: pick the bone
it hangs from in the Shape step).

### Wind poses are yours to author

Open and Volume are generated from the sliders, but a jacket's opening is better made by hand: pose the cloth bones in Pose
mode (the slide of the top bones is part of the pose), press **Save my pose** (or **R → L** / **L → R** to mirror one
side onto the other first). A hand-made pose is never overwritten by Update; **Back to generated** throws it away.

### Wind and turn clips

Instead of Open and Volume, the cloth can be posed by six clips: wind from the front, left, behind and right, and turning
left and right. Each clip starts at rest on frame 1; the later frames are what the cloth goes through as that wind (or the
turn) grows - a jacket in a headwind can open on frame 2 and blow back on frame 3. The game plays each clip as far as the
wind is strong (Motion: *Clips start at* / *Clips end at*), blends the two sides nearest to where the wind comes from, and
blends wind and turning by how far each has gone: poses are mixed, never added on top of each other.

**Generate clips** writes all six, each rest + two frames: a headwind opens the jacket (frame 2 is the Open pose) and
blows it back (frame 3); wind from behind presses the back and kicks the front hems out ahead; a side wind presses the
windward front shut, opens the far one and pushes the rest along with the air; a turn flares the hems and lets the
cloth trail. They are built from the Open pose - your hand-made one when there is one, else a hinge round the spine -
and four strengths (Blown back, Hem lift, Sweep, Turn flare). Clips you made or changed are kept unless you press
**Replace mine too**.

In **6 Wind poses**: **New** makes a clip at rest on frames 1 and 2 and shows it on the armature; pose the cloth bones on a
frame and press **Key pose here** (or key with Blender's own tools). **Key the Open pose here** starts the front clip from
an existing Open pose. Right and turning right are written from the left ones, mirrored (both sides swap), on export or
with **Mirror**; untick the mirror icon to make them by hand. The Preview sliders show the blend the game will show. While
any clip is set, Open and Volume are not exported.

The check poses every clip frame, and the blends halfway between two sides, and reports cloth that goes into a collider
(`POSE-IN-BODY`) or stretches (`POSE-STRETCH`: skinning cannot slide one panel over another).

### Torso collision

**7 Collisions > Torso** adds *surface* capsules from the hips to the chest: one on the spine, and one along each side when
the torso is wider than deep. A surface capsule keeps the cloth itself out - a few vertices each bone carries, the game
turning the bone about its head - not the bones' tips, so bones that run inside the body (a jacket rigged near the
spine) still keep their cloth outside it. Their auto radii end just inside the resting cloth. Leg capsules work on the
tips as before.

### Rigs that already exist

**Adopt an existing rig** (Character step) takes over cloth bones made by hand: name the root bones or pick the model's
`.skmm`, and the strips, settings, colliders and clip names are read from them. Nothing is rebuilt or deleted.

### Preview

**9 Preview** plays the timeline with a port of the engine's `BoneCloth::Step` running on the cloth bones, with the test
motion and wind you choose. Nothing is keyed; Stop puts the pose back. The yellow lines are the strips as the game sees them.

**Start over** (bottom of the main panel) deletes everything the setup made and puts the mesh back from the hidden copy.
Every button is also one undo step.

## Tests

```
python tests/test_engine_contract.py                 # the engine's rules, plain Python + numpy
python tests/verify_glb.py <model.glb>               # reads an exported model the way the engine does
set BC_TEST_BLEND=<a copy of player_coat.blend>
python tests/run_headless.py test_pipeline.py        # Auto Setup + export on a copy of the coat (removes the old rig first)
python tests/run_headless.py test_operators.py       # the same through the buttons' operators, incl. Update all, Start over
python tests/run_headless.py test_bake_and_ui.py     # rest-shape bake vs Blender's own result, every panel's draw code
python tests/run_headless.py render_view.py          # renders panel colours (BC_RENDER_DIR; BC_CLOTH, BC_PRESET, BC_BEND)
python tests/run_headless.py test_cape.py            # a synthetic cape and strand: Auto Setup and a headwind
python tests/run_headless.py test_strand.py
set BC_NO_PREPARE=1                                  # on the saved scenes (tests/data/jacket_reference.blend, a coat copy):
python tests/run_headless.py test_hand_pose.py       # hand poses, mirroring, the generator against the hand-made target
python tests/run_headless.py test_sim.py             # the swing preview: rest, run, headwind, spin, teleport
python tests/run_headless.py test_adopt.py           # adopting the original hand-made coat rig with its .skmm
python tests/run_headless.py test_wind_clips.py <copy of ShooterGame/player_coat.blend>   # clips, mirror, probes, export
python tests/run_headless.py render_clips.py <copy of ShooterGame/player_coat.blend>     # contact sheets of every clip
python tests/run_headless.py test_window.py <copy of ShooterGame/player_coat.blend>      # a walk's sway cancels, a run trails
set BC_NO_PREPARE=1                                  # for scenes that are not the coat copy:
python tests/run_headless.py test_jacket.py          # the jacket scene: Follow-the-spine layout and a 60 degree spine bend
```

`run_headless.py` runs Blender in the background on the blend you name (Blender path inside the script) after
`prepare_coat.py` has stripped the old hand-made coat rig; full output goes to `tests/last_run.txt`.
