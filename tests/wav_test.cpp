#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "lac/wav.hpp"

using Bytes = std::vector<std::uint8_t>;

namespace {

Bytes minimalMono16() {
    return {
        'R', 'I', 'F', 'F', 0x28, 0x00, 0x00, 0x00,
        'W', 'A', 'V', 'E',
        'f', 'm', 't', ' ', 0x10, 0x00, 0x00, 0x00,
        0x01, 0x00,
        0x01, 0x00,
        0x44, 0xAC, 0x00, 0x00,
        0x88, 0x58, 0x01, 0x00,
        0x02, 0x00,
        0x10, 0x00,
        'd', 'a', 't', 'a', 0x04, 0x00, 0x00, 0x00,
        0xE8, 0x03,
        0x00, 0x80,
    };
}

}

TEST(Wav, ParsesMinimalMono16Bit) {
    const Bytes file = minimalMono16();
    const lac::WavFile audio = lac::parseWav(file);
    EXPECT_EQ(audio.channels, 1u);
    EXPECT_EQ(audio.sampleRate, 44100u);
    EXPECT_EQ(audio.bitsPerSample, 16u);
    EXPECT_EQ(audio.frameCount(), 2u);
    EXPECT_EQ(audio.samples, (std::vector<std::int32_t>{1000, -32768}));
}

TEST(Wav, RejectsFileWithoutRiffTag) {
    Bytes file = minimalMono16();
    file[0] = 'X';
    EXPECT_THROW(lac::parseWav(file), lac::WavError);
}

TEST(Wav, Parses8BitAsUnsigned) {
    Bytes file = minimalMono16();
    file[32] = 0x01;
    file[34] = 0x08;
    file[44] = 0x80;
    file[45] = 0x00;
    file[46] = 0xFF;
    file[47] = 0x7F;
    const lac::WavFile audio = lac::parseWav(file);
    EXPECT_EQ(audio.bitsPerSample, 8u);
    EXPECT_EQ(audio.samples, (std::vector<std::int32_t>{0, -128, 127, -1}));
}

TEST(Wav, Parses24BitWithSignExtension) {
    Bytes file = minimalMono16();
    file[32] = 0x03;
    file[34] = 0x18;
    file[40] = 0x06;
    file.resize(44);
    const Bytes data{0x56, 0x34, 0x12, 0xFE, 0xFF, 0xFF};
    file.insert(file.end(), data.begin(), data.end());
    file[4] = 0x2A;
    const lac::WavFile audio = lac::parseWav(file);
    EXPECT_EQ(audio.bitsPerSample, 24u);
    EXPECT_EQ(audio.samples, (std::vector<std::int32_t>{0x123456, -2}));
}

TEST(Wav, SkipsOddSizedUnknownChunk) {
    Bytes file = minimalMono16();
    const Bytes junk{'j', 'u', 'n', 'k', 0x03, 0x00, 0x00, 0x00, 1, 2, 3, 0};
    file.insert(file.begin() + 36, junk.begin(), junk.end());
    const lac::WavFile audio = lac::parseWav(file);
    EXPECT_EQ(audio.samples, (std::vector<std::int32_t>{1000, -32768}));
}

TEST(Wav, RejectsThreeChannels) {
    Bytes file = minimalMono16();
    file[22] = 0x03;
    EXPECT_THROW(lac::parseWav(file), lac::WavError);
}
