#ifndef STRING_HASH_H
#define STRING_HASH_H

#include <string_view>

/**
 * Computes a hash value for a string at compile time
 *
 * @param str The string to hash
 * @return A 64-bit hash value
 */
constexpr uint64_t hash(const std::string_view str) {
    uint64_t hash = 0;
    for (const char c : str) {
        hash = (hash * 131) + c;
    }
    return hash;
}

/**
 * String literal operator for creating hash values
 * Usage: "some_string"_hash
 */
constexpr uint64_t operator"" _hash(const char* str, const size_t len) {
    return hash(std::string_view(str, len));
}

#endif // STRING_HASH_H
