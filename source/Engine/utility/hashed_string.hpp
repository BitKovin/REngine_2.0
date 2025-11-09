#pragma once

#include <string>
#include <functional>
#include <cstddef>
#include <string_view>

// Maximum performance hashed_string with full safety
class hashed_string {
    std::string str_;  // Owned string for safety
    size_t hash_;      // Precomputed hash

public:
    // Constructors - compute hash immediately
    hashed_string() noexcept : str_(), hash_(0) {}

    hashed_string(const std::string& s)
        : str_(s), hash_(compute_hash(s)) {
    }

    hashed_string(const char* s)
        : str_(s ? s : ""), hash_(compute_hash(str_)) {
    }

    // Copy constructors
    hashed_string(const hashed_string& other) = default;
    hashed_string(hashed_string&& other) noexcept = default;

    // Assignment operators
    hashed_string& operator=(const hashed_string&) = default;
    hashed_string& operator=(hashed_string&&) noexcept = default;

    // Accessors - direct and fast
    const std::string& str() const noexcept { return str_; }
    const char* c_str() const noexcept { return str_.c_str(); }
    size_t hash() const noexcept { return hash_; }
    size_t size() const noexcept { return str_.size(); }
    size_t length() const noexcept { return str_.length(); }
    bool empty() const noexcept { return str_.empty(); }

    // Fast equality - hash comparison first (99% case), then string comparison
    bool operator==(const hashed_string& other) const noexcept {
        return hash_ == other.hash_ && str_ == other.str_;
    }

    bool operator!=(const hashed_string& other) const noexcept {
        return !(*this == other);
    }

    // Ordered comparison for containers that need it
    bool operator<(const hashed_string& other) const noexcept {
        return str_ < other.str_;
    }

    // Clear - reset hash to 0
    void clear() noexcept {
        str_.clear();
        hash_ = 0;
    }

private:
    // Fast FNV-1a hash implementation
    static size_t compute_hash(const std::string& s) noexcept {
        return compute_hash(std::string_view(s));
    }

    static size_t compute_hash(std::string_view s) noexcept {
        // FNV-1a constants
        constexpr size_t offset_basis = 14695981039346656037ULL;
        constexpr size_t prime = 1099511628211ULL;

        size_t hash = offset_basis;
        for (char c : s) {
            hash ^= static_cast<size_t>(static_cast<unsigned char>(c));
            hash *= prime;
        }
        return hash;
    }
};

// Specialize std::hash
namespace std {
    template<>
    struct hash<hashed_string> {
        size_t operator()(const hashed_string& hs) const noexcept {
            return hs.hash();  // Direct return of precomputed hash
        }
    };
}