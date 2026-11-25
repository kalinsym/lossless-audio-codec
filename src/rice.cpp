#include "lac/rice.hpp"

#include <algorithm>
#include <stdexcept>

namespace lac {

void writeRice(BitWriter& w, std::int32_t value, unsigned k) {
    if (k > kMaxRiceParameter) {
        throw std::invalid_argument("writeRice: parameter must be at most 30");
    }
    const std::uint32_t u = zigzag(value);
    w.writeUnary(u >> k);
    w.writeBits(u, k);
}

std::int32_t readRice(BitReader& r, unsigned k) {
    const std::uint32_t q = r.readUnary();
    return unzigzag((q << k) | r.readBits(k));
}

std::uint64_t riceBits(std::span<const std::int32_t> residual, unsigned k) {
    std::uint64_t bits = 0;
    for (const std::int32_t r : residual) {
        bits += (zigzag(r) >> k) + 1 + k;
    }
    return bits;
}

unsigned bestRiceParameter(std::span<const std::int32_t> residual) {
    if (residual.empty()) {
        return 0;
    }
    std::uint64_t sum = 0;
    for (const std::int32_t r : residual) {
        sum += zigzag(r);
    }
    const std::uint64_t mean = sum / residual.size();
    unsigned guess = 0;
    while (guess < kMaxRiceParameter && (std::uint64_t{1} << (guess + 1)) <= mean) {
        ++guess;
    }
    unsigned best = guess > 0 ? guess - 1 : 0;
    std::uint64_t bestBits = riceBits(residual, best);
    for (unsigned k = best + 1; k <= std::min(guess + 1, kMaxRiceParameter); ++k) {
        const std::uint64_t bits = riceBits(residual, k);
        if (bits < bestBits) {
            best = k;
            bestBits = bits;
        }
    }
    return best;
}

}
