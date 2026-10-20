#include "lac/wav.hpp"

namespace lac {

WavAudio readWav(std::span<const std::uint8_t>) {
    throw WavError("readWav: not implemented");
}

std::vector<std::uint8_t> writeWav(const WavAudio&) {
    throw WavError("writeWav: not implemented");
}

}
