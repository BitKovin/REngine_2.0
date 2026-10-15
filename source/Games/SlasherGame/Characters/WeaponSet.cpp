#include "WeaponSet.h"

#include <Actions/MoveRunner.h>
#include <Characters/CombatCharacter.h>
#include <Characters/WeaponMesh.h>
#include <Combat/CombatComponents.h>
#include <Combat/CombatEvents.h>
#include <Effects/MeleeTrail.h>
#include <Weapons/Projectile.h>

#include <AiPerception/AiPerceptionSystem.h>
#include <Entities/SoundPlayer.h>
#include <Particle/GlobalParticleSystem.hpp>
#include <Physics.h>
#include <RandomHelper.h>
#include <SkeletalMesh.hpp>

#include <algorithm>
#include <cmath>

namespace
{
	const BodyType MeleeHitMask = static_cast<BodyType>(BodyType::HitBox | BodyType::MainBody);

	glm::mat4 AttachMatrix(const WeaponAttachData& attach, float scale)
	{
		MathHelper::Transform transform;
		transform.Position = attach.offset;
		transform.Rotation = attach.rotation;
		transform.Scale = glm::vec3(scale);
		return transform.ToMatrixEuler();
	}

	glm::vec3 ApplySpread(const glm::vec3& direction, float spreadDegrees)
	{
		if (spreadDegrees <= 0.0f)
			return direction;

		// Uniform direction inside a cone of half-angle spreadDegrees around `direction`.
		const float cosMax = std::cos(glm::radians(spreadDegrees));
		const float cosTheta = 1.0f - RandomHelper::RandomFloat() * (1.0f - cosMax);
		const float sinTheta = std::sqrt(std::max(0.0f, 1.0f - cosTheta * cosTheta));
		const float phi = RandomHelper::RandomFloat() * glm::two_pi<float>();

		const glm::vec3 helper = std::abs(direction.y) < 0.99f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
		const glm::vec3 right = glm::normalize(glm::cross(direction, helper));
		const glm::vec3 up = glm::cross(right, direction);
		return glm::normalize(direction * cosTheta + (right * std::cos(phi) + up * std::sin(phi)) * sinTheta);
	}

	void PreloadEffect(const std::string& name)
	{
		if (!name.empty())
			Entity::PreloadEntityType(name);
	}
}

void WeaponSet::Initialize(CombatCharacter* character)
{
	owner = character;
}

void WeaponSet::LoadAssets(const CharacterData& data)
{
	SkeletalMesh* body = owner->GetMesh();
	const roj::FlatSkeleton* bodySkeleton = body && body->model ? &body->model->skeleton : nullptr;

	for (size_t i = 0; i < slots.size(); ++i)
	{
		SlotState& state = slots[i];
		state.data = data.weaponData[i];
		state.meshes.clear();
		if (!state.data)
			continue;

		const WeaponData& weapon = *state.data;
		for (const WeaponMeshData& meshData : weapon.meshes)
		{
			auto* mesh = new WeaponMesh(owner);
			if (!meshData.textures.empty())
				mesh->TexturesLocation = meshData.textures;
			mesh->LoadFromFile(meshData.model);
			mesh->PreloadAssets();
			mesh->Visible = false;   // shown once posed
			owner->Drawables.push_back(mesh);

			MeshInstance instance;
			instance.mesh = mesh;
			instance.data = &meshData;
			instance.socketBones.assign(weapon.sockets.size(), hashed_string());
			instance.socketBind.assign(weapon.sockets.size(), glm::mat4(1.0f));
			instance.sockets.assign(weapon.sockets.size(), glm::mat4(1.0f));

			if (mesh->model == nullptr)
			{
				Logger::Error("[WeaponSet] %s: could not load weapon model '%s'", weapon.sourcePath.c_str(), meshData.model.c_str());
			}
			else
			{
				for (size_t socket = 0; socket < weapon.sockets.size(); ++socket)
				{
					const hashed_string bone(weapon.sockets[socket]);
					if (mesh->model->skeleton.findBone(bone) == INVALID_BONE_IDX)
						continue;
					instance.socketBones[socket] = bone;
					// Still in the bind pose here (nothing sampled yet); a Bone mesh stays in it.
					instance.socketBind[socket] = AnimationPose::GetModelSpaceTransform(bone, mesh->GetRootNode(), mesh->GetAnimationPoseRef());
				}

				if (meshData.attach == WeaponAttachMode::Rig)
				{
					if (bodySkeleton)
						SetupRig(instance, weapon, *bodySkeleton);
					else
						Logger::Error("[WeaponSet] %s: Rig mesh '%s' needs a loaded body model", weapon.sourcePath.c_str(), meshData.model.c_str());
				}
			}

			state.meshes.push_back(std::move(instance));
		}

		// No meshes: the weapon is part of the character model (an axe or gun modelled into an enemy). Its points use
		// the body's bones and are posed with it.
		if (weapon.meshes.empty() && bodySkeleton)
		{
			MeshInstance instance;
			instance.socketBones.assign(weapon.sockets.size(), hashed_string());
			instance.socketBind.assign(weapon.sockets.size(), glm::mat4(1.0f));
			instance.sockets.assign(weapon.sockets.size(), glm::mat4(1.0f));
			for (size_t socket = 0; socket < weapon.sockets.size(); ++socket)
			{
				const hashed_string bone(weapon.sockets[socket]);
				if (bodySkeleton->findBone(bone) != INVALID_BONE_IDX)
					instance.socketBones[socket] = bone;
			}
			state.meshes.push_back(std::move(instance));
		}

		if (!weapon.extend.bone.empty() && std::none_of(state.meshes.begin(), state.meshes.end(),
				[&](const MeshInstance& instance) { return instance.mesh != nullptr && instance.mesh->model != nullptr
					&& instance.mesh->model->skeleton.findBone(weapon.extend.boneHash) != INVALID_BONE_IDX; }))
			Logger::Error("[WeaponSet] %s: extend bone '%s' does not exist in the weapon's models", weapon.sourcePath.c_str(), weapon.extend.bone.c_str());

		// A mesh may lack a point's bone (akimbo props, one muzzle each); every bone must exist somewhere.
		for (size_t socket = 0; socket < weapon.sockets.size(); ++socket)
		{
			const bool found = std::any_of(state.meshes.begin(), state.meshes.end(),
				[socket](const MeshInstance& instance) { return !instance.socketBones[socket].empty(); });
			if (!found)
				Logger::Error("[WeaponSet] %s: bone '%s' (blade / muzzle point) does not exist in the weapon's models (the body's, without meshes)",
					weapon.sourcePath.c_str(), weapon.sockets[socket].c_str());
		}

		PreloadEffect(weapon.trailParticle);
		if (weapon.kind == WeaponKind::Firearm)
		{
			state.magazine = weapon.firearm.magazine;
			PreloadEffect(weapon.firearm.muzzleParticle);
			PreloadEffect(weapon.firearm.projectile.tracer);
			PreloadEffect(weapon.firearm.projectile.impactParticle);
		}
	}

	// Hand the melee weapon first if there is one.
	activeSlot = slots[static_cast<size_t>(WeaponSlot::Main)].data ? WeaponSlot::Main : WeaponSlot::Secondary;
}

void WeaponSet::Destroy()
{
	StopAllTrails();
	SyncTrails();
	pendingShots.clear();
}

// ---- Rig weapons (COMBAT_DESIGN.md §9.3) ----------------------------------------------------------------

void WeaponSet::SetupRig(MeshInstance& instance, const WeaponData& weapon, const roj::FlatSkeleton& bodySkeleton)
{
	const WeaponMeshData& meshData = *instance.data;
	const roj::FlatSkeleton& skeleton = instance.mesh->model->skeleton;
	const AnimationPose bindPose = instance.mesh->GetAnimationPose();   // nothing sampled yet

	// Weapon bones: everything the body skeleton does not have.
	instance.rigBones.clear();
	auto isWeaponBone = [&](uint16_t index) { return bodySkeleton.findBone(skeleton.bones[index].name) == INVALID_BONE_IDX; };

	for (uint16_t i = 0; i < skeleton.boneCount; ++i)
	{
		if (!isWeaponBone(i))
			continue;
		const uint16_t parent = skeleton.bones[i].parentIdx;
		RigBone bone;
		bone.name = skeleton.bones[i].name;
		bone.parent = parent != INVALID_BONE_IDX ? skeleton.bones[parent].name : hashed_string();
		bone.root = parent == INVALID_BONE_IDX || !isWeaponBone(parent);
		instance.rigBones.push_back(bone);
	}

	if (instance.rigBones.empty())
	{
		Logger::Error("[WeaponSet] %s: Rig model '%s' has no bones beyond the body skeleton", weapon.sourcePath.c_str(), meshData.model.c_str());
		return;
	}

	// Body bones the model lacks (a weapon rig without the coat) are fine: the mesh is drawn with the body's pose, and
	// the animator leaves bones a clip lacks to the pose underneath.

	// Extension: one of the weapon bones (the model may lack it - akimbo props; LoadAssets checks that one has it).
	instance.extendBone = hashed_string();
	if (!weapon.extend.bone.empty() && skeleton.findBone(weapon.extend.boneHash) != INVALID_BONE_IDX)
	{
		const bool weaponBone = std::any_of(instance.rigBones.begin(), instance.rigBones.end(),
			[&](const RigBone& bone) { return bone.name == weapon.extend.boneHash; });
		const glm::vec3 modelAxis = glm::mat3(AnimationPose::GetModelSpaceTransform(weapon.extend.boneHash, instance.mesh->GetRootNode(), bindPose))
			* weapon.extend.resolvedAxis;
		if (!weaponBone)
			Logger::Error("[WeaponSet] %s: extend bone '%s' is a body bone - it must be a weapon bone of '%s'",
				weapon.sourcePath.c_str(), weapon.extend.bone.c_str(), meshData.model.c_str());
		else if (glm::dot(modelAxis, modelAxis) < 1e-12f)
			Logger::Error("[WeaponSet] %s: extend bone '%s' is scaled to nothing in '%s'", weapon.sourcePath.c_str(),
				weapon.extend.bone.c_str(), meshData.model.c_str());
		else
		{
			instance.extendBone = weapon.extend.boneHash;
			instance.extendAxis = weapon.extend.resolvedAxis;
			instance.extendModelAxis = modelAxis;
		}
	}

	const bool handOk = SampleRest(instance, weapon, meshData.hand, bodySkeleton, bindPose, instance.handRest);
	const bool holsterOk = SampleRest(instance, weapon, meshData.holster, bodySkeleton, bindPose, instance.holsterRest);
	instance.rigReady = handOk && holsterOk;
}

bool WeaponSet::SampleRest(MeshInstance& instance, const WeaponData& weapon, const WeaponAttachData& attach,
	const roj::FlatSkeleton& bodySkeleton, const AnimationPose& bindPose, RigRest& rest)
{
	const WeaponMeshData& meshData = *instance.data;

	rest.follow = hashed_string();
	if (!attach.bone.empty())
	{
		if (bodySkeleton.findBone(attach.boneHash) == INVALID_BONE_IDX || instance.mesh->model->skeleton.findBone(attach.boneHash) == INVALID_BONE_IDX)
		{
			Logger::Error("[WeaponSet] %s: follow bone '%s' is not a body bone of '%s'", weapon.sourcePath.c_str(), attach.bone.c_str(), meshData.model.c_str());
			return false;
		}
		rest.follow = attach.boneHash;
	}

	AnimationPose clipPose;
	if (!attach.clip.empty() && !instance.mesh->SampleClipPose(attach.clip, attach.clipTime, clipPose))
	{
		Logger::Error("[WeaponSet] %s: clip '%s' does not exist in '%s'", weapon.sourcePath.c_str(), attach.clip.c_str(), meshData.model.c_str());
		return false;
	}

	rest.values = RigValues(instance, attach.clip.empty() ? bindPose : clipPose, rest.follow, nullptr);
	return true;
}

AnimationPose WeaponSet::RigValues(const MeshInstance& instance, const AnimationPose& clip, const hashed_string& follow,
	const AnimationPose* fallback)
{
	roj::BoneNode* rootNode = instance.mesh->GetRootNode();
	const glm::mat4 followInverse = follow.empty()
		? glm::mat4(1.0f)
		: glm::inverse(AnimationPose::GetModelSpaceTransform(follow, rootNode, clip));

	AnimationPose values;
	for (const RigBone& bone : instance.rigBones)
	{
		auto it = clip.boneTransforms.find(bone.name);
		if (it == clip.boneTransforms.end())
		{
			if (fallback)
				values.boneTransforms[bone.name] = fallback->boneTransforms.at(bone.name);
			continue;
		}

		values.boneTransforms[bone.name] = bone.root && !follow.empty()
			? followInverse * AnimationPose::GetModelSpaceTransform(bone.name, rootNode, clip)
			: it->second;
	}
	return values;
}

const WeaponData* WeaponSet::GetWeapon(WeaponSlot slot) const
{
	return Slot(slot).data.get();
}

void WeaponSet::SetActiveSlot(WeaponSlot slot)
{
	if (!HasWeapon(slot) || slot == activeSlot)
		return;

	activeSlot = slot;
	SlotState& state = Slot(slot);
	state.holster.AddDelay(state.data->holsterDelay);
}

bool WeaponSet::IsHolsterDue(WeaponSlot slot) const
{
	const SlotState& state = Slot(slot);
	return state.data && state.data->holsterDelay > 0.0f && state.holster.GetRemainTime() <= 0.0;
}

// ---- Points ------------------------------------------------------------------------------------------

bool WeaponSet::GetPoint(const MeshInstance& instance, const WeaponPointData& point, glm::vec3& out)
{
	if (!instance.hasTransform)
		return false;

	if (point.socket < 0)
	{
		out = glm::vec3(instance.world * glm::vec4(point.offset, 1.0f));
		return true;
	}

	const size_t socket = static_cast<size_t>(point.socket);
	if (socket >= instance.socketBones.size() || instance.socketBones[socket].empty())
		return false;

	out = glm::vec3(instance.sockets[socket] * glm::vec4(point.offset, 1.0f));
	return true;
}

// ---- Melee -------------------------------------------------------------------------------------------

void WeaponSet::ComputeSamples(const HitWindowData& window, std::vector<glm::vec3>& out) const
{
	out.clear();

	if (!window.bone.empty())
	{
		if (SkeletalMesh* body = owner->GetMesh())
			out.push_back(glm::vec3(body->GetBoneMatrixWorld(window.bone)[3]));
		return;
	}

	const SlotState& state = Slot(window.weapon);
	if (!state.data)
		return;

	const BladeData& blade = state.data->blade;
	const int count = std::max(2, blade.samples);

	for (const MeshInstance& instance : state.meshes)
	{
		glm::vec3 base, tip;
		if (!GetPoint(instance, blade.base, base) || !GetPoint(instance, blade.tip, tip))
			continue;
		for (int i = 0; i < count; ++i)
			out.push_back(glm::mix(base, tip, static_cast<float>(i) / static_cast<float>(count - 1)));
	}
}

void WeaponSet::BeginHitWindow(int windowIndex)
{
	if (windowIndex >= static_cast<int>(windows.size()))
		windows.resize(static_cast<size_t>(windowIndex) + 1);

	HitWindowState& state = windows[static_cast<size_t>(windowIndex)];
	state.previousSamples.clear();
	state.hasPrevious = false;
}

void WeaponSet::EndHitWindow(int windowIndex)
{
	if (windowIndex < static_cast<int>(windows.size()))
		windows[static_cast<size_t>(windowIndex)].hasPrevious = false;
}

void WeaponSet::SweepHitWindow(MoveSlot& slot, int windowIndex)
{
	const HitWindowData& window = slot.move->hits[static_cast<size_t>(windowIndex)];
	if (windowIndex >= static_cast<int>(windows.size()))
		BeginHitWindow(windowIndex);

	HitWindowState& state = windows[static_cast<size_t>(windowIndex)];
	std::vector<entt::entity>& victims = slot.hitVictims[static_cast<size_t>(windowIndex)];

	if (window.direct)
	{
		// Paired moves (finishers, grabs): the move's target is hit without a sweep, once per window.
		auto* target = dynamic_cast<CombatCharacter*>(owner->GetSoftTarget());
		if (target != nullptr && std::find(victims.begin(), victims.end(), target->Handle()) == victims.end())
		{
			victims.push_back(target->Handle());
			const glm::vec3 point = target->GetTargetPoint();
			EmitHit(slot, window, target, point, glm::normalize(owner->Position - point + glm::vec3(0.0f, 1e-4f, 0.0f)), owner->GetForward(), "");
		}
		return;
	}

	ComputeSamples(window, sampleScratch);
	if (sampleScratch.empty())
		return;

	if (window.bone.empty())
		SetActiveSlot(window.weapon);

	const float radius = window.bone.empty() ? Slot(window.weapon).data->blade.radius : window.radius;
	auto ownerTeam = owner->GetTeam();
	const bool canSweep = state.hasPrevious && state.previousSamples.size() == sampleScratch.size();

	for (size_t i = 0; i < sampleScratch.size(); ++i)
	{
		const glm::vec3 current = sampleScratch[i];
		const glm::vec3 previous = canSweep ? state.previousSamples[i] : current;
		const glm::vec3 sweep = current - previous;

		std::vector<Physics::HitResult> hits = canSweep && glm::dot(sweep, sweep) > 1e-8f
			? Physics::MultiSphereTrace(previous, current, radius, MeleeHitMask, {}, { owner })
			: Physics::MultiSphereOverlap(current, radius, MeleeHitMask, {}, { owner });

		std::sort(hits.begin(), hits.end(), [](const Physics::HitResult& a, const Physics::HitResult& b) { return a.fraction < b.fraction; });

		for (const Physics::HitResult& hit : hits)
		{
			Entity* victim = hit.entity;
			if (!hit.hasHit || victim == nullptr || victim == owner || victim->Destroyed)
				continue;

			const entt::entity id = victim->Handle();
			if (std::find(victims.begin(), victims.end(), id) != victims.end())
				continue;

			if (auto victimState = victim->GetComponent<CombatStateComponent>())
				if (victimState->dead)
					continue;

			if (auto victimTeam = victim->GetComponent<TeamComponent>())
				if (ownerTeam && GetAttitude(ownerTeam->team, victimTeam->team) == Attitude::Friendly)
					continue;

			victims.push_back(id);

			const glm::vec3 point = hit.position == glm::vec3(0.0f) ? current : hit.position;
			const glm::vec3 direction = glm::dot(sweep, sweep) > 1e-8f ? glm::normalize(sweep) : owner->GetForward();
			EmitHit(slot, window, victim, point, hit.normal, direction, hit.hitboxName);
		}
	}

	state.previousSamples = sampleScratch;
	state.hasPrevious = true;
}

void WeaponSet::EmitHit(const MoveSlot& slot, const HitWindowData& window, Entity* victim, const glm::vec3& point,
	const glm::vec3& normal, const glm::vec3& sweepDirection, const std::string& hitbox)
{
	HitRequestEvent event;
	event.attacker = EntityHandle::FromEntity(owner);
	event.victim = EntityHandle::FromEntity(victim);
	event.source = window.source;
	event.damage = window.damage;
	event.poiseDamage = window.poiseDamage;
	event.weakPointDamage = window.resolvedWeakPointDamage;
	event.reaction = window.reaction;
	event.knockback = window.knockback;
	event.launch = window.launch;
	event.hitStop = window.hitStop;
	event.hitStopScale = window.hitStopScale;
	event.shake = window.shake;
	event.camera = window.camera;
	event.impactScale = window.impactScale;
	event.airLift = window.air.lift;
	event.airStall = window.air.stall;
	event.heavy = slot.move->heavy;
	event.moveIndex = slot.move->index;
	event.point = point;
	event.normal = normal;
	event.hitbox = hitbox;
	event.hitSound = window.hitSound;
	event.hitParticle = window.hitParticle;
	if (window.source == DamageSource::Finisher)
		event.flags |= HitFlags::IgnoreArmor | HitFlags::IgnoreInvulnerable;   // the victim is protected by the finisher state

	glm::vec3 push = MathHelper::XZ(victim->Position - owner->Position);
	event.direction = glm::dot(push, push) > 1e-6f ? glm::normalize(push) : MathHelper::XZ(sweepDirection);
	if (glm::dot(event.direction, event.direction) < 1e-6f)
		event.direction = owner->GetForward();
	event.spray = glm::dot(sweepDirection, sweepDirection) > 1e-6f ? glm::normalize(sweepDirection) : event.direction;

	owner->QueueHit(event);
}

void WeaponSet::SetTrail(WeaponSlot slot, bool enabled)
{
	Slot(slot).trailWanted = enabled;
}

void WeaponSet::SyncTrails()
{
	for (SlotState& state : slots)
	{
		if (!state.data || state.data->trailParticle.empty())
			continue;

		if (state.trailWanted && state.trail == nullptr)
		{
			Entity* entity = LevelObjectFactory::instance().create(state.data->trailParticle);
			auto* trail = dynamic_cast<particle_system_meleeTrail*>(entity);
			if (trail == nullptr)
			{
				delete entity;
				Logger::Error("[WeaponSet] trail effect '%s' is not a melee trail system", state.data->trailParticle.c_str());
				state.trailWanted = false;   // reported once per request
				continue;
			}

			state.trail = trail;
			PlaceTrail(state);              // before its first frame
			trail->Start();
			Level::Current->AddEntity(trail);
			trail->LoadAssetsIfNeeded();    // already preloaded in LoadAssets: no file access here
		}
		else if (!state.trailWanted && state.trail != nullptr)
		{
			state.trail->StopAll();
			state.trail->DestroyWithDelay(1.0f);
			state.trail = nullptr;
		}
	}
}

void WeaponSet::StopAllTrails()
{
	for (size_t i = 0; i < slots.size(); ++i)
		SetTrail(static_cast<WeaponSlot>(i), false);
}

// ---- Firearms ----------------------------------------------------------------------------------------

void WeaponSet::Tick(float deltaTime)
{
	for (SlotState& state : slots)
	{
		if (state.fireCooldown > 0.0f)
			state.fireCooldown -= deltaTime;

		if (state.reloadRemaining > 0.0f)
		{
			state.reloadRemaining -= deltaTime;
			if (state.reloadRemaining <= 0.0f && state.data)
			{
				state.reloadRemaining = 0.0f;
				state.magazine = state.data->firearm.magazine;
			}
		}
	}
}

bool WeaponSet::IsReadyToFire(WeaponSlot slot) const
{
	const SlotState& state = Slot(slot);
	if (!state.data || state.data->kind != WeaponKind::Firearm)
		return false;
	if (state.fireCooldown > 0.0f || state.reloadRemaining > 0.0f)
		return false;
	return state.data->firearm.magazine == 0 || state.magazine > 0;
}

bool WeaponSet::IsReloading(WeaponSlot slot) const
{
	return Slot(slot).reloadRemaining > 0.0f;
}

int WeaponSet::GetMagazine(WeaponSlot slot) const
{
	const SlotState& state = Slot(slot);
	if (!state.data || state.data->firearm.magazine == 0)
		return -1;
	return state.magazine;
}

bool WeaponSet::StartReload(WeaponSlot slot)
{
	SlotState& state = Slot(slot);
	if (!state.data || state.data->kind != WeaponKind::Firearm || state.data->firearm.magazine == 0)
		return false;
	if (state.reloadRemaining > 0.0f || state.magazine == state.data->firearm.magazine)
		return false;
	state.reloadRemaining = state.data->firearm.reloadTime;
	return true;
}

int WeaponSet::Fire(WeaponSlot slot, const glm::vec3& aimPoint)
{
	if (!IsReadyToFire(slot))
		return -1;

	SlotState& state = Slot(slot);
	const FirearmData& firearm = state.data->firearm;

	// The shot cycle: every muzzle of every mesh. Muzzles that are not posed (hidden mesh, bone missing in that
	// model) are skipped; without any, the shot leaves from in front of the chest.
	const int muzzleCount = static_cast<int>(firearm.muzzles.size());
	const int cycle = static_cast<int>(state.meshes.size()) * muzzleCount;
	glm::vec3 muzzle = owner->Position + glm::vec3(0.0f, 0.4f, 0.0f) + owner->GetForward() * 0.5f;
	int shot = cycle > 0 ? state.nextMuzzle % cycle : 0;
	for (int i = 0; i < cycle; ++i)
	{
		const int candidate = (state.nextMuzzle + i) % cycle;
		const MeshInstance& instance = state.meshes[static_cast<size_t>(candidate / muzzleCount)];
		if (GetPoint(instance, firearm.muzzles[static_cast<size_t>(candidate % muzzleCount)], muzzle))
		{
			shot = candidate;
			break;
		}
	}
	state.nextMuzzle = cycle > 0 ? (shot + 1) % cycle : 0;

	glm::vec3 direction = aimPoint - muzzle;
	direction = glm::dot(direction, direction) > 1e-6f ? glm::normalize(direction) : owner->GetForward();

	pendingShots.push_back({ slot, muzzle, direction });

	state.fireCooldown = 1.0f / firearm.fireRate;
	if (firearm.magazine > 0 && --state.magazine <= 0)
		StartReload(slot);

	SetActiveSlot(slot);
	state.holster.AddDelay(state.data->holsterDelay);
	return shot;
}

void WeaponSet::FlushShots()
{
	for (const PendingShot& shot : pendingShots)
	{
		const SlotState& state = Slot(shot.slot);
		if (!state.data)
			continue;

		const FirearmData& firearm = state.data->firearm;
		for (int i = 0; i < firearm.pellets; ++i)
			Projectile::Spawn(state.data, *owner, shot.muzzle, ApplySpread(shot.direction, firearm.spread));

		if (!firearm.fireSound.empty())
			SoundPlayer::PlayOneshot(firearm.fireSound, 2.0f, 1.0f, false, shot.muzzle);
		if (!firearm.muzzleParticle.empty())
			GlobalParticleSystem::SpawnParticleAt(firearm.muzzleParticle, shot.muzzle, MathHelper::FindLookAtRotation(shot.muzzle, shot.muzzle + shot.direction), glm::vec3(1.0f));
		if (firearm.noiseRadius > 0.0f)
			AiPerceptionSystem::EmitSoundAt(shot.muzzle, firearm.noiseRadius, 2, owner->Id);
	}
	pendingShots.clear();
}

// ---- Visuals -----------------------------------------------------------------------------------------

void WeaponSet::AsyncUpdate(SkeletalMesh& body, const AnimationPose& bodyPose, const CharacterAnimator::WeaponDrive& drive, float deltaTime)
{
	for (size_t i = 0; i < slots.size(); ++i)
	{
		SlotState& state = slots[i];
		const bool inHands = static_cast<WeaponSlot>(i) == activeSlot;

		for (MeshInstance& instance : state.meshes)
		{
			if (instance.mesh == nullptr)
			{
				PoseBuiltIn(instance, body, bodyPose);
				continue;
			}

			const WeaponMeshData& meshData = *instance.data;
			const WeaponAttachData& attach = inHands ? meshData.hand : meshData.holster;

			bool show = visible && (inHands || meshData.visibleWhenHolstered) && instance.mesh->model != nullptr;
			if (meshData.attach == WeaponAttachMode::Bone)
			{
				show = show && !attach.bone.empty();
				if (show)
					PlaceBoneMesh(instance, attach, body);
			}
			else if (show)
			{
				PoseRigMesh(instance, inHands, state.extension, body, bodyPose, drive, deltaTime);
				show = instance.hasTransform;
			}
			else
			{
				instance.state = -1;   // hidden: snap to the current pose when shown again
			}

			if (!show)
				instance.hasTransform = false;

			instance.mesh->Visible = show;

			// Meshes of the model that belong to the other state (the in-hand vs the holstered copy).
			const int hideState = inHands ? 1 : 0;
			if (show && instance.hideState != hideState)
			{
				instance.hideState = hideState;
				instance.mesh->MeshHideList = std::unordered_set<std::string>(attach.hide.begin(), attach.hide.end());
			}
		}
	}
}

void WeaponSet::PoseBuiltIn(MeshInstance& instance, SkeletalMesh& body, const AnimationPose& bodyPose)
{
	instance.world = body.GetWorldMatrix();
	for (size_t socket = 0; socket < instance.sockets.size(); ++socket)
		if (!instance.socketBones[socket].empty())
			instance.sockets[socket] = instance.world * AnimationPose::GetModelSpaceTransform(instance.socketBones[socket], body.GetRootNode(), bodyPose);
	instance.hasTransform = !bodyPose.boneTransforms.empty();
}

void WeaponSet::PlaceBoneMesh(MeshInstance& instance, const WeaponAttachData& attach, SkeletalMesh& body)
{
	const glm::mat4 world = body.GetBoneMatrixWorld(attach.bone) * AttachMatrix(attach, instance.data->scale);

	const MathHelper::Transform transform = MathHelper::DecomposeMatrix(world);
	instance.mesh->Position = transform.Position;
	instance.mesh->Rotation = transform.Rotation;
	instance.mesh->Scale = transform.Scale;

	instance.world = world;
	for (size_t socket = 0; socket < instance.sockets.size(); ++socket)
		if (!instance.socketBones[socket].empty())
			instance.sockets[socket] = world * instance.socketBind[socket];
	instance.hasTransform = true;
}

void WeaponSet::PoseRigMesh(MeshInstance& instance, bool inHands, float extension, SkeletalMesh& body, const AnimationPose& bodyPose,
	const CharacterAnimator::WeaponDrive& drive, float deltaTime)
{
	WeaponMesh& mesh = *instance.mesh;
	if (!instance.rigReady || bodyPose.boneTransforms.empty())
	{
		instance.hasTransform = false;
		return;
	}

	const RigRest& rest = inHands ? instance.handRest : instance.holsterRest;

	// ---- Target: the rest pose, pulled toward the clips driving the body while in the hands ----
	AnimationPose target = rest.values;
	if (inHands)
	{
		if (drive.move && drive.moveWeight > 0.001f)
			target = AnimationPose::Lerp(target, RigValues(instance, *drive.move, rest.follow, &rest.values), drive.moveWeight);
		if (drive.overlay && drive.overlayWeight > 0.001f)
			target = AnimationPose::Lerp(target, RigValues(instance, *drive.overlay, rest.follow, &rest.values), drive.overlayWeight);

		// Reach: the extend bone moves out along its axis. On the target, so a move taking over blends the length to
		// its own like the clip's weapon bones (the transition below).
		if (extension > 0.0f && !instance.extendBone.empty())
		{
			const float metersPerUnit = glm::length(glm::mat3(body.GetWorldMatrix()) * instance.extendModelAxis);
			auto it = target.boneTransforms.find(instance.extendBone);
			if (it != target.boneTransforms.end() && metersPerUnit > 1e-6f)
				it->second = it->second * glm::translate(glm::mat4(1.0f), instance.extendAxis * (extension / metersPerUnit));
		}
	}

	// ---- Transitions: when another clip takes over, cross-fade from the last output (like the body) ----
	const int state = inHands ? 1 : 0;
	const uint32_t driveSerial = inHands ? drive.moveSerial : 0;

	if (instance.state != state)
	{
		// First pose, or hands <-> holster: the value spaces differ, snap.
		instance.state = state;
		instance.driveSerial = driveSerial;
		instance.fadeDuration = 0.0f;
		instance.current = std::move(target);
	}
	else
	{
		if (driveSerial != instance.driveSerial)
		{
			instance.driveSerial = driveSerial;
			instance.fadeDuration = drive.moveTransition;
			instance.fadeElapsed = 0.0f;
			instance.fadeFrom = instance.current;
		}

		if (instance.fadeDuration > 0.001f)
		{
			instance.fadeElapsed += deltaTime;
			const float t = glm::smoothstep(0.0f, 1.0f, instance.fadeElapsed / instance.fadeDuration);
			instance.current = AnimationPose::Lerp(instance.fadeFrom, target, t);
			if (t >= 1.0f)
				instance.fadeDuration = 0.0f;
		}
		else
		{
			instance.current = std::move(target);
		}
	}

	// ---- Final pose: body bones from the body, weapon bones from the values ----
	roj::BoneNode* rootNode = mesh.GetRootNode();
	AnimationPose pose;
	pose.boneTransforms = bodyPose.boneTransforms;

	// The follow bone is a body bone: the weapon values below never affect it.
	const glm::mat4 follow = rest.follow.empty() ? glm::mat4(1.0f) : AnimationPose::GetModelSpaceTransform(rest.follow, rootNode, pose);

	for (const RigBone& bone : instance.rigBones)   // parent-first
	{
		glm::mat4 local = instance.current.boneTransforms.at(bone.name);
		if (bone.root && !rest.follow.empty())
		{
			const glm::mat4 parent = bone.parent.empty() ? glm::mat4(1.0f) : AnimationPose::GetModelSpaceTransform(bone.parent, rootNode, pose);
			local = glm::inverse(parent) * follow * local;
		}
		pose.boneTransforms[bone.name] = local;
	}

	// The rig is the body's skeleton: same transform as the body mesh.
	mesh.Position = body.Position;
	mesh.Rotation = body.Rotation;
	mesh.Scale = body.Scale;

	instance.world = mesh.GetWorldMatrix();
	for (size_t socket = 0; socket < instance.sockets.size(); ++socket)
		if (!instance.socketBones[socket].empty())
			instance.sockets[socket] = instance.world * AnimationPose::GetModelSpaceTransform(instance.socketBones[socket], rootNode, pose);
	instance.hasTransform = true;

	mesh.PasteAnimationPose(std::move(pose));
}

void WeaponSet::LateUpdate()
{
	SyncTrails();
	for (SlotState& state : slots)
		PlaceTrail(state);
}

void WeaponSet::PlaceTrail(SlotState& state)
{
	if (state.trail == nullptr || !state.data || state.meshes.empty())
		return;

	// The ribbon spans the outer part of the blade, from trailStart to the tip.
	glm::vec3 base, tip;
	if (GetPoint(state.meshes[0], state.data->blade.base, base) && GetPoint(state.meshes[0], state.data->blade.tip, tip)
		&& glm::distance(base, tip) > 1e-4f)
		state.trail->SetBlade(glm::mix(base, tip, state.data->trailStart), tip);
}

void WeaponSet::SetVisible(bool value)
{
	visible = value;
}
