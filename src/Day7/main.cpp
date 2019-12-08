#include "intcode.hpp"

int main(int argc, char** argv)
{
    // Part 1
    //std::ifstream infile("./example1.txt");
    std::ifstream infile("./input.txt");
    std::string line;
    std::getline(infile, line);
    auto tokens = split(line, ',');
    std::vector<int> program;

    for (const auto& t : tokens) {
        if (auto res = parse_number<int>(t); res.has_value()) {
            program.push_back(res.value());
        }
    }

    // part 1
    auto try_seq = [&](std::vector<int>& sequence) -> int {
        int inputSignal = 0;
        std::vector<IntComputer> circuit(sequence.size());
        for(size_t i = 0; i < sequence.size(); ++i) {
            circuit[i] = IntComputer(program);
            auto& amp = circuit[i];
            amp.SetInput(sequence[i]);
            amp.Step(); 
            amp.SetInput(inputSignal); // set input
            amp.Step(); 
            amp.Run();  
            inputSignal = amp.GetOutput();
        }
        return inputSignal;
    };

    std::vector<int> s { 0, 1, 2, 3, 4 };

    auto thrust = 0;
    std::vector<int> best;
    while(std::next_permutation(s.begin(), s.end())) {
        auto output = try_seq(s);
        if (thrust < output) { 
            thrust = output; best = s; 
        }
    }
    fmt::print("{}\n", thrust);

    // part 2
    auto try_feedback = [&](std::vector<int>& sequence) -> int {
        // initial setup
        std::vector<IntComputer> circuit(sequence.size());
        for(size_t i = 0; i < sequence.size(); ++i) {
            circuit[i] = IntComputer(program);
            auto& amp = circuit[i];
            amp.SetInput(sequence[i]);
            amp.Step(); 
        }

        int inputSignal = 0;
        for (;;) {
            for(size_t i = 0; i < sequence.size(); ++i) {
                auto& amp = circuit[i];
                amp.SetInput(inputSignal);
                amp.Step();

                while(amp.CurrentOpcode() != 4) { 
                    amp.Step(); 

                    if (amp.CurrentOpcode() == 99) {
                        return inputSignal;
                    }
                }
                amp.Step();                
                inputSignal = amp.GetOutput();
            }
        }
        return inputSignal;
    };
    
    thrust = 0;
    s = { 5 , 6, 7, 8, 9 };
    while(std::next_permutation(s.begin(), s.end())) {
        auto output = try_feedback(s);
        if (thrust < output) { 
            thrust = output; best = s; 
        }
    }
    fmt::print("{}\n", thrust);
    
    return 0;
}
