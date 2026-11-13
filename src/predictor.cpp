#include "lac/predictor.hpp"

#include <cstddef>
#include <stdexcept>

namespace lac {

namespace {

constexpr std::int64_t kFixedCoefficients[kMaxFixedOrder + 1][kMaxFixedOrder] = {
    {0, 0, 0, 0},
    {1, 0, 0, 0},
    {2, -1, 0, 0},
    {3, -3, 1, 0},
    {4, -6, 4, -1},
};

std::int64_t predict(std::span<const std::int32_t> x, std::size_t i, unsigned order) {
    std::int64_t prediction = 0;
    for (unsigned j = 0; j < order; ++j) {
        prediction += kFixedCoefficients[order][j] * x[i - 1 - j];
    }
    return prediction;
}

}

std::vector<std::int32_t> fixedResidual(std::span<const std::int32_t> samples, unsigned order) {
    if (order > kMaxFixedOrder) {
        throw std::invalid_argument("fixedResidual: order must be between 0 and 4");
    }
    std::vector<std::int32_t> residual;
    for (std::size_t i = order; i < samples.size(); ++i) {
        residual.push_back(static_cast<std::int32_t>(samples[i] - predict(samples, i, order)));
    }
    return residual;
}

std::vector<std::int32_t> fixedRestore(std::span<const std::int32_t> warmup,
                                       std::span<const std::int32_t> residual) {
    const auto order = static_cast<unsigned>(warmup.size());
    if (order > kMaxFixedOrder) {
        throw std::invalid_argument("fixedRestore: at most 4 warm-up samples");
    }
    std::vector<std::int32_t> samples(warmup.begin(), warmup.end());
    for (const std::int32_t r : residual) {
        samples.push_back(static_cast<std::int32_t>(r + predict(samples, samples.size(), order)));
    }
    return samples;
}

}
