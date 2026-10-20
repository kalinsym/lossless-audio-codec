#include "lac/wav.hpp"

#include <cstddef>
#include <string_view>

namespace lac {

namespace {

std::uint16_t readU16(std::span<const std::uint8_t> b, std::size_t at) {
    if (at + 2 > b.size()) {
        throw WavError("WAV: unexpected end of data");
    }
    return static_cast<std::uint16_t>(b[at] | (b[at + 1] << 8));
}

std::uint32_t readU32(std::span<const std::uint8_t> b, std::size_t at) {
    if (at + 4 > b.size()) {
        throw WavError("WAV: unexpected end of data");
    }
    return static_cast<std::uint32_t>(b[at])
        | (static_cast<std::uint32_t>(b[at + 1]) << 8)
        | (static_cast<std::uint32_t>(b[at + 2]) << 16)
        | (static_cast<std::uint32_t>(b[at + 3]) << 24);
}

std::int32_t readSample(std::span<const std::uint8_t> b, std::size_t at, unsigned bits) {
    if (bits == 8) {
        return static_cast<std::int32_t>(b[at]) - 128;
    }
    std::uint32_t raw = 0;
    for (unsigned k = 0; k < bits / 8; ++k) {
        raw |= static_cast<std::uint32_t>(b[at + k]) << (8 * k);
    }
    std::int64_t value = raw;
    if ((raw >> (bits - 1)) & 1u) {
        value -= std::int64_t{1} << bits;
    }
    return static_cast<std::int32_t>(value);
}

bool hasTag(std::span<const std::uint8_t> b, std::size_t at, std::string_view tag) {
    if (at + 4 > b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < 4; ++i) {
        if (b[at + i] != static_cast<std::uint8_t>(tag[i])) {
            return false;
        }
    }
    return true;
}

}

WavFile parseWav(std::span<const std::uint8_t> bytes) {
    if (!hasTag(bytes, 0, "RIFF") || !hasTag(bytes, 8, "WAVE")) {
        throw WavError("WAV: not a RIFF/WAVE file");
    }

    WavFile audio;
    bool haveFormat = false;
    std::size_t pos = 12;

    while (pos + 8 <= bytes.size()) {
        const std::uint32_t size = readU32(bytes, pos + 4);
        const std::size_t body = pos + 8;
        if (size > bytes.size() - body) {
            throw WavError("WAV: chunk extends past end of file");
        }

        if (hasTag(bytes, pos, "fmt ")) {
            if (size < 16) {
                throw WavError("WAV: fmt chunk too short");
            }
            if (readU16(bytes, body) != 1) {
                throw WavError("WAV: only PCM format is supported");
            }
            audio.channels = readU16(bytes, body + 2);
            audio.sampleRate = readU32(bytes, body + 4);
            audio.bitsPerSample = readU16(bytes, body + 14);
            if (audio.channels != 1 && audio.channels != 2) {
                throw WavError("WAV: only mono and stereo are supported");
            }
            if (audio.bitsPerSample != 8 && audio.bitsPerSample != 16 && audio.bitsPerSample != 24) {
                throw WavError("WAV: only 8-, 16- and 24-bit samples are supported");
            }
            haveFormat = true;
        } else if (hasTag(bytes, pos, "data")) {
            if (!haveFormat) {
                throw WavError("WAV: data chunk before fmt chunk");
            }
            const std::size_t sampleBytes = audio.bitsPerSample / 8u;
            const std::size_t frameBytes = sampleBytes * audio.channels;
            if (size % frameBytes != 0) {
                throw WavError("WAV: data is not a whole number of frames");
            }
            audio.samples.reserve(size / sampleBytes);
            for (std::size_t i = 0; i < size; i += sampleBytes) {
                audio.samples.push_back(readSample(bytes, body + i, audio.bitsPerSample));
            }
            return audio;
        }

        pos = body + size + (size & 1u);
    }

    throw WavError("WAV: missing fmt or data chunk");
}

std::vector<std::uint8_t> serializeWav(const WavFile&) {
    throw WavError("serializeWav: not implemented");
}

}
