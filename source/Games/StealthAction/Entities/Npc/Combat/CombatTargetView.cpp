#include "CombatTargetView.h"

#include "../NpcBase.h"
#include "CombatBehaviorBase.h"

#include <Entities/Player/Player.hpp>
#include <Entities/Player/Weapons/WeaponFirearm.h>

namespace
{
	// NPC positions are body centres (about 0.9 m up); eyes sit near 1.5 m.
	constexpr float NpcEyeAboveCenter = 0.6f;

	// An aimed-at check is a cone that tightens with distance but never gets
	// narrower than a few degrees, so it still triggers when aiming at the body.
	constexpr float AimConeBodyRadius = 0.6f;
	constexpr float AimConeMinCos = 0.9945f; // ~6 degrees

	bool IsAimedAt(const vec3& aimOrigin, const vec3& aimForward, const vec3& point)
	{
		const vec3 toPoint = point - aimOrigin;
		const float dist = glm::length(toPoint);
		if (dist < 0.001f) return true;

		const float coneCos = std::max(AimConeMinCos, std::cos(std::atan(AimConeBodyRadius / dist)));
		return glm::dot(aimForward, toPoint / dist) >= coneCos;
	}
}

bool CombatTargetView::Fill(CombatTargetView& out, const NpcBase* viewer, Entity* target)
{
	out = CombatTargetView();

	if (!target) return false;

	out.entity = target;
	out.position = target->Position;
	out.eyePosition = target->Position + vec3(0, NpcEyeAboveCenter, 0);
	out.health01 = target->MaxHealth > 0.0f ? glm::clamp(target->Health / target->MaxHealth, 0.0f, 1.0f) : 0.0f;

	if (target == Player::Instance)
	{
		Player* player = Player::Instance;

		out.isPlayer = true;
		out.position = player->controller.GetSmoothPosition();
		out.eyePosition = out.position
			- vec3(0, 1.0f, 0) * player->controller.height * 0.5f
			+ vec3(0, 1.0f, 0) * player->controller.GetCameraHeight();
		out.velocity = player->controller.GetVelocity();
		out.isCrouched = player->controller.isCrouched;

		for (Weapon* weapon : { player->currentWeapon, player->currentOffhandWeapon })
		{
			if (!weapon) continue;

			// No weapon overrides IsMelee(); the ones that block and parry are the blades.
			out.isMeleeArmed |= weapon->IsMelee() || weapon->CanBlockOrParry();
			out.hasGun |= weapon->IsRanged();
			out.isBlocking |= weapon->Blocking;
			out.isParrying |= weapon->Parrying;

			if (auto* firearm = dynamic_cast<WeaponFirearm*>(weapon))
				out.isReloading |= firearm->reloading;
		}

		if (viewer)
			out.isAimingAtMe = IsAimedAt(Camera::position, MathHelper::GetForwardVector(Camera::rotation), viewer->Position);

		return true;
	}

	if (NpcBase* npc = dynamic_cast<NpcBase*>(target))
	{
		out.velocity = npc->GetControllerVelocity();
		out.isCrouched = npc->IsCrouched();
		out.isBlocking = npc->IsBlockingStance();
		out.isStunned = npc->isStunned();

		if (const CombatBehaviorBase* brain = npc->GetCombatBehavior())
		{
			out.isMeleeArmed = brain->IsMeleeMode();
			out.hasGun = !brain->IsMeleeMode();
			out.isReloading = brain->IsReloading();
		}
	}

	return true;
}
