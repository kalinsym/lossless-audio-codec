#ifndef LAC_RICE_HPP
#define LAC_RICE_HPP

#include <cstdint>

namespace lac {

constexpr std::uint32_t zigzag(std::int32_t value) {
    return (static_cast<std::uint32_t>(value) << 1) ^ static_cast<std::uint32_t>(value >> 31);
}

constexpr std::int32_t unzigzag(std::uint32_t value) {
    return static_cast<std::int32_t>((value >> 1) ^ (0u - (value & 1u)));
}

}

#endif
