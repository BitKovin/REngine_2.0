#pragma once
#include <string>
#include <vector>

class UiElem;

// One call of an event attribute: onactivate="Push('a.xml'); PlaySound('click.wav')" is two calls.
struct UiActionCall
{
    std::string Name;
    std::vector<std::string> Args;
};

struct UiActionInfo
{
    std::string Name;
    std::string Args;          // signature for tools, e.g. "path"
    std::string Description;
};

struct UiEventInfo
{
    std::string Name;          // "activate"
    std::string Description;
};

namespace UiActions
{
    // Parses `Name(arg, arg); Name2()`. Arguments are 'text', "text", numbers or bare identifiers (kept as strings).
    bool Parse(const std::string& text, std::vector<UiActionCall>& out, std::string& error);

    // Runs one call for `sender`. The handler search starts at `searchFrom` (the sender when null) and walks up the UiElem ancestors;
    // the first element with a handler of that name runs it, else a built-in runs, else a warning is logged once per element and name.
    void Dispatch(UiElem& sender, UiElem* searchFrom, const UiActionCall& call);
    // Plays a 2D UI sound (the PlaySound action; also the sound-enter / sound-exit style properties).
    // `volume` multiplies the sound; the same path plays at most once per 40 ms (a slider drag or a row of hovers does not machine-gun it).
    void PlaySoundPath(const std::string& path, float volume = 1.f);

    bool IsBuiltin(const std::string& name);
    const std::vector<UiActionInfo>& Builtins();   // sorted by name
    const std::vector<UiEventInfo>& Events();      // sorted by name
    bool IsKnownEvent(const std::string& name);

    // Warns about calls in `root`'s tree that no ancestor-or-self handles and no built-in covers.
    void Validate(UiElem& root);

    // "ui/pause.xml" -> "GameData/ui/pause.xml". Paths that already start with GameData/ are unchanged.
    std::string NormalizeLayoutPath(const std::string& path);
}
