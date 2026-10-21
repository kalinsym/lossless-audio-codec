#include "lac/bit_reader.hpp"

#include <cassert>
#include <stdexcept>

namespace lac {

BitReader::BitReader(std::span<const std::uint8_t> data) : data_(data) {}

std::uint32_t BitReader::readBits(unsigned width) {
    assert(width <= 32);
    if (width == 0) {
        return 0;
    }
    if (width > count_ + 8 * (data_.size() - next_)) {
        throw std::out_of_range("BitReader: read past the end of the data");
    }
    while (count_ < width) {
        acc_ = (acc_ << 8) | data_[next_];
        ++next_;
        count_ += 8;
    }
    count_ -= width;
    const std::uint64_t mask = (std::uint64_t{1} << width) - 1;
    return static_cast<std::uint32_t>((acc_ >> count_) & mask);
}

std::uint32_t BitReader::readUnary() {
    std::uint32_t zeros = 0;
    while (readBits(1) == 0) {
        ++zeros;
    }
    return zeros;
}

std::int32_t BitReader::readSigned(unsigned width) {
    if (width == 0) {
        return 0;
    }
    const std::int64_t raw = readBits(width);
    const std::int64_t half = std::int64_t{1} << (width - 1);
    const std::int64_t value = raw >= half ? raw - 2 * half : raw;
    return static_cast<std::int32_t>(value);
}

void BitReader::alignToByte() {
    count_ = 0;
}

std::size_t BitReader::bytePosition() const {
    return next_;
}

bool BitReader::atEnd() const {
    return count_ == 0 && next_ == data_.size();
}

}
