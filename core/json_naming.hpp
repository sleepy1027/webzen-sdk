#pragma once

#include "core/fixed_string.hpp"

namespace webzen {

// PascalCase -> snake_case at compile time. Assumes acronyms are already
// normalized to a single capital (Url, Id) as required by this project's
// naming convention -- ALL-CAPS runs (URL, ID) would produce
// "u_r_l" here, which is why the convention bans them.
template <fixed_string Name>
consteval auto to_snake_case() {
    constexpr std::size_t in_len = Name.size();

    constexpr std::size_t extra = [] {
        std::size_t count = 0;
        for (std::size_t i = 1; i < in_len; ++i) {
            const char c = Name.data[i];
            if (c >= 'A' && c <= 'Z') {
                ++count;
            }
        }
        return count;
    }();

    fixed_string<in_len + extra + 1> out{};
    std::size_t pos = 0;
    for (std::size_t i = 0; i < in_len; ++i) {
        char c = Name.data[i];
        if (i > 0 && c >= 'A' && c <= 'Z') {
            out.data[pos++] = '_';
        }
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
        out.data[pos++] = c;
    }
    out.data[pos] = '\0';
    return out;
}

}  // namespace webzen

// Computes the snake_case JSON key for `member` from its PascalCase C++ name
// at compile time, so glz::meta specializations never hardcode key strings
// (see .claude/skills/game-sdk-core-architecture/SKILL.md, JSON section).
#define SDK_FIELD(T, member)                                       \
    []() -> const char* {                                          \
        static constexpr auto s_key =                              \
            ::webzen::to_snake_case<::webzen::fixed_string(#member)>(); \
        return s_key.data.data();                                  \
    }(),                                                           \
        &T::member
