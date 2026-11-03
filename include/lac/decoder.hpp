#ifndef LAC_DECODER_HPP
#define LAC_DECODER_HPP

#include <cstdint>
#include <span>

#include "lac/format.hpp"
#include "lac/wav.hpp"

namespace lac {

WavFile decode(std::span<const std::uint8_t> bytes);

}

#endif
