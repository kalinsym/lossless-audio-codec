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
