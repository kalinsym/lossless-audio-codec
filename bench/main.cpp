#include <chrono>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "lac/decoder.hpp"
#include "lac/encoder.hpp"
#include "lac/wav.hpp"

namespace {

using Clock = std::chrono::steady_clock;

double secondsSince(Clock::time_point start) {
    return std::chrono::duration<double>(Clock::now() - start).count();
}

}

int main(int argc, char* argv[]) {
    std::printf("%-32s %11s %11s %7s %9s %9s\n", "file", "wav bytes", "lac bytes", "size", "encode", "decode");
    for (int i = 1; i < argc; ++i) {
        try {
            std::ifstream in(argv[i], std::ios::binary);
            const std::vector<std::uint8_t> wavBytes(std::istreambuf_iterator<char>(in), {});
            const lac::WavFile wav = lac::parseWav(wavBytes);
            Clock::time_point start = Clock::now();
            const std::vector<std::uint8_t> lacBytes = lac::encode(wav);
            const double encodeTime = secondsSince(start);
            start = Clock::now();
            const lac::WavFile decoded = lac::decode(lacBytes);
            const double decodeTime = secondsSince(start);
            const double audio = static_cast<double>(wav.frameCount()) / wav.sampleRate;
            const double size = 100.0 * static_cast<double>(lacBytes.size()) / static_cast<double>(wavBytes.size());
            std::printf("%-32s %11zu %11zu %6.2f%% %8.0fx %8.0fx%s\n", argv[i], wavBytes.size(), lacBytes.size(),
                        size, audio / encodeTime, audio / decodeTime, decoded == wav ? "" : "  MISMATCH");
        } catch (const std::exception& e) {
            std::fprintf(stderr, "%s: %s\n", argv[i], e.what());
        }
    }
    return 0;
}
