#include <Actions/MoveBehavior.h>
#include <Actions/MoveRunner.h>
#include <Characters/CombatCharacter.h>

// "Reload": the reload clip of `weapon` on the Overlay channel (Full when the clip moves the whole body - the rifle's
// spin; its legs hand over as the body runs). The weapon's own timer refills the magazine (FirearmData::reloadTime -
// keep it as long as the clip); the move starts it if it is not running and keeps the weapon in the hands. The body
// moves at moveSpeed meanwhile. The body starts the Reload action of its Secondary weapon by itself while that weapon
// reloads (an empty magazine), interrupting its fire move; firing waits for it.
struct ReloadParams : MoveBehaviorParams
{
	WeaponSlot weapon = WeaponSlot::Secondary;
	float moveSpeed = 1.0f;          // locomotion speed scale while reloading

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ReloadParams, weapon, moveSpeed)

	bool Validate(const std::string& path, DataLoadReport& report) const
	{
		if (weapon == WeaponSlot::Count) report.Error(path + ".weapon", "expected Main or Secondary");
		if (moveSpeed < 0.0f || moveSpeed > 1.0f) report.Error(path + ".moveSpeed", "must be in [0, 1]");
		return report.Ok();
	}
};

class ReloadBehavior : public MoveBehavior
{
public:
	bool CanStart(const CombatCharacter& character, const MoveData& move) const override
	{
		const ReloadParams& params = static_cast<const ReloadParams&>(*move.behaviorParams);
		const WeaponSet& weapons = character.GetWeapons();
		const WeaponData* weapon = weapons.GetWeapon(params.weapon);
		if (weapon == nullptr || weapon->kind != WeaponKind::Firearm || weapon->firearm.magazine == 0)
			return false;
		return weapons.IsReloading(params.weapon) || weapons.GetMagazine(params.weapon) < weapon->firearm.magazine;
	}

	void OnStart(MoveContext& context) override
	{
		const ReloadParams& params = context.GetParams<ReloadParams>();
		WeaponSet& weapons = context.Character.GetWeapons();
		weapons.BringOut(params.weapon);
		weapons.StartReload(params.weapon);
	}

	void OnTick(MoveContext& context) override
	{
		context.Character.RequestSpeedScale(context.GetParams<ReloadParams>().moveSpeed);
	}
};

REGISTER_MOVE_BEHAVIOR("Reload", ReloadBehavior, ReloadParams)
