#pragma once

#include "../json.hpp"

// Field serialization macro (write into JSON)
#define SERIALIZE_FIELD(jsonObj, field) \
    jsonObj[#field] = field;

// Field deserialization macro (read from JSON)
#define DESERIALIZE_FIELD(jsonObj, field) \
    if (jsonObj.contains(#field)) jsonObj.at(#field).get_to(field);

// Compact version inside a member function (uses `this->`)
#define SERIALIZE_MEMBER(jsonObj, field) \
    jsonObj[#field] = this->field;

#define DESERIALIZE_MEMBER(jsonObj, field) \
    if (jsonObj.contains(#field)) jsonObj.at(#field).get_to(this->field);
