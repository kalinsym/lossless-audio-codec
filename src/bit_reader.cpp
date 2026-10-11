#include "lac/bit_reader.hpp"

namespace lac {

BitReader::BitReader(std::span<const std::uint8_t> data) : data_(data) {}

std::uint32_t BitReader::readBits(unsigned) {
    return 0;
}

bool BitReader::atEnd() const {
    return false;
}

}
