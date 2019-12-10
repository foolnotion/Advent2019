#include "../util.hpp"

using Matrix = Eigen::Matrix<int, -1, -1>;
using Block = Eigen::DenseBase<Matrix>::BlockXpr;

using Point = std::pair<int, int>;

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
    auto slope = [](Point p1, Point p2) {
        double dx = p1.first - p2.first;
        double dy = p1.second - p2.second;

        return std::atan2(dx, dy);
    };

    auto visible_quadrant = [&](Point p, Point lower, Point upper) {
        std::vector<double> slopes;
        auto [xmin, ymin] = lower;
        auto [xmax, ymax] = upper;

        Point q = lower;
        for (int i = xmin; i < xmax; ++i) {
            for (int j = ymin; j < ymax; ++j) {
                auto q = std::make_pair(i, j);
                if (p == q || m(i, j) == 0) {
                    continue;
                }
                slopes.push_back(slope(p, q));
            }
        }
        std::sort(slopes.begin(), slopes.end());
        auto it = std::unique(slopes.begin(), slopes.end());
        return std::distance(slopes.begin(), it);
    };

    auto visible = [&](int x, int y) -> int {
        auto vis = 0;
        vis += visible_quadrant({ x, y }, { 0, 0 }, { x + 1, y + 1 });
        vis += visible_quadrant({ x, y }, { x, y }, { m.rows(), m.cols() });
        vis += visible_quadrant({ x, y }, { 0, y + 1 }, { x, m.cols() });
        vis += visible_quadrant({ x, y }, { x + 1, 0 }, { m.rows(), y });

        return vis;
    };

    for (Eigen::Index i = 0; i < m.rows(); ++i) {
        for (Eigen::Index j = 0; j < m.cols(); ++j) {
            if (m(i, j) == 0)
                continue;
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
