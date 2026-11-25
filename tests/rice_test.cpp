#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

#include "lac/rice.hpp"

using Limits = std::numeric_limits<std::int32_t>;

TEST(ZigZag, MapsSmallValuesToSmallCodes) {
    EXPECT_EQ(lac::zigzag(0), 0u);
    EXPECT_EQ(lac::zigzag(-1), 1u);
    EXPECT_EQ(lac::zigzag(1), 2u);
    EXPECT_EQ(lac::zigzag(-2), 3u);
    EXPECT_EQ(lac::zigzag(2), 4u);
}

TEST(ZigZag, HandlesTheInt32Limits) {
    EXPECT_EQ(lac::zigzag(Limits::max()), 0xFFFFFFFEu);
    EXPECT_EQ(lac::zigzag(Limits::min()), 0xFFFFFFFFu);
}

TEST(ZigZag, UnzigzagUndoesZigzag) {
    for (const std::int32_t v : {0, 1, -1, 2, -2, 134217720, -134217720, Limits::max(), Limits::min()}) {
        EXPECT_EQ(lac::unzigzag(lac::zigzag(v)), v);
    }
}

TEST(Rice, WritesQuotientInUnaryAndRemainderInKBits) {
    lac::BitWriter w;
    lac::writeRice(w, 3, 2);
    lac::writeRice(w, -1, 0);
    w.flush();
    EXPECT_EQ(w.bytes(), (std::vector<std::uint8_t>{0x64}));
}

TEST(Rice, ReadsBackWhatWasWritten) {
    const std::vector<std::int32_t> values = {0, 1, -1, 7, -8, 100, -100, 1000, -1000};
    for (const unsigned k : {0u, 1u, 4u, 9u, 30u}) {
        lac::BitWriter w;
        for (const std::int32_t v : values) {
            lac::writeRice(w, v, k);
        }
        w.flush();
        lac::BitReader r(w.bytes());
        for (const std::int32_t v : values) {
            EXPECT_EQ(lac::readRice(r, k), v) << "k " << k;
        }
    }
}

TEST(Rice, RejectsParameterAboveThirty) {
    lac::BitWriter w;
    EXPECT_THROW(lac::writeRice(w, 0, 31), std::invalid_argument);
}

TEST(Rice, CountsBitsPerParameter) {
    const std::vector<std::int32_t> residual = {3, -1};
    EXPECT_EQ(lac::riceBits(residual, 0), 9u);
    EXPECT_EQ(lac::riceBits(residual, 2), 7u);
}

TEST(Rice, BestParameterGrowsWithTheResiduals) {
    EXPECT_EQ(lac::bestRiceParameter(std::vector<std::int32_t>{}), 0u);
    EXPECT_EQ(lac::bestRiceParameter(std::vector<std::int32_t>{0, 0, 1, -1, 0}), 0u);
    EXPECT_EQ(lac::bestRiceParameter(std::vector<std::int32_t>{1000, -1000, 999, -999}), 10u);
}

TEST(Rice, FastParameterChoiceMatchesExhaustiveSearch) {
    std::mt19937 rng(3);
    for (int scale = 0; scale < 24; ++scale) {
        std::exponential_distribution<double> laplace(1.0 / std::ldexp(1.0, scale));
        std::vector<std::int32_t> residual;
        for (int i = 0; i < 1000; ++i) {
            const auto magnitude = static_cast<std::int32_t>(laplace(rng));
            residual.push_back(i % 2 == 0 ? magnitude : -magnitude);
        }
        std::uint64_t smallest = lac::riceBits(residual, 0);
        for (unsigned k = 1; k <= lac::kMaxRiceParameter; ++k) {
            smallest = std::min(smallest, lac::riceBits(residual, k));
        }
        EXPECT_EQ(lac::riceBits(residual, lac::bestRiceParameter(residual)), smallest) << "scale " << scale;
    }
}
