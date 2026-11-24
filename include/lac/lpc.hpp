#ifndef LAC_LPC_HPP
#define LAC_LPC_HPP

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace lac {

inline constexpr unsigned kMaxLpcOrder = 32;
inline constexpr unsigned kMaxLpcShift = 15;

struct QuantizedLpc {
    std::vector<std::int32_t> coefficients;
    unsigned shift = 0;
};

std::vector<double> autocorrelation(std::span<const std::int32_t> samples, unsigned maxLag);
std::vector<double> levinsonDurbin(std::span<const double> r, unsigned order);
QuantizedLpc quantizeLpc(std::span<const double> a, unsigned precision);
std::optional<std::vector<std::int32_t>> lpcResidual(std::span<const std::int32_t> samples, const QuantizedLpc& q);

}

#endif
