#pragma once
#include <unordered_map>
#include <string>
#include <variant>
#include <functional>
#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>
#include "../json.hpp"
#include "../glm.h"
#include "../Helpers/JsonHelper.hpp"
#include "../utility/hashed_string.hpp"
using nlohmann::json;
// Forward declaration only
class BTVariable;

class Blackboard {
public:
    using Value = std::variant<int, float, double, bool, std::string, vec3>;
    using ObserverCallback = std::function<void(const std::string&, const Value&)>;
    using ObserverID = size_t;

private:
    std::unordered_map<hashed_string, int> intData_;
    std::unordered_map<hashed_string, float> floatData_;
    std::unordered_map<hashed_string, double> doubleData_;
    std::unordered_map<hashed_string, bool> boolData_;
    std::unordered_map<hashed_string, std::string> stringData_;
    std::unordered_map<hashed_string, vec3> vec3Data_;

    std::unordered_map<hashed_string, std::vector<std::pair<ObserverID, ObserverCallback>>> observers_;
    ObserverID nextObserverId_ = 1;

    void NotifyObservers(const hashed_string& key, const Value& value) const {
        auto it = observers_.find(key);
        if (it != observers_.end()) {
            for (const auto& pair : it->second) {
                pair.second(key.str(), value);
            }
        }
    }

    template<typename T>
    struct DataMap;

    template<typename T>
    auto& GetDataMap() { return DataMap<T>{}(*this); }

    template<typename T>
    const auto& GetDataMap() const { return DataMap<T>{}(*this); }

public:
    template<typename T>
    void SetValue(const hashed_string& key, const T& value) {
        auto& map = GetDataMap<T>();
        auto it = map.find(key);
        if (it != map.end() && it->second == value) return;
        map[key] = value;
        // Erase from other maps to ensure no overlap
        RemoveValueFromOtherMaps<T>(key);
        NotifyObservers(key, Value{ value });
    }

    template<typename T>
    bool GetValue(const hashed_string& key, T& outValue) const {
        auto& map = GetDataMap<T>();
        auto it = map.find(key);
        if (it != map.end()) {
            outValue = it->second;
            return true;
        }
        return false;
    }

    template<typename T>
    T GetValue(const hashed_string& key, const T& defaultValue = T{}) const {
        T value;
        return GetValue(key, value) ? value : defaultValue;
    }

    bool HasValue(const hashed_string& key) const {
        return intData_.contains(key) || floatData_.contains(key) || doubleData_.contains(key) ||
            boolData_.contains(key) || stringData_.contains(key) || vec3Data_.contains(key);
    }

    void RemoveValue(const hashed_string& key) {
        intData_.erase(key);
        floatData_.erase(key);
        doubleData_.erase(key);
        boolData_.erase(key);
        stringData_.erase(key);
        vec3Data_.erase(key);
        NotifyObservers(key, Value{}); // Optional: notify removal with empty variant or special signal
    }

    void Clear() {
        intData_.clear();
        floatData_.clear();
        doubleData_.clear();
        boolData_.clear();
        stringData_.clear();
        vec3Data_.clear();
        observers_.clear();
        nextObserverId_ = 1;
    }

    // BTVariable resolution helpers - implementation in cpp
    template<typename T>
    bool ResolveBTVariable(const BTVariable& var, T& outValue) const;
    template<typename T>
    T ResolveBTVariable(const BTVariable& var, const T& defaultValue = T{}) const;

    // Editor support methods
    std::vector<hashed_string> GetKeys() const {
        std::vector<hashed_string> keys;
        for (const auto& pair : intData_) keys.push_back(pair.first);
        for (const auto& pair : floatData_) keys.push_back(pair.first);
        for (const auto& pair : doubleData_) keys.push_back(pair.first);
        for (const auto& pair : boolData_) keys.push_back(pair.first);
        for (const auto& pair : stringData_) keys.push_back(pair.first);
        for (const auto& pair : vec3Data_) keys.push_back(pair.first);
        return keys;
    }

    int GetValueType(const std::string& keyStr) const {
        hashed_string key(keyStr);
        if (intData_.contains(key)) return 0;
        if (floatData_.contains(key)) return 1;
        if (doubleData_.contains(key)) return 2;
        if (boolData_.contains(key)) return 3;
        if (stringData_.contains(key)) return 4;
        if (vec3Data_.contains(key)) return 5;
        return -1; // Not found
    }

    std::string GetValueAsString(const std::string& keyStr) const {
        hashed_string key(keyStr);
        if (auto it = intData_.find(key); it != intData_.end()) {
            return std::to_string(it->second);
        }
        if (auto it = floatData_.find(key); it != floatData_.end()) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(6) << it->second;
            std::string str = oss.str();
            str.erase(str.find_last_not_of('0') + 1, std::string::npos);
            if (!str.empty() && str.back() == '.') str.pop_back();
            return str;
        }
        if (auto it = doubleData_.find(key); it != doubleData_.end()) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(6) << it->second;
            std::string str = oss.str();
            str.erase(str.find_last_not_of('0') + 1, std::string::npos);
            if (!str.empty() && str.back() == '.') str.pop_back();
            return str;
        }
        if (auto it = boolData_.find(key); it != boolData_.end()) {
            return it->second ? "true" : "false";
        }
        if (auto it = stringData_.find(key); it != stringData_.end()) {
            return it->second;
        }
        if (auto it = vec3Data_.find(key); it != vec3Data_.end()) {
            std::ostringstream oss;
            oss << "(" << it->second.x << ", " << it->second.y << ", " << it->second.z << ")";
            return oss.str();
        }
        return "";
    }

    std::string GetValueTypeString(const std::string& keyStr) const {
        int type = GetValueType(keyStr);
        switch (type) {
        case 0: return "int";
        case 1: return "float";
        case 2: return "double";
        case 3: return "bool";
        case 4: return "string";
        case 5: return "vec3";
        default: return type == -1 ? "not_found" : "unknown";
        }
    }

    bool SetValueFromString(const std::string& keyStr, const std::string& value, int type) {
        hashed_string key(keyStr);
        try {
            switch (type) {
            case 0: SetValue(key, std::stoi(value)); break;
            case 1: SetValue(key, std::stof(value)); break;
            case 2: SetValue(key, std::stod(value)); break;
            case 3:
                if (value == "true" || value == "1" || value == "True") SetValue(key, true);
                else if (value == "false" || value == "0" || value == "False") SetValue(key, false);
                else return false;
                break;
            case 4: SetValue(key, value); break;
                // case 5: vec3 parsing would need implementation, e.g., parse "(x,y,z)"
            default: return false;
            }
            return true;
        }
        catch (const std::exception&) {
            return false;
        }
    }

    bool RemoveValueByKey(const std::string& keyStr) {
        hashed_string key(keyStr);
        if (HasValue(key)) {
            RemoveValue(key);
            return true;
        }
        return false;
    }

    ObserverID AddObserver(const hashed_string& key, ObserverCallback callback) {
        ObserverID id = nextObserverId_++;
        observers_[key].emplace_back(id, std::move(callback));
        return id;
    }

    void RemoveObserver(const hashed_string& key, ObserverID id) {
        auto it = observers_.find(key);
        if (it != observers_.end()) {
            auto& vec = it->second;
            vec.erase(std::remove_if(vec.begin(), vec.end(), [id](const auto& p) { return p.first == id; }), vec.end());
            if (vec.empty()) observers_.erase(it);
        }
    }

    json ToJson() const;
    void FromJson(const json& j);

private:
    template<typename T>
    void RemoveValueFromOtherMaps(const hashed_string& key) {
        if constexpr (!std::is_same_v<T, int>) intData_.erase(key);
        if constexpr (!std::is_same_v<T, float>) floatData_.erase(key);
        if constexpr (!std::is_same_v<T, double>) doubleData_.erase(key);
        if constexpr (!std::is_same_v<T, bool>) boolData_.erase(key);
        if constexpr (!std::is_same_v<T, std::string>) stringData_.erase(key);
        if constexpr (!std::is_same_v<T, vec3>) vec3Data_.erase(key);
    }
};

// Specializations for DataMap
template<> struct Blackboard::DataMap<int> { auto& operator()(Blackboard& bb) { return bb.intData_; } auto& operator()(const Blackboard& bb) const { return bb.intData_; } };
template<> struct Blackboard::DataMap<float> { auto& operator()(Blackboard& bb) { return bb.floatData_; } auto& operator()(const Blackboard& bb) const { return bb.floatData_; } };
template<> struct Blackboard::DataMap<double> { auto& operator()(Blackboard& bb) { return bb.doubleData_; } auto& operator()(const Blackboard& bb) const { return bb.doubleData_; } };
template<> struct Blackboard::DataMap<bool> { auto& operator()(Blackboard& bb) { return bb.boolData_; } auto& operator()(const Blackboard& bb) const { return bb.boolData_; } };
template<> struct Blackboard::DataMap<std::string> { auto& operator()(Blackboard& bb) { return bb.stringData_; } auto& operator()(const Blackboard& bb) const { return bb.stringData_; } };
template<> struct Blackboard::DataMap<vec3> { auto& operator()(Blackboard& bb) { return bb.vec3Data_; } auto& operator()(const Blackboard& bb) const { return bb.vec3Data_; } };