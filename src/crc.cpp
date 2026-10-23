#include "lac/crc.hpp"

namespace lac {

std::uint8_t crc8(std::span<const std::uint8_t> data) {
    std::uint8_t crc = 0;
    for (std::uint8_t byte : data) {
        crc ^= byte;
        for (int bit = 0; bit < 8; ++bit) {
            const bool top = (crc & 0x80u) != 0;
            crc = static_cast<std::uint8_t>(crc << 1);
            if (top) {
                crc ^= 0x07u;
            }
        }
    }
    return crc;
}

}
