#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

#include "lac/wav.hpp"

using lac::parseWav;
using lac::serializeWav;
using lac::WavError;
using lac::WavFile;
using Bytes = std::vector<std::uint8_t>;

namespace {

void putU16(Bytes& b, std::uint32_t v) {
    b.push_back(static_cast<std::uint8_t>(v & 0xFF));
    b.push_back(static_cast<std::uint8_t>((v >> 8) & 0xFF));
}

void putU32(Bytes& b, std::uint32_t v) {
    putU16(b, v & 0xFFFF);
    putU16(b, v >> 16);
}

void putTag(Bytes& b, const char* tag) {
    for (int i = 0; i < 4; ++i) b.push_back(static_cast<std::uint8_t>(tag[i]));
}

Bytes packSamples(const std::vector<std::int32_t>& samples, unsigned bitsPerSample) {
    Bytes out;
    for (std::int32_t s : samples) {
        if (bitsPerSample == 8) {
            out.push_back(static_cast<std::uint8_t>(s + 128));
        } else if (bitsPerSample == 16) {
            putU16(out, static_cast<std::uint16_t>(s));
        } else {
            const std::uint32_t u = static_cast<std::uint32_t>(s);
            out.push_back(static_cast<std::uint8_t>(u & 0xFF));
            out.push_back(static_cast<std::uint8_t>((u >> 8) & 0xFF));
            out.push_back(static_cast<std::uint8_t>((u >> 16) & 0xFF));
        }
    }
    return out;
}

struct WavSpec {
    std::string riffTag = "RIFF";
    std::string waveTag = "WAVE";
    bool includeFmt = true;
    std::uint16_t formatTag = 1;
    std::uint16_t channels = 2;
    std::uint32_t sampleRate = 44100;
    std::uint16_t bitsPerSample = 16;
    Bytes extraChunkBeforeData;
    std::int32_t dataSizeOverride = -1;
    std::int32_t riffSizeOverride = -1;
};

Bytes buildWav(const WavSpec& spec, const Bytes& data) {
    Bytes body;
    putTag(body, spec.waveTag.c_str());
    if (spec.includeFmt) {
        putTag(body, "fmt ");
        putU32(body, 16);
        putU16(body, spec.formatTag);
        putU16(body, spec.channels);
        putU32(body, spec.sampleRate);
        const std::uint32_t blockAlign = spec.channels * (spec.bitsPerSample / 8u);
        putU32(body, spec.sampleRate * blockAlign);
        putU16(body, blockAlign);
        putU16(body, spec.bitsPerSample);
    }
    body.insert(body.end(), spec.extraChunkBeforeData.begin(), spec.extraChunkBeforeData.end());
    putTag(body, "data");
    putU32(body, spec.dataSizeOverride >= 0 ? static_cast<std::uint32_t>(spec.dataSizeOverride)
                                            : static_cast<std::uint32_t>(data.size()));
    body.insert(body.end(), data.begin(), data.end());

    Bytes file;
    putTag(file, spec.riffTag.c_str());
    putU32(file, spec.riffSizeOverride >= 0 ? static_cast<std::uint32_t>(spec.riffSizeOverride)
                                            : static_cast<std::uint32_t>(body.size()));
    file.insert(file.end(), body.begin(), body.end());
    return file;
}

Bytes canonicalWav(std::uint16_t channels, std::uint16_t bitsPerSample, std::uint32_t sampleRate,
                   const std::vector<std::int32_t>& samples) {
    WavSpec spec;
    spec.channels = channels;
    spec.bitsPerSample = bitsPerSample;
    spec.sampleRate = sampleRate;
    return buildWav(spec, packSamples(samples, bitsPerSample));
}

}  // namespace

TEST(WavParse, Canonical16BitStereo) {
    const std::vector<std::int32_t> samples{1000, -1000, 32767, -32768, 0, 1};
    const WavFile wav = parseWav(canonicalWav(2, 16, 44100, samples));
    EXPECT_EQ(wav.sampleRate, 44100u);
    EXPECT_EQ(wav.channels, 2u);
    EXPECT_EQ(wav.bitsPerSample, 16u);
    EXPECT_EQ(wav.frameCount(), 3u);
    EXPECT_EQ(wav.samples, samples);
}

TEST(WavParse, Mono8BitIsUnsignedInTheFile) {
    WavSpec spec;
    spec.channels = 1;
    spec.bitsPerSample = 8;
    spec.sampleRate = 8000;
    const Bytes file = buildWav(spec, Bytes{0x80, 0x00, 0xFF, 0x7F});
    const WavFile wav = parseWav(file);
    EXPECT_EQ(wav.channels, 1u);
    EXPECT_EQ(wav.bitsPerSample, 8u);
    EXPECT_EQ(wav.frameCount(), 4u);
    EXPECT_EQ(wav.samples, (std::vector<std::int32_t>{0, -128, 127, -1}));
}

TEST(WavParse, TwentyFourBitLittleEndianSignExtends) {
    WavSpec spec;
    spec.channels = 1;
    spec.bitsPerSample = 24;
    spec.sampleRate = 96000;
    const Bytes file = buildWav(spec, Bytes{0x56, 0x34, 0x12, 0xFE, 0xFF, 0xFF, 0x00, 0x00, 0x80, 0xFF, 0xFF, 0x7F});
    const WavFile wav = parseWav(file);
    EXPECT_EQ(wav.bitsPerSample, 24u);
    EXPECT_EQ(wav.sampleRate, 96000u);
    EXPECT_EQ(wav.samples, (std::vector<std::int32_t>{0x123456, -2, -8388608, 8388607}));
}

TEST(WavParse, SkipsUnknownChunkBeforeData) {
    WavSpec spec;
    Bytes list;
    putTag(list, "LIST");
    putU32(list, 4);
    putTag(list, "INFO");
    spec.extraChunkBeforeData = list;
    const std::vector<std::int32_t> samples{7, -7};
    const WavFile wav = parseWav(buildWav(spec, packSamples(samples, 16)));
    EXPECT_EQ(wav.samples, samples);
}

TEST(WavParse, OddSizedChunkIsPaddedToEven) {
    WavSpec spec;
    Bytes junk;
    putTag(junk, "junk");
    putU32(junk, 3);
    junk.push_back(1);
    junk.push_back(2);
    junk.push_back(3);
    junk.push_back(0);
    spec.extraChunkBeforeData = junk;
    const std::vector<std::int32_t> samples{5, 6};
    const WavFile wav = parseWav(buildWav(spec, packSamples(samples, 16)));
    EXPECT_EQ(wav.samples, samples);
}

TEST(WavParse, ToleratesWrongRiffSize) {
    WavSpec spec;
    spec.riffSizeOverride = 0;
    const std::vector<std::int32_t> samples{1, 2, 3, 4};
    const WavFile wav = parseWav(buildWav(spec, packSamples(samples, 16)));
    EXPECT_EQ(wav.samples, samples);
}

TEST(WavParse, TruncatedDataChunkThrows) {
    WavSpec spec;
    spec.dataSizeOverride = 16;
    const Bytes file = buildWav(spec, packSamples({1, 2, 3, 4}, 16));
    EXPECT_THROW(parseWav(file), WavError);
}

TEST(WavParse, DataNotAWholeNumberOfFramesThrows) {
    WavSpec spec;
    const Bytes file = buildWav(spec, Bytes{0x01, 0x00, 0x02, 0x00, 0x03, 0x00});
    EXPECT_THROW(parseWav(file), WavError);
}

TEST(WavParse, RejectsBadTags) {
    WavSpec bad;
    bad.riffTag = "RIFX";
    EXPECT_THROW(parseWav(buildWav(bad, packSamples({1, 2}, 16))), WavError);

    WavSpec bad2;
    bad2.waveTag = "AVI ";
    EXPECT_THROW(parseWav(buildWav(bad2, packSamples({1, 2}, 16))), WavError);
}

TEST(WavParse, RejectsMissingFmt) {
    WavSpec spec;
    spec.includeFmt = false;
    EXPECT_THROW(parseWav(buildWav(spec, packSamples({1, 2}, 16))), WavError);
}

TEST(WavParse, RejectsUnsupportedFormats) {
    WavSpec floatFmt;
    floatFmt.formatTag = 3;
    EXPECT_THROW(parseWav(buildWav(floatFmt, packSamples({1, 2}, 16))), WavError);

    WavSpec twelveBit;
    twelveBit.bitsPerSample = 12;
    EXPECT_THROW(parseWav(buildWav(twelveBit, Bytes{0, 0, 0, 0})), WavError);

    WavSpec threeChannels;
    threeChannels.channels = 3;
    EXPECT_THROW(parseWav(buildWav(threeChannels, packSamples({1, 2, 3}, 16))), WavError);

    WavSpec zeroChannels;
    zeroChannels.channels = 0;
    EXPECT_THROW(parseWav(buildWav(zeroChannels, Bytes{})), WavError);
}

TEST(WavParse, RejectsEmptyAndTinyInput) {
    EXPECT_THROW(parseWav(Bytes{}), WavError);
    EXPECT_THROW(parseWav(Bytes{'R', 'I', 'F', 'F'}), WavError);
}

TEST(WavSerialize, ProducesTheCanonical44ByteHeader) {
    WavFile wav;
    wav.sampleRate = 44100;
    wav.channels = 2;
    wav.bitsPerSample = 16;
    wav.samples = {1, -1};
    const Bytes out = serializeWav(wav);
    ASSERT_EQ(out.size(), 48u);
    EXPECT_EQ(out, canonicalWav(2, 16, 44100, {1, -1}));
}

TEST(WavSerialize, RoundTripIsByteIdenticalForCanonicalFiles) {
    const Bytes stereo16 = canonicalWav(2, 16, 48000, {100, -100, 2000, -2000, 32767, -32768});
    EXPECT_EQ(serializeWav(parseWav(stereo16)), stereo16);

    const Bytes mono8 = canonicalWav(1, 8, 22050, {0, -128, 127, 1, -1});
    EXPECT_EQ(serializeWav(parseWav(mono8)), mono8);

    const Bytes mono24 = canonicalWav(1, 24, 96000, {0x123456, -2, -8388608, 8388607});
    EXPECT_EQ(serializeWav(parseWav(mono24)), mono24);
}

TEST(WavSerialize, ParseAfterSerializeRecoversSamplesAndFormat) {
    WavFile wav;
    wav.sampleRate = 32000;
    wav.channels = 1;
    wav.bitsPerSample = 24;
    wav.samples = {1, 2, -3, 8388607, -8388608};
    const WavFile back = parseWav(serializeWav(wav));
    EXPECT_EQ(back.sampleRate, wav.sampleRate);
    EXPECT_EQ(back.channels, wav.channels);
    EXPECT_EQ(back.bitsPerSample, wav.bitsPerSample);
    EXPECT_EQ(back.samples, wav.samples);
}
