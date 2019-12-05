#ifndef UTIL_HPP
#define UTIL_HPP

#include <fmt/format.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <execution>
#include <fstream>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

template <typename T>
static std::optional<T> parse_number(const std::string& s)
{
    static_assert(std::is_arithmetic_v<T>);
    T result;
    if (auto [p, ec] = std::from_chars(s.data(), s.data() + s.size(), result); ec == std::errc()) {
        return { result };
    }
    return {};
}

std::vector<std::string> split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

#endif
