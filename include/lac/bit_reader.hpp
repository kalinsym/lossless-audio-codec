#ifndef LAC_BIT_READER_HPP
#define LAC_BIT_READER_HPP

#include <cstddef>
#include <cstdint>
#include <span>

namespace lac {

class BitReader {
public:
    explicit BitReader(std::span<const std::uint8_t> data);

    std::uint32_t readBits(unsigned width);
    bool atEnd() const;

private:
    std::span<const std::uint8_t> data_;
    std::size_t next_ = 0;
    std::uint64_t acc_ = 0;
    unsigned count_ = 0;
};

}

#endif
