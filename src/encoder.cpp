#include "lac/encoder.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <span>
#include <vector>

#include "lac/bit_writer.hpp"
#include "lac/crc.hpp"
#include "lac/lpc.hpp"
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

inline constexpr unsigned kLpcSearchOrder = 12;
inline constexpr unsigned kLpcPrecision = 15;

struct LpcPlan {
    QuantizedLpc q;
    unsigned k = 0;
    std::vector<std::int32_t> residual;
    std::uint64_t bits = 0;
};

std::optional<LpcPlan> planLpc(std::span<const std::int32_t> x, unsigned bitsPerSample) {
    std::optional<LpcPlan> best;
    const auto maxOrder = static_cast<unsigned>(std::min<std::size_t>(kLpcSearchOrder, x.size() / 2));
    if (maxOrder == 0) {
        return best;
    }
    const std::vector<double> r = autocorrelation(x, maxOrder);
    for (unsigned order = 1; order <= maxOrder; ++order) {
        QuantizedLpc q = quantizeLpc(levinsonDurbin(r, order), kLpcPrecision);
        std::optional<std::vector<std::int32_t>> residual = lpcResidual(x, q);
        if (!residual) {
            continue;
        }
        const unsigned k = bestRiceParameter(*residual);
        const std::uint64_t bits = 26 + std::uint64_t{order} * (bitsPerSample + kLpcPrecision) + riceBits(*residual, k);
        if (!best || bits < best->bits) {
            best = LpcPlan{std::move(q), k, std::move(*residual), bits};
        }
    }
    return best;
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
    const std::optional<LpcPlan> lpc = planLpc(x, bitsPerSample);
    if (lpc && lpc->bits < bestBits) {
        w.writeBits(kSubframeLpc, 8);
        w.writeBits(static_cast<std::uint32_t>(lpc->q.coefficients.size() - 1), 5);
        w.writeBits(kLpcPrecision - 1, 4);
        w.writeBits(lpc->q.shift, 4);
        w.writeBits(lpc->k, 5);
        for (std::size_t i = 0; i < lpc->q.coefficients.size(); ++i) {
            w.writeSigned(x[i], bitsPerSample);
        }
        for (const std::int32_t c : lpc->q.coefficients) {
            w.writeSigned(c, kLpcPrecision);
        }
        for (const std::int32_t r : lpc->residual) {
            writeRice(w, r, lpc->k);
        }
        return;
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

BitWriter encodeSubframe(std::span<const std::int32_t> x, unsigned bitsPerSample) {
    BitWriter w;
    writeSubframe(w, x, bitsPerSample);
    return w;
}

std::uint32_t chooseStereo(std::span<const std::int32_t> a, std::span<const std::int32_t> b, unsigned bits,
                           std::vector<BitWriter>& chosen) {
    std::vector<std::int32_t> mid(a.size());
    std::vector<std::int32_t> side(a.size());
    for (std::size_t f = 0; f < a.size(); ++f) {
        mid[f] = midOf(a[f], b[f]);
        side[f] = sideOf(a[f], b[f]);
    }
    std::vector<BitWriter> sub;
    sub.push_back(encodeSubframe(a, bits));
    sub.push_back(encodeSubframe(b, bits));
    sub.push_back(encodeSubframe(mid, bits));
    sub.push_back(encodeSubframe(side, bits + 1));
    constexpr std::size_t uses[4][2] = {{0, 1}, {0, 3}, {3, 1}, {2, 3}};
    std::uint32_t mode = kChannelIndependent;
    for (std::uint32_t m = 1; m <= kChannelMidSide; ++m) {
        const std::uint64_t cost = sub[uses[m][0]].bitCount() + sub[uses[m][1]].bitCount();
        if (cost < sub[uses[mode][0]].bitCount() + sub[uses[mode][1]].bitCount()) {
            mode = m;
        }
    }
    chosen.push_back(std::move(sub[uses[mode][0]]));
    chosen.push_back(std::move(sub[uses[mode][1]]));
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
    std::vector<BitWriter> subframes;
    std::uint32_t mode = kChannelIndependent;
    if (wav.channels == 2) {
        mode = chooseStereo(channels[0], channels[1], wav.bitsPerSample, subframes);
    } else {
        subframes.push_back(encodeSubframe(channels[0], wav.bitsPerSample));
    }
    w.writeBits(mode, 8);
    w.writeBits(crc8(bytesFrom(w, start)), 8);
    for (const BitWriter& subframe : subframes) {
        w.append(subframe);
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
