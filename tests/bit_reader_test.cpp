#include <gtest/gtest.h>

#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

#include "lac/bit_reader.hpp"
#include "lac/bit_writer.hpp"

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

TEST(BitIo, RandomMixedStreamRoundTrips) {
    struct Item {
        int kind;
        unsigned width;
        std::int64_t value;
    };
    std::mt19937 rng(11);
    std::vector<Item> items;
    lac::BitWriter w;
    for (int i = 0; i < 3000; ++i) {
        const auto kind = static_cast<int>(rng() % 3);
        const unsigned width = 1 + static_cast<unsigned>(rng() % 32);
        const auto bits = static_cast<std::uint32_t>(rng() >> (32 - width));
        if (kind == 0) {
            w.writeBits(bits, width);
            items.push_back({kind, width, bits});
        } else if (kind == 1) {
            const std::int64_t half = std::int64_t{1} << (width - 1);
            const std::int64_t value = bits >= half ? bits - 2 * half : std::int64_t{bits};
            w.writeSigned(static_cast<std::int32_t>(value), width);
            items.push_back({kind, width, value});
        } else {
            w.writeUnary(bits % 100);
            items.push_back({kind, 0, bits % 100});
        }
    }
    w.flush();
    lac::BitReader r(w.bytes());
    for (const Item& item : items) {
        std::int64_t read = 0;
        if (item.kind == 0) {
            read = r.readBits(item.width);
        } else if (item.kind == 1) {
            read = r.readSigned(item.width);
        } else {
            read = r.readUnary();
        }
        ASSERT_EQ(read, item.value);
    }
}
