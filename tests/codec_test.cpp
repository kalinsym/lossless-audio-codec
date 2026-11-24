#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <random>
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

lac::WavFile mono16(std::size_t frames) {
    lac::WavFile wav;
    wav.channels = 1;
    wav.sampleRate = 44100;
    wav.bitsPerSample = 16;
    wav.samples.resize(frames);
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

TEST(Codec, RoundTripsEveryFormat) {
    for (unsigned bits : {8u, 16u, 24u}) {
        lac::WavFile wav;
        wav.channels = 1;
        wav.sampleRate = 8000;
        wav.bitsPerSample = static_cast<std::uint16_t>(bits);
        const std::int32_t lim = (1 << (bits - 1)) - 1;
        wav.samples = {0, lim, -lim - 1, 1, -1};
        EXPECT_EQ(lac::decode(lac::encode(wav, 2)), wav);
    }
}

TEST(Codec, DetectsEverySingleBitFlip) {
    const std::vector<std::uint8_t> good = lac::encode(stereo16(50), 16);
    for (std::size_t i = 0; i < good.size() * 8; ++i) {
        std::vector<std::uint8_t> bad = good;
        bad[i / 8] ^= static_cast<std::uint8_t>(1u << (i % 8));
        EXPECT_THROW(lac::decode(bad), lac::FormatError) << "bit " << i;
    }
}

TEST(Codec, RejectsTruncatedFile) {
    std::vector<std::uint8_t> out = lac::encode(stereo16(100), 32);
    out.pop_back();
    EXPECT_THROW(lac::decode(out), lac::FormatError);
}

TEST(Codec, CompressesSmoothAudio) {
    lac::WavFile wav = mono16(44100);
    for (std::size_t i = 0; i < wav.samples.size(); ++i) {
        wav.samples[i] = static_cast<std::int32_t>(10000 * std::sin(static_cast<double>(i) / 20));
    }
    const std::vector<std::uint8_t> out = lac::encode(wav);
    EXPECT_LT(out.size(), wav.samples.size() * 2 / 4);
    EXPECT_EQ(lac::decode(out), wav);
}

TEST(Codec, NoiseFallsBackToVerbatim) {
    lac::WavFile wav = mono16(44100);
    std::mt19937 rng(42);
    for (std::int32_t& sample : wav.samples) {
        sample = static_cast<std::int32_t>(rng() % 65536) - 32768;
    }
    const std::vector<std::uint8_t> out = lac::encode(wav);
    EXPECT_LE(out.size(), wav.samples.size() * 2 + 200);
    EXPECT_EQ(lac::decode(out), wav);
}

TEST(Codec, CorrelatedStereoUsesMidSide) {
    lac::WavFile wav = stereo16(0);
    std::mt19937 rng(7);
    for (std::size_t i = 0; i < 4096; ++i) {
        const auto common = static_cast<std::int32_t>(10000 * std::sin(static_cast<double>(i) / 30));
        wav.samples.push_back(common + static_cast<std::int32_t>(rng() % 64));
        wav.samples.push_back(common + static_cast<std::int32_t>(rng() % 64));
    }
    const std::vector<std::uint8_t> out = lac::encode(wav);
    EXPECT_EQ(out[17 + 8], lac::kChannelMidSide);
    EXPECT_EQ(lac::decode(out), wav);
}

TEST(Codec, StereoSideFitsAtTwentyFourBitExtremes) {
    lac::WavFile wav = stereo16(0);
    wav.bitsPerSample = 24;
    for (int i = 0; i < 100; ++i) {
        wav.samples.push_back(i % 2 == 0 ? 8388607 : -8388608);
        wav.samples.push_back(i % 2 == 0 ? -8388608 : 8388607);
    }
    const std::vector<std::uint8_t> out = lac::encode(wav, 16);
    EXPECT_EQ(out[17 + 8], lac::kChannelMidSide);
    EXPECT_EQ(lac::decode(out), wav);
}

TEST(Codec, TonalAudioUsesLpc) {
    lac::WavFile wav = mono16(4096);
    for (std::size_t i = 0; i < wav.samples.size(); ++i) {
        const double t = static_cast<double>(i);
        wav.samples[i] = static_cast<std::int32_t>(6000 * std::sin(t / 7) + 5000 * std::sin(t / 3) + 4000 * std::sin(t / 2));
    }
    const std::vector<std::uint8_t> out = lac::encode(wav);
    EXPECT_EQ(out[17 + 10], lac::kSubframeLpc);
    EXPECT_EQ(lac::decode(out), wav);
}
