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
