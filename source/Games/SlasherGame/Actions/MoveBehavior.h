#pragma once

#include <Data/DataLoadReport.h>

#include <json.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class CombatCharacter;
class MoveRunner;
struct HitResult;
struct MoveData;

// Typed, validated contents of a move's "params" block. Each behaviour defines its own struct
// (NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT) deriving from this; it is parsed once when the moveset loads.
struct MoveBehaviorParams
{
	virtual ~MoveBehaviorParams() = default;
};

enum class MoveEndReason : uint8_t
{
	Completed,     // reached endTime
	Chained,       // replaced by a combo follow-up
	Cancelled,     // replaced through a cancel window
	Interrupted,   // forced move (reaction, death, finisher victim) or external stop
	Released,      // behaviour asked to end (e.g. glide button released)
	Recovered,     // combo window closed unused: control is back, the clip tail plays as an upper-body recovery
};

// Everything a behaviour gets each callback. Times are clip seconds of the running move.
struct MoveContext
{
	CombatCharacter& Character;
	MoveRunner& Runner;
	const MoveData& Move;
	const MoveBehaviorParams* Params;
	float Time = 0.0f;
	float PreviousTime = 0.0f;
	float EndTime = 0.0f;
	float DeltaTime = 0.0f;     // character-local (hit-stop dilated) seconds

	template<typename T>
	const T& GetParams() const { return static_cast<const T&>(*Params); }
};

// The mechanics of a kind of move ("Attack", "Dash", ...). One instance per character per behaviour type,
// reused for every move of that type, so per-move state lives in members reset in OnStart.
// Callbacks run in the body's parallel update (COMBAT_DESIGN.md §2.2) or on the main thread (hit resolution).
// A behaviour only changes its own body directly; anything touching another entity or engine-global state goes
// through CombatCharacter::RunOnMainThread.
class MoveBehavior
{
public:
	virtual ~MoveBehavior() = default;

	// Extra start conditions beyond cancel windows / cooldown / start state (air charges, a finishable target...).
	virtual bool CanStart(const CombatCharacter& character, const MoveData& move) const { (void)character; (void)move; return true; }

	virtual void OnStart(MoveContext& context) { (void)context; }
	virtual void OnTick(MoveContext& context) { (void)context; }
	virtual void OnEnd(MoveContext& context, MoveEndReason reason) { (void)context; (void)reason; }

	// A hit of this move landed (main thread, hit resolution; the move is still the body's primary move).
	virtual void OnHitDealt(MoveContext& context, const HitResult& result) { (void)context; (void)result; }

	// Return true to end the move early (MoveEndReason::Released).
	virtual bool WantsToEnd(const MoveContext& context) const { (void)context; return false; }

	// Return true to hold the move at endTime instead of finishing it (auto-fire while held, glide while held).
	virtual bool Sustains(const MoveContext& context) const { (void)context; return false; }

	// While true the move's clock stands still where it is (clip pose held, windows at that time stay open) - a plunge
	// holding its strike until it lands. Asked every frame before the clock advances.
	virtual bool HoldsClock(const MoveContext& context) const { (void)context; return false; }

	// While true the move cannot be left - no combo link resolves, no cancel window opens, no recovery begins (a plunge
	// while it dives: it ends on the ground). Presses made meanwhile wait (buffered as usual). Forced moves still
	// interrupt it. Asked every frame before the clock advances.
	virtual bool Commits(const MoveContext& context) const { (void)context; return false; }
};

struct MoveBehaviorType
{
	std::string Name;
	std::unique_ptr<MoveBehavior> (*Create)() = nullptr;
	// Parses + validates the "params" block. Returns nullptr and records errors on failure.
	std::shared_ptr<const MoveBehaviorParams> (*ParseParams)(const nlohmann::json& params, const std::string& path, DataLoadReport& report) = nullptr;
	// Every parameter with its default value (editors show what a "params" block can hold).
	nlohmann::json (*DefaultParams)() = nullptr;
};

class MoveBehaviorRegistry
{
public:
	static int Register(MoveBehaviorType type);
	static int Find(const std::string& name);              // -1 if unknown
	static const MoveBehaviorType& Get(int index);
	static int Count();
	static std::string KnownNames();

private:
	static std::vector<MoveBehaviorType>& Types();
};

namespace MoveBehaviorDetail
{
	template<typename ParamsT>
	std::shared_ptr<const MoveBehaviorParams> ParseParams(const nlohmann::json& params, const std::string& path, DataLoadReport& report)
	{
		auto parsed = std::make_shared<ParamsT>();
		const nlohmann::json& source = params.is_null() ? nlohmann::json::object() : params;
		if (!ConvertDataJson(source, *parsed, path, report))
			return nullptr;
		if (!parsed->Validate(path, report))
			return nullptr;
		return parsed;
	}

	template<typename ParamsT>
	nlohmann::json DefaultParams()
	{
		return nlohmann::json(ParamsT());
	}
}

// Params struct for behaviours without parameters.
struct NoMoveParams : MoveBehaviorParams
{
	bool Validate(const std::string&, DataLoadReport&) const { return true; }
	friend void to_json(nlohmann::json& j, const NoMoveParams&) { j = nlohmann::json::object(); }
	friend void from_json(const nlohmann::json&, NoMoveParams&) {}
};

// REGISTER_MOVE_BEHAVIOR("Dash", DashBehavior, DashParams)
// ParamsT needs NLOHMANN serialization and `bool Validate(const std::string& path, DataLoadReport&) const`.
#define REGISTER_MOVE_BEHAVIOR(NameString, BehaviorClass, ParamsClass)                                   \
	namespace {                                                                                          \
		const int BehaviorClass##_registeredIndex = MoveBehaviorRegistry::Register(MoveBehaviorType{      \
			NameString,                                                                                  \
			[]() -> std::unique_ptr<MoveBehavior> { return std::make_unique<BehaviorClass>(); },        \
			&MoveBehaviorDetail::ParseParams<ParamsClass>,                                               			&MoveBehaviorDetail::DefaultParams<ParamsClass> });                                          \
	}
