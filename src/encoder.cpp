#include "lac/encoder.hpp"

#include "lac/bit_writer.hpp"
#include "lac/crc.hpp"

namespace lac {

namespace {

void writeStreamHeader(BitWriter& w, const WavFile& wav, unsigned blockSize) {
    w.writeBits(kMagic, 32);
    w.writeBits(wav.sampleRate, 32);
    w.writeBits(wav.channels, 8);
    w.writeBits(wav.bitsPerSample, 8);
    w.writeBits(blockSize, 16);
    w.writeBits(static_cast<std::uint32_t>(wav.frameCount()), 32);
    w.writeBits(crc8(w.bytes()), 8);
}

}

std::vector<std::uint8_t> encode(const WavFile& wav, unsigned blockSize) {
    if (blockSize == 0 || blockSize > 0xFFFF) {
        throw FormatError("encode: block size must be between 1 and 65535");
    }
    if (wav.frameCount() > 0xFFFFFFFFu) {
        throw FormatError("encode: too many frames");
    }
    BitWriter w;
    writeStreamHeader(w, wav, blockSize);
    w.flush();
    return w.bytes();
}

}
