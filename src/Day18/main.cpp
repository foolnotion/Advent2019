#include "../util.hpp"
#include "gsl/gsl"
#include <deque>
#include <functional>
#include <regex>

#include "xxh/xxhash.hpp"

using gsl::narrow;

using IntMatrix = Eigen::Matrix<int, -1, -1>; // matrix
using IntVector = Eigen::Matrix<int, -1,  1>; // column vector

struct Point {
    int x;
    int y;

    bool operator==(const Point rhs) const { return std::tie(x, y) == std::tie(rhs.x, rhs.y); }
    bool operator<(const Point rhs) const { return std::tie(x, y) < std::tie(rhs.x, rhs.y); }
    friend std::ostream& operator<<(std::ostream& os, Point& p) { os << "(" << p.x << "," << p.y << ")"; return os; }
};

struct Int {
    static constexpr int Max = std::numeric_limits<int>::max();
    static constexpr int Min = std::numeric_limits<int>::min();
};

static bool IsKey(char c) { return c >= 'a' and c <= 'z'; };
static bool IsDoor(char c) { return c >= 'A' and c <= 'Z'; };
static int  KeyIndex(char c) { assert(IsKey(c)); return c-97; }

static uint32_t Key(char c) 
{ 
    return 1u << KeyIndex(c); 
};

template<typename Derived>
uint32_t findKeys(const Eigen::MatrixBase<Derived>& matrix) {
    uint32_t f = 0;
    for (int i = 0; i < matrix.rows(); ++i) {
        for (int j = 0; j < matrix.cols(); ++j) {
            auto c = matrix(i, j);
            if (IsKey(c)) {
                f |= (1 << Key(c));
            }
        }
    }
    return f;
};

void print(const Eigen::Ref<IntMatrix>& matrix) {
    for (int j = 0; j < matrix.cols(); ++j) {
        for (int i = 0; i < matrix.rows(); ++i) {
            auto c = gsl::narrow<char>(matrix(i, j));

            if (c == '#') {
                fmt::print(fmt::fg(fmt::color::gray), "\u2588\u2588");
            } else if (IsKey(c)) {
                fmt::print(fmt::bg(fmt::color::dark_green) | fmt::fg(fmt::color::white), "{} ", c);
            } else if (IsDoor(c)) {
                fmt::print(fmt::bg(fmt::color::dark_red) | fmt::fg(fmt::color::white), "{} ", c);
            } else if (c == '@') {
                fmt::print("{} ", c);
            } else {
                fmt::print(fmt::fg(fmt::color::black), "\u2588\u2588");
            } 
        }
        std::cout << "\n";
    }  
};

template<typename Derived>
struct MapSolver {
    MapSolver(const Eigen::MatrixBase<Derived>& m) : matrix(m) 
    {
        Initialize();
        //std::cout << distanceFromEntrance << "\n";
        //std::cout << keyDistances << "\n";
    }

    void Initialize() {
        for (int i = 0; i < matrix.rows(); ++i) {
            for (int j = 0; j < matrix.cols(); ++j) {
                char c = matrix(i, j);
                if (IsKey(c)) {
                    keys.push_back(c);
                    keyPosition[c] = Point { i, j };
                }  else if (c == '@') {
                    entrance = Point { i, j };
                }
            } 
        }
        std::sort(keys.begin(), keys.end());
        visited = IntMatrix(matrix.rows(), matrix.cols());
        keyDistances = IntMatrix(26, 26);
        distanceFromEntrance = IntVector(26);

        keyDistances.fill(Int::Max);

        for (auto k : keys) {
            visited.fill(Int::Max);
            distanceFromEntrance.fill(Int::Max);
            Fill(k, keyPosition[k], 0);
            auto i = KeyIndex(k);
            keyDistances.row(i) = distanceFromEntrance;
        }

        visited.fill(Int::Max);
        distanceFromEntrance.fill(Int::Max);
        Fill('@', entrance, 0);
    }

    Point entrance;
    std::deque<char> encounteredDoors;
    std::map<char, Point> keyPosition;
    std::map<uint32_t, int> cache;
    std::vector<char> keys;
    std::map<std::pair<char, char>, std::vector<char>> doors;

    IntVector distanceFromEntrance;
    IntMatrix keyDistances;
    IntMatrix visited;

    IntMatrix matrix;

    int Distance(char a, char b) {
        return keyDistances(KeyIndex(a), KeyIndex(b));
    }

    bool Reachable(char a, char b, uint32_t f) {
        if (a == b || f & Key(b)) return false;
        auto it = doors.find({a, b});
        if (it == doors.end()) { 
            return true; 
        }
        // return true if I have the keys to all these doors
        return std::all_of(it->second.begin(), it->second.end(), [&](char d) { return f & Key(d); });
    }

    void Fill(char u, Point p, int d) {
        auto [x, y] = p;
        auto v = matrix(x, y);

        if (v == '#') {
            return; 
        }

        if (visited(x, y) > 0 && visited(x, y) <= d)
            return;

        visited(x, y) = d;

        if (u != v && IsKey(v)) {
            auto i = KeyIndex(v);
            auto e = distanceFromEntrance(i);
            distanceFromEntrance(i) = std::min(d, e);
            if (!encounteredDoors.empty()) {
                doors[{ u, v }] = std::vector<char>(encounteredDoors.begin(), encounteredDoors.end());
            }
        }

        if (IsDoor(v) && u != std::tolower(v)) { encounteredDoors.push_back(std::tolower(v)); }
        if (x > 0)                 { Fill(u, { x - 1, y }, d + 1); }
        if (x < matrix.rows() - 1) { Fill(u, { x + 1, y }, d + 1); }
        if (y > 0)                 { Fill(u, { x, y - 1 }, d + 1); }
        if (y < matrix.cols() - 1) { Fill(u, { x, y + 1 }, d + 1); }
        if (IsDoor(v) && u != std::tolower(v)) { encounteredDoors.pop_back(); }
    }

    int Path(char a, uint32_t f, char goal = 0) {
        if (a == goal) { 
            return 0; 
        }
        auto h = xxh::xxhash3<64>( { Key(a), f });
        if (cache.find(h) != cache.end()) {
            return cache[h];
        }
        assert(!(f & Key(a)));
        f |= Key(a);

        auto d = 0;
        for (auto b : keys) {
            if (!Reachable(a, b, f)) { continue; }
            auto e = Distance(a, b) + Path(b, f, goal);
            d = d == 0 ? e : std::min(e, d);
        }
        cache[h] = d;
        return d;
    }

    int ShortestPath() {
        auto shortest = Int::Max;
        cache.clear();
        for(auto k : keys) {
            if(doors.find({'@', k}) != doors.end()) {
                continue;
            }
            shortest = std::min(shortest, Path(k, 0) + distanceFromEntrance(KeyIndex(k)));
        }
        return shortest;
    }

    int ShortestPath(char goal) {
        auto shortest = Int::Max;
        cache.clear();
        for(auto k : keys) {
            if(doors.find({'@', k}) != doors.end()) {
                continue;
            }
            shortest = std::min(shortest, Path(k, 0, goal) + distanceFromEntrance(KeyIndex(k)));
        }
        return shortest;
    }

    int ShortestPath(uint32_t f) {
        fmt::print("starting from flag: {}\n", f);
        auto shortest = Int::Max;
        cache.clear();
        for(auto k : keys) {
            if(doors.find({'@', k}) != doors.end()) {
                continue;
            }
            shortest = std::min(shortest, Path(k, f) + distanceFromEntrance(KeyIndex(k)));
        }
        return shortest;
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
    IntMatrix m(w, h);

    infile.clear();
    infile.seekg(0);

    Point entrance;

    int j = 0;
    while (std::getline(infile, line)) {
        for (int i = 0; i < line.size(); ++i) {
            auto c = line[i];
            m(i, j) = c;
        }
        ++j;
    }

    MapSolver s(m);
    fmt::print("shortest path: {}\n", s.ShortestPath());

    // part 2
    // reuse the matrix m by splitting it into quadrants
    // and setting the entrances accordingly
    auto midX = m.rows() / 2 + m.rows() % 2;
    auto midY = m.cols() / 2 + m.cols() % 2;

    auto q1 = m.block(0, 0, midX, midY);
    q1(midX-1, midY-2) = '#';
    q1(midX-2, midY-1) = '#';
    q1(midX-1, midY-1) = '#';
    q1(midX-2, midY-2) = '@';

    auto q2 = m.block(0, midY-1, midX, m.cols()-midY+1);
    q2(q2.rows()-1, 1) = '#';
    q2(q2.rows()-2, 1) = '@';

    auto q3 = m.block(midX-1, midY-1, m.rows()-midX+1, m.cols()-midY+1);
    q3(1, 0) = '#';
    q3(1, 1) = '@';

    auto q4 = m.block(midX-1, 0, m.rows()-midX+1, midY);
    q4(1, q4.cols()-2) = '@';

    // this is probably not a rigorous solution (that is, it may fail for some inputs)
    // we simply ignore doors since we assume the shortest paths per quadrant will be unique anyway
    MapSolver s1(q1); s1.doors.clear();
    MapSolver s2(q2); s2.doors.clear();
    MapSolver s3(q3); s3.doors.clear();
    MapSolver s4(q4); s4.doors.clear();
    fmt::print("shortest path: {}\n", s1.ShortestPath() + s2.ShortestPath() + s3.ShortestPath() + s4.ShortestPath());

    return 0;
}

