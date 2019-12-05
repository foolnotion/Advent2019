#include "../util.hpp"

using point = std::array<int, 3>;

int main(int argc, char** argv)
{
    // Part 1
    std::ifstream infile("./day3.txt");
    std::string line;

    auto parse_points = [](const std::vector<std::string>& tokens) -> std::vector<point> {
        std::vector<point> points;

        int x = 0;
        int y = 0;
        int s = 0;

        for (const auto& t : tokens) {
            auto dir = t.substr(0, 1)[0];
            auto sub = t.substr(1, t.size());
            auto res = parse_number<int>(sub);
            if (!res.has_value()) {
                throw std::runtime_error(fmt::format("Cannot parse substring {} as number.\n", sub));
            }
            auto step = res.value();

            auto p = points.back();
            switch (dir) {
            case 'U':
                for (int i = 0; i < step; ++i) {
                    points.push_back({ x, ++y, ++s });
                }
                break;
            case 'R':
                for (int i = 0; i < step; ++i) {
                    points.push_back({ ++x, y, ++s });
                }
                break;
            case 'D':
                for (int i = 0; i < step; ++i) {
                    points.push_back({ x, --y, ++s });
                }
                break;
            case 'L':
                for (int i = 0; i < step; ++i) {
                    points.push_back({ --x, y, ++s });
                }
                break;
            default:
                throw std::runtime_error("Unknown direction");
            }
        }
        return points;
    };

    auto taxi_distance = [](point a, point b) {
        return std::abs(a[0] - b[0]) + std::abs(a[1] - b[1]);
    };

    // first wire
    std::getline(infile, line);
    auto tokens = split(line, ',');
    auto pA = parse_points(tokens);

    // second wire
    std::getline(infile, line);
    tokens = split(line, ',');
    auto pB = parse_points(tokens);

    auto comp = [](point a, point b) {
        return std::lexicographical_compare(a.begin(), a.begin()+2, b.begin(), b.begin()+2);
    };

    std::sort(pA.begin(), pA.end(), comp);
    std::sort(pB.begin(), pB.end(), comp);

    std::vector<point> intersection;
    std::set_intersection(pA.begin(), pA.end(), pB.begin(), pB.end(), std::back_inserter(intersection), comp);

    auto origin = point { 0, 0, 0 };
    auto min_distance = std::numeric_limits<int>::max();

    for (auto point : intersection) {
        auto d = taxi_distance(point, origin);
        if (d < min_distance) {
            min_distance = d;
        }
    }
    fmt::print("{}\n", min_distance);

    // Part 2
    auto minSteps = std::numeric_limits<int>::max();

    auto equals = [&](point a, point b) { return std::equal(a.begin(), a.begin()+2, b.begin()); };

    for (auto point : intersection) {
        auto a = std::find_if(pA.begin(), pA.end(), [&](const auto p) { return equals(p, point); });
        auto b = std::find_if(pB.begin(), pB.end(), [&](const auto p) { return equals(p, point); });

        assert(a != pA.end());
        assert(b != pB.end());

        auto combinedSteps = a->back() + b->back(); 

        if (minSteps > combinedSteps) {
            minSteps = combinedSteps;
        }
    }
    fmt::print("{}\n", minSteps);
    return 0;
}
