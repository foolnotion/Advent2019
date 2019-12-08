#include "intcode.hpp"

int main(int argc, char** argv)
{
    // Part 1
    //std::ifstream infile("./example1.txt");
    std::ifstream infile("./input.txt");
    std::string line;
    std::getline(infile, line);
    auto tokens = split(line, ',');
    std::vector<int> numbers;

    for (const auto& t : tokens) {
        if (auto res = parse_number<int>(t); res.has_value()) {
            numbers.push_back(res.value());
        }
    }

    // part 1
    IntComputer comp(numbers);
    comp.SetInput(1);
    comp.Run();
    fmt::print("{}\n", comp.GetOutput());

    // part 2
    comp = IntComputer(numbers);
    comp.SetInput(5);
    comp.Run();
    fmt::print("{}\n", comp.GetOutput());
    
    return 0;
}
