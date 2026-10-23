#include <gtest/gtest.h>

#include <cstdint>
#include <random>
#include <string_view>
#include <vector>

#include "lac/crc.hpp"

using Bytes = std::vector<std::uint8_t>;

namespace {
Bytes bytesOf(std::string_view s) { return Bytes(s.begin(), s.end()); }
}  // namespace

TEST(Crc, Crc8CheckValue) {
    EXPECT_EQ(lac::crc8(bytesOf("123456789")), 0xF4);
}

TEST(Crc, Crc16CheckValue) {
    EXPECT_EQ(lac::crc16(bytesOf("123456789")), 0xFEE8);
}

TEST(Crc, EmptyInputIsZero) {
    const Bytes empty;
    EXPECT_EQ(lac::crc8(empty), 0);
    EXPECT_EQ(lac::crc16(empty), 0);
}

TEST(Crc, SingleByteValues) {
    EXPECT_EQ(lac::crc8(Bytes{0x00}), 0x00);
    EXPECT_EQ(lac::crc8(Bytes{0x01}), 0x07);
    EXPECT_EQ(lac::crc16(Bytes{0x00}), 0x0000);
    EXPECT_EQ(lac::crc16(Bytes{0x01}), 0x8005);
}

TEST(Crc, EverySingleBitFlipIsDetected) {
    std::mt19937 rng(20261011u);
    Bytes data(64);
    for (auto& b : data) b = static_cast<std::uint8_t>(rng());
    const std::uint16_t original16 = lac::crc16(data);
    const std::uint8_t original8 = lac::crc8(data);
    for (std::size_t i = 0; i < data.size() * 8; ++i) {
        Bytes flipped = data;
        flipped[i / 8] ^= static_cast<std::uint8_t>(1u << (i % 8));
        EXPECT_NE(lac::crc16(flipped), original16) << "bit " << i;
        EXPECT_NE(lac::crc8(flipped), original8) << "bit " << i;
    }
}

TEST(Crc, AppendingTheChecksumGivesZero) {
    const Bytes data = bytesOf("lossless audio codec");
    const std::uint16_t c = lac::crc16(data);
    Bytes withCrc = data;
    withCrc.push_back(static_cast<std::uint8_t>(c >> 8));
    withCrc.push_back(static_cast<std::uint8_t>(c & 0xFF));
    EXPECT_EQ(lac::crc16(withCrc), 0);
}
