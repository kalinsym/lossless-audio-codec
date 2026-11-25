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
