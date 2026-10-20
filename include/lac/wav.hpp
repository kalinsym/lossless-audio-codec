#ifndef LAC_WAV_HPP
#define LAC_WAV_HPP

#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

namespace lac {

struct WavFile {
    std::uint16_t channels = 0;
    std::uint32_t sampleRate = 0;
    std::uint16_t bitsPerSample = 0;
    std::vector<std::int32_t> samples;

    std::size_t frameCount() const {
        return channels == 0 ? 0 : samples.size() / channels;
    }

    bool operator==(const WavFile&) const = default;
};

class WavError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

WavFile parseWav(std::span<const std::uint8_t> bytes);
std::vector<std::uint8_t> serializeWav(const WavFile& audio);

}

#endif
