#pragma once

#include <Entity.h>

#include <Input.h>

#include <MathHelper.hpp>

#include <Camera.h>

#include <Physics.h>

#include <DebugDraw.hpp>

#include <algorithm>   // for std::clamp
#include <cmath>       // for std::max
#include <functional>  // for std::function (inventory callbacks)
#include <array>       // for weapon role slot arrays

#include <Navigation/Navigation.hpp>

#include <SkeletalMesh.hpp>

#include <imgui/imgui.h>

#include <Particle/ParticleSystem.hpp>

#include <Entities/SoundPlayer.h>
#include <SoundSystem/SoundManager.hpp>

#include <Character/CharacterController.h>
#include <Particle/GlobalParticleSystem.hpp>

#include <AiPerception/ObservationTarget.h>

#include "../../UI/Player/PlayerHud.hpp"

#include "Weapons/WeaponBase.h"

#include "PlayerBodyAnimator.h"

#include <ItemsDataBase.h>

#include <InteractionSystem/IInteractive.h>

#include <Entities/Pickup/Keys/KeyTypes.h>

#include <Entities/PointLight.h>

#include <Analytics/AnalyticsSystem.h>
#include <Helpers/StringHelper.h>

#include <Systems/PowerUpSystem/PowerUpManager.hpp>


// Forward declaration for custom item logic
class Player;


// Inventory system structures
struct InventoryItem
{
	std::string itemID;              // Unique item identifier for item database lookup

	std::string uid;                 // Unique instance ID (for tracking specific instances of items, if needed)

	// Persisted weapon state (ammo, className, etc.) for Firearm/Melee/Tool
	// items - unused by CustomLogic items.
	WeaponSlotData weaponData;

	int stackSize = 1;               // Number of items in stack (for stackable items)

	InventoryItem() = default;

	InventoryItem(const std::string& itemID, int stackSize = 1)
		: itemID(itemID), stackSize(stackSize)
	{}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(InventoryItem, itemID, uid, weaponData, stackSize)

};

// ── Movement state machine ────────────────────────────────────────────────────
// Single source of truth for the player's current locomotion mode.
// UpdateWalkMovement() dispatches to a per-state update function each frame.
// Transitions are always explicit: call Enter*/Exit* helpers, never set directly.
enum class MoveState
{
	Default,   // Ground walk, air, wall-jump — the "normal" movement bucket
	Sliding,   // Crouch-slide; shares setup code with Default via UpdateStateGroundAir
	Mantling,  // Ledge pull-up animation; blocks all other input
	OnLadder,  // Ladder climb; constant vertical speed, weapon hidden
};

class Player : public Entity
{

private:

	std::map<WeaponAmmoType, int> ammoCounts = { {WeaponAmmoType::None,0},
												{WeaponAmmoType::PistolBullets,0},
												{WeaponAmmoType::ShotgunShells,0},
												{WeaponAmmoType::CannonBullets,0} };
	std::map<WeaponAmmoType, int> ammoLimits = { {WeaponAmmoType::None,0},
												{WeaponAmmoType::PistolBullets,96},
												{WeaponAmmoType::ShotgunShells,48},
												{WeaponAmmoType::CannonBullets,12} };


	int numTouchingLadders = 0;

	PointLight* playerLight = nullptr;

	float maxSpeed = 8.0f;
	float maxSpeedAir = 2;
	float acceleration = 90;
	float airAcceleration = 10;

	vec3 velocity = vec3(0);

	bool canRun = false;
	bool canDash = true;
	bool canMantle = false;

	vec3 oldPos = vec3();



	bool freeFly = false;


	float bobProgress = 0;
	float bobSpeed = 1.1f;


	SkeletalMesh* bikeMesh = nullptr;

	SkeletalMesh* armsMesh = nullptr;

	bool on_bike = false;

	float cameraHeightOffset = 0;

	Delay stepDelay = Delay();

	float bike_progress = 0;

	vec3 stepForceWalkDirection = vec3();

	Delay afterStepDelay;

	Delay coyoteTime;



	bool teleported = false;

	Delay dashProgress;
	vec3 dashVector = vec3(0);
	bool wasDashing = false;

	std::shared_ptr<ObservationTarget> observationTarget;

	Body* hitbox = nullptr;

	int CurrentMaxRestrictionLevel = 0;
	int CurrentClearance = 0;

	// ── Weapon role system ────────────────────────────────────────────────────
	// See Weapons/WeaponBase.h for WeaponRole (Firearm/Melee/Tool). Each role
	// has up to WeaponRoleSlotCount carried items; at most one role is ever
	// "current" (currentWeapon) at a time. Real-time input (attack2/attack/
	// block/useTool) drives which role is current via UpdateWeaponRoleInput();
	// the inventory wheel drives which *item* occupies each role's active
	// slot via SwitchToInventoryItem().
public:
	static constexpr int WeaponRoleSlotCount = 3;
private:
	// Melee always needs to be equippable even with nothing carried yet (no
	// fists model exists), so this is the hardcoded always-available
	// fallback - spawned directly (currentWeaponUUID stays "") rather than
	// looked up from inventory.
	static constexpr const char* FallbackMeleeClassName = "weapon_twinsword";

	std::array<std::string, WeaponRoleSlotCount> firearmSlotUUID = { "", "", "" };
	std::array<std::string, WeaponRoleSlotCount> meleeSlotUUID = { "", "", "" };
	std::array<std::string, WeaponRoleSlotCount> toolSlotUUID = { "", "", "" };

	int activeFirearmSlot = -1; // which of the 3 above is "the" firearm/melee/tool right now (-1 = none)
	int activeMeleeSlot = -1;
	int activeToolSlot = -1;

	std::array<std::string, WeaponRoleSlotCount>& SlotsForRole(WeaponRole role);
	int& ActiveSlotForRole(WeaponRole role);

	// Finds `uuid` among the 3 slots for `role`; if it's not there yet,
	// assigns it to the first empty slot. Returns the slot index, or -1 if
	// every slot was already full of something else (caller decides whether
	// to overwrite the active slot in that case - see SwitchToInventoryItem).
	int AssignItemToRoleSlots(const std::string& uuid, WeaponRole role);

	// Lazy role switch: mirrors desiredInventoryUUID/pendingInventorySwitch
	// below, but for the real-time input-driven role switches handled by
	// UpdateWeaponRoleInput() (as opposed to explicit wheel selection).
	WeaponRole desiredWeaponRole = WeaponRole::None;
	bool pendingWeaponRoleSwitch = false;

	// Tool auto-return: which role to switch back to once the tool's
	// CanChangeSlot() allows it. WeaponRole::None if there was nothing
	// equipped before the tool (stays empty-handed after use).
	WeaponRole roleBeforeTool = WeaponRole::None;

	// Equips whichever item occupies ActiveSlotForRole(role) as currentWeapon.
	// Respects currentWeapon->CanChangeSlot() - if the current weapon can't
	// be switched away from yet, this just arms desiredWeaponRole/
	// pendingWeaponRoleSwitch for UpdateWeaponRoleInput() to retry next frame,
	// exactly like SwitchToInventoryItem's lazy switch. Melee falls back to
	// FallbackMeleeClassName when its slot is empty; Firearm/Tool do nothing
	// when empty (nothing to equip).
	void TryEquipRole(WeaponRole role, bool forceChange = false);

	// Reads attack2 (hold = want Firearm), attack/block (want Melee),
	// useTool (want Tool + remembers roleBeforeTool), and hideWeapon
	// (RequestHide() on currentWeapon). Drives currentWeaponRole switches and
	// the Tool auto-return via TryEquipRole(). Call once per Update().
	void UpdateWeaponRoleInput();

	// Double-press-the-same-active-item handling for SwitchToInventoryItem:
	// Firearm -> hides it in place (RequestHide()); Melee -> clears the slot
	// and falls back to FallbackMeleeClassName; Tool -> clears the slot and
	// returns to roleBeforeTool. Returns true if it handled the press (caller
	// should stop, no further switch needed).
	bool HandleReselectSameActiveItem(const std::string& uuid, WeaponRole role, int slotIndex);

	// Inventory system
	std::vector<InventoryItem> inventory;

	std::string lastInventoryUUID = "";      // Previously equipped inventory item UUID (for quick switch)
	bool pendingInventorySwitch = false;

	Delay mantleDelay;
	vec3 mantleStartPosition;
	vec3 mantleTargetPosition;
	// Ledge height range relative to the player's current feet position.
	static constexpr float MantleMinLedgeHeight = 0.3f;   // units above feet
	static constexpr float MantleMaxLedgeHeight = 2.5f;   // units above feet
	// How far in front of the player to search for a wall face.
	static constexpr float MantleForwardReach = 1.0f;
	// Minimum free vertical space above the ledge surface for the player to stand.
	static constexpr float MantleStandClearance = 0.15f;
	// Total seconds for the full pull-up + vault-over animation.
	static constexpr float MantleDuration = 1.050f;
	// Seconds before another TryMantle() call is allowed.
	static constexpr float MantleCooldown = 0.35f;
	// 0 → 1 over MantleDuration seconds, drives the two-phase easing curve.
	float mantleProgress = 0.0f;
	// Capsule-center the player is teleported to at mantle start
	vec3  mantleSnapPosition = vec3(0);

	vec3 weaponRunRotation = vec3(-8.9f, 30.0f, -9.21f);
	vec3 weaponSlideRotation = vec3(0, 0, -16);
	vec3 runRotatePoint = vec3(-0.05, -0.1, 0.45);// vec3(-0.1f, -0.290f, 0.45f);

	float slideInterp = 0;

	float WalkSpeed = 7.0f;// 4.5f;
	float CrouchSpeed = 2.5f;
	float RunSpeed = 7.5f;

	// ── Weapon suppression ────────────────────────────────────────────────────
	// True while CanHoldWeapon() == false.  The weapon object is destroyed but
	// all UUID / slot / role state is preserved so RestoreWeapons() can
	// rebuild it correctly.
	bool weaponSuppressed = false;

	// Records whether a live weapon object existed at the moment of
	// suppression, so RestoreWeapons() doesn't recreate an empty-handed state.
	bool weaponWasSuppressed = false;

	// Which role to restore - snapshotted explicitly rather than relying on
	// currentWeaponRole surviving DestroyWeapon() (it doesn't - DestroyWeapon
	// always clears it back to None).
	WeaponRole suppressedWeaponRole = WeaponRole::None;

	// Returns false if the live weapon's CanChangeSlot() blocks removal this frame.
	// TrySuppressWeapons() polls this and is retried next frame if it returns false.
	bool CanSuppressWeapons() const;

	// Destroys the live weapon object, recording whether it was alive.
	// Returns false (and leaves everything untouched) when CanSuppressWeapons()
	// returns false — the caller should retry next frame.
	// Pass forceSuppress = true (mantle / death) to skip CanChangeSlot checks
	// and destroy the weapon immediately regardless of its current state.
	bool TrySuppressWeapons(bool forceSuppress = false);

	// Recreates the weapon object from preserved role / slot / UUID state.
	void RestoreWeapons();

	glm::vec3 Friction(glm::vec3 vel, float factor = 60.0f) {
		vel = MathHelper::XZ(vel);
		float length = glm::length(vel);

		// Avoid division by zero: if length is positive, normalize; otherwise return zero vector.
		glm::vec3 direction = (length > 0.0f) ? glm::normalize(vel) : glm::vec3(0.0f);

		length -= factor * Time::DeltaTimeF;
		length = std::max(0.0f, length);

		return direction * length;
	}

	glm::vec3 UpdateGroundVelocity(glm::vec3 withDir, glm::vec3 vel) {
		vel = MathHelper::XZ(vel);
		vel = Friction(vel, 40);

		// Project current velocity onto the direction
		float currentSpeed = glm::dot(vel, withDir);

		// Clamp the additional speed so that it does not exceed what can be accelerated in the frame.
		float addSpeed = glm::clamp(maxSpeed - currentSpeed, 0.0f, acceleration * Time::DeltaTimeF);

		if (false) {
			if (currentSpeed + addSpeed > maxSpeed)
				addSpeed = maxSpeed - currentSpeed;
		}

		return vel + addSpeed * withDir;
	}

	glm::vec3 UpdateAirVelocity(glm::vec3 wishdir, glm::vec3 vel) {
		vel = MathHelper::XZ(vel);

		float currentSpeed = glm::dot(vel, wishdir);
		float wishspeed = maxSpeedAir;

		float addSpeed = wishspeed - currentSpeed;

		if (addSpeed <= 0.0f) {
			return vel;
		}

		float accelspeed = airAcceleration * Time::DeltaTimeF * wishspeed;

		if (accelspeed > addSpeed) {
			accelspeed = addSpeed;
		}

		return vel + accelspeed * wishdir;
	}

	void TryMantle();
	void StartMantle();
	void FinishMantle(bool isNaturalFinish = true);
	void UpdateMantle();

	void TryWallJump();

	void Jump()
	{

		if (dead)return;

		//ANALYTICS_SEND_EVENT(
		//	"player_jump",
		//	std::unordered_map<std::string, std::string>{
		//		{"position", to_string(Position)},
		//		{ "velocity", to_string(controller.GetVelocity()) },
		//		{ "on_ground", to_string(controller.onGround) },
		//		{ "coyote_time_available", to_string(coyoteTime.Wait()) },
		//		{ "free_walljumps", to_string(freeWalljumps) }
		//}
		//);

		controller.UnCrouch();

		vec3 velocity = controller.GetVelocity();
		velocity.y = 8.0;
		controller.SetVelocity(velocity);

		jumpDelay.AddDelay(0.3);


	}

	bool CheckGroundAt(vec3 location)
	{

		if (jumpDelay.Wait())
			return false;

		if (afterStepDelay.Wait()) return true;

		auto result = Physics::LineTrace(location, location - vec3(0, 0.92, 0), BodyType::GroupCollisionTest, {  });

		return result.hasHit;

	}

	void UpdatePowerUps();

	void SwitchWeapon(const WeaponSlotData& data);

	ItemDbEntry GetItemData(const std::string& itemID);

	vec3 testStart;

	friend class InventoryMenu;

	IInteractive* currentInteractionObject = nullptr;
	bool startedInteracting = false;
	float interactionProgress = 0;

	friend class UseIndicator;

	// ── Movement state machine ────────────────────────────────────────────────
	MoveState moveState = MoveState::Default;

	// ── State predicates (read-only, derived from moveState) ─────────────────
	bool IsMantling()  const { return moveState == MoveState::Mantling; }
	bool IsSliding()   const { return moveState == MoveState::Sliding; }
	bool IsOnLadder()  const { return moveState == MoveState::OnLadder; }

	// ── Per-state update functions ────────────────────────────────────────────
	// UpdateWalkMovement() dispatches to one of these every frame.
	void UpdateStateGroundAir(vec2 input);  // Default + Sliding (shared setup)
	void UpdateStateLadder(vec2 input);     // OnLadder

	// ── Ladder helpers ────────────────────────────────────────────────────────
	// EnterLadder/ExitLadder are the only places that set OnLadder state.
	void EnterLadder(float inputY);
	void TryGrabLadderDeferred(float inputY);
	void ExitLadder();

	// Ladder tuning
	static constexpr float LadderClimbSpeed = 4.0f;   // units/sec up or down
	static constexpr float LadderLookDeadZone = 0.0f;   // degrees of pitch before input activates

	// ── Slide state ───────────────────────────────────────────────────────────
	vec3  slideDir = vec3(0);          // normalized horizontal slide direction
	Delay slideBoostCooldown;          // 2-second cooldown between slide boosts

	bool  wasOnGround = false;
	float airVerticalVelocity = 0.0f;  // vy captured while airborne, consumed on landing

	// Slide tuning constants
	static constexpr float SlideInitialBoost = 1.5f;
	static constexpr float SlideFriction = 4.0f;
	static constexpr float SlopeGravityScale = 1.5f;
	static constexpr float SlideSteerStrength = 0.4f;
	static constexpr float SlideBoostCooldownTime = 2.0f;
	static constexpr float SlideInputAlignment = 0.7f;
	static constexpr float SlideCancelAlignment = -0.6f;
	static constexpr float SlopeTriggerThreshold = 1.5f;
	static constexpr float LandingTransferScale = 0.5f;

	// ── Slope helpers ─────────────────────────────────────────────────────────
	void GetSlopeInfo(const vec3& groundNormal,
		vec3& outDownhillDir,
		float& outNetAccel) const;

	void StartSlide(const vec3& currentVelocity);
	void StopSlide();

	bool ShouldAutoSlide(const vec3& downhillDir, float netSlopeAccel, const vec3& wishDir) const;

	void UpdateSlide(vec2 input, const vec3& downhillDir, float netSlopeAccel);

	void Death();

public:

	PowerUpManager powerUpManager;

	PlayerHud Hud;

	PlayerBodyAnimator bodyAnimator = PlayerBodyAnimator(this);

	std::set<DoorKey> keysInventory;

	bool dead = false;
	Delay deathAnimDelay;

	Delay jumpDelay;

	int freeWalljumps = 1;
	float stamina = 3;
	bool disableStaminaRegenUntilGrounded = false;

	std::string desiredInventoryUUID = "";   // Item player wants to switch to UUID (for lazy switching)
	std::string currentInventoryUUID = "";   // Currently equipped item UUID from inventory (mirrors currentWeaponUUID; kept for UI/back-compat)

	std::string currentWeaponUUID = ""; // inventory uuid behind currentWeapon; "" if none, or if it's the melee fallback (weapon_sword with nothing carried)

	vec3 cameraRotation = vec3(0);

	Weapon* currentWeapon = nullptr;

	CharacterController controller;

	SkeletalMesh* bodyMesh = nullptr;

	std::string currentWeaponType = "";

	WeaponRole currentWeaponRole = WeaponRole::None;

	Delay violanceCrimeActiveDelay;

	float RunProgress = 0;

	bool ThirdPersonView = false;

	bool started = false;

	Player()
	{

		bikeMesh = new SkeletalMesh(this);
		bikeMesh->Visible = false;
		Drawables.push_back(bikeMesh);
		armsMesh = new SkeletalMesh(this);
		Drawables.push_back(armsMesh);

		bodyMesh = new SkeletalMesh(this);
		bodyMesh->TwoSided = true;
		//bodyMesh->CastDetailShadows = false;
		Drawables.push_back(bodyMesh);

		ClassName = "info_player_start";

		SaveGame = true;

		Tags = { "player" };

		Health = 100;

		LateUpdateWhenPaused = true;

		//AddComponent<PlayerTestComponent>();

	}


	~Player() { Logger::Log("player destructor"); }

	float Speed = 5;

	static Player* Instance;

	void FromData(EntityData data)
	{
		Entity::FromData(data);
		Rotation.y = data.GetPropertyFloat("angle") + 90;
	}

	void Start();
	
	void PostLoadStart() override;

	void Heal(float healthToAdd);

	// UpdateWalkMovement is now a thin dispatcher to per-state functions.
	// Add new locomotion modes by adding a MoveState value and an Update* method.
	void UpdateWalkMovement(vec2 input);
	void UpdateBikeMovement(vec2 input);

	// Inventory management
	std::string AddItemToInventory(const std::string& itemID, int stackSize = 1);

	bool RemoveItemFromInventory(const std::string& uuid);
	bool RemoveItemByID(const std::string& itemID);
	InventoryItem* GetInventoryItem(const std::string& uuid);
	InventoryItem* FindInventoryItemByUUID(const std::string& uuid);
	int FindInventoryItemByID(const std::string& itemID);
	const std::vector<InventoryItem>& GetInventory() const { return inventory; }

	int GetInventorySlotIdByUUID(const std::string& uuid);

	std::vector<std::string> GetWeaponQuickSlotUUIDs() const;

	// Inventory weapon switching (with lazy switching support). Handles all
	// four InventoryItemType values - Firearm/Melee/Tool route through the
	// weapon role system (see TryEquipRole); CustomLogic runs its
	// interactionEntityClassname logic and never touches currentWeapon.
	void SwitchToInventoryItem(std::string uuid, bool forceChange = false);
	bool CanSwitchToInventoryItem(const std::string& uuid);
	void UpdateInventoryWeaponSwitch(); // Call in Update() to handle lazy switching


	void CreateWeapon(const string& className);
	void DestroyWeapon();

	int GetAmmoLimit(WeaponAmmoType type);
	int GetAmmo(WeaponAmmoType type);
	int SetAmmo(WeaponAmmoType type, int amount);
	int ConsumeAmmo(WeaponAmmoType type, int amount);
	int AddAmmo(WeaponAmmoType type, int amount);

	vec3 GetBobForMainWeapon();

	IInteractive* UpdateInteractionRaycast();
	void UpdateInteraction();

	void Destroy()
	{
		Entity::Destroy();

		Instance = nullptr;

	}

	bool HasStamina();
	void ConsumeStamina(float amount = 1.0f);
	void UpdateStamina();

	dtObstacleRef playerObstacle = 0;

	void UpdateWeapon();

	void UpdateDebugUI();

	bool OnGround();

	void PerformAttack();

	void TryStep(vec3 dir);

	void Update();
	void AsyncUpdate();
	void LateUpdate();

	void UpdateThirdPersonCamera();

	void UpdateBody();

	bool InThirdPerson();


	void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);
	void OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone, Entity* DamageCauser, Entity* Weapon);

	void UpdateCurrentRestrictedArea();

	void Serialize(json& target);
	void Deserialize(json& source) override;

	void Teleport(vec3 target) override;

	void MoveTo(vec3 target);

	void StartBike();
	void StopBike();
	void ToggleBike();

	void OnLevelEnd();

	void StartedTouchLadder();
	void StoppedTouchLadder();

	// ── Weapon suppression ────────────────────────────────────────────────────

	// Returns false whenever the player must not hold physical weapon objects
	// (mantling, on ladder, on bike, dead, etc.).
	bool CanHoldWeapon() const;

	// Call once per Update(), before UpdateWeapon().
	void UpdateWeaponSuppression();

	// True while weapons are suppressed (objects destroyed, state preserved).
	bool IsWeaponSuppressed() const { return weaponSuppressed; }

	// Expose current locomotion state for external systems (HUD, animation, etc.)
	MoveState GetMoveState() const { return moveState; }

protected:

	void LoadAssets();

};
