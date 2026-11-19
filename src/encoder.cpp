#include "lac/encoder.hpp"

#include <cstddef>
#include <span>
#include <vector>

#include "lac/bit_writer.hpp"
#include "lac/crc.hpp"
#include "lac/predictor.hpp"
#include "lac/rice.hpp"

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

void writeSubframe(BitWriter& w, std::span<const std::int32_t> x, unsigned bitsPerSample) {
    bool verbatim = true;
    unsigned bestOrder = 0;
    unsigned bestK = 0;
    std::uint64_t bestBits = std::uint64_t{bitsPerSample} * x.size();
    for (unsigned order = 0; order <= kMaxFixedOrder && order <= x.size(); ++order) {
        const std::vector<std::int32_t> residual = fixedResidual(x, order);
        const unsigned k = bestRiceParameter(residual);
        const std::uint64_t bits = 8 + std::uint64_t{order} * bitsPerSample + riceBits(residual, k);
        if (bits < bestBits) {
            verbatim = false;
            bestOrder = order;
            bestK = k;
            bestBits = bits;
        }
    }
    if (verbatim) {
        w.writeBits(kSubframeVerbatim, 8);
        for (const std::int32_t sample : x) {
            w.writeSigned(sample, bitsPerSample);
        }
        return;
    }
    w.writeBits(kSubframeFixed, 8);
    w.writeBits(bestOrder, 3);
    w.writeBits(bestK, 5);
    for (unsigned i = 0; i < bestOrder; ++i) {
        w.writeSigned(x[i], bitsPerSample);
    }
    for (const std::int32_t r : fixedResidual(x, bestOrder)) {
        writeRice(w, r, bestK);
    }
}

void writeFrame(BitWriter& w, const WavFile& wav, std::size_t firstFrame,
                std::size_t frames, std::uint32_t index) {
    const std::size_t start = w.bytes().size();
    w.writeBits(kFrameSync, 16);
    w.writeBits(index, 32);
    w.writeBits(static_cast<std::uint32_t>(frames), 16);
    w.writeBits(kChannelIndependent, 8);
    w.writeBits(crc8(bytesFrom(w, start)), 8);
    for (unsigned ch = 0; ch < wav.channels; ++ch) {
        std::vector<std::int32_t> channel(frames);
        for (std::size_t f = 0; f < frames; ++f) {
            channel[f] = wav.samples[(firstFrame + f) * wav.channels + ch];
        }
        writeSubframe(w, channel, wav.bitsPerSample);
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
