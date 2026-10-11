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

bool BitReader::atEnd() const {
    return count_ == 0 && next_ == data_.size();
}

}
