#include "../IntCode/intcode.hpp"
#include "../util.hpp"

struct Point {
    int64_t x;
    int64_t y;

    bool operator==(const Point rhs) const
    {
        return std::tie(x, y) == std::tie(rhs.x, rhs.y);
    }
    bool operator<(const Point rhs) const
    {
        return std::tie(x, y) < std::tie(rhs.x, rhs.y);
    }
};

static const std::array<uint8_t, 4> directions = {
    1, // north
    2, // south
    3, // west
    4 // east
};

static const std::array<uint8_t, 4> opposites = { 2, 1, 4, 3 };
static uint8_t Opposite(uint8_t dir)
{
    return opposites[dir - 1];
};

int main(int argc, char** argv)
{
    std::ifstream infile("./input.txt");
    std::string line;
    std::getline(infile, line);
    auto program = to_vec<int64_t>(line, ',');

    // part 1
    Point pos { 0, 0 };
    auto update_pos = [](Point p, int64_t dir) {
        if (dir == 1) // north
            --p.y;
        if (dir == 2) // south
            ++p.y;
        if (dir == 3) // west
            --p.x;
        if (dir == 4) // east
            ++p.x;
        return p;
    };

    IntComputer comp(program);
    auto move_droid = [&](int64_t dir) {
        comp.SetInput(dir);
        comp.Run();
        return comp.GetOutput();
    };

    std::set<Point> walls;
    std::set<Point> visited;
    Point oxy; // oxygen position
    size_t oxyPathLength = 0;
    size_t openLocations = 0; 

    std::function<void(int64_t)> explore = [&](int64_t dir) {
        auto p = update_pos(pos, dir);
        if (visited.count(p) == 1 || walls.count(p) == 1) {
            // already visited or hit a wall (again)
            return;
        }
        auto s = move_droid(dir);
        assert(s < 3);
        if (s == 0) {
            // the droid hit a wall
            walls.insert(p);
            return;
        }
        ++openLocations;
        pos = update_pos(pos, dir);
        visited.insert(pos);
        if (s == 2) {
            oxy = pos;
            oxyPathLength = visited.size();
        } 
        // continue the exploration
        for (auto d : directions) {
            explore(d);
        }
        visited.erase(pos);
        pos = update_pos(pos, Opposite(dir));
        move_droid(Opposite(dir));
    };

    for (auto d : directions) {
        explore(d);
    }
    fmt::print("found oxygen at ({},{}) and path to the oxygen with length {}\n", oxy.x, oxy.y, oxyPathLength);

    // part 2
    std::set<Point> filled;
    size_t maxDepth = 0;
    std::function<void(Point, size_t)> oxygenate = [&](Point p, size_t depth) {
        if (filled.count(p) > 0 || walls.count(p) > 0) {
            return;
        }
        maxDepth = std::max(depth, maxDepth);
        filled.insert(p);
        if (filled.size() == openLocations) {
            fmt::print("area oxygenated in {} minutes\n", maxDepth);
            return;
        }
        for (auto d : directions) {
            oxygenate(update_pos(p, d), depth + 1);
        }
    };
    oxygenate(oxy, 0);
    return 0;
}
