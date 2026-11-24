#include "lac/lpc.hpp"

#include <cstddef>
#include <stdexcept>

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

std::vector<double> levinsonDurbin(std::span<const double> r, unsigned order) {
    if (r.size() <= order) {
        throw std::invalid_argument("levinsonDurbin: need order + 1 autocorrelation values");
    }
    std::vector<double> a(order, 0.0);
    double error = r[0];
    for (unsigned i = 0; i < order && error > 0.0; ++i) {
        double acc = r[i + 1];
        for (unsigned j = 0; j < i; ++j) {
            acc -= a[j] * r[i - j];
        }
        const double k = acc / error;
        const std::vector<double> previous(a);
        a[i] = k;
        for (unsigned j = 0; j < i; ++j) {
            a[j] = previous[j] - k * previous[i - 1 - j];
        }
        error *= 1.0 - k * k;
    }
    return a;
}

}
