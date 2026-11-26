#pragma once
#include "WeaponBase.h"
#include "../../../Gore/StealthGoreAdapter.h"
#include "../Player.hpp"
#include <Animation.h>
#include <Logger.hpp>
#include <SoundSystem/FmodEventInstance.h>
#include "../../Enemy/IEnemy.h"
#include <ParticleSystems/particle_system_meleeTrail.hpp>
#include <algorithm>

// The sword. Viewmodel animations come from Dark Messiah (sourceAssets/ShooterGame/DarkMessiahArms, exported to
// sword.glb, events in sword.glb.skmm).
//
//   attack (tap)            FastAttack_1 / FastAttack_2, chained
//   attack (hold, release)  Attack_<dir>_In -> _Hold while held -> _Out on release. The direction is the movement input:
//                           A = L, D = R, W = T (thrust), S = B, none = R. It is taken when the charge starts and stays
//                           for the whole attack, whatever the player does with the movement keys afterwards. Releasing
//                           before the wind-up is CommitFraction done cancels the charge.
//   attack in the air       while falling: FallAttack_In -> _Hold until the landing -> _Out, a heavy blow at the landing
//   block (hold)            Block_In -> held, Block_Out on release; the first moments are the parry window. A parry (or a
//                           block) plays the Block_A reaction, a parry opens a counter attack for CounterWindow seconds.
//
// What is dangerous when is authored in the animation events (build_events.py): strike_start / strike_end open and close the
// damage window, recover says a new action may start (what is left of the clip blends into it). In the window the blade
// (trail_start .. trail_end bones of the animated viewmodel) is swept through the world every frame; every victim is hit
// once per swing.
class weapon_sword : public Weapon
{
public:

	// ── Tuning ──────────────────────────────────────────────────────────────

	static constexpr float FastDamage = 30.0f;
	static constexpr float HeavyDamage = 60.0f;
	static constexpr float FallDamage = 70.0f;               // at FallReferenceSpeed
	static constexpr float FallReferenceSpeed = 8.0f;        // m/s of the fall that deals FallDamage
	static constexpr float FallDamageRange[2] = { 0.6f, 2.0f };    // damage factor limits for slow / very fast falls
	static constexpr float CounterMultiplier = 3.0f;
	static constexpr float ImpulsePerDamage = 14.0f;

	static constexpr float BlendIn = 0.25f;                   // fade into a clip: Dark Messiah's own (the events assume it)
	static constexpr float FirstFastExitBlend = 0.5f;        // FastAttack_1 -> Idle / Walk when no second attack follows
	static constexpr float BlockBlend = 0.1f;               // raising and lowering the guard answers the input quicker
	static constexpr float StrikeBlend = 0.1f;               // the release of a charge: the blow has to be on screen when its window opens
	static constexpr float ChargeThreshold = 0.18f;          // s the attack button is held before it is a charge, not a tap
	static constexpr float CommitFraction = 0.7f;            // part of the wind-up after which a release still strikes
	static constexpr float TapBuffer = 0.45f;                // s a tap waits for the weapon to become free
	static constexpr float FallBuffer = 0.25f;               // s an attack pressed in the air waits for the fall to begin
	static constexpr float ComboGrace = 0.3f;                // s after a fast attack ended that the next one still chains
	static constexpr float CounterWindow = 1.0f;
	static constexpr float DrawDuration = 0.35f;
	static constexpr float DrawLowerAngle = 35.0f;           // degrees the sword starts below the view when drawn
	static constexpr float DirectionDeadZone = 0.3f;
	static constexpr float HorizontalBias = 0.5f;            // |x| of the input from which the attack is a side slash

	static constexpr float BladeRadius = 0.2f;               // m, the sweep's thickness
	static constexpr int BladeSamples = 4;                   // points along trail_start .. trail_end that are swept
	static constexpr float LandingShockRadius = 1.5f;
	static constexpr float LandingShockForward = 0.9f;

	static constexpr float WalkClipSpeed = 7.0f;             // m/s of the player at which Walk plays at its own speed
	static constexpr float RunClipSpeed = 7.5f;
	static constexpr float ClipRateRange[2] = { 0.5f, 1.3f };
	static constexpr float MoveEnterSpeed = 0.8f;            // m/s: Idle -> Walk
	static constexpr float MoveExitSpeed = 0.4f;             // m/s: Walk -> Idle
	static constexpr float MinAirTime = 0.3f;                // s in the air for the landing clip (Jump_Out) to play
	static constexpr float LandingMoveAfter = 0.3f;          // s of Jump_Out before walking on may cut it short

	// ── State ───────────────────────────────────────────────────────────────

	enum class Direction { L, R, T, B };

	enum class Action
	{
		None,        // only the locomotion clips play
		Fast,        // FastAttack_1 / _2
		Charge,      // Attack_<dir>_In, then _Hold
		Heavy,       // Attack_<dir>_Out
		FallIn,
		FallHold,
		FallOut,
		Block,       // guard up (Block_In, held), Block_A as a reaction
		BlockLower   // Block_Out
	};

	enum class Locomotion { Idle, Walk, Run, JumpIn, JumpIdle, JumpOut };

	SkeletalMesh* viewmodel = nullptr;
	SkeletalMesh* arms = nullptr;

	vec3 weaponOffset = vec3(0.0f);

	particle_system_meleeTrail* trail = nullptr;

	SoundPlayer* fireSoundPlayer = nullptr;
	SoundPlayer* hitSoundPlayer = nullptr;

	Action action = Action::None;
	Locomotion locomotion = Locomotion::Idle;

	float clipRate = 1.0f;          // playback speed of the viewmodel (Walk / Run follow the player's speed)
	float lastFrameHide = 0.0f;
	float draw = 0.0f;              // 0..1, the sword rising into the view after it is drawn

	// Attack input
	bool attackDown = false;
	bool chargeBegun = false;       // this press became a charge
	double attackDownTime = 0;
	double tapUntil = 0;
	double fallRequestUntil = 0;
	Delay actionLock;               // no new attack right after a cancelled charge / a lowered guard

	// Current action
	bool recovered = false;         // the recover event of the clip came: a new action may start
	int fastIndex = 0;              // the next fast attack of the chain
	Delay comboWindow;
	Direction chargeDir = Direction::R;     // fixed when the charge starts
	Direction pressDir = Direction::R;      // the steering at the moment the attack button went down
	bool pressHadDirection = false;
	bool chargeReady = false;       // the wind-up is done, the sword is held
	float fallSpeed = 0.0f;         // fastest descent since the fall attack began, m/s
	double lastGroundTime = 0;      // last time the player stood on the ground
	double airTime = 0;             // how long the player was in the air before this frame
	double landedAt = 0;

	// Strike
	bool strikeOpen = false;
	bool bladeKnown = false;
	vec3 bladePrev[BladeSamples] = {};
	std::vector<Entity*> struck;
	bool struckWorld = false;
	float strikeDamage = 0.0f;
	bool strikeIsCounter = false;

	// Block
	bool blockReacting = false;
	Delay blockStartDelay;
	Delay parryWindow;
	Delay parrySpamWindow;
	Delay counterWindow;

	std::string clip;               // name of the clip that plays

	weapon_sword()
	{
		LateUpdateWhenPaused = true;
		SupportsOffhandWeapon = true;
		bobScale = 0.0f;            // the clips carry the walk and run motion
	}

	// ── Weapon interface ────────────────────────────────────────────────────

	bool CanBlockOrParry() const override { return true; }

	bool HasOwnLocomotion() const override { return true; }

	// Both hands hold the sword while the guard is up and in the fall attack: an offhand weapon steps aside
	bool UsesLeftHand() override { return action == Action::Block || action == Action::BlockLower || IsFallAction(); }

	bool SupressOffhandWeapon() override { return UsesLeftHand(); }

	// Prevent weapon switching mid-swing
	bool CanChangeSlot() override
	{
		if (SwitchDelay.Wait()) return false;
		return action == Action::None || action == Action::Block || action == Action::Charge || CanAct();
	}

	WeaponSlotData GetDefaultData() override
	{
		WeaponSlotData data;
		data.className = "weapon_sword";
		return data;
	}

	// ── Lifecycle ───────────────────────────────────────────────────────────

	void Start() override
	{
		fireSoundPlayer = SoundPlayer::Create("event:/Weapons/knife/knife_attack");
		hitSoundPlayer = SoundPlayer::Create("event:/Weapons/knife/knife_hit");

		SwitchDelay.AddDelay(DrawDuration);
		draw = 0.0f;
	}

	void LoadAssets() override
	{
		Weapon::LoadAssets();

		SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/Weapons.bank");

		viewmodel = new SkeletalMesh(owner);
		arms = new SkeletalMesh(owner);

		viewmodel->GravityAlignedRotation = true;
		arms->GravityAlignedRotation = true;

		viewmodel->LoadFromFile("GameData/models/player/weapons/sword/sword.glb");
		viewmodel->TexturesLocation = "GameData/models/player/weapons/sword/sword.glb/";
		Play("Idle", true, 0.0f);
		viewmodel->PreloadAssets();
		viewmodel->IsViewmodel = true;
		Drawables.push_back(viewmodel);

		arms->LoadFromFile(ArmsModelPath);
		arms->IsViewmodel = true;
		Drawables.push_back(arms);

		CheckAnimationData();
	}

	// The damage windows are events of sword.glb.skmm: without them no swing would ever hurt
	void CheckAnimationData()
	{
		for (const char* name : { "Attack_L_Out", "Attack_R_Out", "Attack_T_Out", "Attack_B_Out", "FastAttack_1", "FastAttack_2", "FallAttack_Out" })
		{
			const AnimationData* data = viewmodel->GetAnimationDataFromName(name);
			bool open = false, close = false, recover = false;
			if (data != nullptr)
			{
				for (const auto& event : data->animationEvents)
				{
					open |= event.eventName == "strike_start";
					close |= event.eventName == "strike_end";
					recover |= event.eventName == "recover";
				}
			}
			if (!(open && close && recover))
				Logger::Error("weapon_sword: animation %s lacks strike_start / strike_end / recover events (sword.glb.skmm, build_events.py)", name);
		}
	}

	// ── Helpers ─────────────────────────────────────────────────────────────

	bool IsFallAction() const { return action == Action::FallIn || action == Action::FallHold || action == Action::FallOut; }

	static const char* DirectionName(Direction d)
	{
		switch (d)
		{
		case Direction::L: return "L";
		case Direction::R: return "R";
		case Direction::T: return "T";
		default: return "B";
		}
	}

	static std::string DirectionClip(Direction d, const char* phase)
	{
		return std::string("Attack_") + DirectionName(d) + "_" + phase;
	}

	// Plays a clip. The events its predecessor queued are dropped: they belong to the old clip.
	void Play(const std::string& name, bool loop, float blend)
	{
		clip = name;
		viewmodel->PlayAnimation(name, loop, blend);
		viewmodel->PullAnimationEvents();
	}

	// Plays a clip at its last frame (the held guard)
	void PlayHeld(const std::string& name, float blend)
	{
		Play(name, false, blend);
		viewmodel->SetAnimationTime(viewmodel->GetAnimationDuration());
	}

	static void Shake(float strength, vec2 pitchYaw)
	{
		Camera::AddCameraShake(CameraShake(1.0f, 1.0f, vec3(0), vec3(0), vec3(pitchYaw.x, pitchYaw.y, 0) * strength, vec3(7, 7, 0), 1.0f, CameraShake::ShakeType::SingleWave));
	}

	// A new attack or block may start: nothing runs, or the running action has reached its recover event
	bool CanAct() const
	{
		if (actionLock.waitUntilTime >= Time::GameTime) return false;

		switch (action)
		{
		case Action::None: return true;
		case Action::Fast:
		case Action::Heavy:
		case Action::FallOut: return recovered;
		default: return false;
		}
	}

	bool HasDirectionInput() const { return glm::length(owner->moveInput) >= DirectionDeadZone; }

	Direction PickDirection() const
	{
		const vec2 input = owner->moveInput;

		if (!HasDirectionInput()) return Direction::R;
		if (std::abs(input.x) >= HorizontalBias) return input.x < 0 ? Direction::L : Direction::R;
		return input.y > 0 ? Direction::T : Direction::B;
	}

	// ── Trail ───────────────────────────────────────────────────────────────

	void StopTrail()
	{
		if (trail == nullptr) return;
		trail->StopAll();
		trail->DestroyWithDelay(2.0f);
		trail = nullptr;
	}

	void StartNewTrail()
	{
		StopTrail();

		trail = new particle_system_meleeTrail();
		vec3 trailStart = viewmodel->GetBoneMatrixWorld("trail_start")[3];
		vec3 trailEnd = viewmodel->GetBoneMatrixWorld("trail_end")[3];

		trail->SetTrailTransform(trailStart, trailEnd);
		trail->Start();
		Level::Current->AddEntity(trail);
		trail->LoadAssetsIfNeeded();
	}

	void UpdateTrail()
	{
		if (trail == nullptr) return;

		// Feed the two blade endpoints from the animated skeleton
		vec3 trailStart = viewmodel->GetBoneMatrixWorld("trail_start")[3];
		vec3 trailEnd = viewmodel->GetBoneMatrixWorld("trail_end")[3];

		trail->SetTrailTransform(trailStart, trailEnd);
	}

	// ── Strike ──────────────────────────────────────────────────────────────

	// A new swing: nobody is hit yet
	void ResetSwing(float damage, bool counter = false)
	{
		StopTrail();
		strikeOpen = false;
		bladeKnown = false;
		struck.clear();
		struckWorld = false;
		recovered = false;
		strikeDamage = damage;
		strikeIsCounter = counter;
	}

	void BeginStrike()
	{
		strikeOpen = true;
		bladeKnown = false;
		StartNewTrail();
		fireSoundPlayer->Play();
	}

	void EndStrike()
	{
		strikeOpen = false;
		StopTrail();
	}

	void BladePoints(vec3 (&out)[BladeSamples])
	{
		const vec3 start = viewmodel->GetBoneMatrixWorld("trail_start")[3];
		const vec3 end = viewmodel->GetBoneMatrixWorld("trail_end")[3];

		for (int i = 0; i < BladeSamples; i++)
			out[i] = mix(start, end, (float)i / (float)(BladeSamples - 1));
	}

	void PlayHit(vec3 position)
	{
		hitSoundPlayer->Position = position;
		hitSoundPlayer->Play();
	}

	// One body the blade met. `motion` is how the blade point travelled, `bladeAxis` the blade's direction.
	void Strike(const Physics::HitResult& hit, vec3 motion, vec3 bladeAxis)
	{
		if (hit.entity == nullptr)
		{
			// Level geometry: the blade rings once per swing
			if (!struckWorld)
			{
				struckWorld = true;
				PlayHit(hit.position);
			}
			return;
		}

		if (std::find(struck.begin(), struck.end(), hit.entity) != struck.end()) return;
		struck.push_back(hit.entity);

		const vec3 forward = MathHelper::GetForwardVector(Camera::rotation);
		const float travel = glm::length(motion);
		const vec3 direction = travel > 0.001f ? motion / travel : forward;

		// The plane the blade swept: its axis against its motion (a thrust has none: the camera's up is the fallback)
		vec3 sweepNormal = glm::cross(bladeAxis, direction);
		sweepNormal = glm::length(sweepNormal) > 0.1f ? glm::normalize(sweepNormal) : MathHelper::GetUpVector(Camera::rotation);

		DealDamage(hit, direction, sweepNormal);
	}

	void DealDamage(const Physics::HitResult& hit, vec3 direction, vec3 sweepNormal)
	{
		float damage = strikeDamage;

		if (Player::Instance->powerUpManager.IsPowerUpActive(PowerUpManager::PowerUpType::TripleDamage))
			damage *= 3;

		StealthGore::SetNextMeleeSweep(hit.entity, sweepNormal);
		hit.entity->OnPointDamage(damage, hit.position, direction, hit.hitboxName, owner, this);

		if (hit.hitbody != nullptr)
			Physics::AddImpulseAtLocation(hit.hitbody, direction * damage * ImpulsePerDamage, hit.position);

		PlayHit(hit.position);
		Shake(0.5f, vec2(-3, 0));
	}

	// Every frame of the strike window: each swept point of the blade is traced from where it was to where it is
	void SweepBlade()
	{
		vec3 now[BladeSamples];
		BladePoints(now);

		const vec3 bladeAxis = glm::normalize(now[BladeSamples - 1] - now[0]);
		const std::vector<Body*> ignoreBodies = { Player::Instance->LeadBody };
		const std::vector<Entity*> ignoreEntities = { Player::Instance };

		for (int i = 0; i < BladeSamples; i++)
		{
			const vec3 from = bladeKnown ? bladePrev[i] : now[i];
			const vec3 motion = now[i] - from;

			std::vector<Physics::HitResult> hits = glm::length(motion) > 0.002f
				? Physics::MultiSphereTrace(from, now[i], BladeRadius, BodyType::GroupHitTest, ignoreBodies, ignoreEntities)
				: Physics::MultiSphereOverlap(now[i], BladeRadius, BodyType::GroupHitTest, ignoreBodies, ignoreEntities);

			for (const auto& hit : hits)
				Strike(hit, motion, bladeAxis);
		}

		for (int i = 0; i < BladeSamples; i++)
			bladePrev[i] = now[i];

		bladeKnown = true;
	}

	// The landing of the fall attack hurts everything at the player's feet, whatever the blade does
	void LandingShock()
	{
		const vec3 forward = MathHelper::GetForwardVector(Camera::rotation);
		const vec3 flat = glm::length(vec2(forward.x, forward.z)) > 0.001f ? glm::normalize(vec3(forward.x, 0.0f, forward.z)) : vec3(0, 0, 1);
		const vec3 feet = owner->controller.GetSmoothPosition() - vec3(0, owner->controller.height * 0.5f, 0);
		const vec3 center = feet + vec3(0, 0.5f, 0) + flat * LandingShockForward;

		const std::vector<Body*> ignoreBodies = { Player::Instance->LeadBody };
		const std::vector<Entity*> ignoreEntities = { Player::Instance };

		for (const auto& hit : Physics::MultiSphereOverlap(center, LandingShockRadius, BodyType::GroupHitTest, ignoreBodies, ignoreEntities))
		{
			if (hit.entity == nullptr || std::find(struck.begin(), struck.end(), hit.entity) != struck.end()) continue;
			struck.push_back(hit.entity);

			DealDamage(hit, flat, glm::cross(flat, vec3(0, 1, 0)));
		}
	}

	// ── Fast attacks ────────────────────────────────────────────────────────

	void StartFast(bool counter = false)
	{
		if (counter)
		{
			fastIndex = 0;
			counterWindow = Delay();
			Time::AddTimeScaleEffect(0.3f, 0.1f, true, "weapon", 0.3f, 0.1f);
		}
		else if (!comboWindow.Wait())
		{
			fastIndex = 0;
		}

		const int index = fastIndex;
		fastIndex ^= 1;

		action = Action::Fast;
		ResetSwing(FastDamage * (counter ? CounterMultiplier : 1.0f), counter);
		Play(index == 0 ? "FastAttack_1" : "FastAttack_2", false, BlendIn);

		// The next tap chains while this clip runs and shortly after it
		comboWindow.AddDelay(viewmodel->GetAnimationDuration() + ComboGrace);

		Shake(counter ? 1.5f : 1.0f, vec2(-5, index == 0 ? -5 : 5));
	}

	// ── Charged attacks ─────────────────────────────────────────────────────

	bool TryStartCharge()
	{
		if (!CanAct() || !owner->OnGround()) return false;

		// The steering of the moment, or the one the button went down with when the keys were let go meanwhile
		action = Action::Charge;
		chargeDir = HasDirectionInput() || !pressHadDirection ? PickDirection() : pressDir;
		chargeReady = false;
		ResetSwing(0.0f);
		Play(DirectionClip(chargeDir, "In"), false, BlendIn);
		return true;
	}

	// The wind-up runs to its end, then the held pose (its clip is a few frames, it stays on its last one)
	void UpdateCharge()
	{
		if (!chargeReady && !viewmodel->IsAnimationPlaying())
		{
			chargeReady = true;
			Play(DirectionClip(chargeDir, "Hold"), false, 0.05f);
		}
	}

	void CancelCharge()
	{
		actionLock.AddDelay(0.15);
		EndAction();
	}

	void ReleaseCharge()
	{
		const float length = std::max(viewmodel->GetAnimationDuration(), 0.001f);
		const float progress = chargeReady ? 1.0f : viewmodel->GetAnimationTime() / length;

		if (progress < CommitFraction)
		{
			CancelCharge();
			return;
		}

		action = Action::Heavy;
		ResetSwing(HeavyDamage);
		Play(DirectionClip(chargeDir, "Out"), false, StrikeBlend);

		switch (chargeDir)
		{
		case Direction::L: Shake(1.2f, vec2(-4, -6)); break;
		case Direction::R: Shake(1.2f, vec2(-4, 6)); break;
		case Direction::T: Shake(1.2f, vec2(-8, 0)); break;
		default: Shake(1.2f, vec2(4, 0)); break;
		}
	}

	// ── Fall attack ─────────────────────────────────────────────────────────

	bool TryStartFall()
	{
		if (!CanAct() || owner->OnGround()) return false;
		if (owner->controller.GetVelocity().y > -0.5f) return false;       // only while falling

		action = Action::FallIn;
		fallSpeed = 0.0f;
		ResetSwing(0.0f);
		Play("FallAttack_In", false, BlendIn);
		return true;
	}

	void UpdateFall()
	{
		fallSpeed = std::max(fallSpeed, -owner->controller.GetVelocity().y);

		if (owner->OnGround())
		{
			StartFallOut();
		}
		else if (action == Action::FallIn && !viewmodel->IsAnimationPlaying())
		{
			action = Action::FallHold;
			Play("FallAttack_Hold", false, 0.0f);           // a two-frame clip: it stays on its last frame
		}
	}

	void StartFallOut()
	{
		const float factor = std::clamp(fallSpeed / FallReferenceSpeed, FallDamageRange[0], FallDamageRange[1]);

		action = Action::FallOut;
		ResetSwing(FallDamage * factor);
		Play("FallAttack_Out", false, BlendIn);

		LandingShock();
		Shake(1.5f, vec2(6, 0));
	}

	// ── Block / parry ───────────────────────────────────────────────────────

	void StartBlock()
	{
		action = Action::Block;
		blockReacting = false;
		ResetSwing(0.0f);

		Play("Block_In", false, BlockBlend);

		blockStartDelay.AddDelay(0.2f);

		if (parrySpamWindow.Wait() == false)
			parryWindow.AddDelay(0.5);
	}

	void EndBlock()
	{
		parrySpamWindow.AddDelay(0.4);

		action = Action::BlockLower;
		Play("Block_Out", false, BlockBlend);
		actionLock.AddDelay(0.1);
	}

	void UpdateBlock()
	{
		// After a reaction the guard goes back to the held pose
		if (blockReacting && !viewmodel->IsAnimationPlaying())
		{
			blockReacting = false;
			PlayHeld("Block_In", BlockBlend);
		}
	}

	void BlockReaction(float blend)
	{
		blockReacting = true;
		Play("Block_A", false, blend);
	}

	// An enemy attack landed in the guard's parry window
	void OnParried() override
	{
		SoundPlayer::PlayOneshot("event:/Weapons/cane/cane_parry", 1.0f, 1.0f, false);
		Time::AddTimeScaleEffect(0.3f, 0.1f, true, "parry", 0.24f, 0.1f);

		if (action == Action::Block)
			BlockReaction(0.05f);

		Shake(1.0f, vec2(-4, 0));

		// The opening: the next attack is a counter
		counterWindow.AddDelay(CounterWindow);
	}

	// An enemy attack landed in the raised guard
	void OnBlocked() override
	{
		SoundPlayer::PlayOneshot("event:/Weapons/knife/knife_attack", 2.0f, 1.0f, false);

		if (action == Action::Block)
			BlockReaction(0.05f);

		Shake(0.6f, vec2(-3, 0));
	}

	// ── Locomotion ──────────────────────────────────────────────────────────

	void SetLocomotion(Locomotion next, float blend = BlendIn)
	{
		locomotion = next;

		switch (next)
		{
		case Locomotion::Idle: Play("Idle", true, blend); break;
		case Locomotion::Walk: Play("Walk", true, blend); break;
		case Locomotion::Run: Play("Run", true, blend); break;
		case Locomotion::JumpIn: Play("Jump_In", false, blend); break;
		case Locomotion::JumpIdle: Play("Jump_Idle", true, blend); break;
		case Locomotion::JumpOut: Play("Jump_Out", false, blend); landedAt = Time::GameTime; break;
		}
	}

	float GroundSpeed() const
	{
		const vec3 velocity = owner->controller.GetVelocity();
		return glm::length(vec2(velocity.x, velocity.z));
	}

	// The clip that fits the player on the ground
	Locomotion GroundLocomotion(float speed) const
	{
		if (owner->RunProgress > 0.5f && speed > 1.0f) return Locomotion::Run;

		const float threshold = locomotion == Locomotion::Walk ? MoveExitSpeed : MoveEnterSpeed;
		return speed > threshold ? Locomotion::Walk : Locomotion::Idle;
	}

	// Leaving an action: the clip that fits what the player is doing now
	void EnterLocomotion(float blend = BlendIn)
	{
		if (owner->OnGround())
			SetLocomotion(GroundLocomotion(GroundSpeed()), blend);
		else
			SetLocomotion(Locomotion::JumpIdle, blend);
	}

	void UpdateLocomotion()
	{
		const bool onGround = owner->OnGround();
		const float speed = GroundSpeed();
		const bool inAir = locomotion == Locomotion::JumpIn || locomotion == Locomotion::JumpIdle;

		if (!onGround)
		{
			if (!inAir)
			{
				SetLocomotion(Locomotion::JumpIn);
			}
			else if (locomotion == Locomotion::JumpIn && !viewmodel->IsAnimationPlaying())
			{
				SetLocomotion(Locomotion::JumpIdle);
			}
		}
		else if (inAir)
		{
			// Landed: a real jump ends in the landing clip, a hop just goes on
			if (airTime >= MinAirTime)
				SetLocomotion(Locomotion::JumpOut);
			else
				SetLocomotion(GroundLocomotion(speed));
		}
		else if (locomotion == Locomotion::JumpOut)
		{
			const bool moving = speed > MoveEnterSpeed && Time::GameTime - landedAt > LandingMoveAfter;

			if (!viewmodel->IsAnimationPlaying() || moving)
				SetLocomotion(GroundLocomotion(speed));
		}
		else
		{
			const Locomotion wanted = GroundLocomotion(speed);

			if (wanted != locomotion)
				SetLocomotion(wanted);
		}

		// Walk and Run follow the player's pace
		if (locomotion == Locomotion::Walk)
			clipRate = std::clamp(speed / WalkClipSpeed, ClipRateRange[0], ClipRateRange[1]);
		else if (locomotion == Locomotion::Run)
			clipRate = std::clamp(speed / RunClipSpeed, ClipRateRange[0], ClipRateRange[1]);
		else
			clipRate = 1.0f;
	}

	// ── Actions ─────────────────────────────────────────────────────────────

	void EndAction()
	{
		// The first fast attack ends far from the idle pose (blade across the body): it takes longer to settle
		const float blend = clip == "FastAttack_1" ? FirstFastExitBlend : BlendIn;

		EndStrike();
		action = Action::None;
		recovered = false;
		EnterLocomotion(blend);
	}

	// An action whose clip played to its end is over (Block is open-ended)
	void UpdateAction()
	{
		switch (action)
		{
		case Action::Charge:
			UpdateCharge();
			break;

		case Action::FallIn:
		case Action::FallHold:
			UpdateFall();
			break;

		case Action::Block:
			UpdateBlock();
			break;

		case Action::Fast:
		case Action::Heavy:
		case Action::FallOut:
			// The swing is over (recover): a player who walks, runs or jumps on gets the locomotion clip, one who stands
			// still lets the clip play out to the idle pose
			if (!viewmodel->IsAnimationPlaying() || (recovered && (!owner->OnGround() || GroundSpeed() > MoveEnterSpeed)))
				EndAction();
			break;

		case Action::BlockLower:
			if (!viewmodel->IsAnimationPlaying())
				EndAction();
			break;

		default:
			break;
		}

		if (action != Action::None)
			clipRate = 1.0f;
	}

	void PullEvents()
	{
		for (const auto& event : viewmodel->PullAnimationEvents())
		{
			if (event.eventName == "strike_start" && action != Action::None)
				BeginStrike();
			else if (event.eventName == "strike_end")
				EndStrike();
			else if (event.eventName == "recover")
				recovered = true;
		}
	}

	// ── Input ───────────────────────────────────────────────────────────────

	void HandleInput()
	{
		const double now = Time::GameTime;
		const bool onGround = owner->OnGround();
		InputAction* attackInput = Input::GetAction("attack");
		InputAction* blockInput = Input::GetAction("block");

		// Guard: hold to raise, release to lower. A charge in progress gives way to it.
		const bool blockWanted = blockInput->Holding();

		if (blockWanted && action != Action::Block && !IsFallAction() && (CanAct() || action == Action::Charge))
		{
			attackDown = false;
			tapUntil = 0;
			StartBlock();
		}
		else if (!blockWanted && action == Action::Block)
		{
			EndBlock();
		}

		// Attack
		if (attackInput->Pressed())
		{
			pressHadDirection = HasDirectionInput();
			pressDir = PickDirection();

			if (!onGround)
			{
				fallRequestUntil = now + FallBuffer;
			}
			else if (counterWindow.Wait() && (action == Action::Block || CanAct()))
			{
				if (action == Action::Block)
					EndBlockToIdle();
				StartFast(true);
			}
			else if (action != Action::Block)
			{
				attackDown = true;
				attackDownTime = now;
				chargeBegun = false;
			}
		}

		if (attackDown)
		{
			if (!attackInput->Holding())
			{
				attackDown = false;

				if (action == Action::Charge)
					ReleaseCharge();
				else if (!chargeBegun)
					tapUntil = now + TapBuffer;
			}
			else if (!chargeBegun && now - attackDownTime >= ChargeThreshold && action != Action::Block)
			{
				chargeBegun = TryStartCharge();
			}
		}

		if (tapUntil > now && action != Action::Block && CanAct())
		{
			tapUntil = 0;
			StartFast();
		}

		if (fallRequestUntil > now && TryStartFall())
			fallRequestUntil = 0;
	}

	// The counter attack starts straight from the raised guard
	void EndBlockToIdle()
	{
		parrySpamWindow.AddDelay(0.4);
		action = Action::None;
	}

	// ── Update ──────────────────────────────────────────────────────────────

	void Update() override
	{
		if (owner == nullptr) return;

		draw = std::min(1.0f, draw + Time::DeltaTimeF / DrawDuration);

		if (owner->dead)
		{
			EndStrike();
			attackDown = false;
			Parrying = false;
			Blocking = false;
			return;
		}

		airTime = Time::GameTime - lastGroundTime;
		if (owner->OnGround())
			lastGroundTime = Time::GameTime;

		PullEvents();
		HandleInput();
		UpdateAction();

		if (action == Action::None)
			UpdateLocomotion();

		if (strikeOpen)
			SweepBlade();

		Parrying = action == Action::Block && parryWindow.Wait();
		Blocking = (action == Action::Block && blockStartDelay.Wait() == false) || parrySpamWindow.Wait();
	}

	void AsyncUpdate() override
	{
		viewmodel->Update(clipRate);

		// The left arm belongs to the sword's animation unless an offhand weapon has taken it (HideWeapon blends towards 1)
		const float hide = std::max(HideWeapon, lastFrameHide);
		lastFrameHide = HideWeapon;

		auto pose = viewmodel->GetAnimationPose();
		auto leftHandPose = pose.GetBoneTransform("clavicle_l");
		leftHandPose.Rotation += vec3(120, 0, 0) * hide;

		leftHandPose.Scale *= mix(vec3(1.0f), vec3(0.0f), hide);

		pose.SetBoneTransformEuler("clavicle_l", leftHandPose);
		arms->PasteAnimationPose(pose);
	}

	void LateUpdate() override
	{
		viewmodel->Position = Position + (mat3)Camera::GetRotationMatrix() * weaponOffset;
		viewmodel->Rotation = Rotation;

		// Drawn: the sword rises into the view
		const float rise = draw * draw * (3.0f - 2.0f * draw);
		viewmodel->Rotation.x += (1.0f - rise) * DrawLowerAngle;

		arms->Position = viewmodel->Position;
		arms->Rotation = viewmodel->Rotation;

		UpdateTrail();
	}
};

REGISTER_ENTITY(weapon_sword, "weapon_sword")
