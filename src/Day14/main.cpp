#include "../util.hpp"

#include <regex>
#include <functional>

struct Reactant {
    size_t      Qty;   
    std::string Id;

    static Reactant FromString(const std::string& s) {
        auto parts = split(s, ' ');
        auto qty   = parse_number<size_t>(parts.front());

        if (qty.has_value()) {
            return Reactant{ qty.value(), parts.back() };
        } else {
            throw std::runtime_error(fmt::format("Unable to parse quantity string {} into a number.", parts.front()));
        }
    }
};

struct MolekSyntez {
    MolekSyntez(std::unordered_map<std::string, std::vector<Reactant>>& reactions, std::unordered_map<std::string, size_t>& base) 
        : reactions_ref(reactions), base_ref(base)
    {
    }

    size_t operator()(const Reactant& r) {
        return Refine(r);
    }

    size_t Refine(const Reactant& r) {
        if (r.Id == "ORE") {
            return r.Qty;
        }

        auto roundToBase = [](size_t q, size_t b) -> size_t { 
            return q / b + (q % b != 0); 
        };

        auto q = r.Qty;
        auto b = base_ref[r.Id];
        if (b == 0) {
            throw new std::runtime_error(fmt::format("unexpected reactant {}\n", r.Id));
        }
        auto m = roundToBase(q, b);

        auto s = storage[r.Id];

        // update needed amount and quantity left in storage
        if (s >= q) {
            storage[r.Id] -= q; 
            return 0; // enough storage 
        } else {
            q -= s;
            m = roundToBase(q, b);
            storage[r.Id] = 0; 
        }
        storage[r.Id] += b * m - q;

        auto sum = 0ull;
        for (const auto& p : reactions_ref[r.Id]) {
            sum += Refine({p.Qty * m, p.Id});
        }
        return sum;
    }

    std::unordered_map<std::string, size_t> storage;
    std::unordered_map<std::string, std::vector<Reactant>>& reactions_ref;
    std::unordered_map<std::string, size_t>& base_ref;
};

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print("Provide an input file\n");
        return 1;
    }

    std::ifstream infile(argv[1]);
    std::string line;

    std::regex re(" => |, ");

    std::unordered_map<std::string, std::vector<Reactant>> reactions;
    std::unordered_map<std::string, size_t> base;

    // parse input
    while (std::getline(infile, line)) {
        auto result = std::regex_replace(line, re, ",");
        auto tokens = split(result, ',');
        std::vector<Reactant> reactants;
        for (size_t i = 0; i < tokens.size()-1; ++i) {
            reactants.push_back(Reactant::FromString(tokens[i])); 
        }
        auto product          = Reactant::FromString(tokens.back());
        reactions[product.Id] = reactants;
        base[product.Id]      = product.Qty;
    }

    auto react = [&](size_t i) { return MolekSyntez(reactions, base)({ i, "FUEL" }); };
    auto ore = react(1);
    fmt::print("{}\n", ore);

    const size_t n = 2000000;
    std::vector<size_t> indices(n);
    std::iota(indices.begin(), indices.end(), 1);
    auto max = std::partition_point(indices.begin(), indices.end(), [&](size_t i) { return react(i) < 1'000'000'000'000; });
    auto maxOre = react(*(max-1));
    fmt::print("{} {}\n", *(max-1), maxOre);

    return 0;
}
