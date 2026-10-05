#include "lac/bit_writer.hpp"

namespace lac {

void BitWriter::writeBits(std::uint32_t value, unsigned width) {
    if (width == 0) {
        return;
    }
    const std::uint64_t mask = (std::uint64_t{1} << width) - 1;
    acc_ = (acc_ << width) | (value & mask);
    count_ += width;
    total_ += width;
    while (count_ >= 8) {
        buffer_.push_back(static_cast<std::uint8_t>((acc_ >> (count_ - 8)) & 0xFF));
        count_ -= 8;
    }
}

void BitWriter::writeSigned(std::int32_t value, unsigned width) {
    writeBits(static_cast<std::uint32_t>(value), width);
}

void BitWriter::writeUnary(std::uint32_t quotient) {
    while (quotient >= 32) {
        writeBits(0, 32);
        quotient -= 32;
    }
    writeBits(1, quotient + 1);
}

void BitWriter::flush() {
    if (count_ == 0) {
        return;
    }
    buffer_.push_back(static_cast<std::uint8_t>((acc_ << (8 - count_)) & 0xFF));
    count_ = 0;
}

const std::vector<std::uint8_t>& BitWriter::bytes() const {
    return buffer_;
}

std::uint64_t BitWriter::bitCount() const {
    return total_;
}

}
