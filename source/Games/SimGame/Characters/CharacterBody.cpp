// Time.hpp first: engine headers below use Time without including it.
#include <Time.hpp>

#include "CharacterBody.h"

#include <AssetRegistry.h>
#include <EngineMain.h>
#include <Level.hpp>
#include <LevelObjectFactory.h>
#include <MathHelper.hpp>
#include <RegisterLevelObject.h>

namespace
{
	// ShooterGame stands the first-person body this far behind the eye so the torso does not reach into the lens.
	constexpr float FirstPersonBodyBack = 0.2f;
	constexpr float BodyCentreHeight = 0.9f;

	SkeletalMesh* MakeMesh(CharacterBody* owner, const CharacterModel& model)
	{
		SkeletalMesh* mesh = new SkeletalMesh(owner);
		mesh->LoadFromFile(model.mesh);
		mesh->DepthPrePath = false;
		mesh->Masked = true;
		mesh->TwoSided = model.twoSided;
		mesh->PreloadAssets();
		mesh->UpdatePoseOnlyWhenRendered = true;
		return mesh;
	}
}

CharacterBody* CharacterBody::Spawn(const CharacterState& state)
{
	if (!Level::Current)
		return nullptr;
	CharacterBody* body = dynamic_cast<CharacterBody*>(LevelObjectFactory::instance().create("sim_character_body"));
	if (!body)
		return nullptr;
	body->last = state;
	Level::Current->AddEntity(body);
	body->Start();
	body->LoadAssetsIfNeeded();
	return body;
}

CharacterBody::CharacterBody()
{
	ClassName = "sim_character_body";
	SaveGame = false;
}

void CharacterBody::LoadAssets()
{
	const CharacterModel& model = CharacterModels::Get(last.model);
	mesh = MakeMesh(this, model);
	Drawables.push_back(mesh);
	animator = std::make_unique<CharacterAnimator>(this, model);
	animator->LoadAssetsIfNeeded();
}

// An untextured model (base.glb has no textures at all and renders black): a mesh draws with ONE colour texture and one tint (the engine copies both from
// the mesh to every part each frame), so it is a plain white-textured figure tinted per person: readable and varied, but a mannequin (owner: kept on purpose).
void CharacterBody::ApplyPaint(const CharacterState& state)
{
	if (paintApplied || !CharacterModels::Get(state.model).tinted)
		return;
	static const glm::vec4 tints[] = { { 0.62f, 0.50f, 0.40f, 1 }, { 0.40f, 0.48f, 0.62f, 1 }, { 0.58f, 0.58f, 0.50f, 1 }, { 0.46f, 0.58f, 0.44f, 1 }, { 0.68f, 0.42f, 0.40f, 1 }, { 0.54f, 0.52f, 0.60f, 1 } };
	mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/textures/generic/white.png");
	mesh->Color = tints[((state.variant * 2654435761u) >> 8) % 6];
	paintApplied = true;
}

// The shadow mesh is made the first time it is needed and switched with the view; the visible mesh casts its own shadow only when it is the whole body.
void CharacterBody::SetFirstPerson(bool on)
{
	if (on && !shadowMesh)
	{
		shadowMesh = MakeMesh(this, CharacterModels::Get(last.model));
		shadowMesh->OnlyShadows = true;
		Drawables.push_back(shadowMesh);
	}
	if (shadowMesh)
		shadowMesh->Visible = on;
	mesh->CastDetailShadows = !on;
}

void CharacterBody::PlaceMesh(SkeletalMesh& target, const CharacterState& state, bool firstPerson) const
{
	glm::vec3 position = state.feet;
	if (firstPerson)
		position -= MathHelper::GetForwardVector(glm::vec3(0.0f, state.bodyYaw, 0.0f)) * FirstPersonBodyBack;
	target.Position = position;
	target.Rotation = glm::vec3(0.0f, state.bodyYaw, 0.0f);
}

void CharacterBody::Apply(const CharacterState& state)
{
	if (!mesh || !animator)
		return;
	const bool firstPerson = state.visibility == CharacterState::Visibility::FirstPersonOwner;
	const bool warming = state.visibility == CharacterState::Visibility::Warming;
	if (firstPerson != (last.visibility == CharacterState::Visibility::FirstPersonOwner) || (firstPerson && !shadowMesh))
		SetFirstPerson(firstPerson);
	last = state;
	// The engine lights a mesh from the light volume at its entity's position: the middle of the body, not the floor under it (a point on the floor can lie
	// under the baked light grid and come out black).
	Position = state.feet + glm::vec3(0.0f, BodyCentreHeight, 0.0f);

	// Not drawn at all while warming (no shadow either); the animator's clock and the gait phase keep running, so the body is mid-stride when it is shown.
	mesh->Visible = !warming;
	ApplyPaint(state);
	mesh->Color.a = state.alpha;
	mesh->Masked = state.alpha >= 1.0f;

	PlaceMesh(*mesh, state, firstPerson);
	if (shadowMesh)
		PlaceMesh(*shadowMesh, state, firstPerson);

	const glm::vec3 horizontal(state.velocity.x, 0.0f, state.velocity.z);
	animator->relativeMovement = glm::vec2(glm::dot(horizontal, MathHelper::GetRightVector(mesh->Rotation)), glm::dot(horizontal, MathHelper::GetForwardVector(mesh->Rotation)));
	animator->crouching = state.stance == CharacterState::Stance::Crouching;

	if (EngineMain::MainInstance->SimulatingGameTicks)
		mesh->WasRended = false;
	// An unseen body does not pose: the animator still ticks while warming; outside that, a body nobody draws is not animated at all.
	if (warming)
	{
		animator->Update();
		clothNeedsReset = true;
		return;
	}
	// clothNeedsReset is also "has no pose yet": a new body and one that was warming must pose before its first drawn frame, whether or not it was drawn before.
	if (!firstPerson && !mesh->WasRended && !clothNeedsReset)
		return;

	animator->Update();
	const AnimationPose pose = animator->GetResultPose();
	const float deltaTime = Time::DeltaTimeF;
	if (!firstPerson)
	{
		mesh->PasteAnimationPoseWithCloth(pose, deltaTime);
		if (clothNeedsReset)
			mesh->ResetCloth(); // the strips snap to the pose after being out of sight or a teleport
	}
	else
	{
		shadowMesh->PasteAnimationPoseWithCloth(pose, deltaTime);
		if (clothNeedsReset)
			shadowMesh->ResetCloth();
		AnimationPose seen = shadowMesh->GetAnimationPose(); // the pose after the cloth: the coat is simulated once
		const CharacterModel& model = CharacterModels::Get(state.model);
		const glm::mat4 collapsed = glm::scale(glm::mat4(1.0f), glm::vec3(0.0f));
		for (const char* bone : model.firstPersonHidden)
			seen.SetBoneTransform(bone, collapsed);
		mesh->PasteAnimationPose(seen);
	}
	clothNeedsReset = false;
}

REGISTER_ENTITY(CharacterBody, "sim_character_body")
