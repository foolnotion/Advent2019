#include "../util.hpp"
#include <regex>
#include <functional>
#include "gsl/gsl"

int main(int argc, char** argv) {
    if (argc < 2) {
        fmt::print("Provide an input file\n");
        return 1;
    }

    std::ifstream infile(argv[1]);
    std::string line;

    std::vector<int> input;
    std::getline(infile, line);
    for (auto c : line) {
        input.push_back(c - '0');
    }
    fmt::print("input size: {}\n", input.size());

    // part 1
    auto pattern = std::vector<int> { 0, 1, 0, -1 };
    auto get = [&](int i) { return pattern[i % pattern.size()]; };

    auto digits = input;
    auto apply = [&](int p) {
        int m = p+1; // multiplicity of each digit in the pattern

        int last = pattern[0];
        int c = 1; // left shift - skip index 0

        int result = 0;
        int idx = 0;
        while (true) {
            for (int i = 0; i < pattern.size(); ++i) {
                auto d = get(i);
                if (d == 0) {
                    idx += m-c;
                    if (idx >= digits.size()) {
                        goto LAST;
                    }
                } else {
                    for (int j = c; j < m; ++j) {
                        result += (d * digits[idx]);
                        ++idx;
                        if (idx >= digits.size()) {
                            goto LAST;
                        }
                    }
                }
                c = 0;
            }
        }
LAST:
        result += last * digits.back();
        return std::abs(result % 10);
    };

    int nPhases = 100;

    std::vector<int> indices(digits.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::vector<int> tmp(digits.size());
    for (int k = 0; k < nPhases; ++k) {
        std::transform(std::execution::par_unseq, indices.begin(), indices.end(), tmp.begin(), [&](auto i) { return apply(i); });
        std::swap(digits, tmp);
    }

    fmt::print("After 100 phases:\n");
    for(auto d : digits) { fmt::print("{}", d); } 
    fmt::print("\n");

    size_t offset = 0;
    for (int i = 0; i < 7; ++i) {
        offset += input[i] * std::pow(10, 6 - i);
    }
    fmt::print("offset: {}\n", offset);

    // part 2
    // first we need to calculate the offset
    digits.clear(); digits.reserve(input.size() * 10000);
    for(int i = 0; i < 10000; ++i) {
        std::copy(input.begin(), input.end(), std::back_inserter(digits));
    }
    digits.erase(digits.begin(), digits.begin() + offset);
    fmt::print("digits size: {}\n", digits.size());

    for (int k = 0; k < nPhases; ++k) {
        for (int i = digits.size() - 2; i >= 0; --i) {
            digits[i] = std::abs(digits[i] + digits[i+1]) % 10;
        }
    }

    for (int i = 0; i < 8; ++i) {
        fmt::print("{}", digits[i]);
    }
    fmt::print("\n");

    return 0;
}
