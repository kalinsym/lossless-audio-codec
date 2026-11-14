#ifndef LAC_RICE_HPP
#define LAC_RICE_HPP

#include <cstdint>

#include "lac/bit_reader.hpp"
#include "lac/bit_writer.hpp"

namespace lac {

inline constexpr unsigned kMaxRiceParameter = 30;

constexpr std::uint32_t zigzag(std::int32_t value) {
    return (static_cast<std::uint32_t>(value) << 1) ^ static_cast<std::uint32_t>(value >> 31);
}

constexpr std::int32_t unzigzag(std::uint32_t value) {
    return static_cast<std::int32_t>((value >> 1) ^ (0u - (value & 1u)));
}

void writeRice(BitWriter& w, std::int32_t value, unsigned k);
std::int32_t readRice(BitReader& r, unsigned k);

}

#endif
