#pragma once
#include <string>
#include <vector>

class UiElem;

// Replaces {name} and {name:fmt} with values found by walking from `element` up its UiElem ancestors.
// "${KEY}" is localisation, not a variable: a '{' directly after '$' is skipped. "{{" is a literal '{' and "}}" a literal '}'.
// Then localises, then substitutes again (translated strings may contain {vars}). Values may contain
// {refs} themselves; substitution repeats at most 4 levels deep.
std::string UiSubstitute(const UiElem& element, const std::string& raw, std::vector<std::string>* usedNames);
