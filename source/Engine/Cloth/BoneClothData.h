#pragma once

#include <string>
#include <vector>

#include "../glm.h"
#include "../json.hpp"
#include "../Helpers/JsonHelper.hpp"

// Bone cloth settings, stored in a skeletal mesh's .skmm (SkeletalMeshMetaData::cloth) - see BoneCloth.h. Lengths in
// metres, speeds in m/s, angles in degrees. Pairs written <name>Root / <name>Tip go from a strip's top to its free end,
// changing evenly along it.

// A capsule the strips' joints stay out of, from `bone`'s head to `to`'s head. Radius 0 = fitted to the vertices of
// the limb between the two.
struct BoneClothCollider
{
	std::string bone;
	std::string to;
	float radius = 0.0f;      // at bone
	float endRadius = 0.0f;   // at to

	// A surface collider (a torso) keeps the cloth itself out - points on the cloth each joint carries - instead of the
	// joints' tips, which may lie inside the body when the bones run near the spine. No padding is added to its radius.
	bool surface = false;
	// Moves both ends by this much, model space, as the model rests; each end then moves with its bone (a capsule along
	// the side of a torso, beside the spine).
	glm::vec3 offset = glm::vec3(0.0f);

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(BoneClothCollider, bone, to, radius, endRadius, surface, offset)
};

struct BoneClothData
{
	// Every child chain of these bones with vertices skinned to it is a strip (bones without vertices - rig helpers -
	// are skipped). A strip whose last bone points straight at another strip's top continues into it: a coat's upper
	// panel and the skirt bones it ends at, though they hang from different bones, are one strip.
	std::vector<std::string> roots;
	glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);   // model-space facing (up is +Y); left is up x forward
	float tipLength = 0.0f;   // length of each strip's last bone; 0 = to its farthest vertex

	float followRoot = 60.0f;      // how fast a bone's tip catches up with where it would hang, 1/s...
	float followTip = 8.0f;        // ...down to the free end (low: trails further, floatier)
	float trail = 0.2f;            // share of the character's movement the strips trail behind (its turns always are)
	float maxAngleRoot = 8.0f;     // most a bone turns away from its rest direction...
	float maxAngleTip = 70.0f;     // ...down to the free end
	float radius = 0.02f;          // how far the joints keep from colliders
	float teleportDistance = 2.0f; // the character moves farther in a frame: the strips snap to rest
	float swing = 1.0f;            // how much the strips trail and swing (0: they hold the pose exactly)
	// Windowed trailing (motionWindow > 0, replacing trail): each tip trails the average speed at which the spot it
	// hangs from moved over the last motionWindow seconds, so a sway back and forth within the window cancels out and
	// only steady movement (a run, a turn) shows. lagRoot..lagTip: seconds of that speed the tip trails behind.
	float motionWindow = 0.0f;
	float lagRoot = 0.0f;
	float lagTip = 0.02f;
	// ...scaled by which way the body moves: forward, backward, sideways, up and down, and the share that comes from
	// the body turning (around its axis, the line through the roots). The trail never carries the cloth toward that
	// axis, only out from it or along it.
	float physicsFront = 1.0f;
	float physicsBack = 1.0f;
	float physicsSides = 1.0f;
	float physicsVertical = 1.0f;
	float physicsSpin = 1.0f;
	float collision = 1.0f;        // how hard the colliders push them (0: not at all; both 0: the clips alone)

	std::vector<BoneClothCollider> colliders;

	// Wind clips: the poses the strips take as the wind blows from each side - frame 0 the rest shape, then on through
	// the clip as the apparent wind (world wind less the character's own velocity) grows from windStart to windFull. A
	// wind between two sides blends their clips. Empty = the strips keep their rest shape for wind from that side.
	std::string windFront;
	std::string windBack;
	std::string windLeft;            // wind from the character's left
	std::string windRight;
	float windStart = 2.0f;          // apparent wind speed where the clips start, m/s...
	float windFull = 12.0f;          // ...and reach their last frame
	// Turn clips: the same for the body turning (spinStart..spinFull, settling over spinRelease once the turn ends),
	// on top of the wind's pose. While the body spins the wind counts as headwind whatever its side (it sweeps round too
	// fast for cloth to follow), and spinOpen adds spinOpen of windFull of headwind at a full spin: the front opens.
	std::string turnLeft;
	std::string turnRight;
	// Strength: how far the clips play for the same wind or turn (1 as made; below calmer, above reaches the end sooner).
	float windStrengthFront = 1.0f;
	float windStrengthSides = 1.0f;
	float windStrengthBack = 1.0f;
	float spinStrength = 1.0f;

	// Wind layers (the older way; the clips above replace them): additive clips of the model (two frames: neutral, full)
	// that turn the strips' rest shape - the shape they swing back to. The apparent wind drives them.
	std::string openClip;            // opens the strips: weighted by the headwind, more on the windward side
	std::string volumeClip;          // flares the strips: `volume` always, more with wind speed
	float volume = 0.0f;             // base weight of volumeClip
	float openWindStart = 2.0f;      // headwind where opening starts...
	float openWindFull = 8.0f;       // ...and is full
	float windwardBias = 0.5f;       // share of the side wind that opens the strips on that side
	float volumeWindStart = 2.0f;    // wind speed where the extra volume starts...
	float volumeWindFull = 10.0f;    // ...and is full
	float layerSmoothing = 0.2f;     // seconds the layer weights and the wind clips take to follow the wind

	// Spin: the body turning fast (a quick turn, a spinning attack) opens the strips as if the air filled them - the
	// turn rate about the world's up - the fastest of the bones the roots hang from and their ancestors (pelvis, spine),
	// as the character and its animation turn them - adds to the wind layers. 0 weights = off.
	float spinOpen = 0.0f;           // weight added to openClip at full spin; with the clips, share of windFull as headwind
	float spinVolume = 0.0f;         // weight added to volumeClip at full spin
	float spinStart = 180.0f;        // turn rate where it starts, degrees/s...
	float spinFull = 720.0f;         // ...and is full
	float spinRelease = 0.5f;        // seconds it takes to settle once the turn ends (it rises with layerSmoothing)

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(BoneClothData, roots, forward, tipLength,
		followRoot, followTip, trail, maxAngleRoot, maxAngleTip, radius, teleportDistance, swing, collision,
		motionWindow, lagRoot, lagTip, physicsFront, physicsBack, physicsSides, physicsVertical, physicsSpin,
		colliders, windFront, windBack, windLeft, windRight, windStart, windFull, turnLeft, turnRight,
		windStrengthFront, windStrengthSides, windStrengthBack, spinStrength,
		openClip, volumeClip, volume, openWindStart, openWindFull, windwardBias, volumeWindStart, volumeWindFull,
		layerSmoothing, spinOpen, spinVolume, spinStart, spinFull, spinRelease)
};
