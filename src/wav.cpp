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

void putU16(std::vector<std::uint8_t>& out, std::uint32_t v) {
    out.push_back(static_cast<std::uint8_t>(v & 0xFFu));
    out.push_back(static_cast<std::uint8_t>((v >> 8) & 0xFFu));
}

void putU32(std::vector<std::uint8_t>& out, std::uint32_t v) {
    putU16(out, v & 0xFFFFu);
    putU16(out, v >> 16);
}

void putTag(std::vector<std::uint8_t>& out, std::string_view tag) {
    for (char c : tag) {
        out.push_back(static_cast<std::uint8_t>(c));
    }
}

void writeSample(std::vector<std::uint8_t>& out, std::int32_t sample, unsigned bits) {
    const std::int64_t limit = std::int64_t{1} << (bits - 1);
    if (sample < -limit || sample >= limit) {
        throw WavError("WAV: sample does not fit the bit depth");
    }
    if (bits == 8) {
        out.push_back(static_cast<std::uint8_t>(sample + 128));
        return;
    }
    const auto u = static_cast<std::uint32_t>(sample);
    for (unsigned k = 0; k < bits / 8; ++k) {
        out.push_back(static_cast<std::uint8_t>((u >> (8 * k)) & 0xFFu));
    }
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

std::vector<std::uint8_t> serializeWav(const WavFile& wav) {
    if (wav.channels != 1 && wav.channels != 2) {
        throw WavError("WAV: only mono and stereo are supported");
    }
    if (wav.bitsPerSample != 8 && wav.bitsPerSample != 16 && wav.bitsPerSample != 24) {
        throw WavError("WAV: only 8-, 16- and 24-bit samples are supported");
    }
    if (wav.samples.size() % wav.channels != 0) {
        throw WavError("WAV: samples are not a whole number of frames");
    }

    const std::uint32_t sampleBytes = wav.bitsPerSample / 8u;
    const std::uint32_t blockAlign = sampleBytes * wav.channels;
    const std::size_t dataSize = wav.samples.size() * sampleBytes;
    if (dataSize > 0xFFFFFFFFu - 36u) {
        throw WavError("WAV: too much audio for one file");
    }

    std::vector<std::uint8_t> out;
    out.reserve(44 + dataSize);
    putTag(out, "RIFF");
    putU32(out, static_cast<std::uint32_t>(36 + dataSize));
    putTag(out, "WAVE");

    putTag(out, "fmt ");
    putU32(out, 16);
    putU16(out, 1);
    putU16(out, wav.channels);
    putU32(out, wav.sampleRate);
    putU32(out, wav.sampleRate * blockAlign);
    putU16(out, blockAlign);
    putU16(out, wav.bitsPerSample);

    putTag(out, "data");
    putU32(out, static_cast<std::uint32_t>(dataSize));
    for (std::int32_t sample : wav.samples) {
        writeSample(out, sample, wav.bitsPerSample);
    }
    return out;
}

}
