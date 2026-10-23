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

std::uint16_t crc16(std::span<const std::uint8_t> data) {
    std::uint16_t crc = 0;
    for (std::uint8_t byte : data) {
        crc ^= static_cast<std::uint16_t>(byte << 8);
        for (int bit = 0; bit < 8; ++bit) {
            const bool top = (crc & 0x8000u) != 0;
            crc = static_cast<std::uint16_t>(crc << 1);
            if (top) {
                crc ^= 0x8005u;
            }
        }
    }
    return crc;
}

}
