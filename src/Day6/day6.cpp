#include "../util.hpp"

// Advent of Code 2019 Day 6
int main(int argc, char** argv)
{
    // Part 1
    std::ifstream infile("./day6_input.txt");
    std::string line;
 
    std::unordered_map<std::string, std::vector<std::string>> objects;
    std::unordered_set<std::string> names;

    while (std::getline(infile, line)) {
        auto tokens = split(line, ')');
        auto key = tokens[0];
        auto val = tokens[1];
        names.insert(key);
        names.insert(val);
        if (auto [it, ok] = objects.insert({ key, { val } }); !ok) {
            objects[key].push_back(val);
        }
    }

    std::vector<std::string> keys(names.begin(), names.end());
    std::sort(keys.begin(), keys.end());

    auto get_index = [&](const auto& v) { 
        auto res = std::lower_bound(keys.begin(), keys.end(), v);
        assert(res != keys.end());
        return std::distance(keys.begin(), res);
    };

    Eigen::Matrix<int, -1, -1> graph(names.size(), names.size());
    graph.fill(0);
    for (size_t i = 0; i < keys.size(); ++i) {
        for (const auto& v : objects[keys[i]]) {
            auto j = get_index(v);
            graph(i, j) = 1;
        }
    }

    std::function<void(int)> add = [&](int i) {
        for (Eigen::Index j = 0; j < graph.cols(); ++j) {
            if (i == j) continue;
            if (graph(i, j) > 0) {
                graph(i, j) += graph.col(i).sum();
                add(j);
            }
        }
    };

    auto c = get_index("COM");
    add(c);
    fmt::print("{}\n", graph.sum());

    // Part 2
    std::vector<int> you;
    std::vector<int> san;

    std::function<void(std::vector<int>&, int)> get_path = [&](std::vector<int>& path, int col) {
        int row;
        if (auto c = graph.col(col).maxCoeff(&row); c > 0) {
            path.push_back(col);
            get_path(path, row);
        }
    };

    get_path(you, get_index("YOU"));
    get_path(san, get_index("SAN"));

    auto s = 0;

    auto yy = you.rbegin();
    auto ss = san.rbegin();
    for (; yy != you.rend() && ss != san.rend() && *ss == *yy; ++yy, ++ss) {
        ++s; 
    }
    fmt::print("{}\n", you.size() + san.size() - 2 * s - 2);

    return 0;
}
