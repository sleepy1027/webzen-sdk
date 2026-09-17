#pragma once

#include <array>
#include <cstddef>
#include <string_view>

namespace webzen::json {

// A compile-time string usable as a non-type template parameter (structural
// type: only public members, no user-declared destructor). Backs
// to_snake_case() in json_naming.hpp so DTO field names never have to be
// hand-typed as JSON keys.
template <std::size_t N>
struct fixed_string {
    std::array<char, N> data{};

    constexpr fixed_string() = default;

    constexpr fixed_string(const char (&str)[N]) {
        for (std::size_t i = 0; i < N; ++i) {
            data[i] = str[i];
        }
    }

    [[nodiscard]] constexpr std::size_t size() const { return N - 1; }
    [[nodiscard]] constexpr std::string_view view() const { return {data.data(), N - 1}; }
    [[nodiscard]] constexpr const char* c_str() const { return data.data(); }
};

template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N>;

}  // namespace webzen::json
