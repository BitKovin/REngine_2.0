#include "Blackboard.h"
#include "BTVariable.h"

template<typename T>
bool Blackboard::ResolveBTVariable(const BTVariable& var, T& outValue) const {
    return var.Resolve(this, outValue);
}

template<typename T>
T Blackboard::ResolveBTVariable(const BTVariable& var, const T& defaultValue) const {
    return var.Resolve(this, defaultValue);
}

json Blackboard::ToJson() const {
    json j;
    for (const auto& [key, val] : intData_) j[key.str()] = val;
    for (const auto& [key, val] : floatData_) j[key.str()] = val;
    for (const auto& [key, val] : doubleData_) j[key.str()] = val;
    for (const auto& [key, val] : boolData_) j[key.str()] = val;
    for (const auto& [key, val] : stringData_) j[key.str()] = val;
    for (const auto& [key, val] : vec3Data_) {
        j[key.str()] = { val.x, val.y, val.z }; // Assuming vec3 to array
    }
    return j;
}

void Blackboard::FromJson(const json& j) {
    Clear(); // Clear existing data
    for (auto it = j.begin(); it != j.end(); ++it) {
        const std::string& keyStr = it.key();
        hashed_string key(keyStr);
        const auto& value = it.value();
        if (value.is_number_integer()) {
            SetValue(key, value.get<int>());
        }
        else if (value.is_number_float()) {
            SetValue(key, value.get<float>());
        }
        else if (value.is_boolean()) {
            SetValue(key, value.get<bool>());
        }
        else if (value.is_string()) {
            SetValue(key, value.get<std::string>());
        }
        else if (value.is_array() && value.size() == 3) {
            // Assuming vec3 from array
            SetValue(key, vec3(value[0].get<float>(), value[1].get<float>(), value[2].get<float>()));
        }
    }
}

// Explicit template instantiations
// ResolveBTVariable
template bool Blackboard::ResolveBTVariable<int>(const BTVariable&, int&) const;
template bool Blackboard::ResolveBTVariable<float>(const BTVariable&, float&) const;
template bool Blackboard::ResolveBTVariable<double>(const BTVariable&, double&) const;
template bool Blackboard::ResolveBTVariable<bool>(const BTVariable&, bool&) const;
template bool Blackboard::ResolveBTVariable<std::string>(const BTVariable&, std::string&) const;
template bool Blackboard::ResolveBTVariable<vec3>(const BTVariable&, vec3&) const;

template int Blackboard::ResolveBTVariable<int>(const BTVariable&, const int&) const;
template float Blackboard::ResolveBTVariable<float>(const BTVariable&, const float&) const;
template double Blackboard::ResolveBTVariable<double>(const BTVariable&, const double&) const;
template bool Blackboard::ResolveBTVariable<bool>(const BTVariable&, const bool&) const;
template std::string Blackboard::ResolveBTVariable<std::string>(const BTVariable&, const std::string&) const;
template vec3 Blackboard::ResolveBTVariable<vec3>(const BTVariable&, const vec3&) const;

// GetValue
template bool Blackboard::GetValue<int>(const hashed_string&, int&) const;
template bool Blackboard::GetValue<float>(const hashed_string&, float&) const;
template bool Blackboard::GetValue<double>(const hashed_string&, double&) const;
template bool Blackboard::GetValue<bool>(const hashed_string&, bool&) const;
template bool Blackboard::GetValue<std::string>(const hashed_string&, std::string&) const;
template bool Blackboard::GetValue<vec3>(const hashed_string&, vec3&) const;

template int Blackboard::GetValue<int>(const hashed_string&, const int&) const;
template float Blackboard::GetValue<float>(const hashed_string&, const float&) const;
template double Blackboard::GetValue<double>(const hashed_string&, const double&) const;
template bool Blackboard::GetValue<bool>(const hashed_string&, const bool&) const;
template std::string Blackboard::GetValue<std::string>(const hashed_string&, const std::string&) const;
template vec3 Blackboard::GetValue<vec3>(const hashed_string&, const vec3&) const;

// SetValue
template void Blackboard::SetValue<int>(const hashed_string&, const int&);
template void Blackboard::SetValue<float>(const hashed_string&, const float&);
template void Blackboard::SetValue<double>(const hashed_string&, const double&);
template void Blackboard::SetValue<bool>(const hashed_string&, const bool&);
template void Blackboard::SetValue<std::string>(const hashed_string&, const std::string&);
template void Blackboard::SetValue<vec3>(const hashed_string&, const vec3&);