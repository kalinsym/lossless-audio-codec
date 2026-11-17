#ifndef LAC_STEREO_HPP
#define LAC_STEREO_HPP

#include <cstdint>

namespace lac {

constexpr std::int32_t sideOf(std::int32_t left, std::int32_t right) {
    return left - right;
}

constexpr std::int32_t midOf(std::int32_t left, std::int32_t right) {
    return (left + right) >> 1;
}

constexpr std::int32_t leftOf(std::int32_t mid, std::int32_t side) {
    return ((mid << 1) + (side & 1) + side) >> 1;
}

}

#endif
