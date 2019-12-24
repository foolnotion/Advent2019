#ifndef UTIL_HPP
#define UTIL_HPP

#include <fmt/format.h>
#include <fmt/color.h>

#include <iostream>
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
#include <unordered_set>
#include <unordered_map>
#include <set>

#include <Eigen/Eigen>

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

static std::vector<std::string> split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

template<typename T>
static std::vector<T> to_vec(const std::string& s, char delimiter) {
    auto tokens = split(s, delimiter);
    std::vector<T> vec;
    for(const auto& t : tokens) {
        if (auto res = parse_number<T>(t); !res.has_value()) {
            throw new std::runtime_error(fmt::format("Error: cannot parse '{}' as an {}\n", t, typeid(T).name()));
        } else {
            vec.push_back(res.value());
        }
    }
    return vec;
}

#endif
