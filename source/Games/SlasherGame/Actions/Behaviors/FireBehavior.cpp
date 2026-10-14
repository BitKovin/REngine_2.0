#include <Actions/MoveBehavior.h>
#include <Actions/MoveRunner.h>
#include <Characters/CombatCharacter.h>

// "Fire": firearm, usually on the Overlay channel so the body can run while shooting. Fires on start and then
// at the weapon's fire rate while the button is held (automatic) or once per press (semi-automatic). The body
// faces the aim direction (strafes) and moves at moveSpeed while the move runs; the move sustains while held.
// With restartOnShot every shot plays the clip from the start - clip n for the weapon's n-th muzzle (0 = clip,
// n = variants[n - 1]), so akimbo shots alternate their recoil clips.
struct FireParams : MoveBehaviorParams
{
	WeaponSlot weapon = WeaponSlot::Secondary;
	InputButton button = InputButton::Secondary;
	bool automatic = true;
	float aimTurnRate = 1080.0f;     // deg/s toward the aim direction
	bool restartOnShot = false;
	float moveSpeed = 1.0f;          // locomotion speed scale while firing

	friend void to_json(nlohmann::json& j, const FireParams& p)
	{
		j = nlohmann::json{ { "weapon", p.weapon }, { "button", InputButtonNames[static_cast<size_t>(p.button)] },
			{ "automatic", p.automatic }, { "aimTurnRate", p.aimTurnRate }, { "restartOnShot", p.restartOnShot },
			{ "moveSpeed", p.moveSpeed } };
	}

	friend void from_json(const nlohmann::json& j, FireParams& p)
	{
		const FireParams defaults;
		p.weapon = j.value("weapon", defaults.weapon);
		const int button = FindSlotIndex(InputButtonNames, j.value("button", std::string(InputButtonNames[static_cast<size_t>(defaults.button)])));
		p.button = button < 0 ? InputButton::Count : static_cast<InputButton>(button);
		p.automatic = j.value("automatic", defaults.automatic);
		p.aimTurnRate = j.value("aimTurnRate", defaults.aimTurnRate);
		p.restartOnShot = j.value("restartOnShot", defaults.restartOnShot);
		p.moveSpeed = j.value("moveSpeed", defaults.moveSpeed);
	}

	bool Validate(const std::string& path, DataLoadReport& report) const
	{
		if (weapon == WeaponSlot::Count) report.Error(path + ".weapon", "expected Main or Secondary");
		if (button == InputButton::Count) report.Error(path + ".button", "unknown button");
		if (moveSpeed < 0.0f || moveSpeed > 1.0f) report.Error(path + ".moveSpeed", "must be in [0, 1]");
		return report.Ok();
	}
};

class FireBehavior : public MoveBehavior
{
public:
	bool CanStart(const CombatCharacter& character, const MoveData& move) const override
	{
		const FireParams& params = static_cast<const FireParams&>(*move.behaviorParams);
		const WeaponData* weapon = character.GetWeapons().GetWeapon(params.weapon);
		return weapon != nullptr && weapon->kind == WeaponKind::Firearm;
	}

	void OnStart(MoveContext& context) override
	{
		const FireParams& params = context.GetParams<FireParams>();
		lastShotPressId = context.Character.GetInput().Get(params.button).pressId;

		// A weapon drawn by this move has no muzzle in the hands until the next pose: the first shot waits a frame.
		WeaponSet& weapons = context.Character.GetWeapons();
		pendingShot = weapons.GetActiveSlot() != params.weapon;
		weapons.SetActiveSlot(params.weapon);
		if (!pendingShot)
			TryShoot(context, params);
	}

	void OnTick(MoveContext& context) override
	{
		CombatCharacter& character = context.Character;
		const FireParams& params = context.GetParams<FireParams>();

		character.RequestFacing(character.GetInput().aimDirection, params.aimTurnRate);
		character.RequestSpeedScale(params.moveSpeed);

		if (pendingShot)
		{
			pendingShot = false;
			TryShoot(context, params);
			return;
		}

		const ButtonState& button = character.GetInput().Get(params.button);
		const bool wantsShot = params.automatic ? button.held : button.pressId != lastShotPressId;
		if (wantsShot)
		{
			lastShotPressId = button.pressId;
			TryShoot(context, params);
		}
	}

	bool Sustains(const MoveContext& context) const override
	{
		const FireParams& params = context.GetParams<FireParams>();
		return context.Character.GetInput().Get(params.button).held;
	}

private:
	uint32_t lastShotPressId = 0;
	bool pendingShot = false;

	static void TryShoot(MoveContext& context, const FireParams& params)
	{
		CombatCharacter& character = context.Character;
		WeaponSet& weapons = character.GetWeapons();

		if (weapons.GetMagazine(params.weapon) == 0)
		{
			weapons.StartReload(params.weapon);
			return;
		}

		const int muzzle = weapons.Fire(params.weapon, character.GetAimPoint());
		if (muzzle >= 0 && params.restartOnShot)
			context.Runner.RestartClip(context.Move, muzzle);
	}
};

REGISTER_MOVE_BEHAVIOR("Fire", FireBehavior, FireParams)
