#include <gtest/gtest.h>

#include <cstdint>
#include <limits>

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
