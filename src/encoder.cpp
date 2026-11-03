#include "lac/encoder.hpp"

#include <cstddef>
#include <span>

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

std::span<const std::uint8_t> bytesFrom(const BitWriter& w, std::size_t start) {
    return std::span<const std::uint8_t>(w.bytes()).subspan(start);
}

void writeFrame(BitWriter& w, const WavFile& wav, std::size_t firstFrame,
                std::size_t frames, std::uint32_t index) {
    const std::size_t start = w.bytes().size();
    w.writeBits(kFrameSync, 16);
    w.writeBits(index, 32);
    w.writeBits(static_cast<std::uint32_t>(frames), 16);
    w.writeBits(crc8(bytesFrom(w, start)), 8);
    for (unsigned ch = 0; ch < wav.channels; ++ch) {
        w.writeBits(kSubframeVerbatim, 8);
        for (std::size_t f = 0; f < frames; ++f) {
            w.writeSigned(wav.samples[(firstFrame + f) * wav.channels + ch], wav.bitsPerSample);
        }
    }
    w.flush();
    w.writeBits(crc16(bytesFrom(w, start)), 16);
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
    const std::size_t total = wav.frameCount();
    std::uint32_t index = 0;
    for (std::size_t first = 0; first < total; first += blockSize) {
        const std::size_t frames = total - first < blockSize ? total - first : blockSize;
        writeFrame(w, wav, first, frames, index++);
    }
    w.flush();
    return w.bytes();
}

}
