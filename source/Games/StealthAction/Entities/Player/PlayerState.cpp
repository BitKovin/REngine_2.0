#include "PlayerState.h"
#include "Player.hpp"
#include "Weapons/WeaponFirearm.h"

PlayerState PlayerState::FromPlayerPtr(Player* player)
{
    PlayerState state;

    state.position      = player->bodyMesh->Position;
    state.position.y    = player->Position.y;
    state.rotation      = player->Rotation;
    state.cameraRotation = player->cameraRotation;
    state.velocity      = player->controller.GetVelocity();
    state.playerHeight  = player->controller.isCrouched
        ? player->controller.crouchHeight
        : player->controller.height;

    state.crouching = player->controller.isCrouched;

    if (player->currentWeapon)
    {
        WeaponFirearm* fw = dynamic_cast<WeaponFirearm*>(player->currentWeapon);
        state.weaponRModelPath = fw ? fw->params.modelPathTp
                                    : player->currentWeapon->thirdPersonModelPath;
        state.weaponRHandlingType = player->currentWeapon->weaponHandlingType;

        // weaponL only ever mirrors weaponR now, for akimbo (two copies of
        // the same firearm) - there's no standalone offhand weapon anymore,
        // currentWeapon is the only weapon a player can hold.
        if (fw && fw->akimbo)
        {
            state.weaponRHandlingType = 2;
            state.weaponLModelPath = state.weaponRModelPath;
        }

    }

    return state;
}
