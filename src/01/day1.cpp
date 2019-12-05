#include "../util.hpp"

int main(int argc, char** argv)
{
    std::ifstream infile("./day1.txt");
    std::string line;
    int m;
    std::vector<int> mass;

    while (std::getline(infile, line)) {
        if (auto res = parse_number<int>(line); res.has_value()) {
            mass.push_back(res.value());
        }
    }

    auto convert = [](int mass) { return static_cast<int>(std::floor(mass / 3.0) - 2); };

    std::function<int(int)> adjust = [&](int mass) {
        int total = 0;

        for(;;) {
            mass = convert(mass);
            if (mass > 0) {
                total += mass;
            } else {
                break;
            }
        }

        return total;
    };

    std::vector<int> fuel;
    std::transform(mass.begin(), mass.end(), std::back_inserter(fuel), convert);
    auto total = std::reduce(std::execution::seq, fuel.begin(), fuel.end(), 0, std::plus{});
    fmt::print("{}\n", total);
    
    std::vector<int> fuelAdjusted;
    std::transform(mass.begin(), mass.end(), std::back_inserter(fuelAdjusted), adjust);
    auto totalAdjusted = std::reduce(std::execution::seq, fuelAdjusted.begin(), fuelAdjusted.end(), 0, std::plus{});

    fmt::print("{} ({})\n", totalAdjusted);

    return 0;
}
