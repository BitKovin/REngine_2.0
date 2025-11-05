#pragma once

#include <string>
#include <functional>
#include <cstddef>

// Custom string wrapper that caches its hash value
class hashed_string {
    std::string str_;
    std::size_t hash_;

public:
    // Default constructor
    hashed_string() noexcept
        : str_(), hash_(0) {
    }

    // Constructor from std::string
    hashed_string(const std::string& s)
        : str_(s), hash_(std::hash<std::string>{}(s)) {
    }

    // Constructor from C-string
    hashed_string(const char* s)
        : str_(s ? s : ""), hash_(std::hash<std::string>{}(str_)) {
    }

    // Copy & move constructors (defaulted are fine)
    hashed_string(const hashed_string&) = default;
    hashed_string(hashed_string&&) noexcept = default;
    hashed_string& operator=(const hashed_string&) = default;
    hashed_string& operator=(hashed_string&&) noexcept = default;

    // Accessors
    const std::string& str() const noexcept { return str_; }
    const const char* c_str() const noexcept { return str_.c_str(); }
    std::size_t hash() const noexcept { return hash_; }

    // Comparison for equality (required for unordered containers)
    bool operator==(const hashed_string& other) const noexcept {
        return hash_ == other.hash_ && str_ == other.str_;
    }

    bool operator!=(const hashed_string& other) const noexcept {
        return !(*this == other);
    }

    // Less-than operator (required for std::set or std::map)
    bool operator<(const hashed_string& other) const noexcept {
        // compare by hash first, then lexicographically
        if (hash_ != other.hash_) return hash_ < other.hash_;
        return str_ < other.str_;
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
