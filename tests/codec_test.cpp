#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "lac/decoder.hpp"
#include "lac/encoder.hpp"

namespace {

lac::WavFile stereo16(std::size_t frames) {
    lac::WavFile wav;
    wav.channels = 2;
    wav.sampleRate = 44100;
    wav.bitsPerSample = 16;
    for (std::size_t i = 0; i < frames; ++i) {
        wav.samples.push_back(static_cast<std::int32_t>(i * 37 % 65536) - 32768);
        wav.samples.push_back(-static_cast<std::int32_t>(i % 1000));
    }
    return wav;
}

}

TEST(Codec, StreamHeaderStartsWithMagic) {
    const std::vector<std::uint8_t> out = lac::encode(stereo16(0));
    ASSERT_GE(out.size(), 4u);
    EXPECT_EQ(out[0], 'L');
    EXPECT_EQ(out[1], 'A');
    EXPECT_EQ(out[2], 'C');
    EXPECT_EQ(out[3], '1');
}

TEST(Codec, RoundTripsAcrossSeveralBlocks) {
    const lac::WavFile wav = stereo16(10000);
    EXPECT_EQ(lac::decode(lac::encode(wav, 4096)), wav);
}
