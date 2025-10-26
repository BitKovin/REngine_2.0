#include "Blackboard.h"
#include "BTVariable.h"

// Implement the template methods here after including BTVariable.h
template<typename T>
bool Blackboard::ResolveBTVariable(const BTVariable& var, T& outValue) const {
    return var.Resolve(this, outValue);
}

template<typename T>
T Blackboard::ResolveBTVariable(const BTVariable& var, const T& defaultValue) const {
    return var.Resolve(this, defaultValue);
}

Blackboard::ObserverID Blackboard::AddObserver(const std::string& key, ObserverCallback callback) {
    ObserverID id = nextObserverId_++;
    observers_[key][id] = callback;
    return id;
}

void Blackboard::RemoveObserver(const std::string& key, ObserverID id) {
    auto keyIt = observers_.find(key);
    if (keyIt != observers_.end()) {
        keyIt->second.erase(id);
        if (keyIt->second.empty()) {
            observers_.erase(keyIt);
        }
    }
}

void Blackboard::NotifyObservers(const std::string& key, const Value& value) {
    auto keyIt = observers_.find(key);
    if (keyIt != observers_.end()) {
        // Create a copy of callbacks to avoid issues if callbacks modify the observers
        auto callbacks = keyIt->second;
        for (const auto& [id, callback] : callbacks) {
            callback(key, value);
        }
    }
}

json Blackboard::ToJson() const {
    json j;
    for (const auto& [key, value] : data_) {
        std::visit([&j, &key](const auto& v) {
            j[key] = v;
            }, value);
    }
    return j;
}

void Blackboard::FromJson(const json& j) {
    data_.clear();
    for (auto it = j.begin(); it != j.end(); ++it) {
        const std::string& key = it.key();
        const auto& value = it.value();

        if (value.is_number_integer()) {
            data_[key] = value.get<int>();
        }
        else if (value.is_number_float()) {
            data_[key] = value.get<float>();
        }
        else if (value.is_boolean()) {
            data_[key] = value.get<bool>();
        }
        else if (value.is_string()) {
            data_[key] = value.get<std::string>();
        }
    }
}

// Explicit template instantiation for the bool-returning version (with reference parameter)
template bool Blackboard::ResolveBTVariable<int>(const BTVariable&, int&) const;
template bool Blackboard::ResolveBTVariable<float>(const BTVariable&, float&) const;
template bool Blackboard::ResolveBTVariable<double>(const BTVariable&, double&) const;
template bool Blackboard::ResolveBTVariable<bool>(const BTVariable&, bool&) const;
template bool Blackboard::ResolveBTVariable<std::string>(const BTVariable&, std::string&) const;
template bool Blackboard::ResolveBTVariable<vec3>(const BTVariable&, vec3&) const;

// Explicit template instantiation for the value-returning version (with const reference parameter)
template int Blackboard::ResolveBTVariable<int>(const BTVariable&, const int&) const;
template float Blackboard::ResolveBTVariable<float>(const BTVariable&, const float&) const;
template double Blackboard::ResolveBTVariable<double>(const BTVariable&, const double&) const;
template bool Blackboard::ResolveBTVariable<bool>(const BTVariable&, const bool&) const;
template std::string Blackboard::ResolveBTVariable<std::string>(const BTVariable&, const std::string&) const;
template vec3 Blackboard::ResolveBTVariable<vec3>(const BTVariable&, const vec3&) const;

// Also instantiate the GetValue templates
template bool Blackboard::GetValue<int>(const std::string&, int&) const;
template bool Blackboard::GetValue<float>(const std::string&, float&) const;
template bool Blackboard::GetValue<double>(const std::string&, double&) const;
template bool Blackboard::GetValue<bool>(const std::string&, bool&) const;
template bool Blackboard::GetValue<std::string>(const std::string&, std::string&) const;
template bool Blackboard::GetValue<vec3>(const std::string&, vec3&) const;

template int Blackboard::GetValue<int>(const std::string&, const int&) const;
template float Blackboard::GetValue<float>(const std::string&, const float&) const;
template double Blackboard::GetValue<double>(const std::string&, const double&) const;
template bool Blackboard::GetValue<bool>(const std::string&, const bool&) const;
template std::string Blackboard::GetValue<std::string>(const std::string&, const std::string&) const;
template vec3 Blackboard::GetValue<vec3>(const std::string&, const vec3&) const;

// And SetValue templates
template void Blackboard::SetValue<int>(const std::string&, const int&);
template void Blackboard::SetValue<float>(const std::string&, const float&);
template void Blackboard::SetValue<double>(const std::string&, const double&);
template void Blackboard::SetValue<bool>(const std::string&, const bool&);
template void Blackboard::SetValue<std::string>(const std::string&, const std::string&);
template void Blackboard::SetValue<vec3>(const std::string&, const vec3&);