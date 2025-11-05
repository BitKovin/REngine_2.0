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

    template<typename T>
    void SetValue(const hashed_string& key, const T& value) {
        data_[key] = value;
        //NotifyObservers(key, data_[key]);
    }

    template<typename T>
    bool GetValue(const hashed_string& key, T& outValue) const {
        auto it = data_.find(key);
        if (it != data_.end()) {
            try {
                outValue = std::get<T>(it->second);
                return true;
            }
            catch (const std::bad_variant_access&) {
                return false;
            }
        }
        return false;
    }

    template<typename T>
    T GetValue(const hashed_string& key, const T& defaultValue = T{}) const {
        T value;
        return GetValue(key, value) ? value : defaultValue;
    }

    bool HasValue(const hashed_string& key) const {
        return data_.count(key) > 0;
    }

    void RemoveValue(const hashed_string& key) {
        data_.erase(key);
    }

    void Clear() {
        data_.clear();
        observers_.clear();
        nextObserverId_ = 0;
    }

    // BTVariable resolution helpers - implementation in cpp
    template<typename T>
    bool ResolveBTVariable(const BTVariable& var, T& outValue) const;

    template<typename T>
    T ResolveBTVariable(const BTVariable& var, const T& defaultValue = T{}) const;

    // Observer system for abort conditions
    ObserverID AddObserver(const std::string& key, ObserverCallback callback);
    void RemoveObserver(const std::string& key, ObserverID id);

    // Editor support methods
    std::vector<hashed_string> GetKeys() const {
        std::vector<hashed_string> keys;
        for (const auto& pair : data_) {
            keys.push_back(pair.first);
        }
        return keys;
    }

    int GetValueType(const std::string& key) const {
        auto it = data_.find(key);
        if (it != data_.end()) {
            return static_cast<int>(it->second.index());
        }
        return -1; // Not found
    }

    std::string GetValueAsString(const std::string& key) const {
        auto it = data_.find(key);
        if (it != data_.end()) {
            return std::visit([](const auto& value) -> std::string {
                using T = std::decay_t<decltype(value)>;

                if constexpr (std::is_same_v<T, int>) {
                    return std::to_string(value);
                }
                else if constexpr (std::is_same_v<T, float>) {
                    std::ostringstream oss;
                    oss << std::fixed << std::setprecision(6) << value;
                    std::string str = oss.str();
                    // Remove trailing zeros and decimal point if not needed
                    str.erase(str.find_last_not_of('0') + 1, std::string::npos);
                    if (str.back() == '.') {
                        str.pop_back();
                    }
                    return str;
                }
                else if constexpr (std::is_same_v<T, double>) {
                    std::ostringstream oss;
                    oss << std::fixed << std::setprecision(6) << value;
                    std::string str = oss.str();
                    str.erase(str.find_last_not_of('0') + 1, std::string::npos);
                    if (str.back() == '.') {
                        str.pop_back();
                    }
                    return str;
                }
                else if constexpr (std::is_same_v<T, bool>) {
                    return value ? "true" : "false";
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    return value;
                }
                else {
                    return "unknown_type";
                }
                }, it->second);
        }
        return "";
    }

    std::string GetValueTypeString(const std::string& key) const {
        auto it = data_.find(key);
        if (it != data_.end()) {
            switch (it->second.index()) {
            case 0: return "int";
            case 1: return "float";
            case 2: return "double";
            case 3: return "bool";
            case 4: return "string";
            default: return "unknown";
            }
        }
        return "not_found";
    }

    bool SetValueFromString(const std::string& key, const std::string& value, int type) {
        try {
            switch (type) {
            case 0: // int
                SetValue(key, std::stoi(value));
                break;
            case 1: // float
                SetValue(key, std::stof(value));
                break;
            case 2: // double
                SetValue(key, std::stod(value));
                break;
            case 3: // bool
                if (value == "true" || value == "1" || value == "True") {
                    SetValue(key, true);
                }
                else if (value == "false" || value == "0" || value == "False") {
                    SetValue(key, false);
                }
                else {
                    return false; // Invalid boolean value
                }
                break;
            case 4: // string
                SetValue(key, value);
                break;
            default:
                return false; // Unknown type
            }
            return true;
        }
        catch (const std::exception&) {
            return false; // Conversion failed
        }
    }

    bool RemoveValueByKey(const std::string& key) {
        auto it = data_.find(key);
        if (it != data_.end()) {
            data_.erase(it);
            return true;
        }
        return false;
    }

    json ToJson() const;
    void FromJson(const json& j);

private:
    void NotifyObservers(const std::string& key, const Value& value);

    std::unordered_map<hashed_string, Value> data_;
    std::unordered_map<std::string, std::unordered_map<ObserverID, ObserverCallback>> observers_;
    ObserverID nextObserverId_ = 1;
};