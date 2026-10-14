#pragma once

#include <memory>
#include <string>
#include <vector>

class AIController;
class CombatCharacter;
class Entity;
struct CharacterInput;

struct AIContext
{
	CombatCharacter& Self;
	AIController& Controller;
	CharacterInput& Input;
	float DeltaTime = 0.0f;
	double Now = 0.0;
};

// Decision making for an AI-driven body (COMBAT_DESIGN.md §8). Runs inside the body's parallel update (worker
// threads, many bodies at once). A brain only uses AIController services (perception, movement, tokens, attacks,
// buttons) - it never touches health, meshes or other entities directly, which is what keeps it thread-safe. A
// Behaviour Tree-backed brain can implement this interface later without changing bodies or controllers.
class AIBrain
{
public:
	virtual ~AIBrain() = default;

	virtual void OnActivated(AIContext& context) { (void)context; }   // brain (re)takes control of the body
	virtual void Think(AIContext& context) = 0;
	virtual const char* DescribeState() const { return ""; }
};

struct AIBrainType
{
	std::string Name;
	std::unique_ptr<AIBrain> (*Create)() = nullptr;
};

class AIBrainRegistry
{
public:
	static int Register(AIBrainType type);
	static std::unique_ptr<AIBrain> Create(const std::string& name);   // nullptr if unknown
	static std::string KnownNames();

private:
	static std::vector<AIBrainType>& Types();
};

// REGISTER_AI_BRAIN("Melee", MeleeBrain)
#define REGISTER_AI_BRAIN(NameString, BrainClass)                                                        \
	namespace {                                                                                          \
		const int BrainClass##_registeredIndex = AIBrainRegistry::Register(AIBrainType{                   \
			NameString, []() -> std::unique_ptr<AIBrain> { return std::make_unique<BrainClass>(); } });  \
	}
