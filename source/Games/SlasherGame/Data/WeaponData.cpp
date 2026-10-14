#include "WeaponData.h"

#include <algorithm>

bool WeaponData::HasRigMesh() const
{
	for (const WeaponMeshData& mesh : meshes)
		if (mesh.attach == WeaponAttachMode::Rig)
			return true;
	return false;
}

namespace
{
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
		ResolveAttach(mesh.holster, mesh.attach, path + ".holster", report);
	}

	// Stance
	WeaponStanceData& stance = data.stance;
	stance.resolvedAnimation = !stance.animation.empty() ? stance.animation : (data.meshes.empty() ? std::string() : data.meshes[0].model);
	if ((!stance.idle.empty() || !stance.hold.empty()) && stance.resolvedAnimation.empty())
		report.Error("stance.animation", "required when the weapon has no mesh");
	if (!stance.hold.empty() && stance.holdBones.empty())
		report.Error("stance.holdBones", "required with stance.hold (the bones whose subtrees take the hold clip)");
	if (stance.hold.empty() && !stance.holdBones.empty())
		report.Warning("stance.holdBones", "ignored without stance.hold");
	for (size_t i = 0; i < stance.holdBones.size(); ++i)
		if (stance.holdBones[i].empty())
			report.Error("stance.holdBones[" + std::to_string(i) + "]", "empty bone name");
	for (size_t i = 0; i < stance.aimBones.size(); ++i)
		if (stance.aimBones[i].empty())
			report.Error("stance.aimBones[" + std::to_string(i) + "]", "empty bone name");
	for (size_t i = 0; i < stance.handBones.size(); ++i)
		if (stance.handBones[i].empty())
			report.Error("stance.handBones[" + std::to_string(i) + "]", "empty bone name");
	if (!stance.handBones.empty() && stance.hold.empty())
		report.Error("stance.handBones", "require stance.hold (the grip when no clip of this weapon drives it)");
	if (stance.aimMaxPitch < 0.0f || stance.aimMaxPitch > 89.0f) report.Error("stance.aimMaxPitch", "must be in [0, 89]");
	if (stance.aimMaxYaw < 0.0f || stance.aimMaxYaw > 180.0f) report.Error("stance.aimMaxYaw", "must be in [0, 180]");
	if (stance.aimTurnRate <= 0.0f) report.Error("stance.aimTurnRate", "must be > 0");
	if (stance.blend < 0.0f) report.Error("stance.blend", "must be >= 0");
	if (data.holsterDelay < 0.0f) report.Error("holsterDelay", "must be >= 0");

	// Points
	data.sockets.clear();
	if (data.kind == WeaponKind::Melee)
	{
		ResolvePoint(data, data.blade.base, "blade.base", report);
		ResolvePoint(data, data.blade.tip, "blade.tip", report);
		if (data.blade.samples < 2) report.Error("blade.samples", "must be >= 2");
		if (data.blade.radius <= 0.0f) report.Error("blade.radius", "must be > 0");
	}

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
