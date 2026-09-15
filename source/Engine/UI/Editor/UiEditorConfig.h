#pragma once

// ─────────────────────────────────────────────────────────────────────────
// Single switch for the UI layout hot-reload system: the per-frame file
// mtime polling AND the `ui_reload` console command path. Every other file
// in the layout/editor system checks THIS macro rather than re-deriving its
// own condition, so turning hot reload off is always a one-line change here.
//
// Deliberately its own macro rather than reusing DISTRIBUTION directly:
// - keeps every other file decoupled from knowing what "DISTRIBUTION" means
// - lets you flip hot reload off independently of DISTRIBUTION later
//   (e.g. a "locked down QA build" that's still a dev build otherwise)
// - when disabled, the polling/bookkeeping code doesn't exist at all (not
//   just "disabled at runtime"), so it can't cost anything in a shipping
//   binary and can't accidentally ship a working reload path.
// ─────────────────────────────────────────────────────────────────────────
#ifndef DISTRIBUTION
    #define UI_LAYOUT_HOT_RELOAD_ENABLED 1
#else
    #define UI_LAYOUT_HOT_RELOAD_ENABLED 0
#endif

// The ImGui-based editor itself (tree view, details panel, picking) rides
// on top of the existing DebugUiEnabled machinery and is a separate concern
// from hot reload -- it's gated by UI_EDITOR_ENABLED so it can also be
// compiled out of DISTRIBUTION even if you ever decoupled hot reload from
// DISTRIBUTION above.
#ifndef DISTRIBUTION
    #define UI_EDITOR_ENABLED 1
#else
    #define UI_EDITOR_ENABLED 0
#endif
