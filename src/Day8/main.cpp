#include "../util.hpp"

constexpr int W = 25;
constexpr int H = 6;

template<typename T> struct Merge {
  EIGEN_EMPTY_STRUCT_CTOR(Merge)
  using result_type = T;
  int operator()(const T& a, const T& b) const { 
      return a == 2 ? b : a;
  }
};

int main(int argc, char** argv)
{
    // Part 1
    std::ifstream infile("./input.txt");
    std::string line;
    std::getline(infile, line);

    auto imgsize = W * H;
    auto nLayers = line.size() / imgsize;

    using Image = Eigen::Array<int, H, W>;

    // part 1
    std::vector<Image> layers;
    for (size_t i = 0; i < nLayers; ++i) {
        Image m; 
        int start = imgsize * i;
        int end   = start + imgsize;
        for (int j = start; j < end; ++j) {
            auto row = (j - start) / W;
            auto col = (j - start) % W;
            m(row, col) = line[j] - '0';
        }
        layers.push_back(m);
    }

    auto [minElem, maxElem] = std::minmax_element(layers.begin(), layers.end(), [](const auto& lhs, const auto &rhs) { return (lhs == 0).count() < (rhs == 0).count(); });
    fmt::print("{}\n", (*minElem == 1).count() * (*minElem == 2).count());

    // part 2
    auto img = layers.front();
    for (size_t i = 1; i < layers.size(); ++i) {
        img = img.binaryExpr(layers[i], Merge<int>());
    }
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            fmt::print("{} ", img(i, j));
        }
        fmt::print("\n");
    }
    fmt::print("\n");

    return 0;
}
