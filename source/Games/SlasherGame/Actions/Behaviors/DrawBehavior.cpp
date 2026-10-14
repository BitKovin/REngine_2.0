#include <Actions/MoveBehavior.h>
#include <Actions/MoveRunner.h>
#include <Characters/CombatCharacter.h>

// "Draw": puts `weapon` in the hands when the move starts - the clip begins with the grab, so the other weapon is
// put away and this one shown at once - and plays the draw clip, usually as an UpperBody Overlay so the legs keep
// running. The body starts the moveset's Draw action by itself once another weapon's holsterDelay ran out.
struct DrawParams : MoveBehaviorParams
{
	WeaponSlot weapon = WeaponSlot::Main;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(DrawParams, weapon)

	bool Validate(const std::string& path, DataLoadReport& report) const
	{
		if (weapon == WeaponSlot::Count) report.Error(path + ".weapon", "expected Main or Secondary");
		return report.Ok();
	}
};

class DrawBehavior : public MoveBehavior
{
public:
	bool CanStart(const CombatCharacter& character, const MoveData& move) const override
	{
		const DrawParams& params = static_cast<const DrawParams&>(*move.behaviorParams);
		const WeaponSet& weapons = character.GetWeapons();
		return weapons.HasWeapon(params.weapon) && weapons.GetActiveSlot() != params.weapon;
	}

	void OnStart(MoveContext& context) override
	{
		context.Character.GetWeapons().SetActiveSlot(context.GetParams<DrawParams>().weapon);
	}
};

REGISTER_MOVE_BEHAVIOR("Draw", DrawBehavior, DrawParams)
