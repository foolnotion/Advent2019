#include "../IntCode/intcode.hpp"
#include "../util.hpp"
#include <fmt/color.h>

#include <unordered_map>

struct Point {
    int64_t x;
    int64_t y;
    char c;

    bool operator==(const Point rhs) const
    {
        return std::tie(x, y) == std::tie(rhs.x, rhs.y);
    }
    bool operator<(const Point rhs) const
    {
        return std::tie(x, y) < std::tie(rhs.x, rhs.y);
    }
};

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print("Provide an input file\n");
        return 1;
    }
    std::ifstream infile(argv[1]);
    std::string line;
    std::getline(infile, line);
    auto program = to_vec<int64_t>(line, ',');

    // part 1
    IntComputer computer(program);

    std::vector<Point> points;
    int x = 0;
    int y = 0;
    int n = 0;

    Point robot{0,0,'^'};
    Point start = robot; // to reuse later

    char c{0}, d{0};
    while (!computer.Halted()) {
        computer.Run();
        d = c;
        c = static_cast<char>(computer.GetOutput());
        if (c == '\n') {
            if (c != d) {
                x = 0;
                y = y + 1;
            }
        } else {
            ++n;
            if (c == '#') {
                points.push_back(Point { x, y, c });
            }
            if (c == '^') {
                robot = { x, y, c };
            }
            x = x + 1;
        }
    }

    fmt::print("points: {}\n", points.size());
    int h = y;
    int w = n / h;
    fmt::print("w: {}, h: {}\n", w, h);

    int rows = w+1;
    int cols = h+1;

    Eigen::Matrix<char, -1, -1> m(rows, cols);
    m.fill(static_cast<int>('.'));

    for(auto p : points) {
        m(p.x, p.y) = p.c;
    }
    m(robot.x, robot.y) = robot.c;

    auto sum = 0u;
    auto intersections = 0u;
    for(auto p : points) {
        auto [i, j, ch] = p;
        if (ch != '#' || i == 0 || j == 0 || i == w || j == h) {
            continue;
        }
        if (ch == m(i-1, j) && ch == m(i+1, j) && ch == m(i, j-1) && ch == m(i, j+1)) {
            //m(i, j) = 'O';
            sum += i * j;
            ++intersections;
        }
    }
    std::cout << m.transpose() << "\n";
    fmt::print("{}\n", sum);

    auto canMoveUp    = [&]() { return robot.y > 0 && m(robot.x, robot.y-1) == '#'; };
    auto canMoveDown  = [&]() { return robot.y < h && m(robot.x, robot.y+1) == '#'; };
    auto canMoveLeft  = [&]() { return robot.x > 0 && m(robot.x-1, robot.y) == '#'; };
    auto canMoveRight = [&]() { return robot.x < w && m(robot.x+1, robot.y) == '#'; };

    // used to keep track of the robot's orientation
    auto advanceRobot = [&]() {
        switch(robot.c) {
            case '^':
                if (canMoveUp()) {
                    robot.y--;
                    return true;
                }
                break;
            case 'v':
                if(canMoveDown()) {
                    robot.y++;
                    return true;
                }
                break;
            case '<':
                if (canMoveLeft()) {
                    robot.x--;
                    return true;
                }
                break;
            case '>': 
                if (canMoveRight()) {
                    robot.x++;
                    return true;
                }
                break;
        }
        return false;
    };

    // part 2
    // we first want to generate a path starting from the robot's initial position
    // we try some simple movement rules for a very simple path finder
    // - move forward as far as possible
    // - follow the scaffold (turn as needed and generate turn instructions)
    // - prefer moving forward if we have that choice 
    int remaining = points.size() + intersections; // remaining scaffold tiles unvisited. intersection points are visited twice

    std::vector<std::string> path;
    std::map<std::pair<char, char>, char> moving = {
        { std::make_pair('^', '>'), 'R' },
        { std::make_pair('^', '<'), 'L' },
        { std::make_pair('v', '<'), 'R' },
        { std::make_pair('v', '>'), 'L' },
        { std::make_pair('>', 'v'), 'R' },
        { std::make_pair('>', '^'), 'L' },
        { std::make_pair('<', '^'), 'R' },
        { std::make_pair('<', 'v'), 'L' },
    };

    while (remaining > 0) {
        // move forward until end 
        int steps = 0;

        char d = robot.c;
        if (canMoveUp() && robot.c != 'v') {
            robot.c = '^';
        }
        else if (canMoveDown() && robot.c != '^') {
            robot.c = 'v';
        }
        else if (canMoveLeft() && robot.c != '>') {
            robot.c = '<';
        }
        else if (canMoveRight() && robot.c != '<') {
            robot.c = '>';
        }

        while (advanceRobot()) {
            --remaining;
            ++steps;
        }

        if (steps > 0) {
            auto p = std::make_pair(d, robot.c);
            auto turn = moving.find(p);
            path.push_back(fmt::format("{}", turn->second));
            path.push_back(fmt::format("{}", steps));
        }
    }
    for (auto ch : path) { fmt::print("{},", ch); } fmt::print("\n");

    // ok, now we have a path
    // L,12,R,4,R,4,L,6,L,12,R,4,R,4,R,12,L,12,R,4,R,4,L,6,L,10,L,6,R,4,L,12,R,4,R,4,L,6,L,12,R,4,R,4,R,12,L,10,L,6,R,4,L,12,R,4,R,4,R,12,L,10,L,6,R,4,L,12,R,4,R,4,L,6 
    // in ascii code:
    // 76,60,82,52,82,52,76,54,76,60,82,52,82,52,82,60,76,60,82,52,82,52,76,54,76,58,76,54,82,52,76,60,82,52,82,52,76,54,76,60,82,52,82,52,82,60,76,58,76,54,82,52,76,60,82,52,82,52,82,60,76,58,76,54,82,52,76,60,82,52,82,52,76,54
    
    // compressed:
    // P = A,B,A,C,A,B,C,B,C,A
    // A = L,12,R,4,R,4,L,6
    // B = L,12,R,4,R,4,R,12
    // C = L,10,L,6,R,4
    
    std::string mainR { "A,B,A,C,A,B,C,B,C,A\n" };
    std::string subA { "L,12,R,4,R,4,L,6\n" }; 
    std::string subB { "L,12,R,4,R,4,R,12\n" }; 
    std::string subC { "L,10,L,6,R,4\n" }; 
    std::string feed { "n\n" };

    std::string input = mainR + subA + subB + subC + feed;

    std::vector<int64_t> values;
    std::transform(input.begin(), input.end(), std::back_inserter(values), [](char ch) { return static_cast<int64_t>(ch); });

    fmt::print("ascii codes:\n");
    for (auto v : values ) {
        fmt::print("{}", v);
        fmt::print(v == 10 ? "\n" : ",");
    }

    computer = IntComputer(program);
    computer[0] = 2; // write 2 to memory at address 0
    computer.Step();

    int i = 0;

    while (!computer.Halted()) {
        if (computer.CurrentOpcode() == OpCode::In) {
            computer.SetInput(values[i++]);
            computer.Step();
        } else {
            while (computer.CurrentOpcode() != OpCode::In && !computer.Halted()) {
                auto op = computer.CurrentOpcode();
                computer.Step();
                if (op == OpCode::Out) {
                    fmt::print("{}", static_cast<char>(computer.GetOutput()));
                }
            }
        }
    }
    computer.Run();
    fmt::print("\n{}\n", computer.GetOutput());

    return 0;
}

