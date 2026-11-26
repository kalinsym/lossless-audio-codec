#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

#include "lac/decoder.hpp"
#include "lac/encoder.hpp"

namespace {

using Bytes = std::vector<std::uint8_t>;

bool rejected(const Bytes& bytes) {
    try {
        lac::decode(bytes);
        return false;
    } catch (const lac::FormatError&) {
        return true;
    }
}

Bytes tonalStereo() {
    lac::WavFile wav;
    wav.channels = 2;
    wav.sampleRate = 44100;
    wav.bitsPerSample = 16;
    for (int i = 0; i < 3000; ++i) {
        const double t = static_cast<double>(i);
        const auto common = static_cast<std::int32_t>(9000 * std::sin(t / 9) + 4000 * std::sin(t / 4));
        wav.samples.push_back(common);
        wav.samples.push_back(common / 2 + static_cast<std::int32_t>(3000 * std::sin(t / 5)));
    }
    return lac::encode(wav, 1024);
}

}

TEST(Hardening, DecoderRejectsRandomBytes) {
    std::mt19937 rng(5);
    for (int i = 0; i < 2000; ++i) {
        Bytes junk(rng() % 200);
        for (std::uint8_t& b : junk) {
            b = static_cast<std::uint8_t>(rng());
        }
        if (i % 2 == 0 && junk.size() >= 4) {
            junk[0] = 'L';
            junk[1] = 'A';
            junk[2] = 'C';
            junk[3] = '1';
        }
        EXPECT_TRUE(rejected(junk)) << "case " << i;
    }
}

TEST(Hardening, DecoderSurvivesDamagedFiles) {
    const Bytes good = tonalStereo();
    std::mt19937 rng(6);
    int rejections = 0;
    const int trials = 3000;
    for (int i = 0; i < trials; ++i) {
        Bytes bad = good;
        if (i % 3 == 0) {
            for (std::uint32_t n = 1 + rng() % 8; n > 0; --n) {
                bad[rng() % bad.size()] ^= static_cast<std::uint8_t>(1u << (rng() % 8));
            }
        } else if (i % 3 == 1) {
            bad.resize(rng() % good.size());
        } else {
            const std::size_t start = rng() % bad.size();
            for (std::size_t j = start; j < bad.size() && j < start + 1 + rng() % 16; ++j) {
                bad[j] = static_cast<std::uint8_t>(rng());
            }
        }
        rejections += rejected(bad) ? 1 : 0;
    }
    EXPECT_GE(rejections, trials * 99 / 100);
}
