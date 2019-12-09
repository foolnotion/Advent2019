#include "../IntCode/intcode.hpp"

int main(int argc, char** argv)
{
    // Part 1
    //std::ifstream infile("./example1.txt");
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

    fmt::print("program length: {}\n", program.size());
    for(auto p : program) fmt::print("{}, ", p);
    fmt::print("\n");

    // part 1
    IntComputer comp(program);
    comp.SetInput(1);
    comp.Run();
    fmt::print("{}\n", comp.GetOutput());

    for(size_t i = 0; i < program.size(); ++i) {
        fmt::print("{}, ", comp.GetMemory()[i]);
    }
    fmt::print("\n");

    // part 2
    comp = IntComputer(program);
    comp.SetInput(5);
    comp.Run();
    fmt::print("{}\n", comp.GetOutput());
    
    return 0;
}
