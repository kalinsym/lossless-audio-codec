#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "lac/predictor.hpp"

using Samples = std::vector<std::int32_t>;

TEST(FixedPredictor, OrderZeroKeepsTheSamples) {
    const Samples squares = {1, 4, 9, 16, 25, 36};
    EXPECT_EQ(lac::fixedResidual(squares, 0), squares);
}

TEST(FixedPredictor, ResidualsOfSquares) {
    const Samples squares = {1, 4, 9, 16, 25, 36};
    EXPECT_EQ(lac::fixedResidual(squares, 1), (Samples{3, 5, 7, 9, 11}));
    EXPECT_EQ(lac::fixedResidual(squares, 2), (Samples{2, 2, 2, 2}));
    EXPECT_EQ(lac::fixedResidual(squares, 3), (Samples{0, 0, 0}));
    EXPECT_EQ(lac::fixedResidual(squares, 4), (Samples{0, 0}));
}

TEST(FixedPredictor, ShortInputHasNoResiduals) {
    EXPECT_TRUE(lac::fixedResidual(Samples{7, 8}, 4).empty());
}

TEST(FixedPredictor, ExtremeTwentyFourBitSamples) {
    const Samples x = {8388607, -8388608, 8388607, -8388608, 8388607};
    EXPECT_EQ(lac::fixedResidual(x, 4), (Samples{134217720}));
}

TEST(FixedPredictor, RejectsOrderAboveFour) {
    EXPECT_THROW(lac::fixedResidual(Samples{1, 2, 3, 4, 5, 6}, 5), std::invalid_argument);
}
