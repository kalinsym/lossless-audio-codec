#ifndef LAC_CRC_HPP
#define LAC_CRC_HPP

#include <cstdint>
#include <span>

namespace lac {

std::uint8_t crc8(std::span<const std::uint8_t> data);
std::uint16_t crc16(std::span<const std::uint8_t> data);

}

#endif
