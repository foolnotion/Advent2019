#include "../util.hpp"
#include "../IntCode/intcode.hpp"

using Matrix = Eigen::Matrix<int, -1, -1>;
using Point = std::pair<int, int>;

struct Direction {
    static constexpr int Up    = 0;
    static constexpr int Right = 1;
    static constexpr int Down  = 2;
    static constexpr int Left  = 3;
    int Current;
    void TurnLeft() { if (--Current < Up) Current = Left; }
    void TurnRight() { if (++Current > Left) Current = Up; }
};

int main(int argc, char** argv)
{
    std::ifstream infile("./input.txt");
    std::string line;
    std::getline(infile, line);
    auto tokens = split(line, ',');
    std::vector<int64_t> program;

    for (const auto& t : tokens) {
        if (auto res = parse_number<int64_t>(t); res.has_value()) {
            program.push_back(res.value());
        }
    }

    int w = 500;
    int h = 500;
    Matrix grid(w, h); grid.fill(0);

    // set robot start in the middle of the grid
    Point p { w / 2, h / 2 };

    // robot starts looking up
    Direction dir { Direction::Up };

    auto turn = [&](int t) {
        switch(t) {
            case 0:
                dir.TurnLeft();
                break;
            case 1:
                dir.TurnRight();
                break;
            default:
                throw std::runtime_error(fmt::format("Turn: argument {} out of range\n", t));
        }
    };

    auto move = [&]() {
        switch(dir.Current) {
            case Direction::Up:
                p.first--;
                break;
            case Direction::Left:
                p.second--; 
                break;
            case Direction::Down:
                p.first++;
                break;
            case Direction::Right:
                p.second++;
                break;
        }
    };

    IntComputer comp(program);

    // used to center the painted area at the end
    auto [xmin, ymin] = p;
    auto [xmax, ymax] = p;

    std::set<Point> painted;
    int64_t input, output; 

    // for part 1 set this to 0, 
    // for part 2 set this to 1
    grid(p.first, p.second) = 1;
    while(!comp.Halted()) {
        // run code to determine color
        auto [x, y] = p;
        input = grid(x, y);
        comp.SetInput(input);
        comp.Run();
        output = comp.GetOutput();
        grid(x, y) = output;
        painted.insert(p);

        // run code to determine where to turn
        comp.SetInput(input);
        comp.Run();
        output = comp.GetOutput();

        // turn and move
        turn(output);
        move();

        if (xmin > x) { xmin = x; }
        if (xmax < x) { xmax = x; }
        if (ymin > y) { ymin = y; }
        if (ymax < y) { ymax = y; }
    }

    auto area = grid.block(xmin, ymin, xmax-xmin+1, ymax-ymin+1);
    for (int i = 0; i < area.rows(); ++i) {
        for (int j = 0; j < area.cols(); ++j) {
            auto c = area(i, j) == 0 ? " " : "#";
            fmt::print("{}", c);
        }
        fmt::print("\n");
    }
    fmt::print("{}\n", painted.size());
    return 0;
}
