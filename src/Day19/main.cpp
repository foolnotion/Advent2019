#include "../IntCode/intcode.hpp"
#include "../util.hpp"
#include <fmt/color.h>

#include <unordered_map>

#include "tbb/parallel_for.h"

int main(int argc, char** argv)
{
    std::vector<int64_t> program { 109,424,203,1,21101,11,0,0,1105,1,282,21102,1,18,0,1105,1,259,1201,1,0,221,203,1,21102,31,1,0,1106,0,282,21101,38,0,0,1106,0,259,21001,23,0,2,22102,1,1,3,21101,0,1,1,21102,57,1,0,1106,0,303,1202,1,1,222,20102,1,221,3,21002,221,1,2,21101,259,0,1,21102,80,1,0,1106,0,225,21101,0,51,2,21101,0,91,0,1106,0,303,1202,1,1,223,20101,0,222,4,21101,259,0,3,21102,225,1,2,21101,225,0,1,21101,118,0,0,1105,1,225,20102,1,222,3,21102,1,152,2,21102,133,1,0,1105,1,303,21202,1,-1,1,22001,223,1,1,21102,1,148,0,1105,1,259,1202,1,1,223,20101,0,221,4,21002,222,1,3,21102,1,17,2,1001,132,-2,224,1002,224,2,224,1001,224,3,224,1002,132,-1,132,1,224,132,224,21001,224,1,1,21101,195,0,0,105,1,108,20207,1,223,2,21002,23,1,1,21102,1,-1,3,21102,214,1,0,1105,1,303,22101,1,1,1,204,1,99,0,0,0,0,109,5,1202,-4,1,249,22101,0,-3,1,21202,-2,1,2,22102,1,-1,3,21101,250,0,0,1106,0,225,22101,0,1,-4,109,-5,2105,1,0,109,3,22107,0,-2,-1,21202,-1,2,-1,21201,-1,-1,-1,22202,-1,-2,-2,109,-3,2106,0,0,109,3,21207,-2,0,-1,1206,-1,294,104,0,99,22101,0,-2,-2,109,-3,2105,1,0,109,5,22207,-3,-4,-1,1206,-1,346,22201,-4,-3,-4,21202,-3,-1,-1,22201,-4,-1,2,21202,2,-1,-1,22201,-4,-1,1,21201,-2,0,3,21102,1,343,0,1105,1,303,1106,0,415,22207,-2,-3,-1,1206,-1,387,22201,-3,-2,-3,21202,-2,-1,-1,22201,-3,-1,3,21202,3,-1,-1,22201,-3,-1,2,21202,-4,1,1,21102,1,384,0,1105,1,303,1105,1,415,21202,-4,-1,-4,22201,-4,-3,-4,22202,-3,-2,-2,22202,-2,-4,-4,22202,-3,-2,-3,21202,-4,-1,-2,22201,-3,-2,1,22102,1,1,-4,109,-5,2105,1,0 };

    const int size = 50;

    Eigen::Matrix<char, -1, -1> grid(size, size);
    grid.fill(0);

    int64_t out;

    auto getValue = [&](int x, int y) {
        std::array<int, 2> inputs = { x, y };
        int64_t out = 0;
        int idx = 0;

        IntComputer computer(program);
        while (!computer.Halted()) {
            auto op = computer.CurrentOpcode();
            if (op == OpCode::In) {
                computer.SetInput(inputs[idx++]);
            }
            computer.Step();
            if (op == OpCode::Out) {
                return computer.GetOutput();
            }
        }
        throw std::runtime_error("no output instructions.");
    };

    for (int i = 0; i < grid.rows(); ++i) {
        for (int j = 0; j < grid.cols(); ++j) {
            grid(i, j) = getValue(i, j) ? '#' : '.';
        }
    }

    fmt::print("{}\n", (grid.array() == '#').count());

    auto limit = 100000;
    std::vector<int> indices(limit);
    std::iota(indices.begin(), indices.end(), 0);

    int dim = 100;

    // determined by visual inspection
    auto a1 = 0.53;
    auto a2 = 0.64;

    for (int i = 0; i < limit; ++i) {
        auto left = std::floor(i * a1);
        auto right = std::ceil(i * a2);

        for (int j = left; j < right; ++j) {
            if (getValue(i, j+dim-1) == 0) break; // no point iterating further
            if (getValue(i, j) == 0 || getValue(i+dim-1, j) == 0) continue;
            fmt::print("i = {}, j = {}, ans = {}\n", i, j, i * 10000 + j);
            return 0;
        }
    }

    return 0;
}

