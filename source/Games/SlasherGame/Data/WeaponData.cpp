#include "WeaponData.h"

#include <algorithm>
#include <cmath>

bool WeaponData::HasRigMesh() const
{
	for (const WeaponMeshData& mesh : meshes)
		if (mesh.attach == WeaponAttachMode::Rig)
			return true;
	return false;
}

// The clip is level at its middle; each half covers the pitches from level to one end (validated: the ends have
// opposite signs).
float WeaponSweepData::TimeAt(float pitchDegrees, float duration) const
{
	const float middle = duration * 0.5f;
	const bool towardEnd = (pitchDegrees >= 0.0f) == (endPitch >= 0.0f);
	const float limit = towardEnd ? endPitch : startPitch;
	const float fraction = limit != 0.0f ? std::clamp(pitchDegrees / limit, 0.0f, 1.0f) : 0.0f;
	return towardEnd ? middle + fraction * middle : middle - fraction * middle;
}

namespace
{
	void ResolveStance(WeaponStanceData& stance, const std::string& defaultAnimation, const std::string& path, DataLoadReport& report)
	{
		WeaponSweepData& sweep = stance.sweep;
		stance.resolvedAnimation = !stance.animation.empty() ? stance.animation : defaultAnimation;
		stance.resolvedHold = sweep.IsEnabled() ? sweep.clip : stance.hold;
		if ((!stance.idle.empty() || !stance.resolvedHold.empty()) && stance.resolvedAnimation.empty())
			report.Error(path + ".animation", "required when the weapon has no mesh");
		if (sweep.IsEnabled() && !stance.hold.empty())
			report.Error(path + ".hold", "a weapon with a sweep holds the sweep's middle frame - leave hold empty");
		if (sweep.IsEnabled() && stance.holdTime >= 0.0f)
			report.Error(path + ".holdTime", "a sweep's hold is its middle frame");
		if (!stance.resolvedHold.empty() && stance.holdBones.empty())
			report.Error(path + ".holdBones", "required with a hold or a sweep (the bones whose subtrees take the hold pose)");
		if (stance.resolvedHold.empty() && !stance.holdBones.empty())
			report.Warning(path + ".holdBones", "ignored without a hold");
		if (stance.idle.empty() && stance.idleTime >= 0.0f)
			report.Warning(path + ".idleTime", "ignored without an idle");
		if (stance.hold.empty() && stance.holdTime >= 0.0f)
			report.Warning(path + ".holdTime", "ignored without a hold");
		for (size_t i = 0; i < stance.holdBones.size(); ++i)
			if (stance.holdBones[i].empty())
				report.Error(path + ".holdBones[" + std::to_string(i) + "]", "empty bone name");
		for (size_t i = 0; i < stance.moveHoldBones.size(); ++i)
			if (stance.moveHoldBones[i].empty())
				report.Error(path + ".moveHoldBones[" + std::to_string(i) + "]", "empty bone name");
		if (!stance.moveHoldBones.empty() && stance.resolvedHold.empty())
			report.Error(path + ".moveHoldBones", "require a hold or a sweep");
		for (size_t i = 0; i < stance.handBones.size(); ++i)
			if (stance.handBones[i].empty())
				report.Error(path + ".handBones[" + std::to_string(i) + "]", "empty bone name");
		if (!stance.handBones.empty() && stance.resolvedHold.empty())
			report.Error(path + ".handBones", "require a hold or a sweep (the grip when no clip of this weapon drives it)");
		if (stance.aimTurnRate <= 0.0f) report.Error(path + ".aimTurnRate", "must be > 0");
		if (stance.blend < 0.0f) report.Error(path + ".blend", "must be >= 0");
		if (stance.holdSpace == PoseSpace::Invalid) report.Error(path + ".holdSpace", "expected Default, Local or Mesh");

		sweep.boneHashes.clear();
		sweep.yawBoneHashes.clear();
		if (sweep.IsEnabled())
		{
			for (size_t i = 0; i < sweep.yawBones.size(); ++i)
			{
				if (sweep.yawBones[i].empty())
					report.Error(path + ".sweep.yawBones[" + std::to_string(i) + "]", "empty bone name");
				sweep.yawBoneHashes.emplace_back(sweep.yawBones[i]);
			}
			if (sweep.maxYaw < 0.0f || sweep.maxYaw > 180.0f)
				report.Error(path + ".sweep.maxYaw", "must be within 0..180 degrees");
			if (sweep.bones.empty())
				report.Error(path + ".sweep.bones", "required (the subtrees the sweep turns)");
			for (size_t i = 0; i < sweep.bones.size(); ++i)
			{
				if (sweep.bones[i].empty())
					report.Error(path + ".sweep.bones[" + std::to_string(i) + "]", "empty bone name");
				sweep.boneHashes.emplace_back(sweep.bones[i]);
			}
			if (sweep.space != PoseSpace::Local && sweep.space != PoseSpace::Mesh)
				report.Error(path + ".sweep.space", "expected Local or Mesh");
			if (!(sweep.startPitch * sweep.endPitch < 0.0f) || std::abs(sweep.startPitch) > 90.0f || std::abs(sweep.endPitch) > 90.0f)
				report.Error(path + ".sweep", "startPitch and endPitch must lie on either side of level (the middle frame), within 90 degrees");
		}
	}

	HandMask ResolveHands(const std::vector<WeaponHand>& hands, const std::string& path, DataLoadReport& report)
	{
		HandMask mask = 0;
		for (size_t i = 0; i < hands.size(); ++i)
		{
			if (hands[i] == WeaponHand::Count)
				report.Error(path + "[" + std::to_string(i) + "]", "expected Right or Left");
			else
				mask |= HandBit(hands[i]);
		}
		return mask;
	}

	void ResolveAttach(WeaponAttachData& attach, WeaponAttachMode mode, const std::string& path, DataLoadReport& report)
	{
		attach.boneHash = hashed_string(attach.bone);

		for (size_t i = 0; i < attach.hide.size(); ++i)
			if (attach.hide[i].empty())
				report.Error(path + ".hide[" + std::to_string(i) + "]", "empty mesh name");

		if (mode == WeaponAttachMode::Bone)
		{
			if (!attach.clip.empty())
				report.Warning(path + ".clip", "only used by Rig meshes - ignored");
		}
		else if (mode == WeaponAttachMode::Rig)
		{
			if (attach.offset != glm::vec3(0.0f) || attach.rotation != glm::vec3(0.0f))
				report.Warning(path, "offset / rotation are only used by Bone meshes - ignored (pose the weapon in its clip)");
			if (attach.clipTime < 0.0f)
				report.Error(path + ".clipTime", "must be >= 0");
		}
	}

	// Gives the point the index of its bone in data.sockets (added on first use).
	void ResolvePoint(WeaponData& data, WeaponPointData& point, const std::string& path, DataLoadReport& report)
	{
		point.socket = -1;
		if (point.bone.empty())
		{
			if (data.HasRigMesh())
				report.Error(path + ".bone", "required with Rig meshes (the weapon bones move independently of the model origin)");
			return;
		}

		auto it = std::find(data.sockets.begin(), data.sockets.end(), point.bone);
		point.socket = static_cast<int>(it - data.sockets.begin());
		if (it == data.sockets.end())
			data.sockets.push_back(point.bone);
	}
}

bool ResolveWeapon(WeaponData& data, DataLoadReport& report)
{
	if (data.version != 1)
		report.Warning("version", "expected 1, got " + std::to_string(data.version));

	if (data.kind == WeaponKind::Invalid)
		report.Error("kind", "expected Melee or Firearm");


	for (size_t i = 0; i < data.meshes.size(); ++i)
	{
		WeaponMeshData& mesh = data.meshes[i];
		const std::string path = "meshes[" + std::to_string(i) + "]";

		if (mesh.model.empty())
			report.Error(path + ".model", "required");

		switch (mesh.attach)
		{
		case WeaponAttachMode::Bone:
			if (mesh.scale <= 0.0f)
				report.Error(path + ".scale", "must be > 0");
			if (mesh.hand.bone.empty())
				report.Error(path + ".hand.bone", "required for Bone meshes");
			break;

		case WeaponAttachMode::Rig:
			if (mesh.scale != 1.0f)
				report.Warning(path + ".scale", "Rig meshes use the body's transform - ignored");
			if (mesh.visibleWhenHolstered && mesh.holster.clip.empty() && mesh.holster.hide.empty())
				report.Warning(path + ".holster", "no holster pose and nothing hidden: the holstered weapon bones rest in the bind pose");
			break;

		default:
			report.Error(path + ".attach", "expected Bone or Rig");
			break;
		}

		ResolveAttach(mesh.hand, mesh.attach, path + ".hand", report);
		ResolveAttach(mesh.sheathed, mesh.attach, path + ".sheathed", report);
		ResolveAttach(mesh.holster, mesh.attach, path + ".holster", report);
	}

	// Hands
	data.handMask = ResolveHands(data.hands, "hands", report);
	data.sheathedHandMask = ResolveHands(data.sheathedHands, "sheathedHands", report);
	if (data.handMask == 0 && !data.meshes.empty())
		report.Error("hands", "a weapon with meshes holds at least one hand");
	if (data.sheathedHandMask != 0 && data.kind != WeaponKind::Melee)
		report.Error("sheathedHands", "only melee weapons have a sheathed state");
	for (size_t i = 0; i < data.meshes.size(); ++i)
	{
		const WeaponAttachData& sheathed = data.meshes[i].sheathed;
		if (!data.CanSheathe() && (!sheathed.bone.empty() || !sheathed.clip.empty() || !sheathed.hide.empty()))
			report.Warning("meshes[" + std::to_string(i) + "].sheathed", "ignored without sheathedHands");
	}

	// Stances (the sheathed one only for a sheathable weapon)
	const std::string defaultAnimation = data.meshes.empty() ? std::string() : data.meshes[0].model;
	ResolveStance(data.stance, defaultAnimation, "stance", report);
	ResolveStance(data.sheathedStance, defaultAnimation, "sheathedStance", report);
	if (!data.CanSheathe() && (!data.sheathedStance.idle.empty() || !data.sheathedStance.resolvedHold.empty()))
		report.Warning("sheathedStance", "ignored without sheathedHands");
	if (data.sheathedStance.IsAiming())
		report.Error("sheathedStance.sweep", "a sheathed weapon does not aim");

	// Parts held by other hands
	for (size_t i = 0; i < data.meshes.size(); ++i)
	{
		for (size_t c = 0; c < data.meshes[i].carried.size(); ++c)
		{
			const WeaponCarriedData& carried = data.meshes[i].carried[c];
			const std::string path = "meshes[" + std::to_string(i) + "].carried[" + std::to_string(c) + "]";
			if (carried.hand == WeaponHand::Count)
				report.Error(path + ".hand", "expected Right or Left");
			else if ((data.handMask & HandBit(carried.hand)) != 0)
				report.Warning(path + ".hand", "the weapon holds this hand itself - its parts would hide whenever another weapon is out");
			if (carried.meshes.empty())
				report.Warning(path + ".meshes", "empty - nothing to hide");
			for (const std::string& mesh : carried.meshes)
				if (mesh.empty())
					report.Error(path + ".meshes", "empty mesh name");
		}
	}

	if (data.holsterDelay < 0.0f) report.Error("holsterDelay", "must be >= 0");
	if (data.holsterAfterFire && data.kind != WeaponKind::Firearm)
		report.Error("holsterAfterFire", "only firearms fire");


	// Points
	data.sockets.clear();
	if (data.kind == WeaponKind::Melee)
	{
		ResolvePoint(data, data.blade.base, "blade.base", report);
		ResolvePoint(data, data.blade.tip, "blade.tip", report);
		if (data.blade.samples < 2) report.Error("blade.samples", "must be >= 2");
		if (data.blade.radius <= 0.0f) report.Error("blade.radius", "must be > 0");
	}

	// Extension: a weapon bone of a Rig model (a Bone prop is rigid).
	WeaponExtendData& extend = data.extend;
	extend.boneHash = hashed_string(extend.bone);
	extend.resolvedAxis = glm::vec3(0.0f, 1.0f, 0.0f);
	if (!extend.bone.empty())
	{
		if (!data.HasRigMesh())
			report.Error("extend.bone", "requires a Rig mesh (the bone moves inside the weapon model)");
		if (glm::dot(extend.axis, extend.axis) < 1e-8f)
			report.Error("extend.axis", "must not be zero");
		else
			extend.resolvedAxis = glm::normalize(extend.axis);
	}

	if (data.trailStart < 0.0f || data.trailStart >= 1.0f)
		report.Error("trailStart", "must be in [0, 1)");

	if (data.kind == WeaponKind::Firearm)
	{
		FirearmData& f = data.firearm;
		if (f.muzzles.empty()) report.Error("firearm.muzzles", "at least one muzzle is required");
		for (size_t i = 0; i < f.muzzles.size(); ++i)
			ResolvePoint(data, f.muzzles[i], "firearm.muzzles[" + std::to_string(i) + "]", report);
		if (f.fireRate <= 0.0f) report.Error("firearm.fireRate", "must be > 0");
		if (f.magazine < 0) report.Error("firearm.magazine", "must be >= 0");
		if (f.pellets < 1) report.Error("firearm.pellets", "must be >= 1");
		if (f.projectile.speed <= 0.0f) report.Error("firearm.projectile.speed", "must be > 0");
		if (f.projectile.maxDistance <= 0.0f) report.Error("firearm.projectile.maxDistance", "must be > 0");
		if (f.projectile.reaction == HitReaction::Count) report.Error("firearm.projectile.reaction", "unknown reaction");
		if (f.projectile.hitStopScale <= 0.0f || f.projectile.hitStopScale > 1.0f)
			report.Error("firearm.projectile.hitStopScale", "must be in (0, 1]");
	}

	return report.Ok();
}
