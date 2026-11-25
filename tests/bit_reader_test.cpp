#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "lac/bit_reader.hpp"

using Bytes = std::vector<std::uint8_t>;

TEST(BitReader, ReadsHighBitsFirstAndPadding) {
    const Bytes data = {0xB0};
    lac::BitReader r(data);
    EXPECT_EQ(r.readBits(1), 1u);
    EXPECT_EQ(r.readBits(4), 0b0110u);
    EXPECT_FALSE(r.atEnd());
    r.alignToByte();
    EXPECT_TRUE(r.atEnd());
}

TEST(BitReader, ZeroWidthReadConsumesNothing) {
    const Bytes data = {0xFF};
    lac::BitReader r(data);
    EXPECT_EQ(r.readBits(0), 0u);
    EXPECT_EQ(r.bytePosition(), 0u);
    EXPECT_EQ(r.readBits(8), 0xFFu);
}

TEST(BitReader, ThirtyTwoBitValueAcrossByteBoundaries) {
    const Bytes data = {0xEF, 0x56, 0xDF, 0x77, 0x80};
    lac::BitReader r(data);
    EXPECT_EQ(r.readBits(1), 1u);
    EXPECT_EQ(r.readBits(32), 0xDEADBEEFu);
}

TEST(BitReader, ReadsUnaryAndSignedValues) {
    const Bytes unary = {0x88};
    lac::BitReader u(unary);
    EXPECT_EQ(u.readUnary(), 0u);
    EXPECT_EQ(u.readUnary(), 3u);
    const Bytes signedBytes = {0xF5, 0x80, 0x00, 0x00};
    lac::BitReader s(signedBytes);
    EXPECT_EQ(s.readSigned(4), -1);
    EXPECT_EQ(s.readSigned(4), 5);
    EXPECT_EQ(s.readSigned(24), -8388608);
}

TEST(BitReader, AlignSkipsToTheNextByte) {
    const Bytes data = {0x80, 0xFF};
    lac::BitReader r(data);
    EXPECT_EQ(r.readBits(1), 1u);
    EXPECT_EQ(r.bytePosition(), 1u);
    r.alignToByte();
    EXPECT_EQ(r.readBits(8), 0xFFu);
    EXPECT_TRUE(r.atEnd());
}

TEST(BitReader, ThrowsAtTheEndOfTheData) {
    const Bytes data = {0xA0};
    lac::BitReader r(data);
    EXPECT_EQ(r.readBits(4), 0xAu);
    EXPECT_THROW(r.readBits(5), std::out_of_range);
    const Bytes allZeros = {0x00};
    lac::BitReader zeros(allZeros);
    EXPECT_THROW(zeros.readUnary(), std::out_of_range);
}
