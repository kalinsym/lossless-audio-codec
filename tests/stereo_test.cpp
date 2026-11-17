#include <gtest/gtest.h>

#include <cstdint>

#include "lac/stereo.hpp"

TEST(MidSide, WorkedExample) {
    EXPECT_EQ(lac::sideOf(7, 4), 3);
    EXPECT_EQ(lac::midOf(7, 4), 5);
    EXPECT_EQ(lac::leftOf(5, 3), 7);
}

TEST(MidSide, RestoresEveryPair) {
    for (const std::int32_t left : {-8388608, -5, -2, -1, 0, 1, 2, 7, 8388607}) {
        for (const std::int32_t right : {-8388608, -5, -2, -1, 0, 1, 2, 7, 8388607}) {
            const std::int32_t side = lac::sideOf(left, right);
            const std::int32_t restored = lac::leftOf(lac::midOf(left, right), side);
            EXPECT_EQ(restored, left) << left << " " << right;
            EXPECT_EQ(restored - side, right) << left << " " << right;
        }
    }
}
