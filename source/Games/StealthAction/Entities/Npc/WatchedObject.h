#pragma once

// Makes a door or an item something NPCs can notice has changed (NPC_AI_DESIGN.md §13.3). The object gets an observation
// target with its state as a tag ("state:open"); an NPC that looks at it remembers the state, and a state that is not the one
// it saw last is a Curious event for it. A new state only shows after it has held for the settle time of the data, so a
// door that an NPC opened to walk through (and that closes behind it) changes nothing.

#include <AiPerception/ObservationTarget.h>

#include <memory>
#include <string>

class Entity;

class WatchedObject
{
public:

	~WatchedObject();

	// Call once the entity stands in the level (its Id exists).
	void Start(const Entity& owner);

	// The point NPCs look at, and the state the object is in now. `settle`: the state shows after it has held for the settle
	// time (a door); otherwise at once (an item that is gone stays gone). The first call shows the state at once.
	void Update(const glm::vec3& sightPoint, const std::string& state, bool settle);

	void Stop();

private:

	std::shared_ptr<ObservationTarget> target;

	std::string shown;        // the state tag the NPCs see
	std::string wanted;       // the state the object is in
	double wantedSince = 0.0;

	void Show(const std::string& state);
};
