#ifndef LAC_ENCODER_HPP
#define LAC_ENCODER_HPP

#include <cstdint>
#include <vector>

#include "lac/format.hpp"
#include "lac/wav.hpp"

namespace lac {

std::vector<std::uint8_t> encode(const WavFile& wav, unsigned blockSize = kDefaultBlockSize);

}

#endif
