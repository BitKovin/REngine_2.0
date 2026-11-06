#pragma once
#include <UI/Style/UiCssTokenizer.h>
#include <string>
#include <vector>

enum class UiPseudo { Hover, Active, Focus, FocusWithin, Disabled, Selected, Root, Entering, Leaving, Checked };

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

// Names of all supported pseudo-classes, in declaration order (used by the UI database).
const std::vector<std::string>& UiPseudoNames();
