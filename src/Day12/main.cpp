#include "../util.hpp"

#include <regex>

using Mat = Eigen::Matrix<int64_t, 4, 3>;

int main(int argc, char** argv)
{
    std::ifstream infile("./input.txt");
    std::string line;

    Mat pos = Mat::Zero(4, 3); // position
    Mat vel = Mat::Zero(4, 3); // velocity
    
    std::regex re("<|>|x|y|z|=| ");
    int i = 0;
    while(std::getline(infile, line)) {
        auto result = std::regex_replace(line, re, "");
        auto values = to_vec<int>(result, ',');

        pos(i, 0) = values[0];
        pos(i, 1) = values[1];
        pos(i, 2) = values[2];
        ++i;
    }
    size_t total       = 0;
    Mat p0             = pos;

    std::array<int64_t, 6> cycles;
    cycles.fill(-1);

    for (size_t step = 1; ; ++step) {
        total = 0;
        for (size_t i = 0; i < pos.rows(); ++i) {
            for (size_t j = i+1; j < pos.rows(); ++j) {
                auto g = (pos.row(j)-pos.row(i)).cwiseMax(-1l).cwiseMin(1l);
                vel.row(i) += g;
                vel.row(j) -= g;
            }
            total  += pos.row(i).array().abs().sum() * vel.row(i).array().abs().sum();
        }
        pos += vel;

        for(int i = 0; i < 3; ++i) {
            if (cycles[i]   < 0 && pos.col(i) == p0.col(i)) { cycles[i]   = step+1; }
            if (cycles[i+3] < 0 && vel.col(i).isZero(0))    { cycles[i+3] = step;   } 
        }

        if (std::none_of(cycles.begin(), cycles.end(), [](int64_t c) { return c < 0; })) {
            break;
        }
    }
    auto cycle = std::reduce(std::execution::seq, cycles.begin(), cycles.end(), 1l, [](auto a, auto b) { return std::lcm(a,b); });
    fmt::print("{}\n", cycle);

    return 0;
}

