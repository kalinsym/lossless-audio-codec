#include "lac/predictor.hpp"

#include <cstddef>

namespace lac {

namespace {

constexpr std::int64_t kFixedCoefficients[kMaxFixedOrder + 1][kMaxFixedOrder] = {
    {0, 0, 0, 0},
    {1, 0, 0, 0},
    {2, -1, 0, 0},
    {3, -3, 1, 0},
    {4, -6, 4, -1},
};

}

std::vector<std::int32_t> fixedResidual(std::span<const std::int32_t> samples, unsigned order) {
    std::vector<std::int32_t> residual;
    for (std::size_t i = order; i < samples.size(); ++i) {
        std::int64_t prediction = 0;
        for (unsigned j = 0; j < order; ++j) {
            prediction += kFixedCoefficients[order][j] * samples[i - 1 - j];
        }
        residual.push_back(static_cast<std::int32_t>(samples[i] - prediction));
    }
    return residual;
}

}
