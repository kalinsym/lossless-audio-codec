#include <gtest/gtest.h>

#include <cstddef>
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

TEST(FixedPredictor, RestoresSquaresFromResiduals) {
    EXPECT_EQ(lac::fixedRestore(Samples{1, 4}, Samples{2, 2, 2, 2}), (Samples{1, 4, 9, 16, 25, 36}));
}

TEST(FixedPredictor, RestoreUndoesResidualForEveryOrder) {
    const Samples x = {8388607, -8388608, 0, 5, -3, 8388607, -8388608, 100, -100, 7};
    for (unsigned order = 0; order <= lac::kMaxFixedOrder; ++order) {
        const Samples warmup(x.begin(), x.begin() + static_cast<std::ptrdiff_t>(order));
        EXPECT_EQ(lac::fixedRestore(warmup, lac::fixedResidual(x, order)), x) << "order " << order;
    }
}

TEST(FixedPredictor, RestoreRejectsTooManyWarmUpSamples) {
    EXPECT_THROW(lac::fixedRestore(Samples{1, 2, 3, 4, 5}, Samples{}), std::invalid_argument);
}
