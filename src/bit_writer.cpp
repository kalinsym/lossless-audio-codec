#include "lac/bit_writer.hpp"

namespace lac {

void BitWriter::writeBits(std::uint32_t value, unsigned width) {
    if (width == 0) {
        return;
    }
    const std::uint64_t mask = (std::uint64_t{1} << width) - 1;
    acc_ = (acc_ << width) | (value & mask);
    count_ += width;
    while (count_ >= 8) {
        buffer_.push_back(static_cast<std::uint8_t>((acc_ >> (count_ - 8)) & 0xFF));
        count_ -= 8;
    }
}

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
