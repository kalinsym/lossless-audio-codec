#include "lac/decoder.hpp"

#include <stdexcept>

#include "lac/bit_reader.hpp"
#include "lac/crc.hpp"

namespace lac {

namespace {

StreamInfo readStreamHeader(BitReader& r, std::span<const std::uint8_t> bytes) {
    if (r.readBits(32) != kMagic) {
        throw FormatError("decode: not a lac file");
    }
    StreamInfo info;
    info.sampleRate = r.readBits(32);
    info.channels = static_cast<std::uint8_t>(r.readBits(8));
    info.bitsPerSample = static_cast<std::uint8_t>(r.readBits(8));
    info.blockSize = static_cast<std::uint16_t>(r.readBits(16));
    info.totalFrames = r.readBits(32);
    if (r.readBits(8) != crc8(bytes.subspan(0, 16))) {
        throw FormatError("decode: stream header CRC mismatch");
    }
    return info;
}

}

WavFile decode(std::span<const std::uint8_t> bytes) {
    try {
        BitReader r(bytes);
        const StreamInfo info = readStreamHeader(r, bytes);
        WavFile out;
        out.sampleRate = info.sampleRate;
        out.channels = info.channels;
        out.bitsPerSample = info.bitsPerSample;
        return out;
    } catch (const std::out_of_range&) {
        throw FormatError("decode: file is truncated");
    }
}

}
