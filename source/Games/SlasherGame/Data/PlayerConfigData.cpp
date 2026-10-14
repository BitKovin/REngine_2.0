#include "PlayerConfigData.h"

#include <Combat/Spring.h>

#include <LevelObjectFactory.h>

float PlayerCameraImpactData::MaxKickVelocity() const
{
	// A kick peaking at maxKick starts at maxKick * omega / peak factor and only slows down from there.
	return maxKick * glm::two_pi<float>() * kickFrequency / DampedSpring::PeakFactor(kickDamping);
}

float PlayerCameraImpactData::KickPeakSpeed() const
{
	return kickAngle * MaxKickVelocity();
}

float PlayerCameraImpactData::ShakePeakSpeed() const
{
	// Each axis moves at most 2 pi f * its amplitude (the rumble's waves are at or below f).
	return glm::two_pi<float>() * shakeFrequency * shakeAngle * std::sqrt(1.0f + shakeHorizontal * shakeHorizontal);
}

bool ResolvePlayerConfig(PlayerConfigData& data, DataLoadReport& report)
{
	if (data.version != 1)
		report.Warning("version", "expected 1, got " + std::to_string(data.version));

	if (data.heroEntity.empty() || !LevelObjectFactory::instance().isRegistered(data.heroEntity))
		report.Error("heroEntity", "'" + data.heroEntity + "' is not a registered entity type");

	const PlayerCameraData& c = data.camera;
	if (c.distance <= 0.0f) report.Error("camera.distance", "must be > 0");
	if (c.fov <= 10.0f || c.fov >= 170.0f) report.Error("camera.fov", "must be in (10, 170)");
	if (c.collisionRadius <= 0.0f) report.Error("camera.collisionRadius", "must be > 0");
	if (c.minPitch >= c.maxPitch) report.Error("camera", "minPitch must be < maxPitch");
	if (c.defaultPitch < c.minPitch || c.defaultPitch > c.maxPitch) report.Error("camera.defaultPitch", "must be within [minPitch, maxPitch]");
	if (c.followSharpness <= 0.0f) report.Error("camera.followSharpness", "must be > 0");
	if (c.frameHeight < 0.0f) report.Error("camera.frameHeight", "must be >= 0");
	if (c.lookUpDistance <= 0.0f || c.lookUpDistance > c.distance) report.Error("camera.lookUpDistance", "must be in (0, distance]");

	// Camera impact: hard limits on what blurs (frequency) and rings (damping), warnings on what disorients (speed).
	const PlayerCameraImpactData& impact = c.impact;
	using Impact = PlayerCameraImpactData;
	if (impact.kickAngle < 0.0f || impact.kickOffset < 0.0f || impact.shakeAngle < 0.0f || impact.shakeOffset < 0.0f || impact.shakeHorizontal < 0.0f)
		report.Error("camera.impact", "kick / shake amplitudes must be >= 0");
	if (impact.kickFrequency <= 0.0f || impact.kickFrequency > Impact::MaxFrequency || impact.shakeFrequency <= 0.0f || impact.shakeFrequency > Impact::MaxFrequency)
		report.Error("camera.impact", "frequencies must be in (0, " + std::to_string(static_cast<int>(Impact::MaxFrequency)) + "] Hz - faster waves read as blur at 60 fps");
	if (impact.kickDamping < Impact::MinKickDamping) report.Error("camera.impact.kickDamping", "must be >= 0.4 (a lower damping rings)");
	if (impact.kickVerticalBias < 0.0f || impact.kickVerticalBias > 1.0f) report.Error("camera.impact.kickVerticalBias", "must be in [0, 1]");
	if (impact.maxKick <= 0.0f) report.Error("camera.impact.maxKick", "must be > 0");
	if (impact.shakeDecay <= 0.0f) report.Error("camera.impact.shakeDecay", "must be > 0");
	if (report.Ok() && impact.KickPeakSpeed() > Impact::MaxKickSpeed)
		report.Warning("camera.impact", "a maxKick kick turns the view at " + std::to_string(static_cast<int>(impact.KickPeakSpeed())) + " deg/s - above 60 it disorients");
	if (report.Ok() && impact.ShakePeakSpeed() > Impact::MaxShakeSpeed)
		report.Warning("camera.impact", "full rumble turns the view at " + std::to_string(static_cast<int>(impact.ShakePeakSpeed())) + " deg/s - above 40 it disorients");

	const PlayerFeedbackData& f = data.feedback;
	if (f.dealtShake < 0.0f || f.takenKick < 0.0f || f.takenShake < 0.0f || f.takenFirearmKick < 0.0f || f.killShake < 0.0f || f.poiseBreakShake < 0.0f || f.finisherShake < 0.0f)
		report.Error("feedback", "values must be >= 0");

	if (data.possession.releaseMode == PossessionReleaseMode::Invalid)
		report.Error("possession.releaseMode", "expected Ally or Revert");
	if (data.possession.range <= 0.0f) report.Error("possession.range", "must be > 0");

	return report.Ok();
}
