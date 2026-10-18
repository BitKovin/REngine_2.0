#pragma once

#ifndef DISTRIBUTION

#include <Data/MoveSetData.h>

#include <memory>
#include <string>
#include <vector>

// Moveset editor of the combat debug panel (COMBAT_DESIGN.md §10.4, §11). Edits a copy of a body's moveset - every
// field: the moveset's buffers and slot tables, and for the selected move everything its JSON holds (general fields,
// entries, hits, combo link, hold, cancels, recovery, root motion, tracking, armor, i-frames, cues, behaviour params with
// their defaults). It works on the move's serialized form, so fields added to the data later appear by themselves.
//   - typed widgets: enums and move references as dropdowns, tags as toggles, lists with add / remove
//   - dimmed = the default value; right-click a field to reset it; hover any field for what it does
//   - the copy is validated after every edit (errors / warnings listed); Apply makes it live at the bodies' next move
//     boundary, Apply + Save also writes it into the authored file (only the edited values change, §10.4)
//   - moves can be added, duplicated and deleted
class MoveSetEditor
{
public:
	// The selected body's moveset (the live, cached one).
	void Draw(const MoveSetData& live);
	// Another body selected: drop the edits.
	void Reset();

private:
	std::shared_ptr<MoveSetData> editing;   // the edited copy
	std::shared_ptr<MoveSetData> base;      // what it started from (or was last saved as): Save writes the difference
	int selected = 0;
	std::string status;
	std::vector<std::string> errors;
	std::vector<std::string> warnings;
	char newMoveId[64] = {};

	void Validate();
	bool DrawMoveSetFields();
	bool DrawMoveList();
	bool DrawMove(MoveData& move);
	void DrawActions();
};

#endif
