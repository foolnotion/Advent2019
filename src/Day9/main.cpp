#include "../util.hpp"
#include "../IntCode/intcode.hpp"

constexpr int W = 25;
constexpr int H = 6;

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

    // part 1
    IntComputer comp(program);
    comp.SetInput(1);
    comp.Run();
    fmt::print("{}\n", comp.GetOutput());

    // part 2
    comp = IntComputer(program);
    comp.SetInput(2);
    comp.Run();
    fmt::print("{}\n", comp.GetOutput());

    return 0;
}
