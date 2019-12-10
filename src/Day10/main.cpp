#include "../util.hpp"

using Matrix = Eigen::Matrix<int, -1, -1>;
using Block  = Eigen::DenseBase<Matrix>::BlockXpr;

int main(int argc, char** argv)
{
    std::ifstream infile("./input.txt");
    std::string line;

    // read input
    Matrix m(50, 50);
    int i = 0;
    int w, h;
    while (std::getline(infile, line)) {
        h = line.size();
        for (size_t j = 0; j < line.size(); ++j) {
            char c = line[j];
            m(i, j) = c == '#'; 
        }
        ++i;
    }
    w = i;

    m.conservativeResize(w, h);

    // part 1
    auto slope = [](int x0, int y0, int x1, int y1) -> double {
        double dx = x0 - x1;
        int dy = y0 - y1;

        if (dy == 0) return std::numeric_limits<double>::max();
        return dx / dy;
    };

    auto visible_quadrant = [&](int x, int y, int xLower, int yLower, int xUpper, int yUpper) {
        std::vector<double> slopes;
        for (int i = xLower; i < xUpper; ++i) {
            for (int j = yLower; j < yUpper; ++j) {
                if (i == x && j == y) continue;
                if (m(i, j) == 0) continue;
                slopes.push_back(slope(i, j, x, y));
            }
        }
        std::sort(slopes.begin(), slopes.end());
        auto it = std::unique(slopes.begin(), slopes.end());
        return std::distance(slopes.begin(), it);
    };

    auto visible = [&](int x, int y) -> int {
        auto vis = 0;
        vis += visible_quadrant(x, y, 0, 0, x+1, y+1);
        vis += visible_quadrant(x, y, x, y, m.rows(), m.cols()); 
        vis += visible_quadrant(x, y, 0, y+1, x, m.cols());
        vis += visible_quadrant(x, y, x+1, 0, m.rows(), y);

        return vis;
    };

    for (Eigen::Index i = 0; i < m.rows(); ++i) {
        for (Eigen::Index j = 0; j < m.cols(); ++j) {
            if (m(i, j) == 0) continue;
            m(i, j) = visible(i, j);
        }
    }
    int x, y;
    fmt::print("{}\n", m.maxCoeff(&x, &y));
    fmt::print("coordinates: ({},{})\n", x, y);

    // part 2
    std::unordered_map<double, std::pair<int, int>> points;

    return 0;
}
