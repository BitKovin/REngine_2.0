#pragma once

#include <string>
#include <functional>
#include <cstddef>

class hashed_string {
    std::string str_;
    std::size_t hash_;

public:

    hashed_string()
        : str_(), hash_(std::hash<std::string>{}(str_)) {
    }

    // Constructor from std::string
    hashed_string(const std::string& s)
        : str_(s), hash_(std::hash<std::string>{}(s)) {
    }

    // Constructor from C-string
    hashed_string(const char* s)
        : hashed_string(std::string(s)) {
    }


    // Accessors
    const std::string& str() const noexcept { return str_; }
    std::size_t hash() const noexcept { return hash_; }

    // Equality comparison (checks hash first, then string)
    bool operator==(const hashed_string& other) const {
        return hash_ == other.hash_ && str_ == other.str_;
    }

    bool operator!=(const hashed_string& other) const {
        return !(*this == other);
    }
};

// Specialize std::hash for hashed_string
namespace std {
    template<>
    struct hash<hashed_string> {
        std::size_t operator()(const hashed_string& hs) const noexcept {
            return hs.hash();
        }
    };
}