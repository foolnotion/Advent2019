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
    auto atan2 = [](Point p1, Point p2) {
        return std::atan2(p1.first - p2.first, p1.second - p2.second);
    };

    auto visible = [&](int x, int y) -> int {
        std::vector<double> slopes;

        for (int i = 0; i < m.rows(); ++i) {
            for (int j = 0; j < m.cols(); ++j) {
                if (m(i,j) == 0 || (i == x && j == y)) {
                    continue;
                }
                slopes.push_back(std::atan2(x-i, y-j));
            }
        }
        std::sort(slopes.begin(), slopes.end());
        auto it = std::unique(slopes.begin(), slopes.end());
        return std::distance(slopes.begin(), it); 
    };

    for (Eigen::Index i = 0; i < m.rows(); ++i) {
        for (Eigen::Index j = 0; j < m.cols(); ++j) {
            if (m(i, j) == 0) {
                continue;
            }
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
            auto s = atan2(ims, p);
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
