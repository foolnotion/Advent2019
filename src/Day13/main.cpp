#include "../util.hpp"
#include "../IntCode/intcode.hpp"

struct Tile {
    static constexpr int Empty   = 0;
    static constexpr int Wall    = 1;
    static constexpr int Block   = 2;
    static constexpr int Paddle  = 3;
    static constexpr int Ball    = 4;
};

int main(int argc, char** argv)
{
    std::ifstream infile("./input.txt");
    std::string line;
    std::getline(infile, line);
    auto program = to_vec<int64_t>(line, ',');

    // part 1 + 2
    IntComputer comp(program);
    size_t nb = 0; // number of blocks
    auto step = [&]() -> int64_t { comp.Run(); return comp.GetOutput(); };
    int64_t score = 0;
    int64_t xb, xp; // paddle and ball positions on the X axis

    comp[0] = 2l; // insert coins
    while(!comp.Halted()) {
        auto x = step();
        auto y = step();
        auto z = step();
        nb += z == Tile::Block;

        if (z == Tile::Ball)   { xb = x; }
        if (z == Tile::Paddle) { xp = x; } 
        if (z > Tile::Ball || z < Tile::Empty) { score = z; }
        auto dx = std::clamp(xb-xp, -1l, 1l);
        comp.SetInput(dx);
    }

    fmt::print("{}\n", nb);
    fmt::print("{}\n", score);
    return 0;
}
