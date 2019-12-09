#ifndef INTCODE_HPP
#define INTCODE_HPP

#include "gsl/gsl"
#include "util.hpp"

struct Chunk {
    size_t Base;
    size_t Size;
    std::vector<int64_t> Storage;

    Chunk(size_t base, size_t size)
        : Base(base)
        , Size(size)
        , Storage(size)
    {
    }

    int64_t& operator[](size_t i) { return Storage[i]; }
    int64_t operator[](size_t i) const { return Storage[i]; }
};

template <size_t S = 1024>
class Memory {
public:
    static constexpr size_t ChunkSize = S;

    Memory(std::vector<int64_t> data)
    {
        auto size = ChunkSize * (data.size() / ChunkSize + 1);
        chunks.emplace_back(0, size);
        for (auto i = 0ul; i < data.size(); ++i) {
            chunks.back()[i] = data[i];
        }
    }

    int64_t& operator[](int64_t addr)
    {
        auto pred = [&](const auto& c) { return c.Base + c.Size < addr; };
        auto p = std::partition_point(chunks.begin(), chunks.end(), pred);

        if (p != chunks.end()) {
            return p->Storage[addr - p->Base];
        } else {
            auto base = (addr / ChunkSize) * ChunkSize;
            assert(base % ChunkSize == 0);
            chunks.emplace_back(base, ChunkSize);
            auto& chunk = chunks.back();
            std::sort(chunks.begin(), chunks.end(), [](const auto& lhs, const auto& rhs) { return lhs.Base < rhs.Base; });
            return chunk[addr - chunk.Base];
        }
    }

    int64_t operator[](int64_t addr) const
    {
        auto pred = [&](const auto& c) { return c.Base + c.Size < addr; };
        auto p = std::partition_point(chunks.begin(), chunks.end(), pred);

        if (p != chunks.end()) {
            return p->Storage[addr - p->Base];
        } else {
            throw std::runtime_error(fmt::format("Invalid access: {}\n", addr));
        }
    }

    int64_t Size() const
    {
        const auto& c = chunks.back();
        return c.Base + c.Size;
    };

    const std::vector<Chunk>& Chunks() const
    {
        return chunks;
    };

private:
    std::vector<Chunk> chunks;
};

struct OpCode {
    constexpr static int Add = 1;
    constexpr static int Mul = 2;
    constexpr static int In = 3;
    constexpr static int Out = 4;
    constexpr static int JmpIfTrue = 5;
    constexpr static int JmpIfFalse = 6;
    constexpr static int LessThan = 7;
    constexpr static int Equals = 8;
    constexpr static int RelBase = 9;
    constexpr static int Halt = 99;
};

struct ParameterMode {
    constexpr static int Position = 0;
    constexpr static int Immediate = 1;
    constexpr static int Relative = 2;
};

class IntComputer {
public:
    using Tape = Memory<1024>;

    IntComputer() 
        : IntComputer(std::vector<int64_t>{})
    {
    }

    IntComputer(std::vector<int64_t> program)
        : reg(0)
        , ip(0)
        , base(0)
        , mem(program)
    {
    }

    bool Step()
    {
        auto [opcode, mode1, mode2, mode3] = UnpackInstr(mem[ip]);

        switch (opcode) {
        case OpCode::Add: {
            auto arg1 = Read(ip + 1, mode1);
            auto arg2 = Read(ip + 2, mode2);
            Write(ip + 3, mode3, arg1 + arg2);
            ip += 4;
            break;
        }
        case OpCode::Mul: {
            auto arg1 = Read(ip + 1, mode1);
            auto arg2 = Read(ip + 2, mode2);
            Write(ip + 3, mode3, arg1 * arg2);
            ip += 4;
            break;
        }
        case OpCode::In: {
            Write(ip + 1, mode1, reg);
            ip += 2;
            break;
        }
        case OpCode::Out: {
            reg = Read(ip + 1, mode1);
            ip += 2;
            break;
        }
        case OpCode::JmpIfTrue: {
            auto arg1 = Read(ip + 1, mode1);
            auto arg2 = Read(ip + 2, mode2);
            ip = arg1 == 0 ? ip + 3 : arg2;
            break;
        }
        case OpCode::JmpIfFalse: {
            auto arg1 = Read(ip + 1, mode1);
            auto arg2 = Read(ip + 2, mode2);
            ip = arg1 != 0 ? ip + 3 : arg2;
            break;
        }
        case OpCode::LessThan: {
            auto arg1 = Read(ip + 1, mode1);
            auto arg2 = Read(ip + 2, mode2);
            Write(ip + 3, mode3, arg1 < arg2 ? 1 : 0);
            ip += 4;
            break;
        }
        case OpCode::Equals: {
            auto arg1 = Read(ip + 1, mode1);
            auto arg2 = Read(ip + 2, mode2);
            Write(ip + 3, mode3, arg1 == arg2 ? 1 : 0);
            ip += 4;
            break;
        }
        case OpCode::RelBase: {
            auto arg1 = Read(ip + 1, mode1);
            base += arg1;
            ip += 2;
            break;
        }

        case OpCode::Halt: {
            return false;
        }
        default:
            throw std::runtime_error(fmt::format("Unknown opcode {} unpacked from instruction {} at ip = {}\n", opcode, mem[ip], ip));
        }
        return ip < mem.Size();
    }

    void Run()
    {
        while (Step()) {
            // step until opcode 99 or end of program
        }
    }

    void SetInput(int64_t v) { reg = v; }
    int64_t GetOutput() const { return reg; }

    int64_t operator[](int64_t addr) { return mem[addr]; }

    int64_t CurrentOpcode() const
    {
        auto [opcode, m1, m2, m3] = UnpackInstr(mem[ip]);
        return opcode;
    }

    const Tape& GetMemory() const { return mem; }

private:
    int64_t reg; // register to use as input
    int64_t ip; // instruction pointer
    int64_t base; // relative base

    Tape mem;

    std::tuple<int, int, int, int> UnpackInstr(int64_t v) const
    {
        int64_t opcode = v % 100;
        v /= 100;
        int64_t mode1 = v % 10;
        v /= 10;
        int64_t mode2 = v % 10;
        v /= 10;
        int64_t mode3 = v % 10;
        return { opcode, mode1, mode2, mode3 };
    }

    int64_t Read(int64_t ip, int64_t mode)
    {
        switch (mode) {
        case ParameterMode::Position: {
            auto addr = mem[ip];
            assert(addr >= 0);
            return mem[addr];
        }
        case ParameterMode::Immediate: {
            return mem[ip];
        }
        case ParameterMode::Relative: {
            auto addr = mem[ip] + base;
            assert(addr >= 0);
            return mem[addr];
        }
        default:
            throw std::runtime_error(fmt::format("Unknown parameter mode: {}\n", mode));
        }
    };

    void Write(int64_t ip, int64_t mode, int64_t value)
    {
        // Parameters that an instruction writes to will never be in immediate mode.
        switch (mode) {
        case ParameterMode::Position: {
            auto addr = mem[ip];
            assert(addr >= 0);
            mem[addr] = value;
            break;
        }
        case ParameterMode::Relative: {
            auto addr = mem[ip] + base;
            assert(addr >= 0);
            mem[addr] = value;
            break;
        }
        default:
            throw std::runtime_error(fmt::format("Unknown parameter mode: {}\n", mode));
        }
    }
};

#endif

