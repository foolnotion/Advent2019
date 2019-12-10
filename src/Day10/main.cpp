#include "../util.hpp"

using Matrix = Eigen::Matrix<int, -1, -1>;
using Block = Eigen::DenseBase<Matrix>::BlockXpr;

using Point = std::pair<int, int>;

static constexpr double HALF_PI = M_PI / 2;

int main(int argc, char** argv)
{
    std::ifstream infile("./input.txt");
    std::string line;

    // read input
    Matrix m(50, 50);
    int i = 0;
    int w, h;

    Point ims; // instant monitoring station (part 2)
    while (std::getline(infile, line)) {
        h = line.size();
        for (size_t j = 0; j < line.size(); ++j) {
            char c = line[j];
            m(i, j) = c == '#';
            if (c == 'X') { 
                ims = { i, j }; 
            }
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

    int row, col;
    auto max = m.maxCoeff(&row, &col);
    fmt::print("{}\n", max);

    // part 2
    ims = Point{row, col};
    fmt::print("ims({},{})\n", ims.first, ims.second);
    std::cout << m << "\n";
    auto dist = [](Point p, Point q) { return std::hypot(p.first - q.first, p.second - q.second); };

    std::vector<Point> points;
    std::vector<std::pair<double, double>> params;

    for (Eigen::Index i = 0; i < m.rows(); ++i) {
        for (Eigen::Index j = 0; j < m.cols(); ++j) {
            if (m(i, j) == 0) {
                continue;
            }
            Point p{i, j};
            if (p == ims) {
                continue;
            }
            auto s = slope(ims, p);
            auto d = dist(ims, p);
            params.push_back({s, d});
            points.push_back(p);
        }
    }
    std::vector<int> indices(points.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&](int i, int j) { return params[i] < params[j]; });
    auto pt = std::partition_point(indices.begin(), indices.end(), [&](int i) { return params[i].first < HALF_PI; });
    std::rotate(indices.begin(), indices.begin() + std::distance(indices.begin(), pt), indices.end());
    std::vector<bool> vaporized(points.size(), false);

    Matrix order(m.rows(), m.cols());
    order.fill(0);
    order(ims.first, ims.second) = 1000;

    size_t count = 0;

    for (size_t i = 0; i < points.size(); ++i) {
        auto idx = indices[i];

        if (vaporized[idx]) { 
            continue; 
        }

        auto [s, d] = params[idx];
        auto [x, y] = points[idx];
        order(x, y) = count+1;
        vaporized[idx] = true;

        while (params[indices[i+1]].first == s) { 
            ++i; 
        };

        ++count;
        if (count == 200 || count == points.size()) {
            fmt::print("({},{}) = {}\n", y, x, y * 100 + x);
            break;
        }
    }
    fmt::print("Shooting order:\n");
    std::cout << order << "\n";
    return 0;
}
