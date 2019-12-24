
#include "../IntCode/intcode.hpp"

int main(int argc, char** argv) {
    std::vector<int64_t> code { 1005,50,6,104,1,99,104,-1,99 };
    IntComputer comp(code);
    comp.Run();
    fmt::print("output: {}\n", comp.GetOutput());

    return 0;
}
