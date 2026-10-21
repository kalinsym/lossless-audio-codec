#include <gtest/gtest.h>

#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

#include "lac/bit_reader.hpp"
#include "lac/bit_writer.hpp"

using lac::BitReader;
using lac::BitWriter;
using Bytes = std::vector<std::uint8_t>;

TEST(BitWriter, PacksMsbFirstAndPadsWithZeros) {
    BitWriter w;
    w.writeBits(5, 3);
    w.writeBits(1, 1);
    w.writeBits(9, 6);
    EXPECT_EQ(w.bitCount(), 10u);
    w.flush();
    EXPECT_EQ(w.bytes(), (Bytes{0xB2, 0x40}));
}

TEST(BitWriter, ZeroWidthWritesNothing) {
    BitWriter w;
    w.writeBits(0, 0);
    w.writeBits(0xFFFFFFFFu, 0);
    EXPECT_EQ(w.bitCount(), 0u);
    w.flush();
    EXPECT_TRUE(w.bytes().empty());
}

TEST(BitWriter, FullWidthValuesSurvive) {
    BitWriter w;
    w.writeBits(0xFFFFFFFFu, 32);
    w.writeBits(0x80000001u, 32);
    w.flush();
    EXPECT_EQ(w.bytes(), (Bytes{0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x01}));
}

TEST(BitWriter, FullWidthAfterOddOffset) {
    BitWriter w;
    w.writeBits(1, 1);
    w.writeBits(0xFFFFFFFFu, 32);
    w.writeBits(0, 7);
    w.flush();
    EXPECT_EQ(w.bytes(), (Bytes{0xFF, 0xFF, 0xFF, 0xFF, 0x80}));
}

TEST(BitWriter, UnaryIsZerosThenOne) {
    BitWriter w;
    w.writeUnary(0);
    w.writeUnary(3);
    w.writeUnary(2);
    w.flush();
    EXPECT_EQ(w.bytes(), (Bytes{0x89}));
}

TEST(BitWriter, LongUnaryRun) {
    BitWriter w;
    w.writeUnary(100);
    w.flush();
    ASSERT_EQ(w.bytes().size(), 13u);
    for (std::size_t i = 0; i < 12; ++i) {
        EXPECT_EQ(w.bytes()[i], 0x00) << "byte " << i;
    }
    EXPECT_EQ(w.bytes()[12], 0x08);
}

TEST(BitWriter, SignedIsTwosComplement) {
    BitWriter w;
    w.writeSigned(-1, 4);
    w.writeSigned(5, 4);
    w.writeSigned(-128, 8);
    w.writeSigned(-3, 3);
    w.writeBits(0, 5);
    w.flush();
    EXPECT_EQ(w.bytes(), (Bytes{0xF5, 0x80, 0xA0}));
}

TEST(BitWriter, SignedExtremesAtEveryWidth) {
    for (unsigned width = 1; width <= 32; ++width) {
        const std::int64_t lo = -(std::int64_t{1} << (width - 1));
        const std::int64_t hi = (std::int64_t{1} << (width - 1)) - 1;
        BitWriter w;
        w.writeSigned(static_cast<std::int32_t>(lo), width);
        w.writeSigned(static_cast<std::int32_t>(hi), width);
        w.writeSigned(0, width);
        w.flush();
        BitReader r(w.bytes());
        EXPECT_EQ(r.readSigned(width), static_cast<std::int32_t>(lo)) << "width " << width;
        EXPECT_EQ(r.readSigned(width), static_cast<std::int32_t>(hi)) << "width " << width;
        EXPECT_EQ(r.readSigned(width), 0) << "width " << width;
    }
}

TEST(BitWriter, FlushIsIdempotent) {
    BitWriter w;
    w.writeBits(1, 1);
    w.flush();
    w.flush();
    EXPECT_EQ(w.bytes(), (Bytes{0x80}));
    EXPECT_EQ(w.bitCount(), 1u);
}

TEST(BitWriter, WritingAfterFlushStartsANewByte) {
    BitWriter w;
    w.writeBits(1, 1);
    w.flush();
    w.writeBits(1, 1);
    w.flush();
    EXPECT_EQ(w.bytes(), (Bytes{0x80, 0x80}));
}

TEST(BitReader, ReadsBackTheWorkedExample) {
    const Bytes data{0xB2, 0x40};
    BitReader r(data);
    EXPECT_EQ(r.readBits(3), 5u);
    EXPECT_EQ(r.readBits(1), 1u);
    EXPECT_EQ(r.readBits(6), 9u);
    EXPECT_EQ(r.readBits(6), 0u);
    EXPECT_TRUE(r.atEnd());
}

TEST(BitReader, ZeroWidthReadReturnsZeroAndConsumesNothing) {
    const Bytes data{0xFF};
    BitReader r(data);
    EXPECT_EQ(r.readBits(0), 0u);
    EXPECT_EQ(r.readBits(8), 0xFFu);
    EXPECT_TRUE(r.atEnd());
}

TEST(BitReader, ReadsFullWidthValues) {
    const Bytes data{0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x01};
    BitReader r(data);
    EXPECT_EQ(r.readBits(32), 0xFFFFFFFFu);
    EXPECT_EQ(r.readBits(32), 0x80000001u);
    EXPECT_TRUE(r.atEnd());
}

TEST(BitReader, ReadsUnary) {
    const Bytes data{0x89};
    BitReader r(data);
    EXPECT_EQ(r.readUnary(), 0u);
    EXPECT_EQ(r.readUnary(), 3u);
    EXPECT_EQ(r.readUnary(), 2u);
    EXPECT_TRUE(r.atEnd());
}

TEST(BitReader, ReadsSigned) {
    const Bytes data{0xF5, 0x80, 0xA0};
    BitReader r(data);
    EXPECT_EQ(r.readSigned(4), -1);
    EXPECT_EQ(r.readSigned(4), 5);
    EXPECT_EQ(r.readSigned(8), -128);
    EXPECT_EQ(r.readSigned(3), -3);
}

TEST(BitReader, AlignToByteSkipsPadding) {
    const Bytes data{0xB2, 0x40, 0xFF};
    BitReader r(data);
    EXPECT_EQ(r.readBits(3), 5u);
    r.alignToByte();
    EXPECT_EQ(r.bytePosition(), 1u);
    EXPECT_EQ(r.readBits(8), 0x40u);
    EXPECT_EQ(r.bytePosition(), 2u);
    r.alignToByte();
    EXPECT_EQ(r.bytePosition(), 2u);
    EXPECT_EQ(r.readBits(8), 0xFFu);
    EXPECT_TRUE(r.atEnd());
}

TEST(BitReader, ThrowsInsteadOfReadingPastTheEnd) {
    const Bytes data{0xAB};
    BitReader r(data);
    EXPECT_EQ(r.readBits(8), 0xABu);
    EXPECT_THROW(r.readBits(1), std::out_of_range);

    BitReader r2(data);
    EXPECT_THROW(r2.readBits(9), std::out_of_range);

    const Bytes zeros{0x00, 0x00};
    BitReader r3(zeros);
    EXPECT_THROW(r3.readUnary(), std::out_of_range);

    const Bytes empty;
    BitReader r4(empty);
    EXPECT_TRUE(r4.atEnd());
    EXPECT_THROW(r4.readBits(1), std::out_of_range);
}

TEST(BitIo, RandomUnsignedRoundTrip) {
    std::mt19937 rng(20260929u);
    std::uniform_int_distribution<unsigned> widthDist(0, 32);
    std::vector<std::pair<std::uint32_t, unsigned>> pairs;
    BitWriter w;
    for (int i = 0; i < 10000; ++i) {
        const unsigned width = widthDist(rng);
        const std::uint32_t mask = width == 32 ? 0xFFFFFFFFu : ((std::uint32_t{1} << width) - 1u);
        const std::uint32_t value = static_cast<std::uint32_t>(rng()) & mask;
        pairs.emplace_back(value, width);
        w.writeBits(value, width);
    }
    w.flush();
    BitReader r(w.bytes());
    for (std::size_t i = 0; i < pairs.size(); ++i) {
        ASSERT_EQ(r.readBits(pairs[i].second), pairs[i].first) << "pair " << i;
    }
}

TEST(BitIo, RandomSignedRoundTrip) {
    std::mt19937 rng(20261002u);
    std::uniform_int_distribution<unsigned> widthDist(1, 32);
    std::vector<std::pair<std::int32_t, unsigned>> pairs;
    BitWriter w;
    for (int i = 0; i < 10000; ++i) {
        const unsigned width = widthDist(rng);
        const std::uint32_t raw = static_cast<std::uint32_t>(rng());
        const unsigned shift = 32 - width;
        const std::int32_t value = static_cast<std::int32_t>(raw << shift) >> shift;
        pairs.emplace_back(value, width);
        w.writeSigned(value, width);
    }
    w.flush();
    BitReader r(w.bytes());
    for (std::size_t i = 0; i < pairs.size(); ++i) {
        ASSERT_EQ(r.readSigned(pairs[i].second), pairs[i].first) << "pair " << i;
    }
}

TEST(BitIo, RandomUnaryRoundTrip) {
    std::mt19937 rng(20261004u);
    std::uniform_int_distribution<std::uint32_t> qDist(0, 300);
    std::vector<std::uint32_t> values;
    BitWriter w;
    for (int i = 0; i < 2000; ++i) {
        values.push_back(qDist(rng));
        w.writeUnary(values.back());
    }
    w.flush();
    BitReader r(w.bytes());
    for (std::size_t i = 0; i < values.size(); ++i) {
        ASSERT_EQ(r.readUnary(), values[i]) << "value " << i;
    }
}

TEST(BitIo, MixedStreamRoundTrip) {
    std::mt19937 rng(20261007u);
    BitWriter w;
    struct Item { int kind; std::int64_t value; unsigned width; };
    std::vector<Item> items;
    for (int i = 0; i < 5000; ++i) {
        const int kind = static_cast<int>(rng() % 3);
        if (kind == 0) {
            const unsigned width = rng() % 33;
            const std::uint32_t mask = width == 32 ? 0xFFFFFFFFu : ((std::uint32_t{1} << width) - 1u);
            const std::uint32_t v = static_cast<std::uint32_t>(rng()) & mask;
            w.writeBits(v, width);
            items.push_back({0, v, width});
        } else if (kind == 1) {
            const unsigned width = 1 + rng() % 32;
            const unsigned shift = 32 - width;
            const std::int32_t v = static_cast<std::int32_t>(static_cast<std::uint32_t>(rng()) << shift) >> shift;
            w.writeSigned(v, width);
            items.push_back({1, v, width});
        } else {
            const std::uint32_t q = rng() % 64;
            w.writeUnary(q);
            items.push_back({2, q, 0});
        }
    }
    w.flush();
    BitReader r(w.bytes());
    for (std::size_t i = 0; i < items.size(); ++i) {
        const Item& it = items[i];
        if (it.kind == 0) {
            ASSERT_EQ(r.readBits(it.width), static_cast<std::uint32_t>(it.value)) << "item " << i;
        } else if (it.kind == 1) {
            ASSERT_EQ(r.readSigned(it.width), static_cast<std::int32_t>(it.value)) << "item " << i;
        } else {
            ASSERT_EQ(r.readUnary(), static_cast<std::uint32_t>(it.value)) << "item " << i;
        }
    }
}
