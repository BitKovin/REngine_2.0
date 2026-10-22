#pragma once

#include "BoneClothData.h"
#include "../SkeletalMesh.hpp"

#include <vector>

// Bone cloth: coats, skirts and capes rigged as strips of bones, swinging the way action games of the early 2000s did
// their coats - spring bones, not a cloth simulation.
//
// A strip runs from its top, held by the body, to its free end; a coat's upper panel continues into the skirt bones
// it ends at, so a panel is one strip, chest to hem. Each bone's tip trails a point: where the bone would hang - its
// rest direction on the bone above as that bone hangs now - and eases toward it a share of the way each frame, quick at
// the top and slow at the end, so the strip bends smoothly, trails the body and catches up. The strips trail a share
// of the character's movement (BoneClothData::trail) and all of its turning. No speed is kept, so nothing overshoots or
// jitters when the body starts or stops. Leg capsules push the tips out. The animation's
// keys on the strip bones and their roots are not used: the strips hang from the body in their rest shape, turned by
// the wind layers (BoneClothData::openClip / volumeClip, additive clips) before they swing. A fast turn of the body
// drives those layers too (BoneClothData::spinOpen / spinVolume): the coat opens as it spins.
class BoneCloth
{
public:
	// Strips, colliders and wind layers from `data` on `model`, which needs its geometry: the vertices pick the strips
	// and fit the colliders. False (logged) when the model lacks what data names; the cloth is then unusable.
	bool Build(const BoneClothData& data, roj::SkinnedModel& model);

	// Swings the strips hanging from `pose`'s body and writes their bones (and their roots) into it. world: the mesh's
	// world matrix this frame. deltaTime: the owner's; 0 holds the strips as they are on the body (hitstop).
	void Step(AnimationPose& pose, const glm::mat4& world, float deltaTime);

	// The strips snap to rest on the next Step.
	void Reset() { needsReset = true; }

	static inline bool DebugDrawEnabled = false;   // console: cloth_debug

private:
	// One bone of a strip.
	struct Joint
	{
		uint16_t bone = 0;
		uint16_t above = 0;                       // the bone its rest direction is kept on: the joint above, or for a
		                                          // strip's top its skeleton parent
		glm::vec3 tipLocal = glm::vec3(0.0f);     // its tip in its local frame: the next joint's head, or the strip's end
		float follow = 0.0f;                      // how fast the tail catches up with where it would hang, 1/s
		float maxAngle = 0.0f;                    // radians
		glm::vec3 head = glm::vec3(0.0f);         // world
		glm::vec3 tail = glm::vec3(0.0f);         // world, trailing
		glm::mat3 basis = glm::mat3(1.0f);        // world, turned to the tail
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
	};

	struct RestVertex;   // BoneCloth.cpp

	bool BuildStrips(const std::vector<glm::mat4>& restWorld, const std::vector<RestVertex>& vertices);
	void BuildColliders(const std::vector<glm::mat4>& restWorld, const std::vector<RestVertex>& vertices);
	void BuildWindLayer(const std::string& clip, AnimationPose& difference, bool Strip::* uses);
	const char* Label() const { return data.roots.empty() ? "" : data.roots.front().c_str(); }

	void UpdateWindWeights(const AnimationPose& pose, const glm::mat4& world, float deltaTime);
	float SpinRate(const AnimationPose& pose, const glm::mat4& world, float deltaTime);
	void RestShape(AnimationPose& pose) const;
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

	std::vector<glm::mat4> modelSpace;   // the body as animated, the strips and roots at rest (with the wind layers)
	std::vector<glm::vec3> colliderStart;   // world, this frame
	std::vector<glm::vec3> colliderEnd;

	glm::vec3 previousOrigin = glm::vec3(0.0f);
	bool needsReset = true;
	bool built = false;
};
