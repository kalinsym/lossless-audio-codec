#ifndef LAC_FORMAT_HPP
#define LAC_FORMAT_HPP

#include <cstdint>
#include <stdexcept>

namespace lac {

inline constexpr std::uint32_t kMagic = 0x4C414331;
inline constexpr std::uint16_t kFrameSync = 0xFFF8;
inline constexpr unsigned kDefaultBlockSize = 4096;
inline constexpr std::uint8_t kSubframeVerbatim = 0;

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
