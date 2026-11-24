#ifndef LAC_LPC_HPP
#define LAC_LPC_HPP

#include <cstdint>
#include <span>
#include <vector>

namespace lac {

inline constexpr unsigned kMaxLpcOrder = 32;

std::vector<double> autocorrelation(std::span<const std::int32_t> samples, unsigned maxLag);

}

#endif
