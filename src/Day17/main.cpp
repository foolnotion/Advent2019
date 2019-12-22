#include "../IntCode/intcode.hpp"
#include "../util.hpp"

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

static const std::array<uint8_t, 4> directions = {
    1, // north
    2, // south
    3, // west
    4 // east
};

int main(int argc, char** argv)
{
    std::ifstream infile("./input.txt");
    std::string line;
    std::getline(infile, line);
    auto program = to_vec<int64_t>(line, ',');

    // part 1
    IntComputer computer(program);

    std::vector<Point> points;
    int x = 0;
    int y = 0;
    char c{0}, d{0};
    int n = 0;
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
            if (c == '#' || c == '^') {
                points.push_back(Point { x, y, c });
            }
            x = x + 1;
        }
    }
    fmt::print("points: {}\n", points.size());
    int h = y;
    int w = n / h;
    fmt::print("w: {}, h: {}\n", w, h);

    Eigen::Matrix<char, -1, -1> m(w+1, h+1);
    m.fill(46);
    for(auto p : points) {
        m(p.x, p.y) = p.c;
    }

    auto sum = 0u;
    for (int i = 1; i < w; ++i) {
        for (int j = 1; j < h; ++j) {
            auto c = m(i, j);
            if (c != '#') {
                continue;
            }
            if (c == m(i-1, j) && c == m(i+1, j) && c == m(i, j-1) && c == m(i, j+1)) {
                m(i, j) = 'O';
                sum += i * j;
            }
        }
    }

    std::cout << m << "\n";
    fmt::print("{}\n", sum);
    return 0;
}
