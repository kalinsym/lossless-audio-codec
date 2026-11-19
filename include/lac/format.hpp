#ifndef LAC_FORMAT_HPP
#define LAC_FORMAT_HPP

#include <cstdint>
#include <stdexcept>

namespace lac {

inline constexpr std::uint32_t kMagic = 0x4C414331;
inline constexpr std::uint16_t kFrameSync = 0xFFF8;
inline constexpr unsigned kDefaultBlockSize = 4096;
inline constexpr std::uint8_t kSubframeVerbatim = 0;
inline constexpr std::uint8_t kSubframeFixed = 1;
inline constexpr std::uint8_t kChannelIndependent = 0;
inline constexpr std::uint8_t kChannelLeftSide = 1;
inline constexpr std::uint8_t kChannelRightSide = 2;
inline constexpr std::uint8_t kChannelMidSide = 3;

constexpr bool isSideChannel(std::uint32_t mode, unsigned channel) {
    return mode == kChannelRightSide ? channel == 0 : mode != kChannelIndependent && channel == 1;
}

struct StreamInfo {
    std::uint32_t sampleRate = 0;
    std::uint8_t channels = 0;
    std::uint8_t bitsPerSample = 0;
    std::uint16_t blockSize = 0;
    std::uint32_t totalFrames = 0;
};

class FormatError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

}

#endif
