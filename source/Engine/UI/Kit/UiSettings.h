#pragma once
#include <functional>
#include <string>
#include <vector>

// The settings the markup can bind to (bind="setting:<key>"). Registered by the engine kit and by games; values are numbers:
// Bool 0/1, Int, Float, Choice (an index into Choices).
namespace UiSettings
{
    enum class Kind { Bool, Int, Float, Choice };
    struct Entry
    {
        std::string Key;
        Kind Type = Kind::Float;
        std::function<double()> Get;
        std::function<void(double)> Set;
        std::vector<std::string> Choices;                      // Choice: labels or ${KEYS}
        double Min = 0, Max = 1, Step = 0;
        std::function<std::vector<std::string>()> GetChoices;  // optional: a list that is only known at run time (display modes)
        std::string Label;                                     // ${KEY} of the row label, for tools
        bool ReadOnly = false;
    };
    void Register(const Entry& e);                             // replaces an entry with the same key
    const Entry* Find(const std::string& key);
    std::vector<std::string> Keys();                           // sorted
    std::vector<std::string> ChoicesOf(const Entry& e);        // GetChoices() when set, else Choices
    void Snapshot(const std::vector<std::string>& keys, std::vector<double>& out);
    void Restore(const std::vector<std::string>& keys, const std::vector<double>& values);
    const char* KindName(Kind k);

    // Change journal for pages that confirm their changes: a session remembers the value a key had before its first
    // change made through a bound widget, so the page can revert them.
    int BeginSession();
    void NoteChange(const std::string& key, double previousValue);   // called by bound widgets before Set
    bool SessionChanged(int session);                                // some recorded key differs from its remembered value now
    void RevertSession(int session);                                 // restores the remembered values
    void EndSession(int session);
}
