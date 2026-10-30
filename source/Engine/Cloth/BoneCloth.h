#pragma once

#include "BoneClothData.h"
#include "../SkeletalMesh.hpp"

#include <array>
#include <deque>
#include <vector>

// Bone cloth: coats, skirts and capes rigged as strips of bones, swinging the way action games of the early 2000s did
// their coats - spring bones, not a cloth simulation.
//
// A strip runs from its top, held by the body, to its free end; a coat's upper panel continues into the skirt bones
// it ends at, so a panel is one strip, chest to hem. Each bone's tip trails a point: where the bone would hang - its
// rest direction on the bone above as that bone hangs now - and eases toward it a share of the way each frame, quick at
// the top and slow at the end, so the strip bends smoothly, trails the body and catches up. The strips trail a share
// of the character's movement (BoneClothData::trail) and all of its turning. No speed is kept, so nothing overshoots or
// jitters when the body starts or stops. Leg capsules push the tips out; surface capsules (a torso) push the cloth itself
// out - a few of each joint's vertices it carries along, turning the joint about its head - so bones that run inside the
// body still keep their cloth outside it. The animation's keys on the strip bones and their roots are not used: the
// strips hang from the body in their rest shape, posed by the wind and turn clips (BoneClothData::windFront...turnRight:
// each played as far as the wind from that side or the turn has grown, and blended) or by the older wind layers
// (openClip / volumeClip, additive clips, with spinOpen / spinVolume) before they swing.
class BoneCloth
{
public:
	// Strips, colliders, probes and wind poses from `data` on `model`, which needs its geometry: the vertices pick the
	// strips and their probes and fit the colliders. False (logged) when the model lacks what data names; the cloth is
	// then unusable.
	bool Build(const BoneClothData& data, roj::SkinnedModel& model);

	// Swings the strips hanging from `pose`'s body and writes their bones (and their roots) into it. world: the mesh's
	// world matrix this frame. deltaTime: the owner's; 0 holds the strips as they are on the body (hitstop).
	void Step(AnimationPose& pose, const glm::mat4& world, float deltaTime);

	// The strips snap to rest on the next Step.
	void Reset() { needsReset = true; }

	static inline bool DebugDrawEnabled = false;   // console: cloth_debug

private:
	// A point on the cloth a joint carries, kept out of the surface colliders.
	struct Probe
	{
		glm::vec3 local = glm::vec3(0.0f);   // in the joint's bone frame
		uint32_t ignore = 0;                 // per collider: it rests inside that one, so that one leaves it alone
	};

	// A probe for the legs (the leg colliders, with surface colliders on the cloth): the joint's probes and its tail.
	struct LegProbe
	{
		glm::vec3 local = glm::vec3(0.0f);   // in the joint's bone frame
		std::vector<float> allowance;        // per collider: how deep it may go into that leg (as deep as it rests);
		                                     // < 0: that one leaves it alone
	};

	// A leg probe of a joint at or below the one being turned, in that one's bone frame (Swing scratch).
	struct LegPoint
	{
		glm::vec3 local = glm::vec3(0.0f);
		const std::vector<float>* allowance = nullptr;
		float joints = 1.0f;                 // the joints from the one turned to the probe's own: they split its push
	};

	// One bone of a strip.
	struct Joint
	{
		uint16_t bone = 0;
		uint16_t above = 0;                       // the bone its rest direction is kept on: the joint above, or for a
		                                          // strip's top its skeleton parent
		glm::vec3 tipLocal = glm::vec3(0.0f);     // its tip in its local frame: the next joint's head, or the strip's end
		float follow = 0.0f;                      // how fast the tail catches up with where it would hang, 1/s
		float along = 0.0f;                       // 0 at the strip's top .. 1 at its end
		float maxAngle = 0.0f;                    // radians
		glm::vec3 head = glm::vec3(0.0f);         // world
		glm::vec3 tail = glm::vec3(0.0f);         // world, trailing
		glm::mat3 basis = glm::mat3(1.0f);        // world, turned to the tail
		std::vector<Probe> probes;
		std::vector<LegProbe> legProbes;
		std::deque<glm::vec4> anchors;            // windowed trailing: where the tail would hang (xyz, world) and when (w)
		glm::vec3 lag = glm::vec3(0.0f);          // windowed trailing: the tail's smoothed offset from there, world
	};

	struct Strip
	{
		std::vector<Joint> joints;   // top first
		int side = 0;                // +1 left, -1 right, 0 middle (which side the wind comes from opens it more)
		bool opens = false;          // the wind layers turn it
		bool flares = false;
		float openWeight = 0.0f;     // smoothed
	};

	struct Collider
	{
		uint16_t bone = 0;
		uint16_t to = 0;
		float radius = 0.0f;
		float endRadius = 0.0f;
		bool surface = false;
		glm::vec3 startLocal = glm::vec3(0.0f);   // the offset end points in their bones' frames
		glm::vec3 endLocal = glm::vec3(0.0f);
	};

	// A clip's change of every strip bone from its first frame, sampled evenly over the clip.
	struct BoneChange
	{
		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);   // in the parent's space
		glm::vec3 translation = glm::vec3(0.0f);
		glm::vec3 scale = glm::vec3(1.0f);
	};
	struct ClipSamples
	{
		std::vector<std::vector<BoneChange>> samples;   // [sample][strip bone]; empty: no clip (the rest shape)
	};
	enum ClipSlot { WindFront, WindLeft, WindBack, WindRight, TurnLeft, TurnRight, ClipCount };   // wind: by angle

	struct RestVertex;   // BoneCloth.cpp

	bool BuildStrips(const std::vector<glm::mat4>& restWorld, const std::vector<RestVertex>& vertices);
	void BuildColliders(const std::vector<glm::mat4>& restWorld, const std::vector<RestVertex>& vertices);
	void BuildWindLayer(const std::string& clip, AnimationPose& difference, bool Strip::* uses);
	void BuildClip(const std::string& clip, ClipSamples& out);
	void BuildProbes(const std::vector<glm::mat4>& restWorld, const std::vector<RestVertex>& vertices);
	const char* Label() const { return data.roots.empty() ? "" : data.roots.front().c_str(); }

	void UpdateWindWeights(const AnimationPose& pose, const glm::mat4& world, float deltaTime);
	float SpinRate(const AnimationPose& pose, const glm::mat4& world, float deltaTime);
	void AddClip(ClipSlot slot, float position, float weight);
	void RestShape(AnimationPose& pose);
	glm::vec3 ColliderPoint(const glm::mat4& world, uint16_t bone, const glm::vec3& local) const;
	void ComputeModelSpace(const AnimationPose& pose);
	void Swing(const glm::mat4& world, float deltaTime, bool snap);
	void WriteBones(AnimationPose& pose, const glm::mat4& world);
	void DrawDebug(const glm::mat4& world) const;

	BoneClothData data;
	roj::SkinnedModel* model = nullptr;

	std::vector<Strip> strips;
	std::vector<uint16_t> roots;
	std::vector<Collider> colliders;
	glm::vec3 leftAxis = glm::vec3(1.0f, 0.0f, 0.0f);   // model space

	AnimationPose openDifference;     // MakeAdditive of the wind clips, Local
	AnimationPose volumeDifference;
	float volumeWeight = 0.0f;        // smoothed wind part
	float spinWeight = 0.0f;          // smoothed spin part, 0..1
	std::vector<uint16_t> spinBones;  // the body bones the roots hang from and their ancestors (pelvis, spine), parents first
	std::vector<int> spinParents;     // per bone, its parent's index in spinBones (-1: the top, under the world)
	std::vector<glm::mat3> spinTurns; // per bone, world, this frame (scratch)
	std::vector<glm::quat> previousSpinTurns;

	std::vector<uint16_t> clothBones;           // every strip bone once (the clips' samples are in this order)
	std::array<ClipSamples, ClipCount> clips;
	bool hasClips = false;
	glm::vec2 windAir = glm::vec2(0.0f);        // smoothed apparent wind, model space: along forward, along left
	float windSpeed = 0.0f;                     // smoothed apparent wind speed (does not fade while the body spins)
	float turnAmount = 0.0f;                    // smoothed, -1 (full right) .. 1 (full left)
	std::vector<glm::vec4> blendRotation;       // per strip bone, this frame (scratch)
	std::vector<glm::vec3> blendTranslation;
	std::vector<glm::vec3> blendScale;
	AnimationPose clipChange;                   // the blended clips as a Local additive pose (scratch)

	std::vector<glm::mat4> modelSpace;   // the body as animated, the strips and roots at rest (with the wind layers)
	std::vector<LegPoint> legPoints;        // Swing scratch
	std::vector<glm::vec3> colliderStart;   // world, this frame
	std::vector<glm::vec3> colliderEnd;

	glm::vec3 previousOrigin = glm::vec3(0.0f);
	float clock = 0.0f;                  // seconds the cloth has run (windowed trailing)
	std::deque<glm::vec4> bodyTrack;     // windowed trailing: the body axis's lowest point (xyz, world) and when (w)
	bool needsReset = true;
	bool built = false;
};
