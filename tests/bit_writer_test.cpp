#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "lac/bit_writer.hpp"

using Bytes = std::vector<std::uint8_t>;

TEST(BitWriter, PacksMsbFirst) {
    lac::BitWriter w;
    w.writeBits(5, 3);
    w.writeBits(1, 1);
    w.writeBits(9, 6);
    w.flush();
    EXPECT_EQ(w.bytes(), (Bytes{0xB2, 0x40}));
}

TEST(BitWriter, ZeroWidthWritesNothing) {
    lac::BitWriter w;
    w.writeBits(0, 0);
    w.writeBits(0xFFFFFFFFu, 0);
    w.flush();
    EXPECT_EQ(w.bitCount(), 0u);
    EXPECT_TRUE(w.bytes().empty());
}

TEST(BitWriter, FullWidthSurvives) {
    lac::BitWriter w;
    w.writeBits(1, 1);
    w.writeBits(0xFFFFFFFFu, 32);
    w.writeBits(0, 7);
    w.flush();
    EXPECT_EQ(w.bytes(), (Bytes{0xFF, 0xFF, 0xFF, 0xFF, 0x80}));
    EXPECT_EQ(w.bitCount(), 40u);
}

TEST(BitWriter, FlushTwice) {
    lac::BitWriter w;
    w.writeBits(1, 1);
    w.flush();
    w.flush();
    EXPECT_EQ(w.bytes(), (Bytes{0x80}));
    EXPECT_EQ(w.bitCount(), 1u);
}
