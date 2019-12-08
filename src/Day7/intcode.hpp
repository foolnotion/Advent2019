#ifndef INTCODE_HPP
#define INTCODE_HPP

#include "gsl/gsl"
#include "util.hpp"

class IntComputer {
public:
    IntComputer() = default;

    IntComputer(std::vector<int> m)
        : reg(0)
        , ip(0)
        , mem(m)
    {
    }

    bool Step()
    {
        auto [opcode, mode1, mode2, mode3] = UnpackInstr(mem[ip]);

        switch (opcode) {
        case 1: {
            auto arg1 = Read(mem[ip + 1], mode1);
            auto arg2 = Read(mem[ip + 2], mode2);
            auto addr = mem[ip + 3];
            auto res = arg1 + arg2;
            mem[addr] = res;
            ip += 4;
            break;
        }
        case 2: {
            auto arg1 = Read(mem[ip + 1], mode1);
            auto arg2 = Read(mem[ip + 2], mode2);
            auto addr = mem[ip + 3];
            auto res = arg1 * arg2;
            mem[addr] = res;
            ip += 4;
            break;
        }
        case 3: {
            auto addr = mem[ip + 1];
            mem[addr] = reg;
            ip += 2;
            break;
        }
        case 4: {
            auto addr = mem[ip + 1];
            reg = mem[addr];
            ip += 2;
            break;
        }
        case 5: {
            auto arg1 = Read(mem[ip + 1], mode1);
            auto arg2 = Read(mem[ip + 2], mode2);
            ip = arg1 == 0 ? ip + 3 : arg2;
            break;
        }
        case 6: {
            auto arg1 = Read(mem[ip + 1], mode1);
            auto arg2 = Read(mem[ip + 2], mode2);
            ip = arg1 != 0 ? ip + 3 : arg2;
            break;
        }
        case 7: {
            auto arg1 = Read(mem[ip + 1], mode1);
            auto arg2 = Read(mem[ip + 2], mode2);
            auto addr = mem[ip + 3];
            mem[addr] = arg1 < arg2 ? 1 : 0;
            ip += 4;
            break;
        }
        case 8: {
            auto arg1 = Read(mem[ip + 1], mode1);
            auto arg2 = Read(mem[ip + 2], mode2);
            auto addr = mem[ip + 3];
            mem[addr] = arg1 == arg2 ? 1 : 0;
            ip += 4;
            break;
        }
        case 99: {
            return false;
        }
        default:
            throw std::runtime_error(fmt::format("Unknown opcode {} unpacked from instruction {} at ip = {}\n", opcode, mem[ip], ip));
        }
        return ip < mem.size();
    }

    void Run()
    {
        while (Step())
            ;
    }

    void SetInput(int v) { reg = v; }
    int GetOutput() const { return reg; }

    int operator[](int addr) { return mem[addr]; }

    int CurrentOpcode() const { 
        auto [opcode, m1, m2, m3] = UnpackInstr(mem[ip]);
        return opcode;
    }

private:
    int reg; // register to use as input
    int ip;
    std::array<int, 4> len { 4, 4, 2, 2 };

    std::tuple<int, int, int, int> UnpackInstr(int v) const
    {
        int opcode = v % 100; // last 2 digits
        v /= 100;
        int mode1 = v % 10;
        v /= 10; // mode of 1st parameter
        int mode2 = v % 10;
        v /= 10; // mode of 2nd parameter
        int mode3 = v % 10;
        v /= 10; // mode of 3rd parameter
        return { opcode, mode1, mode2, mode3 };
    }

    int Read(int p, int m)
    {
        return m == 0 ? mem[p] : p;
    };

    std::vector<int> mem;
};

#endif
