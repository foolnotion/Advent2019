#include "../util.hpp"

int main(int argc, char** argv)
{
    // Part 1
    std::ifstream infile("./day2.txt");
    std::string line;
    std::getline(infile, line);
    auto tokens = split(line, ',');
    std::vector<int> numbers;

    for (const auto& t : tokens) {
        if (auto res = parse_number<int>(t); res.has_value()) {
            numbers.push_back(res.value());
        }
    }

    std::vector<std::function<int(int, int)>> functions {
        std::plus {},
        std::multiplies {}
    };

    auto run = [&](std::vector<int>& program) -> int {
        for (int i = 0; i < program.size(); i += 4) {
            auto opcode = program[i] - 1;

            if (opcode > 1) {
                break;
            }

            assert(i + 3 < program.size());

            auto lhs_pos = program[i + 1];
            auto rhs_pos = program[i + 2];
            if (lhs_pos >= program.size() || rhs_pos >= program.size()) {
                break;
            }

            auto out_pos = program[i + 3];

            auto res = functions[opcode](program[lhs_pos], program[rhs_pos]);
            program[out_pos] = res;
        }

        return 100 * program[1] + program[2];
    };

    // "1202 program alarm" -> replace position 1 with the value 12 and replace position 2 with the value 2
    numbers[1] = 12;
    numbers[2] = 2;

    auto program = numbers;
    auto res = run(program);

    fmt::print("{}\n", program.front());
    fmt::print("{}\n", res);

    // Part 2
    const int target = 19690720;

    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 100; ++j) {
            program    = numbers;
            program[1] = i;
            program[2] = j;

            res = run(program);
            if (target == program[0]) {
                fmt::print("noun = {}, verb = {}, program[0] = {}, res = {}\n", i, j, program[0], res);
                break;
            }
        }
    }

    return 0;
}
