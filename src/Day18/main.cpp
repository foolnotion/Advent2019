#include "../util.hpp"
#include "gsl/gsl"
#include <deque>
#include <functional>
#include <regex>

#include "xxh/xxhash.hpp"

using gsl::narrow;

struct Point {
    int x;
    int y;

    bool operator==(const Point rhs) const { return std::tie(x, y) == std::tie(rhs.x, rhs.y); }
    bool operator<(const Point rhs) const { return std::tie(x, y) < std::tie(rhs.x, rhs.y); }
    friend std::ostream& operator<<(std::ostream& os, Point& p) { os << "(" << p.x << "," << p.y << ")"; return os; }
};

int main(int argc, char** argv)
{
    if (argc < 2) {
        fmt::print("Provide an input file\n");
        return 1;
    }

    std::ifstream infile(argv[1]);
    std::string line;

    int w = 0, h = 0;
    while (std::getline(infile, line)) {
        w = line.size();
        h = h + 1;
    }
    Eigen::Matrix<char, -1, -1> m(w, h);

    infile.clear();
    infile.seekg(0);

    Point entrance;

    auto isKey = [](char c) { return c >= 'a' and c <= 'z'; };
    auto isDoor = [](char c) { return c >= 'A' and c <= 'Z'; };

    auto keyIndex = [&](char c) {
        assert(isKey(c));
        return c - 97;
    };

    std::vector<char> keys;
    std::unordered_map<char, Point> keyLocations;

    int j = 0;
    while (std::getline(infile, line)) {
        for (int i = 0; i < line.size(); ++i) {
            auto c = line[i];
            m(i, j) = c;

            if (isKey(c)) {
                keys.push_back(c);
                keyLocations[c] = Point { i, j };
            }  else if (c == '@') {
                entrance = Point { i, j };
            }
        }
        ++j;
    }

    std::sort(keys.begin(), keys.end(), [&](auto a, auto b) { return keyIndex(a) < keyIndex(b); });

    uint32_t allFound = (1 << keyLocations.size()) - 1;
    // part 1
    // we should cache the distances between the keyLocations,
    // as well as keep track of the doors blocking those paths
    Eigen::Matrix<int, -1, 1> keyDistances(keys.size());
    keyDistances.fill(0);

    Eigen::Matrix<int, -1, -1> keyDistanceMatrix(keys.size(), keys.size());
    keyDistanceMatrix.fill(0);

    Eigen::Matrix<bool, -1, -1> visited(m.rows(), m.cols());
    visited.fill(false);

    std::map<std::pair<char, char>, std::vector<char>> doors;
    std::deque<char> encounteredDoors;

    // calculates distances from the entrance to any key, between all key pairs,
    // and maps the doors blocking the paths between keyLocations
    std::function<void(char, Point, int)> bfs = [&](char u, Point p, int d) {
        auto [x, y] = p;
        auto v = m(x, y);
        if (visited(x, y) or v == '#')
            return;

        visited(x, y) = true;

        if (u != v && isKey(v)) {
            auto i = keyIndex(v);
            auto e = keyDistances(i);
            keyDistances(i) = e == 0 ? d : std::min(d, e);
            if (!encounteredDoors.empty()) {
                doors[{ u, v }] = std::vector<char>(encounteredDoors.begin(), encounteredDoors.end());
            }
        }

        if (isDoor(v) && u != std::tolower(v)) { encounteredDoors.push_back(std::tolower(v)); }
        if (x > 0)            { bfs(u, { x - 1, y }, d + 1); }
        if (x < m.rows() - 1) { bfs(u, { x + 1, y }, d + 1); }
        if (y > 0)            { bfs(u, { x, y - 1 }, d + 1); }
        if (y < m.cols() - 1) { bfs(u, { x, y + 1 }, d + 1); }
        if (isDoor(v) && u != std::tolower(v)) { encounteredDoors.pop_back(); }
    };

    for (auto k : keys) {
        visited.fill(false);
        keyDistances.fill(0);
        bfs(k, keyLocations[k], 0);
        auto i = keyIndex(k);
        keyDistanceMatrix.row(i) = keyDistances;
    }

    visited.fill(false);
    keyDistances.fill(0);
    bfs('@', entrance, 0);

    for (int j = 0; j < m.cols(); ++j) {
        for (int i = 0; i < m.rows(); ++i) {
            auto c = m(i, j);

            if (c == '#') {
                fmt::print(fmt::fg(fmt::color::gray), "\u2588\u2588");
            } else if (isKey(c)) {
                fmt::print(fmt::bg(fmt::color::green), "{} ", c);
            } else if (isDoor(c)) {
                fmt::print(fmt::bg(fmt::color::dark_red), "{} ", c);
            } else if (c == '@') {
                fmt::print("{} ", c);
            } else {
                fmt::print(fmt::fg(fmt::color::black), "\u2588\u2588");
            } 
        }
        std::cout << "\n";
    }   
    //std::cout << m.transpose() << "\n"; 
    //fmt::print("Entrance to keys:\n");
    //std::cout << keyDistances << "\n";
    //fmt::print("Key distance matrix:\n");
    //std::cout << keyDistanceMatrix << "\n";
    //fmt::print("Doors:\n");
    //for (const auto& t : doors) {
    //    fmt::print("{}-{}: ", t.first.first, t.first.second);
    //    for (auto d : t.second) {
    //        fmt::print("{} ", d);
    //    }
    //    fmt::print("\n");
    //}

    // now we should have all the ingredients necessary to implement a solver
    // sketch of the algorithm:
    // - start from the entrance
    // - iterate through reachable keys and add up distances 

    // utils 
    auto dist = [&](char a, char b) { return keyDistanceMatrix(keyIndex(a), keyIndex(b)); };
    auto key  = [&](char c) -> uint32_t { return 1u << keyIndex(c); };

    std::unordered_map<uint64_t, int> cache;
    
    // check if key b can be reached from a 
    // (returns false if key b was already collected)
    auto reachable = [&](char a, char b, uint32_t f) {
        if (a == b || f & key(b)) return false;
        auto it = doors.find({a, b});
        if (it == doors.end()) { 
            return true; 
        }
        // return true if I have the keys to all these doors
        return std::all_of(it->second.begin(), it->second.end(), [&](char d) { return f & key(d); });
    };

    std::function<int(char, uint32_t)> path = [&](char a, uint32_t f) {
        auto h = xxh::xxhash3<64>( { key(a), f });
        for (auto k : keys) {
            if (f & key(k)) {
                fmt::print(fmt::fg(fmt::color::green), "{} ", k);
            } else if (a == k) {
                fmt::print(fmt::fg(fmt::color::yellow), "{} ", k);
            } else if (reachable(a, k, f)) {
                fmt::print(fmt::fg(fmt::color::dark_gray), "{} ", k);
            } else {
                fmt::print(fmt::fg(fmt::color::dark_red), "{} ", k);
            }
        }

        if (cache.find(h) != cache.end()) {
            fmt::print(fmt::fg(fmt::color::green), "cached result: {}\n", cache[h]);
            return cache[h];
        }

        fmt::print("\n");

        assert(!(f & key(a)));
        f |= key(a);

        auto d = 0;
        for (auto b : keys) {
            if (!reachable(a, b, f)) { continue; }
            auto e = dist(a, b) + path(b, f);
            d = d == 0 ? e : std::min(e, d);
        }
        cache[h] = d;
        return d;
    };

    auto dmin = std::numeric_limits<int>::max();
    for (auto k : keys)
    {
        if (doors.find({'@', k}) != doors.end()) {
            continue;
        }
        //cache.clear();
        
        auto d = path(k, 0);
        auto e = keyDistances(keyIndex(k));
        fmt::print("final path length: {}+{} = {}\n", d, e, d+e);
        dmin = std::min(dmin, d + e); 
    }
    fmt::print("min distance: {}\n", dmin);

    return 0;
}

