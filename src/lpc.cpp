#include "lac/lpc.hpp"

#include <cstddef>

namespace lac {

std::vector<double> autocorrelation(std::span<const std::int32_t> samples, unsigned maxLag) {
    std::vector<double> r(maxLag + 1, 0.0);
    for (unsigned lag = 0; lag <= maxLag; ++lag) {
        for (std::size_t i = lag; i < samples.size(); ++i) {
            r[lag] += static_cast<double>(samples[i]) * static_cast<double>(samples[i - lag]);
        }
    }
    return r;
}

}
