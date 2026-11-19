#include "lac/decoder.hpp"

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "lac/bit_reader.hpp"
#include "lac/crc.hpp"
#include "lac/predictor.hpp"
#include "lac/rice.hpp"
#include "lac/stereo.hpp"

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
    if (info.channels < 1 || info.channels > 2) {
        throw FormatError("decode: unsupported channel count");
    }
    if (info.bitsPerSample != 8 && info.bitsPerSample != 16 && info.bitsPerSample != 24) {
        throw FormatError("decode: unsupported sample size");
    }
    if (info.blockSize == 0) {
        throw FormatError("decode: block size is zero");
    }
    return info;
}

std::vector<std::int32_t> readSubframe(BitReader& r, unsigned bitsPerSample, std::size_t frames) {
    const std::uint32_t type = r.readBits(8);
    std::vector<std::int32_t> samples;
    if (type == kSubframeVerbatim) {
        for (std::size_t f = 0; f < frames; ++f) {
            samples.push_back(r.readSigned(bitsPerSample));
        }
        return samples;
    }
    if (type != kSubframeFixed) {
        throw FormatError("decode: unknown subframe type");
    }
    const unsigned order = r.readBits(3);
    const unsigned k = r.readBits(5);
    if (order > kMaxFixedOrder || order > frames || k > kMaxRiceParameter) {
        throw FormatError("decode: invalid fixed subframe");
    }
    for (unsigned i = 0; i < order; ++i) {
        samples.push_back(r.readSigned(bitsPerSample));
    }
    std::vector<std::int32_t> residual;
    for (std::size_t f = order; f < frames; ++f) {
        residual.push_back(readRice(r, k));
    }
    return fixedRestore(samples, residual);
}

void undoStereo(std::uint32_t mode, std::vector<std::int32_t>& a, std::vector<std::int32_t>& b) {
    for (std::size_t f = 0; f < a.size(); ++f) {
        if (mode == kChannelLeftSide) {
            b[f] = a[f] - b[f];
        } else if (mode == kChannelRightSide) {
            a[f] = a[f] + b[f];
        } else if (mode == kChannelMidSide) {
            a[f] = leftOf(a[f], b[f]);
            b[f] = a[f] - b[f];
        }
    }
}

void readFrame(BitReader& r, std::span<const std::uint8_t> bytes, const StreamInfo& info,
               std::uint32_t index, std::size_t expected, WavFile& out) {
    const std::size_t start = r.bytePosition();
    if (r.readBits(16) != kFrameSync || r.readBits(32) != index) {
        throw FormatError("decode: frame header is damaged");
    }
    const std::size_t frames = r.readBits(16);
    const std::uint32_t mode = r.readBits(8);
    if (r.readBits(8) != crc8(bytes.subspan(start, 9))) {
        throw FormatError("decode: frame header CRC mismatch");
    }
    if (frames != expected) {
        throw FormatError("decode: unexpected frame length");
    }
    const std::size_t base = out.samples.size();
    out.samples.resize(base + frames * info.channels);
    if (mode > kChannelMidSide || (mode != kChannelIndependent && info.channels != 2)) {
        throw FormatError("decode: invalid channel mode");
    }
    std::vector<std::vector<std::int32_t>> channels;
    for (unsigned ch = 0; ch < info.channels; ++ch) {
        const unsigned bits = info.bitsPerSample + (isSideChannel(mode, ch) ? 1u : 0u);
        channels.push_back(readSubframe(r, bits, frames));
    }
    if (info.channels == 2) {
        undoStereo(mode, channels[0], channels[1]);
    }
    for (unsigned ch = 0; ch < info.channels; ++ch) {
        for (std::size_t f = 0; f < frames; ++f) {
            out.samples[base + f * info.channels + ch] = channels[ch][f];
        }
    }
    r.alignToByte();
    const std::size_t end = r.bytePosition();
    if (r.readBits(16) != crc16(bytes.subspan(start, end - start))) {
        throw FormatError("decode: frame CRC mismatch");
    }
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
        std::uint32_t index = 0;
        for (std::size_t done = 0; done < info.totalFrames; done += info.blockSize) {
            const std::size_t left = info.totalFrames - done;
            readFrame(r, bytes, info, index++, left < info.blockSize ? left : info.blockSize, out);
        }
        if (!r.atEnd()) {
            throw FormatError("decode: data after the last frame");
        }
        return out;
    } catch (const std::out_of_range&) {
        throw FormatError("decode: file is truncated");
    }
}

}
