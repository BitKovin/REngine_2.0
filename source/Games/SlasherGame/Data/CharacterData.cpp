#include "CharacterData.h"

#include <algorithm>

bool ResolveCharacter(CharacterData& data, DataLoadReport& report)
{
	if (data.version != 1)
		report.Warning("version", "expected 1, got " + std::to_string(data.version));

	if (data.team == Team::Count) report.Error("team", "expected Neutral, Player or Enemy");
	if (data.mesh.model.empty()) report.Error("mesh.model", "required");
	if (data.mesh.scale <= 0.0f) report.Error("mesh.scale", "must be > 0");
	if (data.moveset.empty()) report.Error("moveset", "required");
	if (data.health <= 0.0f) report.Error("health", "must be > 0");
	if (data.locomotion.runClipSpeed <= 0.0f) report.Error("locomotion.runClipSpeed", "must be > 0");
	static constexpr const char* LocomotionClipFields[] = { "idle", "runForward", "runBackward", "runLeft", "runRight", "fall", "glide" };
	for (const auto& [field, file] : data.locomotion.sources)
	{
		if (FindSlotIndex(LocomotionClipFields, field) < 0)
			report.Error("locomotion.sources." + field, "unknown clip (allowed: idle, runForward, runBackward, runLeft, runRight, fall, glide)");
		if (file.empty())
			report.Error("locomotion.sources." + field, "empty path");
	}

	data.locomotion.legBoneHashes.clear();
	for (size_t i = 0; i < data.locomotion.legBones.size(); ++i)
	{
		if (data.locomotion.legBones[i].empty())
			report.Error("locomotion.legBones[" + std::to_string(i) + "]", "empty bone name");
		data.locomotion.legBoneHashes.emplace_back(data.locomotion.legBones[i]);
	}

	StrafeData& strafe = data.locomotion.strafe;
	if (strafe.maxTwist < 0.0f || strafe.maxTwist > 90.0f) report.Error("locomotion.strafe.maxTwist", "must be in [0, 90]");
	if (strafe.turnRate <= 0.0f) report.Error("locomotion.strafe.turnRate", "must be > 0");
	if (strafe.switchTime < 0.0f) report.Error("locomotion.strafe.switchTime", "must be >= 0");
	if (strafe.enabled && strafe.legsBone.empty()) report.Error("locomotion.strafe.legsBone", "required while strafe is enabled");
	strafe.legsBoneHash = hashed_string(strafe.legsBone);
	strafe.counterBoneHashes.clear();
	for (const std::string& bone : strafe.counterBones)
		strafe.counterBoneHashes.emplace_back(bone);

	LeanData& lean = data.locomotion.lean;
	if (lean.maxForward < 0.0f || lean.maxSide < 0.0f) report.Error("locomotion.lean", "maxForward / maxSide must be >= 0");
	if (lean.sharpness <= 0.0f) report.Error("locomotion.lean.sharpness", "must be > 0");
	lean.boneHash = hashed_string(lean.bone);

	const TargetingData& targeting = data.targeting;
	if (targeting.distanceWeight < 0.0f) report.Error("targeting.distanceWeight", "must be >= 0");
	if (targeting.keepBonus < 0.0f) report.Error("targeting.keepBonus", "must be >= 0");
	if (targeting.inputThreshold <= 0.0f || targeting.inputThreshold > 1.0f) report.Error("targeting.inputThreshold", "must be in (0, 1]");
	if (targeting.airHeightRange <= 0.0f) report.Error("targeting.airHeightRange", "must be > 0");

	if (data.hitEffect.scale < 0.0f) report.Error("hitEffect.scale", "must be >= 0");

	const AirCombatData& air = data.airCombat;
	if (air.riseGravityScale < 0.0f || air.fallGravityScale < 0.0f) report.Error("airCombat", "gravity scales must be >= 0");
	if (air.maxFallSpeed <= 0.0f) report.Error("airCombat.maxFallSpeed", "must be > 0");
	if (air.fallEase <= 0.0f) report.Error("airCombat.fallEase", "must be > 0");
	if (air.hold < 0.0f || air.victimHold < 0.0f || air.firearmHold < 0.0f) report.Error("airCombat", "hold / victimHold / firearmHold must be >= 0");
	if (air.gravityRamp < 0.0f) report.Error("airCombat.gravityRamp", "must be >= 0");
	if (air.maxHangTime <= 0.0f) report.Error("airCombat.maxHangTime", "must be > 0");
	if (air.hangFade <= 0.0f) report.Error("airCombat.hangFade", "must be > 0");
	if (air.hitLift < 0.0f) report.Error("airCombat.hitLift", "must be >= 0");
	if (air.pullTime <= 0.0f) report.Error("airCombat.pullTime", "must be > 0");
	if (air.pullRange < 0.0f || air.pullMaxHeight < 0.0f || air.pullMaxSpeed <= 0.0f) report.Error("airCombat", "pullRange / pullMaxHeight must be >= 0 and pullMaxSpeed > 0");
	if (air.launchScale < 0.0f || air.launchScale > 1.0f) report.Error("airCombat.launchScale", "must be in [0, 1]");
	if (air.launchDecay <= 0.0f || air.launchDecay > 1.0f) report.Error("airCombat.launchDecay", "must be in (0, 1]");
	if (air.launchRecover < 0.0f) report.Error("airCombat.launchRecover", "must be >= 0");
	if (air.launchMinScale < 0.0f || air.launchMinScale > 1.0f) report.Error("airCombat.launchMinScale", "must be in [0, 1]");
	if (air.rootMotionScale < 0.0f || air.knockbackScale < 0.0f) report.Error("airCombat", "rootMotionScale / knockbackScale must be >= 0");

	HitRecoilData& recoil = data.hitRecoil;
	if (recoil.degreesPerKnockback < 0.0f) report.Error("hitRecoil.degreesPerKnockback", "must be >= 0");
	if (recoil.maxAngle < 0.0f || recoil.maxAngle > 60.0f) report.Error("hitRecoil.maxAngle", "must be in [0, 60]");
	if (recoil.frequency <= 0.0f) report.Error("hitRecoil.frequency", "must be > 0");
	if (recoil.damping <= 0.0f) report.Error("hitRecoil.damping", "must be > 0");
	for (size_t i = 0; i < recoil.bones.size(); ++i)
	{
		HitRecoilBoneData& bone = recoil.bones[i];
		if (bone.bone.empty()) report.Error("hitRecoil.bones[" + std::to_string(i) + "].bone", "must not be empty");
		if (bone.weight < 0.0f) report.Error("hitRecoil.bones[" + std::to_string(i) + "].weight", "must be >= 0");
		bone.boneHash = hashed_string(bone.bone);
	}

	if (data.animationLod.halfRateDistance <= 0.0f || data.animationLod.quarterRateDistance < data.animationLod.halfRateDistance)
		report.Error("animationLod", "requires 0 < halfRateDistance <= quarterRateDistance");

	const MovementData& m = data.movement;
	if (m.quality == MovementQuality::Invalid) report.Error("movement.quality", "expected Player, NpcGeneric or NpcLowQuality");
	if (m.radius <= 0.0f || m.height <= 2.0f * m.radius) report.Error("movement", "requires radius > 0 and height > 2 * radius");
	if (m.walkSpeed < 0.0f || m.runSpeed < m.walkSpeed) report.Error("movement", "requires 0 <= walkSpeed <= runSpeed");
	if (m.acceleration <= 0.0f || m.deceleration <= 0.0f) report.Error("movement", "acceleration/deceleration must be > 0");
	if (m.turnRate <= 0.0f) report.Error("movement.turnRate", "must be > 0");
	if (m.turnSharpness <= 0.0f) report.Error("movement.turnSharpness", "must be > 0");

	if (data.poise.enabled && data.poise.max <= 0.0f) report.Error("poise.max", "must be > 0");

	for (size_t i = 0; i < data.weakPoints.size(); ++i)
	{
		WeakPointData& wp = data.weakPoints[i];
		const std::string path = "weakPoints[" + std::to_string(i) + "]";
		if (wp.name.empty()) report.Error(path + ".name", "required");
		if (wp.hitboxes.empty()) report.Error(path + ".hitboxes", "at least one hitbox bone is required");
		if (wp.health <= 0.0f) report.Error(path + ".health", "must be > 0");

		wp.sourceMask = 0;
		for (DamageSource source : wp.sources)
		{
			if (source == DamageSource::Count) report.Error(path + ".sources", "unknown damage source");
			else wp.sourceMask |= SourceBit(source);
		}

		wp.breakPoise = wp.kill = wp.damageBonus = false;
		for (WeakPointEffect effect : wp.onBreak)
		{
			switch (effect)
			{
			case WeakPointEffect::BreakPoise:  wp.breakPoise = true; break;
			case WeakPointEffect::Kill:        wp.kill = true; break;
			case WeakPointEffect::DamageBonus: wp.damageBonus = true; break;
			default: report.Error(path + ".onBreak", "unknown effect"); break;
			}
		}

		wp.hitboxHashes.clear();
		for (const std::string& hitbox : wp.hitboxes)
			wp.hitboxHashes.emplace_back(hitbox);
	}

	size_t weaponCount = 0;
	for (const auto& [slotName, files] : data.weapons)
	{
		if (FindSlotIndex(WeaponSlotNames, slotName) < 0)
			report.Error("weapons." + slotName, "unknown slot (allowed: Main, Secondary)");
		if (files.empty())
			report.Warning("weapons." + slotName, "no weapon listed");
		for (size_t i = 0; i < files.size(); ++i)
			if (files[i].empty())
				report.Error("weapons." + slotName + "[" + std::to_string(i) + "]", "empty path");
		weaponCount += files.size();
	}
	if (weaponCount > CharacterData::MaxWeapons)
		report.Error("weapons", "at most " + std::to_string(CharacterData::MaxWeapons) + " weapons");

	for (size_t i = 0; i < data.ai.attacks.size(); ++i)
	{
		AiAttackData& attack = data.ai.attacks[i];
		const std::string path = "ai.attacks[" + std::to_string(i) + "]";
		attack.comboSlot = attack.slot.empty() ? -1 : FindSlotIndex(ComboSlotNames, attack.slot);
		attack.actionSlot = attack.slot.empty() || attack.comboSlot >= 0 ? -1 : FindSlotIndex(ActionSlotNames, attack.slot);
		if (attack.move.empty() == attack.slot.empty())
			report.Error(path, "set either move or slot");
		else if (!attack.slot.empty() && attack.comboSlot < 0 && attack.actionSlot < 0)
			report.Error(path + ".slot", "unknown combo or action slot '" + attack.slot + "'");
		if (attack.maxRange < attack.minRange) report.Error(path, "maxRange must be >= minRange");
		if (attack.weight <= 0.0f) report.Error(path + ".weight", "must be > 0");
		if (attack.token == AttackToken::Count) report.Error(path + ".token", "unknown token");
		if (attack.holdTime < 0.0f) report.Error(path + ".holdTime", "must be >= 0");
		if (attack.holdButton == InputButton::Count) report.Error(path + ".holdButton", "unknown button");
	}

	if (data.ai.preferredMaxRange < data.ai.preferredMinRange)
		report.Error("ai", "preferredMaxRange must be >= preferredMinRange");
	if (data.ai.decisionInterval <= 0.0f) report.Error("ai.decisionInterval", "must be > 0");
	const float chances[] = { data.ai.aggression, data.ai.comboChance, data.ai.heavyChance };
	for (float chance : chances)
		if (chance < 0.0f || chance > 1.0f) report.Error("ai", "aggression / comboChance / heavyChance must be in [0, 1]");

	return report.Ok();
}

bool ResolveCharacterReferences(CharacterData& data, DataLoadReport& report)
{
	if (!data.moveSet)
	{
		report.Error("moveset", "failed to load '" + data.moveset + "'");
		return false;
	}

	for (AiAttackData& attack : data.ai.attacks)
	{
		if (attack.move.empty())
			continue;   // a slot: resolved through the equipped weapons' tables at run time
		attack.moveIndex = data.moveSet->FindMove(attack.move);
		if (attack.moveIndex < 0)
			report.Error("ai.attacks", "move '" + attack.move + "' does not exist in " + data.moveset);
	}

	// A Rig weapon model carries the attack clips (COMBAT_DESIGN.md §9.3): the body plays them from that file.
	const std::vector<std::string>& sources = data.moveSet->animations;
	for (size_t slot = 0; slot < data.weaponData.size(); ++slot)
	{
		for (const auto& weapon : data.weaponData[slot])
		{
			for (const WeaponMeshData& mesh : weapon->meshes)
			{
				if (mesh.attach == WeaponAttachMode::Rig && std::find(sources.begin(), sources.end(), mesh.model) == sources.end())
					report.Warning(std::string("weapons.") + WeaponSlotNames[slot], "Rig weapon model '" + mesh.model
						+ "' is not an animation source of " + data.moveset + " - the attack clips should come from it, otherwise its weapon bones stay at rest");
			}
		}
	}

	// Per-weapon move tables (MoveSetData::weaponMoves) name weapons of this character, and only fill slots that
	// weapon's slot owns (ActionWeaponSlot): a melee weapon's table never decides what Secondary fires.
	for (const auto& [name, table] : data.moveSet->weaponMoves)
	{
		WeaponSlot owner = WeaponSlot::Count;
		for (size_t slot = 0; slot < data.weaponData.size(); ++slot)
			for (const auto& weapon : data.weaponData[slot])
				if (weapon->name == name)
					owner = static_cast<WeaponSlot>(slot);
		if (owner == WeaponSlot::Count)
		{
			report.Warning("moveset.weaponMoves." + name, "no weapon of this character is named '" + name + "' - the table is never used");
			continue;
		}
		if (owner != WeaponSlot::Main && (!table.combos.empty() || !table.holdOpeners.empty()))
			report.Warning("moveset.weaponMoves." + name, "combos / holdOpeners belong to the Main weapon - ignored for a Secondary weapon");
		for (const auto& [slotName, moveId] : table.actions)
		{
			const int action = FindSlotIndex(ActionSlotNames, slotName);
			if (action >= 0 && ActionWeaponSlot(static_cast<ActionSlot>(action)) != owner)
				report.Warning("moveset.weaponMoves." + name + ".actions." + slotName, std::string("belongs to the ")
					+ WeaponSlotNames[static_cast<size_t>(ActionWeaponSlot(static_cast<ActionSlot>(action)))] + " weapon - ignored");
		}
	}

	// A move's reach lengthens the weapon whose clips it plays (or the slot's first) through its extend bone (§9.3).
	for (const MoveData& move : data.moveSet->moves)
	{
		if (!move.reach.IsActive())
			continue;
		const auto& candidates = data.weaponData[static_cast<size_t>(move.reach.weapon)];
		const WeaponData* weapon = candidates.empty() ? nullptr : candidates.front().get();
		for (const auto& candidate : candidates)
			for (const WeaponMeshData& mesh : candidate->meshes)
				if (mesh.model == move.animation)
					weapon = candidate.get();
		if (weapon == nullptr || weapon->extend.bone.empty())
			report.Warning("moveset", "move '" + move.id + "' has a reach, but its " + WeaponSlotNames[static_cast<size_t>(move.reach.weapon)]
				+ " weapon has no extend bone - the weapon keeps its length");
	}

	return report.Ok();
}
