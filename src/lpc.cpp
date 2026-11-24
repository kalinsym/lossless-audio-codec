#include "lac/lpc.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace lac {

namespace {

std::int64_t lpcPredict(std::span<const std::int32_t> x, std::size_t i, const QuantizedLpc& q) {
    std::int64_t sum = 0;
    for (std::size_t j = 0; j < q.coefficients.size(); ++j) {
        sum += std::int64_t{q.coefficients[j]} * x[i - 1 - j];
    }
    return sum >> q.shift;
}

}

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

QuantizedLpc quantizeLpc(std::span<const double> a, unsigned precision) {
    double largest = 0.0;
    for (const double c : a) {
        largest = std::max(largest, std::abs(c));
    }
    int exponent = 0;
    std::frexp(largest, &exponent);
    const int shift = std::clamp(static_cast<int>(precision) - 1 - exponent, 0, static_cast<int>(kMaxLpcShift));
    const double limit = std::ldexp(1.0, static_cast<int>(precision) - 1) - 1.0;
    QuantizedLpc q;
    q.shift = static_cast<unsigned>(shift);
    double carry = 0.0;
    for (const double c : a) {
        const double scaled = std::clamp(std::ldexp(c, shift) + carry, -limit - 1.0, limit);
        const double rounded = std::round(scaled);
        carry = scaled - rounded;
        q.coefficients.push_back(static_cast<std::int32_t>(rounded));
    }
    return q;
}

std::optional<std::vector<std::int32_t>> lpcResidual(std::span<const std::int32_t> samples, const QuantizedLpc& q) {
    std::vector<std::int32_t> residual;
    for (std::size_t i = q.coefficients.size(); i < samples.size(); ++i) {
        const std::int64_t r = samples[i] - lpcPredict(samples, i, q);
        if (r < std::numeric_limits<std::int32_t>::min() || r > std::numeric_limits<std::int32_t>::max()) {
            return std::nullopt;
        }
        residual.push_back(static_cast<std::int32_t>(r));
    }
    return residual;
}

}
