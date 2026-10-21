#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "lac/bit_reader.hpp"

using Bytes = std::vector<std::uint8_t>;

TEST(BitReader, ReadsBackTheWorkedExample) {
    const Bytes data{0xB2, 0x40};
    lac::BitReader r(data);
    EXPECT_EQ(r.readBits(3), 5u);
    EXPECT_EQ(r.readBits(1), 1u);
    EXPECT_EQ(r.readBits(6), 9u);
    EXPECT_FALSE(r.atEnd());
    EXPECT_EQ(r.readBits(6), 0u);
    EXPECT_TRUE(r.atEnd());
}

TEST(BitReader, ReadsFullWidthAtAnOddOffset) {
    const Bytes data{0xFF, 0xFF, 0xFF, 0xFF, 0x80};
    lac::BitReader r(data);
    EXPECT_EQ(r.readBits(1), 1u);
    EXPECT_EQ(r.readBits(32), 0xFFFFFFFFu);
    EXPECT_EQ(r.readBits(7), 0u);
    EXPECT_TRUE(r.atEnd());
}

TEST(BitReader, ThrowsInsteadOfReadingPastTheEnd) {
    const Bytes data{0xAB};
    lac::BitReader r(data);
    EXPECT_EQ(r.readBits(8), 0xABu);
    EXPECT_TRUE(r.atEnd());
    EXPECT_THROW(r.readBits(1), std::out_of_range);

    lac::BitReader r2(data);
    EXPECT_THROW(r2.readBits(9), std::out_of_range);
    EXPECT_EQ(r2.readBits(8), 0xABu);
}

TEST(BitReader, ReadsUnaryCodes) {
    const Bytes data{0x89};
    lac::BitReader r(data);
    EXPECT_EQ(r.readUnary(), 0u);
    EXPECT_EQ(r.readUnary(), 3u);
    EXPECT_EQ(r.readUnary(), 2u);
    EXPECT_TRUE(r.atEnd());

    const Bytes zeros{0x00, 0x00};
    lac::BitReader r2(zeros);
    EXPECT_THROW(r2.readUnary(), std::out_of_range);
}

TEST(BitReader, ReadsSignedValues) {
    const Bytes data{0xF5, 0x80, 0xA0};
    lac::BitReader r(data);
    EXPECT_EQ(r.readSigned(4), -1);
    EXPECT_EQ(r.readSigned(4), 5);
    EXPECT_EQ(r.readSigned(8), -128);
    EXPECT_EQ(r.readSigned(3), -3);
    EXPECT_EQ(r.readSigned(0), 0);

    const Bytes extremes{0x80, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xFF};
    lac::BitReader r2(extremes);
    EXPECT_EQ(r2.readSigned(32), INT32_MIN);
    EXPECT_EQ(r2.readSigned(32), INT32_MAX);
}

TEST(BitReader, AlignToByteSkipsPadding) {
    const Bytes data{0xB2, 0x40, 0xFF};
    lac::BitReader r(data);
    EXPECT_EQ(r.readBits(3), 5u);
    r.alignToByte();
    EXPECT_EQ(r.readBits(8), 0x40u);
    r.alignToByte();
    EXPECT_EQ(r.readBits(8), 0xFFu);
    EXPECT_TRUE(r.atEnd());
}

TEST(BitReader, BytePositionCountsFetchedBytes) {
    const Bytes data{0xB2, 0x40, 0xFF};
    lac::BitReader r(data);
    EXPECT_EQ(r.bytePosition(), 0u);
    r.readBits(3);
    EXPECT_EQ(r.bytePosition(), 1u);
    r.alignToByte();
    EXPECT_EQ(r.bytePosition(), 1u);
    r.readBits(8);
    r.readBits(8);
    EXPECT_EQ(r.bytePosition(), 3u);
}
