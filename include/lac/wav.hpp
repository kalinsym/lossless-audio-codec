#ifndef LAC_WAV_HPP
#define LAC_WAV_HPP

#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

namespace lac {

struct WavAudio {
    std::uint16_t channels = 0;
    std::uint32_t sampleRate = 0;
    std::uint16_t bitsPerSample = 0;
    std::vector<std::int32_t> samples;

    bool operator==(const WavAudio&) const = default;
};

class WavError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

WavAudio readWav(std::span<const std::uint8_t> bytes);
std::vector<std::uint8_t> writeWav(const WavAudio& audio);

}

#endif
