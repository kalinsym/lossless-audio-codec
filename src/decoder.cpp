#include "lac/decoder.hpp"

#include <stdexcept>

#include "lac/bit_reader.hpp"

namespace lac {

WavFile decode(std::span<const std::uint8_t> bytes) {
    try {
        BitReader r(bytes);
        if (r.readBits(32) != kMagic) {
            throw FormatError("decode: not a lac file");
        }
        return WavFile{};
    } catch (const std::out_of_range&) {
        throw FormatError("decode: file is truncated");
    }
}

}
