#include "lac/rice.hpp"

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

}
