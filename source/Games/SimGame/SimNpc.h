#pragma once

// The physical body of a sim agent (SLICE_M2C_BODIES.md). An Entity on the engine's CharacterController with a skeletal mesh
// and a locomotion animator. It runs its own navigation on the level's nav mesh (PathFollowQuery) and its own state code; the sim
// only dictates targets: SetGoal(next corner of the agent's route), Stop(). Where the body stands is reported back by the
// bridge, and the sim moves the agent's progress along its route to match.
//
// Never saved with the level: the sim owns the agent, the body is recreated when the sim proposes one.

#include <Character/CharacterController.h>
#include <Entity.h>
#include <Animator.h>
#include <Navigation/PathFollowQuery.h>
#include <SkeletalMesh.hpp>

#include <Sim/Movement/SimSpeedTable.h>

// Idle / walk / run layer over the animations that ship in the model (idle, walk, run), picked by the speed on the ground.
class SimNpcAnimator : public Animator
{
public:
	SimNpcAnimator(Entity* owner, std::string modelPath) : Animator(owner), modelPath(std::move(modelPath)) {}

	float movementSpeed = 0.0f;

	void Update() override;

protected:
	void LoadAssets() override;
	// The base class returns an empty pose (a T-pose); this animator's pose is its one locomotion layer.
	AnimationPose ProcessResultPose() override { return locomotion ? locomotion->GetAnimationPose() : AnimationPose(); }

private:
	std::string modelPath;
	Animation* locomotion = nullptr;
};

class SimNpc : public Entity
{
public:
	enum class State : uint8_t
	{
		Idle,    // no goal, or standing at the end of the route
		Moving,  // walking to the goal
		Blocked, // wants to move but is not getting anywhere
	};

	// Models the sim can ask for (Appearance::model).
	static const char* ModelPath(int model);

	// Creates a body standing at `feet` (level-local) looking along `heading`. Null when no level is loaded.
	// `variant` (the sim entity's number) picks the clothes colours of the untextured citizen model, so the same person always looks the same.
	static SimNpc* SpawnAt(const glm::vec3& feet, const glm::vec3& heading, int model, float speedScale, uint32_t variant);

	SimNpc();
	~SimNpc() override;

	void Start() override;
	void Update() override;
	void LoadAssets() override;

	// The sim's target (level-local feet position). `final` marks the end of the route; otherwise `next` (when `hasNext`) is the
	// corner after it, which the body walks on to without stopping at the goal.
	void SetGoal(const glm::vec3& feet, const glm::vec3& next, bool hasNext, sim::movement::Gait gait, bool final);
	void Stop();

	// Appearing and disappearing: the model's alpha runs 0 -> 1 (FadeIn, right after SpawnAt) or 1 -> 0 (FadeOut) over FadeSeconds. A body that is
	// faded out is invisible and should be destroyed by its owner. Without either call the body is simply there.
	static constexpr float FadeSeconds = 1.0f;
	void FadeIn();
	void FadeOut();
	bool FadedOut() const { return fadingOut && alpha <= 0.0f; }
	bool FadingOut() const { return fadingOut; }

	glm::vec3 FeetPosition() const;
	State GetState() const { return state; }
	int Model() const { return model; }
	// Set once the mesh was drawn since the last frame; animations only advance for visible bodies.
	bool WasRendered() const { return mesh && mesh->WasRended; }

private:
	CharacterController* controller = nullptr;
	SkeletalMesh* mesh = nullptr;
	std::unique_ptr<SimNpcAnimator> animator;
	PathFollowQuery pathFollow;

	int model = 0;
	uint32_t variant = 0;
	bool paintApplied = false;
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

	void ApplyPaint();
	void UpdateFade(float deltaTime);
	void Navigate(float deltaTime);
	void UpdateAnimation(float horizontalSpeed);
};
