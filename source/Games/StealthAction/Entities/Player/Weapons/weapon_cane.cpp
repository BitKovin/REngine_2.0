#include "WeaponTool.h"

#include "Projectiles/CaneProjectile.h"
#include <SoundSystem/FmodEventInstance.h>
#include "../../Enemy/IEnemy.h"
#include <Logger.hpp>


// Cane is a Tool: press "useTool" once to throw it, again to grab it back
// out of an enemy or retrieve it from the ground. Player automatically
// switches back to whatever was equipped before (once CanChangeSlot()
// allows it), same as every other Tool.
//
// NOTE: the old attack2-driven parry/counter (PerformParry / OnParried) has
// been removed here - attack2 is now the global firearm ready/aim input, and
// a fire-and-forget Tool isn't a great fit for a sustained block/parry
// stance anyway. If you want a parry with the cane, give it to a real Melee
// weapon instead (weapon_sword/weapon_twinsword already have "block" wired
// up for that).
class weapon_cane : public WeaponTool
{
public:

	vec3 projectileOffset = vec3(0.03f, -0.15f, -0.3f);

	const Body* bodyToPush = nullptr;
	vec3 impulseToApply = vec3();
	Delay pushDelay = Delay(1000000000);

	Delay grabDelay;
	bool grabing = false;

	vec3 grabStartPos = vec3();
	float grabCollisionRadius = 0.35f;         // should match the player's actual collision capsule radius
	bool grabSpaceClearApplied = false;        // ensures the "make room" nudge only fires once per grab
	const float grabCrowdedTolerance = 0.15f;  // how far short of target counts as "not enough room"
	const float grabClearPushForce = 6.0f;     // impulse for the one-time nudge - tune against your existing knockback values

	bool thrown = false;

	SoundPlayer* fireSoundPlayer = nullptr;

	weapon_cane()
	{
		modelPath = "GameData/models/player/weapons/cane/cane.glb";
		texturesLocation = "GameData/models/player/weapons/cane/cane.glb/";
		weaponOffset = vec3(0.0, 0.00, -0.0);

		thirdPersonModelPath = "GameData/models/player/weapons/cane/cane_tp.glb";
	}

	void Start() override
	{
		WeaponTool::Start();

		fireSoundPlayer = SoundPlayer::Create("event:/Weapons/pistol/pistol_fire");
		fireSoundPlayer->Volume = 0.5f;
		fireSoundPlayer->Is2D = true;

		pushDelay.AddDelay(100000000);
	}

	bool CanChangeSlot() override
	{
		if (viewmodel->GetAnimationName() == "throw")
			return false;

		if (viewmodel->GetAnimationName() == "grab" && viewmodel->IsAnimationPlaying())
			return false;
		if (viewmodel->GetAnimationName() == "take" && viewmodel->IsAnimationPlaying())
			return false;

		return true;
	}

	void LoadAssets() override
	{
		WeaponTool::LoadAssets();

		PreloadEntityType("caneProjectile");
	}

	void ReturnCane()
	{
		auto projectiles = Level::Current->FindAllEntitiesWithName("caneProjectile");

		for (auto p : projectiles)
		{
			p->Destroy();
		}

		viewmodel->PlayAnimation("idle", true, 0.3f);

		thrown = false;

		attackDelay.AddDelay(0.3);
	}

	void GrabCane()
	{
		attackDelay.AddDelay(1);

		MathHelper::Transform projectileTransform;

		auto projectiles = Level::Current->FindAllEntitiesWithName("caneProjectile");

		for (auto p : projectiles)
		{
			CaneProjectile* proj = (CaneProjectile*)p;

			if (proj != nullptr)
			{
				proj->DamageEntity();

				proj->Destroy();

				projectileTransform.Position = proj->Position;
				projectileTransform.Rotation = proj->Rotation;

				bodyToPush = proj->bodyToPush;
				impulseToApply = proj->impulseToApply;
				pushDelay.AddDelay(0.07f);
			}
		}

		vec3 safePosition = projectileTransform.Position + MathHelper::GetForwardVector(projectileTransform.Rotation) * -0.3f;

		vec3 directionToPlayer = normalize(MathHelper::XZ((MathHelper::GetForwardVector(projectileTransform.Rotation) * -1.0f)));

		vec3 playerToCameraDif = Camera::position - Player::Instance->Position;

		auto hit = Physics::SphereTrace(safePosition, safePosition - vec3(0, 1, 0), 0.01f, BodyType::World | BodyType::MainBody);

		if (hit.hasHit)
		{
			safePosition = hit.shapePosition + hit.normal;
		}

		vec3 finalTarget = safePosition + directionToPlayer - playerToCameraDif + vec3(0, 0.1f, 0);
		Player::Instance->MoveTo(SweepAndSlide(Player::Instance->Position, finalTarget, grabCollisionRadius));

		SetViewmodelScaleFactor(0.5);

		viewmodel->PlayAnimation("grab", false, 0.0f);
		Time::AddTimeScaleEffect(0.65, 0.2, true, "weapon", 0.15f, 0.2);

		thrown = false;
	}

	void StartGrab()
	{
		grabing = true;
		thrown = false;

		grabDelay.AddDelay(0.2f);

		attackDelay.AddDelay(1);

		viewmodel->PlayAnimation("take", false, 0);

		grabStartPos = Player::Instance->Position;
		grabSpaceClearApplied = false;

		SoundPlayer::PlayOneshot("event:/General/BassDrop", 3, 1, true);
	}

	void UpdateGrab()
	{
		if (!grabing) return;

		if (grabDelay.GetProgress() >= 1)
		{
			grabing = false;
			GrabCane();
			return;
		}

		viewmodel->SetAnimationTime(viewmodel->GetAnimationDuration() * grabDelay.GetProgress());

		auto projectiles = Level::Current->FindAllEntitiesWithName("caneProjectile");

		MathHelper::Transform projectileTransform;
		const Body* grabTargetBody = nullptr;

		for (auto p : projectiles)
		{
			CaneProjectile* proj = (CaneProjectile*)p;

			if (proj != nullptr)
			{
				projectileTransform.Position = proj->Position;
				projectileTransform.Rotation = proj->Rotation;
				proj->movingTo = true;
				grabTargetBody = proj->bodyToPush;
			}
		}

		vec3 safePosition = projectileTransform.Position + MathHelper::GetForwardVector(projectileTransform.Rotation) * -0.3f;
		vec3 directionToPlayer = normalize(MathHelper::XZ((MathHelper::GetForwardVector(projectileTransform.Rotation) * -1.0f)));
		vec3 playerToCameraDif = Camera::position - Player::Instance->Position;

		auto hit = Physics::SphereTrace(safePosition, safePosition - vec3(0, 1, 0), 0.001f, BodyType::World | BodyType::MainBody);

		if (hit.hasHit)
		{
			safePosition = hit.shapePosition + hit.normal;
		}

		vec3 destinationPos = safePosition + directionToPlayer - playerToCameraDif + vec3(0, 0.1f, 0);
		vec3 idealPos = lerp(grabStartPos, destinationPos, grabDelay.GetProgress());

		// Move from where the player actually is, sliding along anything it hits, instead of
		// teleporting along a straight line that ignores geometry.
		vec3 resolvedPos = SweepAndSlide(Player::Instance->Position, idealPos, grabCollisionRadius);

		// Still well short of where we should be? There isn't enough room next to the target -
		// nudge it clear once (not every frame, to keep it controlled) instead of stalling or clipping.
		if (!grabSpaceClearApplied && grabTargetBody != nullptr && length(idealPos - resolvedPos) > grabCrowdedTolerance)
		{
			vec3 pushDir = normalize(idealPos - Player::Instance->Position);
			Physics::AddImpulse(grabTargetBody, pushDir * grabClearPushForce);
			grabSpaceClearApplied = true;
		}

		Player::Instance->MoveTo(resolvedPos);
	}

	// Cane's own trigger has three outcomes (throw / grab / retrieve)
	// depending on state, so it reads "useTool" itself rather than using the
	// simpler single-outcome StartUse() hook.
	void Update() override
	{
		auto projectile = (CaneProjectile*)Level::Current->FindEntityWithName("caneProjectile");

		thrown = projectile != nullptr;

		if (Input::GetAction("useTool")->Pressed() && CanAttack() && !attackDelay.Wait())
		{
			if (thrown)
			{
				if (projectile->inEnemy)
					StartGrab();
				else
					ReturnCane();
			}
			else
			{
				StartUse();
			}

			NotifyUsed(); // (re)start the 3s auto-hide countdown from this use
		}

		UpdateGrab();

		if (!pushDelay.Wait())
		{
			pushDelay.AddDelay(100000000);

			if (bodyToPush != nullptr)
			{
				auto hitMesh = Physics::GetBodyData(bodyToPush)->OwnerSkeletalMesh;

				if (hitMesh)
				{
					hitMesh->ApplyImpulseToAllHitboxes(impulseToApply * 0.01f, true);
					Physics::AddImpulse(bodyToPush, impulseToApply * 0.25f);
				}
				else
				{
					Physics::AddImpulse(bodyToPush, impulseToApply);
				}

				bodyToPush = nullptr;
			}
		}

		if (!attackDelay.Wait())
		{
			projectile = (CaneProjectile*)Level::Current->FindEntityWithName("caneProjectile");

			if (projectile == nullptr && viewmodel->currentAnimationData->animationName == "throw")
			{
				ReturnCane();
			}
			else if (projectile != nullptr)
			{
				if (viewmodel->currentAnimationData->animationName != "throw")
				{
					viewmodel->PlayAnimation("throw", false, 0);
					viewmodel->SetAnimationTime(viewmodel->GetAnimationDuration() - 0.5f);
					viewmodel->Update();
					viewmodel->PullAnimationEvents();
				}
			}
		}

		auto events = viewmodel->PullAnimationEvents();

		for (auto event : events)
		{
			if (event.eventName == "throw")
				PerformAttack();

			if (event.eventName == "restore_size")
				SetViewmodelScaleFactor(2);
		}

		UpdateAutoHideTimer();
		UpdateDrawProgress(WantsPresented());
	}

	// The "throw" outcome of Update()'s dispatch.
	void StartUse() override
	{
		SwitchDelay.AddDelay(0.2f);

		SetViewmodelScaleFactor(2);

		viewmodel->PlayAnimation("throw", false, 0);
		Camera::AddCameraShake(CameraShake(
			0.13f,                             // interpIn
			0.0f,                              // duration
			vec3(0.0f, 0.0f, -0.1f),           // positionAmplitude
			vec3(0.0f, 0.0f, 3.4f),            // positionFrequency
			vec3(-4, 0.15f, 0.0f),             // rotationAmplitude
			vec3(-2.0f, 18.8f, 0.0f),          // rotationFrequency
			0.5f,                              // falloff
			CameraShake::ShakeType::SingleWave // shakeType
		));

		attackDelay.AddDelay(1.0f);
	}

	void PerformAttack()
	{
		vec3 startLoc = Camera::position +
			MathHelper::TransformVector(projectileOffset,
				Camera::GetRotationMatrix());

		auto projectiles = Level::Current->FindAllEntitiesWithName("caneProjectile");

		for (auto p : projectiles)
		{
			p->Destroy();
		}

		CaneProjectile* bullet = new CaneProjectile();
		bullet->owner = Player::Instance;
		Level::Current->AddEntity(bullet);

		vec4 offset = vec4(0);

		vec3 endLoc = Position + MathHelper::GetForwardVector(Camera::rotation) * 80.0f + vec3(offset);

		bullet->Speed = 60.f;
		bullet->MaxDistance = 120;
		bullet->Position = startLoc + vec3(offset) * 0.002f;
		bullet->Rotation = MathHelper::FindLookAtRotation(startLoc, endLoc);
		bullet->Start();
		bullet->LoadAssetsIfNeeded();
		bullet->Damage = 60;

		//fireSoundPlayer->Play();
	}

	void LateUpdate() override
	{
		WeaponTool::LateUpdate();

		viewmodel->Visible = !thrown;
	}

	void Serialize(json& target)
	{
		SERIALIZE_FIELD(target, attackDelay);
		SERIALIZE_FIELD(target, SwitchDelay);

		auto viewmodelData = viewmodel->GetAnimationState();
		SERIALIZE_FIELD(target, viewmodelData);
	}

	void Deserialize(json& source)
	{
		DESERIALIZE_FIELD(source, attackDelay);
		DESERIALIZE_FIELD(source, SwitchDelay);

		AnimationState viewmodelData;
		DESERIALIZE_FIELD(source, viewmodelData);
		viewmodel->SetAnimationState(viewmodelData);
	}

	WeaponSlotData GetDefaultData() override
	{
		WeaponSlotData data;

		data.className = "weapon_cane";

		return data;
	}

private:

	// Sweeps the player's collision volume from `from` toward `to`, sliding along any surface it
	// hits instead of stopping dead, and returns the furthest position reachable without
	// penetrating world geometry. A few iterations let it slide around corners in one call.
	vec3 SweepAndSlide(vec3 from, vec3 to, float radius)
	{
		const int maxIterations = 3;
		const float skin = 0.02f; // stay a hair off the surface so we don't re-hit the same plane next iteration

		vec3 currentPos = from;
		vec3 remaining = to - from;

		for (int i = 0; i < maxIterations; i++)
		{
			if (length(remaining) < 0.0001f)
				break;

			vec3 target = currentPos + remaining;

			auto hit = Physics::SphereTrace(currentPos, target, radius, BodyType::World | BodyType::MainBody, {}, { Player::Instance });

			if (!hit.hasHit)
			{
				currentPos = target;
				break;
			}

			vec3 traveled = hit.shapePosition - currentPos;
			currentPos = hit.shapePosition + hit.normal * skin;

			vec3 leftover = remaining - traveled;
			remaining = leftover - hit.normal * dot(leftover, hit.normal);
		}

		return currentPos;
	}

};

REGISTER_ENTITY(weapon_cane, "weapon_cane")
