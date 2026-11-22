#include "lac/encoder.hpp"

#include <cstddef>
#include <span>
#include <vector>

#include "lac/bit_writer.hpp"
#include "lac/crc.hpp"
#include "lac/predictor.hpp"
#include "lac/rice.hpp"
#include "lac/stereo.hpp"

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

std::uint64_t subframeBits(std::span<const std::int32_t> x, unsigned bitsPerSample) {
    BitWriter scratch;
    writeSubframe(scratch, x, bitsPerSample);
    return scratch.bitCount();
}

std::uint32_t chooseStereo(std::vector<std::int32_t>& a, std::vector<std::int32_t>& b, unsigned bits) {
    std::vector<std::int32_t> mid(a.size());
    std::vector<std::int32_t> side(a.size());
    for (std::size_t f = 0; f < a.size(); ++f) {
        mid[f] = midOf(a[f], b[f]);
        side[f] = sideOf(a[f], b[f]);
    }
    const std::uint64_t l = subframeBits(a, bits);
    const std::uint64_t r = subframeBits(b, bits);
    const std::uint64_t s = subframeBits(side, bits + 1);
    const std::uint64_t cost[] = {l + r, l + s, s + r, subframeBits(mid, bits) + s};
    std::uint32_t mode = kChannelIndependent;
    for (std::uint32_t m = 1; m <= kChannelMidSide; ++m) {
        if (cost[m] < cost[mode]) {
            mode = m;
        }
    }
    if (mode == kChannelMidSide) {
        a = mid;
    }
    if (isSideChannel(mode, 0)) {
        a = side;
    }
    if (isSideChannel(mode, 1)) {
        b = side;
    }
    return mode;
}

void writeFrame(BitWriter& w, const WavFile& wav, std::size_t firstFrame,
                std::size_t frames, std::uint32_t index) {
    const std::size_t start = w.bytes().size();
    w.writeBits(kFrameSync, 16);
    w.writeBits(index, 32);
    w.writeBits(static_cast<std::uint32_t>(frames), 16);
    std::vector<std::vector<std::int32_t>> channels(wav.channels, std::vector<std::int32_t>(frames));
    for (unsigned ch = 0; ch < wav.channels; ++ch) {
        for (std::size_t f = 0; f < frames; ++f) {
            channels[ch][f] = wav.samples[(firstFrame + f) * wav.channels + ch];
        }
    }
    const std::uint32_t mode =
        wav.channels == 2 ? chooseStereo(channels[0], channels[1], wav.bitsPerSample) : kChannelIndependent;
    w.writeBits(mode, 8);
    w.writeBits(crc8(bytesFrom(w, start)), 8);
    for (unsigned ch = 0; ch < wav.channels; ++ch) {
        writeSubframe(w, channels[ch], wav.bitsPerSample + (isSideChannel(mode, ch) ? 1u : 0u));
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
