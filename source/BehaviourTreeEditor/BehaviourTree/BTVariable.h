#pragma once
#include <string>
#include <variant>
#include <json.hpp>

using nlohmann::json;

#include "Blackboard.h"

class BTVariable {
public:
    enum class SourceType {
        Constant,
        Blackboard
    };

    using Value = std::variant<int, float, double, bool, std::string>;

    // Constructors for constant values
    BTVariable() : sourceType_(SourceType::Constant), value_() {}
    BTVariable(int value) : sourceType_(SourceType::Constant), value_(value) {}
    BTVariable(float value) : sourceType_(SourceType::Constant), value_(value) {}
    BTVariable(double value) : sourceType_(SourceType::Constant), value_(value) {}
    BTVariable(bool value) : sourceType_(SourceType::Constant), value_(value) {}
    BTVariable(const char* value) : sourceType_(SourceType::Constant), value_(std::string(value)) {}
    BTVariable(const std::string& value) : sourceType_(SourceType::Constant), value_(value) {}

    // Constructor for blackboard binding
    static BTVariable FromBlackboard(const std::string& key) {
        BTVariable var;
        var.sourceType_ = SourceType::Blackboard;
        var.blackboardKey_ = key;
        return var;
    }

    // Value resolution
    template<typename T>
    bool Resolve(const Blackboard* blackboard, T& outValue) const {
        if (sourceType_ == SourceType::Constant) {
            try {
                outValue = std::get<T>(value_);
                return true;
            }
            catch (const std::bad_variant_access&) {
                return false;
            }
        }
        else { // Blackboard
            return blackboard && blackboard->GetValue(blackboardKey_, outValue);
        }
    }

    template<typename T>
    T Resolve(const Blackboard* blackboard, const T& defaultValue = T{}) const {
        T value;
        return Resolve(blackboard, value) ? value : defaultValue;
    }

    // Getters
    SourceType GetSourceType() const { return sourceType_; }
    const std::string& GetBlackboardKey() const { return blackboardKey_; }
    const Value& GetConstantValue() const { return value_; }

    // Setters
    void SetConstantValue(const Value& value) {
        sourceType_ = SourceType::Constant;
        value_ = value;
    }
    void SetBlackboardKey(const std::string& key) {
        sourceType_ = SourceType::Blackboard;
        blackboardKey_ = key;
    }

    // JSON Serialization
    json ToJson() const;
    void FromJson(const json& j);

private:
    SourceType sourceType_ = SourceType::Constant;
    Value value_;
    std::string blackboardKey_;
};