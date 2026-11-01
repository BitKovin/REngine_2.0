#include "CombatThreat.h"

#include "CombatTargetView.h"
#include "../NpcBase.h"

namespace
{
	constexpr float CloseMeleeRange = 5.0f;   // a sword this close is a real threat
	constexpr float FullProximityRange = 5.0f;
	constexpr float FarProximityRange = 30.0f;
	constexpr float FarProximityFactor = 0.3f;

	constexpr float MeleeCloseDanger = 0.8f;
	constexpr float MeleeFarDanger = 0.4f;
	constexpr float RecentAttackerFactor = 1.5f;
	constexpr double RecentAttackerWindow = 3.0; // seconds

	constexpr float AimingAtMeFactor = 1.3f;
	constexpr float PlayerFactor = 1.2f;
}

float CombatThreat::Score(const NpcBase* viewer, const std::string& targetId)
{
	Entity* entity = Level::Current->FindEntityWithId(targetId);

	CombatTargetView view;
	if (!CombatTargetView::Fill(view, viewer, entity)) return 0.0f;

	const float distance = glm::distance(viewer->Position, view.position);

	// Without a gun the target can only hurt the NPC up close.
	float danger = 1.0f;
	if (!view.hasGun)
		danger = distance < CloseMeleeRange ? MeleeCloseDanger : MeleeFarDanger;

	if (viewer->GetLastAttackerId() == targetId && Time::GameTime - viewer->GetLastAttackedTime() < RecentAttackerWindow)
		danger *= RecentAttackerFactor;

	const float farFraction = glm::clamp((distance - FullProximityRange) / (FarProximityRange - FullProximityRange), 0.0f, 1.0f);
	const float proximity = glm::mix(1.0f, FarProximityFactor, farFraction);

	float attention = 1.0f;
	if (view.isAimingAtMe) attention *= AimingAtMeFactor;
	if (view.isPlayer) attention *= PlayerFactor;

	return danger * proximity * attention;
}
