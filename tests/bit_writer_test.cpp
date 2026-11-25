#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "lac/bit_writer.hpp"

TEST(BitWriter, AppendKeepsBitsBackToBack) {
    lac::BitWriter part;
    part.writeBits(0b1, 1);
    part.writeBits(0b001001, 6);
    lac::BitWriter whole;
    whole.writeBits(0b101, 3);
    whole.append(part);
    EXPECT_EQ(whole.bitCount(), 10u);
    whole.flush();
    EXPECT_EQ(whole.bytes(), (std::vector<std::uint8_t>{0xB2, 0x40}));
}

TEST(BitWriter, AppendCopiesWholeBytes) {
    lac::BitWriter part;
    part.writeBits(0xABCD, 16);
    lac::BitWriter whole;
    whole.append(part);
    EXPECT_EQ(whole.bytes(), (std::vector<std::uint8_t>{0xAB, 0xCD}));
}

TEST(BitWriter, WritesHighBitsFirst) {
    lac::BitWriter w;
    w.writeBits(0b1, 1);
    w.writeBits(0b0110, 4);
    EXPECT_EQ(w.bitCount(), 5u);
    w.flush();
    EXPECT_EQ(w.bytes(), (std::vector<std::uint8_t>{0xB0}));
}

TEST(BitWriter, ZeroWidthAndEmptyFlushAddNothing) {
    lac::BitWriter w;
    w.writeBits(123, 0);
    w.flush();
    EXPECT_EQ(w.bitCount(), 0u);
    EXPECT_TRUE(w.bytes().empty());
}

TEST(BitWriter, ThirtyTwoBitValueAcrossByteBoundaries) {
    lac::BitWriter w;
    w.writeBits(0b1, 1);
    w.writeBits(0xDEADBEEF, 32);
    w.flush();
    EXPECT_EQ(w.bytes(), (std::vector<std::uint8_t>{0xEF, 0x56, 0xDF, 0x77, 0x80}));
}

TEST(BitWriter, UnaryCodes) {
    lac::BitWriter w;
    w.writeUnary(0);
    w.writeUnary(3);
    w.flush();
    EXPECT_EQ(w.bytes(), (std::vector<std::uint8_t>{0x88}));
    lac::BitWriter longRun;
    longRun.writeUnary(40);
    EXPECT_EQ(longRun.bitCount(), 41u);
}

TEST(BitWriter, SignedValuesInTwosComplement) {
    lac::BitWriter w;
    w.writeSigned(-1, 4);
    w.writeSigned(5, 4);
    w.writeSigned(-8388608, 24);
    EXPECT_EQ(w.bytes(), (std::vector<std::uint8_t>{0xF5, 0x80, 0x00, 0x00}));
}

TEST(BitWriter, SecondFlushAddsNoPadding) {
    lac::BitWriter w;
    w.writeBits(1, 1);
    w.flush();
    w.flush();
    w.writeBits(0xFF, 8);
    EXPECT_EQ(w.bytes(), (std::vector<std::uint8_t>{0x80, 0xFF}));
}
