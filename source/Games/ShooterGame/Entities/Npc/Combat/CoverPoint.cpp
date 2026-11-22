#include <Entity.h>
#include <Physics.h>

#include "CoverFinder.h"

// Optional map hint for combat cover (COMBAT_AI_DESIGN.md §10.3): "an NPC
// could fight from here". It only adds a candidate to the cover search; the
// finder still checks it against the real threat, so a badly placed point
// never produces bad cover.
//
//   type      auto | low | corner   expected kind of cover (auto = classify)
//   priority  0..1                   score bonus
class CoverPoint : public Entity
{
public:
	void FromData(EntityData data) override
	{
		Entity::FromData(data);

		const std::string typeName = data.GetPropertyString("type");
		if (typeName == "low") type = CoverType::Low;
		else if (typeName == "corner") type = CoverType::Full;

		priority = glm::clamp(data.GetPropertyFloat("priority", priority), 0.0f, 1.0f);
	}

	void Start() override
	{
		Entity::Start();

		vec3 floor = Position;

		const auto hit = Physics::LineTrace(Position + vec3(0, 0.5f, 0), Position - vec3(0, 3.0f, 0), BodyType::World);
		if (hit.hasHit) floor = hit.position;

		CoverHint hint;
		hint.entityId = Id;
		hint.floor = floor;
		hint.priority = priority;
		hint.type = type;

		CoverFinder::AddHint(hint);
	}

	void Destroy() override
	{
		CoverFinder::RemoveHint(Id);

		Entity::Destroy();
	}

private:
	CoverType type = CoverType::None;
	float priority = 0.0f;
};

REGISTER_ENTITY(CoverPoint, "cover_point")
