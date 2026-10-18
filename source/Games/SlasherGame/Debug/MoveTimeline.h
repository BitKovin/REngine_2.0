#pragma once

#ifndef DISTRIBUTION

struct MoveSlot;
struct RecoverySlot;

// Move timeline of the combat debug panel (COMBAT_DESIGN.md §11) for one move slot, in clip seconds (the move clock;
// real seconds = clip seconds / playbackSpeed):
//   header     move, clock, speed; a (?) legend
//   Now        what the move is doing at this moment, in words (wind-up, hit, light / heavy zone, open cancels...)
//   ruler      seconds; the view zooms to where the move's windows are (the rest of the clip is only the tail)
//   lanes      Hits, Combo (light / heavy zone), one Cancel lane per window (what it lets in), Armor, Invulnerable
//   markers    hold commit (yellow), recovery start (purple), now (white)
// Hovering the timeline lists everything at the time under the cursor; hovering a lane name explains the lane.
void DrawMoveTimeline(const char* label, const MoveSlot& slot, float playbackSpeed);

// The recovery of an attack that already ended (its clip playing out), one line with its explanation.
void DrawRecoveryStatus(const RecoverySlot& recovery);

#endif
