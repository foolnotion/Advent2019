#include "../util.hpp"
#include "gsl/gsl"
#include <deque>
#include <functional>
#include <regex>

#include "xxh/xxhash.hpp"

using gsl::narrow;

using IntMatrix = Eigen::Matrix<int, -1, -1>; // matrix
using IntVector = Eigen::Matrix<int, -1,  1>; // column vector

struct Point {
    int x;
    int y;

    bool operator==(const Point rhs) const { return std::tie(x, y) == std::tie(rhs.x, rhs.y); }
    bool operator<(const Point rhs) const { return std::tie(x, y) < std::tie(rhs.x, rhs.y); }
    friend std::ostream& operator<<(std::ostream& os, Point& p) { os << "(" << p.x << "," << p.y << ")"; return os; }
};

struct Int {
    static constexpr int Max = std::numeric_limits<int>::max();
    static constexpr int Min = std::numeric_limits<int>::min();
};

template<typename Derived>
struct MapSolver {
    MapSolver(const Eigen::MatrixBase<Derived>& m) : matrix(m) 
    {
        Initialize();
    }

    void Initialize() {
    }

    Point entrance;
    Derived matrix;
};

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print("Provide an input file\n");
        return 1;
    }

    std::ifstream infile(argv[1]);
    std::string line;

    int w = 0, h = 0;
    while (std::getline(infile, line)) {
        w = line.size();
        h = h + 1;
    }
    IntMatrix m(w, h);

    infile.clear();
    infile.seekg(0);

    Point entrance;

    int j = 0;
    while (std::getline(infile, line)) {
        for (int i = 0; i < line.size(); ++i) {
            auto c = line[i];
            m(i, j) = c;
        }
        ++j;
    }

    std::cout << m.cast<char>().transpose() << "\n";

    return 0;
}

