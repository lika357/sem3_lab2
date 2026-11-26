#pragma once

#include <string>
#include <type_traits>

template <typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
size_t GetHashCode(T value) {
    unsigned long long x = static_cast<unsigned long long>(value);
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    return static_cast<size_t>(x & 0x7fffffff);
}
inline size_t GetHashCode(const std::string& value) {
    unsigned int hash = 2166136261u;
    for(char c : value) {
        hash ^= static_cast<unsigned char>(c);
        hash *= 16777619u;
    }
    return static_cast<size_t>(hash & 0x7fffffff);
}
