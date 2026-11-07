#include "UiSettings.h"

#include <UI/Preview/UiDatabase.h>

#include <map>
#include <vector>

namespace
{
    std::map<std::string, UiSettings::Entry>& Entries()
    {
        static std::map<std::string, UiSettings::Entry> entries;
        return entries;
    }
}

void UiSettings::Register(const Entry& e)
{
    Entries()[e.Key] = e;
    UiDatabase::MarkDirty("setting " + e.Key);
}

const UiSettings::Entry* UiSettings::Find(const std::string& key)
{
    const auto it = Entries().find(key);
    return it == Entries().end() ? nullptr : &it->second;
}

std::vector<std::string> UiSettings::Keys()
{
    std::vector<std::string> keys;
    for (const auto& [key, entry] : Entries())
        keys.push_back(key);
    return keys;
}

std::vector<std::string> UiSettings::ChoicesOf(const Entry& e)
{
    return e.GetChoices ? e.GetChoices() : e.Choices;
}

void UiSettings::Snapshot(const std::vector<std::string>& keys, std::vector<double>& out)
{
    out.clear();
    for (const std::string& key : keys)
    {
        const Entry* e = Find(key);
        out.push_back(e != nullptr && e->Get ? e->Get() : 0.0);
    }
}

void UiSettings::Restore(const std::vector<std::string>& keys, const std::vector<double>& values)
{
    for (size_t i = 0; i < keys.size() && i < values.size(); ++i)
    {
        const Entry* e = Find(keys[i]);
        if (e != nullptr && e->Set && !e->ReadOnly)
            e->Set(values[i]);
    }
}

namespace
{
    struct Session { int Id; std::map<std::string, double> Before; };
    std::vector<Session>& Sessions()
    {
        static std::vector<Session> sessions;
        return sessions;
    }
    int g_nextSession = 1;
}

int UiSettings::BeginSession()
{
    Sessions().push_back(Session{ g_nextSession, {} });
    return g_nextSession++;
}

void UiSettings::NoteChange(const std::string& key, double previousValue)
{
    for (Session& s : Sessions())
        s.Before.emplace(key, previousValue);   // keeps the first value
}

bool UiSettings::SessionChanged(int session)
{
    for (const Session& s : Sessions())
        if (s.Id == session)
            for (const auto& [key, before] : s.Before)
            {
                const Entry* e = Find(key);
                if (e != nullptr && e->Get && e->Get() != before)
                    return true;
            }
    return false;
}

void UiSettings::RevertSession(int session)
{
    for (const Session& s : Sessions())
        if (s.Id == session)
            for (const auto& [key, before] : s.Before)
            {
                const Entry* e = Find(key);
                if (e != nullptr && e->Set && !e->ReadOnly)
                    e->Set(before);
            }
}

void UiSettings::EndSession(int session)
{
    auto& all = Sessions();
    for (size_t i = 0; i < all.size(); ++i)
        if (all[i].Id == session)
        {
            all.erase(all.begin() + static_cast<long>(i));
            return;
        }
}

const char* UiSettings::KindName(Kind k)
{
    switch (k)
    {
    case Kind::Bool: return "bool";
    case Kind::Int: return "int";
    case Kind::Float: return "float";
    case Kind::Choice: return "choice";
    }
    return "float";
}
