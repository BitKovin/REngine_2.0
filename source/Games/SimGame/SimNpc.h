#pragma once

// The physical body of a sim agent (SLICE_M2C_BODIES.md): the driver of a character. An Entity on the engine's CharacterController that runs its own
// navigation on the level's nav mesh (PathFollowQuery) and its own state code; the sim only dictates targets: SetGoal(next corner of the agent's
// route), Stop(). Where the body stands is reported back by the bridge, and the sim moves the agent's progress along its route to match.
//
// How it looks is a CharacterBody (SLICE_M2F_CHARACTERS.md): every frame this fills a CharacterState and applies it. This class owns when it is shown
// (the warm-up ring) and the fade.
//
// Never saved with the level: the sim owns the agent, the body is recreated when the sim proposes one.

#include <Characters/CharacterBody.h>

#include <Character/CharacterController.h>
#include <Entity.h>
#include <EntityHandle.h>
#include <Navigation/PathFollowQuery.h>

#include <Sim/Movement/SimSpeedTable.h>

class SimNpc : public Entity
{
public:
	enum class State : uint8_t
	{
		Idle,    // no goal, or standing at the end of the route
		Moving,  // walking to the goal
		Blocked, // wants to move but is not getting anywhere
	};

	// Seconds a body has to live before it may be shown walking in from the ring (SLICE_M2F_CHARACTERS.md section 6).
	static constexpr float WarmupSeconds = 1.5f;

	// Creates a body standing at `feet` (level-local) looking along `heading`, in the ring (Warming: not drawn) when `warming`. Null when no level is loaded.
	// `model` is a CharacterModels index; `variant` (the sim entity's number) picks the clothes colours of an untextured model, so the same person always looks the same.
	static SimNpc* SpawnAt(const glm::vec3& feet, const glm::vec3& heading, int model, float speedScale, uint32_t variant, bool warming);

	SimNpc();
	~SimNpc() override;

	void Start() override;
	void Update() override;
	void Destroy() override;

	// Shown or in the ring. A body that is Warming walks and animates but is not drawn; it is shown at once (SetWarming(false)) or faded in (then FadeIn).
	void SetWarming(bool warming) { this->warming = warming; }
	bool Warming() const { return warming; }
	// Lived long enough to be mid-stride when it is shown.
	bool WarmedUp() const { return age >= WarmupSeconds; }
	float Age() const { return age; }
	// Drawn in the last frame (false while warming or out of the camera's view).
	bool WasRendered() const;

	// The sim's target (level-local feet position). `final` marks the end of the route; otherwise `next` (when `hasNext`) is the
	// corner after it, which the body walks on to without stopping at the goal.
	void SetGoal(const glm::vec3& feet, const glm::vec3& next, bool hasNext, sim::movement::Gait gait, bool final);
	void Stop();

	// Appearing and disappearing: the character's alpha runs 0 -> 1 (FadeIn, right after SpawnAt) or 1 -> 0 (FadeOut) over FadeSeconds. A body that is
	// faded out is invisible and should be destroyed by its owner. Without either call the body is simply there.
	static constexpr float FadeSeconds = 1.0f;
	void FadeIn();
	void FadeOut();
	bool FadedOut() const { return fadingOut && alpha <= 0.0f; }
	bool FadingOut() const { return fadingOut; }

	glm::vec3 FeetPosition() const;
	State GetState() const { return state; }
	int Model() const { return model; }
private:
	CharacterController* controller = nullptr;
	EntityHandle bodyHandle;      // the CharacterBody that is how this character looks
	PathFollowQuery pathFollow;

	int model = 0;
	uint32_t variant = 0;
	bool warming = false;
	float age = 0.0f;
	float alpha = 1.0f;
	bool fadingIn = false;
	bool fadingOut = false;
	float speedScale = 1.0f;
	State state = State::Idle;
	sim::movement::Gait gait = sim::movement::Gait::Walk;
	bool hasGoal = false;
	bool finalGoal = false;
	glm::vec3 goal{ 0.0f };           // level-local feet position
	glm::vec3 nextGoal{ 0.0f };
	bool hasNextGoal = false;
	glm::vec3 facing{ 0.0f, 0.0f, 1.0f };
	float blockedFor = 0.0f;

	void UpdateFade(float deltaTime);
	void Navigate(float deltaTime);
	CharacterState MakeState(const glm::vec3& velocity) const;
	CharacterBody* Body() const;
};
