#include <Actions/MoveBehavior.h>
#include <Actions/MoveRunner.h>
#include <Characters/CombatCharacter.h>

// "Sheathe": a sheathable Main weapon (WeaponData::sheathedHands) goes back into its sheath - usually an UpperBody
// Overlay so the legs keep running. The move's cue with `sheathe: 1` switches the meshes on the frame the blade is in;
// interrupted before (an attack, firing), the weapon stays drawn and the body plays the move again once it is free.
// The body starts the Sheathe action of its Main weapon by itself once the weapon has nothing left to do.
class SheatheBehavior : public MoveBehavior
{
public:
	bool CanStart(const CombatCharacter& character, const MoveData& move) const override
	{
		(void)move;
		const WeaponSet& weapons = character.GetWeapons();
		const WeaponData* weapon = weapons.GetWeapon(WeaponSlot::Main);
		return weapon != nullptr && weapon->CanSheathe() && !weapons.IsSheathed() && weapons.IsInHands(WeaponSlot::Main);
	}
};

REGISTER_MOVE_BEHAVIOR("Sheathe", SheatheBehavior, NoMoveParams)
