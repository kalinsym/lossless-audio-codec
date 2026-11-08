#ifndef LAC_PREDICTOR_HPP
#define LAC_PREDICTOR_HPP

#include <cstdint>
#include <span>
#include <vector>

namespace lac {

inline constexpr unsigned kMaxFixedOrder = 4;

std::vector<std::int32_t> fixedResidual(std::span<const std::int32_t> samples, unsigned order);

}

#endif
