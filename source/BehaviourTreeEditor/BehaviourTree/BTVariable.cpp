#include "BTVariable.h"
#include "Blackboard.h"


json BTVariable::ToJson() const {
    json j;
    j["sourceType"] = static_cast<int>(sourceType_);

    if (sourceType_ == SourceType::Constant) {
        std::visit([&j](const auto& v) {
            j["value"] = v;
            }, value_);
    }
    else {
        j["blackboardKey"] = blackboardKey_;
    }

    return j;
}

void BTVariable::FromJson(const json& j) {
    if (j.contains("sourceType")) {
        sourceType_ = static_cast<SourceType>(j["sourceType"].get<int>());
    }

    if (sourceType_ == SourceType::Constant && j.contains("value")) {
        const auto& value = j["value"];
        if (value.is_number_integer()) {
            value_ = value.get<int>();
        }
        else if (value.is_number_float()) {
            value_ = value.get<float>();
        }
        else if (value.is_boolean()) {
            value_ = value.get<bool>();
        }
        else if (value.is_string()) {
            value_ = value.get<std::string>();
        }
    }
    else if (sourceType_ == SourceType::Blackboard && j.contains("blackboardKey")) {
        blackboardKey_ = j["blackboardKey"].get<std::string>();
    }
}