#ifndef LAC_BIT_WRITER_HPP
#define LAC_BIT_WRITER_HPP

#include <cstdint>
#include <vector>

namespace lac {

class BitWriter {
public:
    void writeBits(std::uint32_t value, unsigned width);
    void writeSigned(std::int32_t value, unsigned width);
    void writeUnary(std::uint32_t quotient);
    void flush();

    const std::vector<std::uint8_t>& bytes() const;
    std::uint64_t bitCount() const;

private:
    std::vector<std::uint8_t> buffer_;
    std::uint64_t acc_ = 0;
    unsigned count_ = 0;
    std::uint64_t total_ = 0;
};

}

#endif
