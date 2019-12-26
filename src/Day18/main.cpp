#include "../util.hpp"
#include "gsl/gsl"
#include <deque>
#include <functional>
#include <regex>

#include "xxh/xxhash.hpp"

using Map = Eigen::Matrix<char, -1, -1>;

struct Point {
    int x;
    int y;

    bool operator==(const Point rhs) const
    {
        return std::tie(x, y) == std::tie(rhs.x, rhs.y);
    }
    bool operator<(const Point rhs) const
    {
        return std::tie(x, y) < std::tie(rhs.x, rhs.y);
    }
    friend std::ostream& operator<<(std::ostream& os, Point& p)
    {
        os << "(" << p.x << "," << p.y << ")";
        return os;
    }
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
    Map m(w, h);

    infile.clear();
    infile.seekg(0);

    Point entrance;

    auto isKey = [](char c) { return c >= 'a' and c <= 'z'; };
    auto isDoor = [](char c) { return c >= 'A' and c <= 'Z'; };

    auto keyIndex = [&](char c) {
        assert(isKey(c));
        return c - 97;
    };
    auto doorIndex = [&](char c) {
        assert(isDoor(c));
        return c - 65;
    };

    std::vector<char> keyNames;
    std::unordered_map<char, Point> doors;
    std::unordered_map<char, Point> keyLocations;

    int j = 0;
    while (std::getline(infile, line)) {
        for (int i = 0; i < line.size(); ++i) {
            auto c = line[i];
            m(i, j) = c;

            if (isKey(c)) {
                keyNames.push_back(c);
                keyLocations[c] = Point { i, j };
            } else if (isDoor(c)) {
                doors[std::tolower(c)] = Point { i, j };
            } else if (c == '@') {
                entrance = Point { i, j };
            }
        }
        ++j;
    }

    std::sort(keyNames.begin(), keyNames.end(), [&](auto a, auto b) { return keyIndex(a) < keyIndex(b); });

    uint32_t allFound = (1 << keyLocations.size()) - 1;
    // part 1
    // we should cache the distances between the keyLocations,
    // as well as keep track of the doors blocking those paths
    Eigen::Matrix<int, -1, 1> keyDistances(keyLocations.size());
    keyDistances.fill(0);

    Eigen::Matrix<int, -1, -1> keyDistanceMatrix(keyLocations.size(), keyLocations.size());
    keyDistanceMatrix.fill(0);

    Eigen::Matrix<bool, -1, -1> visited(m.rows(), m.cols());
    visited.fill(false);

    std::map<std::pair<char, char>, std::vector<char>> doorsBetweenPaths;
    std::deque<char> encounteredDoors;

    // calculates distances from the entrance to any key, between all key pairs,
    // and maps the doors blocking the paths between keyLocations
    std::function<void(char, Point, int)> bfsKeyDistance = [&](char u, Point p, int d) {
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
                doorsBetweenPaths[{ u, v }] = std::vector<char>(encounteredDoors.begin(), encounteredDoors.end());
            }
        }

        if (isDoor(v) && u != std::tolower(v)) { encounteredDoors.push_back(std::tolower(v)); }
        if (x > 0) { bfsKeyDistance(u, { x - 1, y }, d + 1); }
        if (x < m.rows() - 1) { bfsKeyDistance(u, { x + 1, y }, d + 1); }
        if (y > 0) { bfsKeyDistance(u, { x, y - 1 }, d + 1); }
        if (y < m.cols() - 1) { bfsKeyDistance(u, { x, y + 1 }, d + 1); }
        if (isDoor(v) && u != std::tolower(v)) { encounteredDoors.pop_back(); }
    };

    for (auto k : keyNames) {
        visited.fill(false);
        keyDistances.fill(0);
        bfsKeyDistance(k, keyLocations[k], 0);
        auto i = keyIndex(k);
        keyDistanceMatrix.row(i) = keyDistances;
    }

    visited.fill(false);
    keyDistances.fill(0);
    bfsKeyDistance('@', entrance, 0);

    // now we should have all the ingredients necessary to implement a solver
    // sketch of the algorithm:
    // - start from the entrance
    // - iterate through keys
    // - if a key is not reachable directly, iterate through the doors in beween locations
    // - recurse and iterate through the respective keys
    // utils 
    auto dist         = [&](char a, char b) { return keyDistanceMatrix(keyIndex(a), keyIndex(b)); };
    auto key          = [&](char c) -> uint32_t { return 1u << keyIndex(c); };

    std::deque<char> path;
    auto pathLength = [&]() -> int {
        auto len = keyDistances(keyIndex(path[0]));
        for (size_t i = 0; i < path.size()-1; ++i) { len += dist(path[i], path[i+1]); }
        return len;
    };

    uint32_t keys = 0;
    std::unordered_map<uint64_t, int> cache;
    int dmin = std::numeric_limits<int>::max();

    auto canReach = [&](char a, char b, uint32_t f) {
        auto it = doorsBetweenPaths.find({a, b});
        if (it == doorsBetweenPaths.end()) { 
            return true; 
        }
        auto const& doors = it->second;
        // return true if I have the keys to all these doors
        return std::all_of(doors.begin(), doors.end(), [&](char d) { return f & key(d); });
    };

    std::function<void(char, uint32_t, int)> findPath = [&](char a, uint32_t f, int d) {
        assert(!(f & key(a)));
        f |= key(a);

        if (f == allFound) {
            dmin = std::min(d, dmin);
            return;
        } 

        auto h = xxh::xxhash3<64>( { f, (uint32_t)a });

        if (cache.find(h) == cache.end()) {
            cache[h] = d;
        } else {
            if (cache[h] < d) {
                return;
            } else {
                cache[h] = d;
            }
        }

        // predicate to help sort keys/doors by reachability and proximity to the current point
        auto pred = [&](char c) {
            if (a == c) return false;
            if (f & key(c)) return false;
            if (!canReach(a, c, f)) return false;
            return true;
        };

        std::vector<char> keys;
        std::copy_if(keyNames.begin(), keyNames.end(), std::back_inserter(keys), pred);
        std::sort(keys.begin(), keys.end(), [&](auto lhs, auto rhs) { return dist(a, lhs) < dist(a, rhs); });

        for (auto b : keys) {
            auto it = doorsBetweenPaths.find({a, b});

            if (it != doorsBetweenPaths.end()) {
                std::vector<char> doors;
                std::copy_if(it->second.begin(), it->second.end(), std::back_inserter(keys), pred);
                std::sort(doors.begin(), doors.end(), [&](auto lhs, auto rhs) { return dist(a, lhs) < dist(a, rhs); });

                if (!doors.empty()) {
                    for (auto c : doors) {
                        findPath(c, f, d + dist(a, c));
                        b = c;
                    }
                }
            } 
            // continue from the last point (could be a key that was needed to unlock a door)
            findPath(b, f, d + dist(a, b));
        }
    };

    for (auto k : keyNames)
    {
        path.clear();
        findPath(k, 0, keyDistances(keyIndex(k)));
    }
    fmt::print("min distance: {}\n", dmin);

    return 0;
}

