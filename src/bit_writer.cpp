#include "lac/bit_writer.hpp"

namespace lac {

void BitWriter::writeBits(std::uint32_t, unsigned) {}

void BitWriter::writeSigned(std::int32_t, unsigned) {}

void BitWriter::writeUnary(std::uint32_t) {}

void BitWriter::flush() {}

const std::vector<std::uint8_t>& BitWriter::bytes() const {
    return buffer_;
}

std::uint64_t BitWriter::bitCount() const {
    return 0;
}

}
