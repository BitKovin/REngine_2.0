#pragma once

#include <Data/DataLoadReport.h>
#include <Data/JsonEnum.h>

#include <string>

// Data contract of GameData/combat/player.json: which entity type the player starts as, third-person camera
// tuning and possession rules.

// Impact response of the camera (COMBAT_DESIGN.md §9.4, Control/CameraImpact), in impact units (a hit window's
// `camera`, feedback below):
//   kick   a jolt along the blow (biased downward) on a damped spring: mostly a camera slide, a little turn, no roll;
//          settles in ~0.25 s
//   shake  rumble for the big moments (finishers, poise breaks, heavy hits taken): amplitude = trauma^2, the strongest
//          trauma wins, it decays linearly. Light hits do neither - the hit-stop, the victim and the blood carry them.
// Comfort limits (validated): frequencies <= MaxFrequency, the view never turns faster than MaxKickSpeed /
// MaxShakeSpeed.
struct PlayerCameraImpactData
{
	float kickAngle = 0.5f;          // degrees of view turn per impact unit
	float kickOffset = 0.06f;        // m of camera slide per impact unit
	float kickFrequency = 6.0f;      // Hz
	float kickDamping = 0.6f;        // damping ratio (>= MinKickDamping: one small overshoot, no ringing)
	float kickVerticalBias = 0.4f;   // 0 = along the blow, 1 = straight down: impacts read through their vertical part
	float maxKick = 1.5f;            // impact units, stacked kicks included
	float shakeAngle = 0.5f;         // degrees at full trauma (vertical axis)
	float shakeOffset = 0.05f;       // m at full trauma (vertical axis)
	float shakeHorizontal = 0.5f;    // horizontal amplitude as a share of the vertical one
	float shakeFrequency = 8.0f;     // Hz
	float shakeDecay = 2.2f;         // trauma lost per second (full trauma lasts 1 / shakeDecay s)

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayerCameraImpactData, kickAngle, kickOffset, kickFrequency, kickDamping,
		kickVerticalBias, maxKick, shakeAngle, shakeOffset, shakeHorizontal, shakeFrequency, shakeDecay)

	static constexpr float MaxFrequency = 12.0f;   // Hz: 5 frames per wave at 60 fps; faster reads as blur
	static constexpr float MaxKickSpeed = 60.0f;   // deg/s the view turns at most during a kick
	static constexpr float MaxShakeSpeed = 40.0f;  // deg/s the view turns at most in a rumble
	static constexpr float MinKickDamping = 0.4f;  // below it the kick rings

	float MaxKickVelocity() const;   // impact units/s at the start of a maxKick kick (its fastest moment)
	float KickPeakSpeed() const;     // deg/s the view turns then
	float ShakePeakSpeed() const;    // deg/s at full trauma, upper bound
};

// Third-person camera. Framing is in camera space: the camera looks along its rotation with the view center
// frameHeight above (and shoulderOffset to the right of) the focus point on the body, measured along the camera's own
// axes - the body keeps its place on screen at every pitch.
struct PlayerCameraData
{
	float distance = 3.2f;           // boom length
	float focusHeight = 0.6f;        // the framed point on the body, above the capsule center (upper chest)
	float frameHeight = 0.6f;        // view center above the focus point along the camera's up axis: body below center
	float shoulderOffset = 0.45f;    // view center to the right of the focus point (camera space)
	float collisionRadius = 0.25f;
	float sensitivity = 1.0f;        // multiplier on engine mouse delta (deg per unit)
	float defaultPitch = 10.0f;      // degrees down when the camera snaps behind a body
	float minPitch = -60.0f;
	float maxPitch = 70.0f;
	// Looking up, the boom shortens toward lookUpDistance at minPitch so the camera stays above the floor.
	float lookUpDistance = 2.2f;
	float followSharpness = 14.0f;   // pivot smoothing (higher = tighter)
	float fov = 75.0f;
	PlayerCameraImpactData impact;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayerCameraData, distance, focusHeight, frameHeight, shoulderOffset,
		collisionRadius, sensitivity, defaultPitch, minPitch, maxPitch, lookUpDistance, followSharpness, fov, impact)
};

// How combat moments drive the camera (impact units / trauma, see PlayerCameraImpactData). A hit the player lands
// kicks by its own `camera` weight along the strike; these add the rest. Rumble is for rare moments only.
struct PlayerFeedbackData
{
	float dealtShake = 0.6f;         // trauma per unit of a landed hit's `camera` weight (heavy enders)
	float takenKick = 0.8f;          // kick when a melee hit lands on the player's body (along the blow)
	float takenShake = 0.5f;         // trauma when a melee hit lands on the player's body
	float takenFirearmKick = 0.15f;  // kick per bullet that hits the player's body (no rumble: automatic fire)
	float killShake = 0.0f;          // trauma when the player kills
	float poiseBreakShake = 0.7f;    // trauma when the player breaks a poise
	float finisherShake = 0.9f;      // trauma when a finisher lands

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayerFeedbackData, dealtShake, takenKick, takenShake, takenFirearmKick,
		killShake, poiseBreakShake, finisherShake)
};

enum class PossessionReleaseMode : uint8_t { Invalid, Ally, Revert };
SLASHER_JSON_ENUM(PossessionReleaseMode, { { PossessionReleaseMode::Invalid, nullptr }, { PossessionReleaseMode::Ally, "Ally" },
	{ PossessionReleaseMode::Revert, "Revert" } })

struct PlayerPossessionData
{
	bool enabled = true;
	float range = 15.0f;             // max distance to a possession candidate
	float aimAngle = 12.0f;          // candidate must be within this angle of the camera forward
	PossessionReleaseMode releaseMode = PossessionReleaseMode::Ally;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayerPossessionData, enabled, range, aimAngle, releaseMode)
};

struct PlayerConfigData
{
	int version = 1;
	std::string heroEntity = "slasher_hero";    // registered entity type spawned at the player start
	float respawnDelay = 3.0f;                   // seconds after the hero dies before the level restarts
	PlayerCameraData camera;
	PlayerFeedbackData feedback;
	PlayerPossessionData possession;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayerConfigData, version, heroEntity, respawnDelay, camera, feedback, possession)

	// Resolved
	std::string sourcePath;
};

bool ResolvePlayerConfig(PlayerConfigData& data, DataLoadReport& report);
