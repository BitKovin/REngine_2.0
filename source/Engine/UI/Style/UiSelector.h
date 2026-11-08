#pragma once
#include <UI/Style/UiCssTokenizer.h>
#include <string>
#include <vector>

// Pulse pseudo-classes (PulseFirst..) are not states: an event puts the element in them for pulse-duration (see UiPulses()).
enum class UiPseudo { Hover, Active, Focus, FocusWithin, Disabled, Selected, Root, Entering, Leaving, Checked, Dragging,
                      PulseFirst };

struct UiCompound
{
    std::string Tag;                 // "" = any; "*" also stored as ""
    std::string Id;                  // "" = none
    std::vector<std::string> Classes;
    std::vector<UiPseudo> Pseudos;
};

enum class UiCombinator { Descendant, Child };

struct UiSelector
{
    std::vector<UiCompound> Parts;            // left to right
    std::vector<UiCombinator> Combinators;    // Parts.size() - 1 entries, Combinators[i] joins Parts[i] and Parts[i+1]
    int Specificity = 0;                      // ids * 10000 + (classes + pseudos) * 100 + tags
};

// Parses a selector list from prelude tokens (no trailing '{'). On error logs and returns false.
// An unknown pseudo-class only warns and drops that one selector.
bool UiParseSelectorList(const std::vector<UiCssToken>& prelude, const std::string& path, std::vector<UiSelector>& out);

class UiElem;
bool UiSelectorMatches(const UiSelector& selector, const UiElem& element);

// The events that start a pulse. A pulse is the element's reaction to something that happened to it (a click, a double click, a
// refused press, an edit): the pseudo-class named here matches for pulse-duration, so a rule can animate it and play sound-enter.
// The pseudo-class of entry i is UiPseudo(int(UiPseudo::PulseFirst) + i).
struct UiPulseInfo
{
    const char* Name;          // :click
    const char* Event;         // the UI event that starts it
    const char* AltEvent;      // a second event with the same meaning, or nullptr
    const char* Description;
};
const std::vector<UiPulseInfo>& UiPulses();

// True when the selector is about the disabled state (:disabled or :denied): its sounds still play on a disabled element.
bool UiSelectorMatchesDisabledState(const UiSelector& selector);

// Names of all supported pseudo-classes, in declaration order (used by the UI database).
const std::vector<std::string>& UiPseudoNames();
