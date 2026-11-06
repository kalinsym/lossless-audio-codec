#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include "lac/decoder.hpp"
#include "lac/encoder.hpp"
#include "lac/wav.hpp"

namespace {

std::vector<std::uint8_t> readFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("cannot open " + path);
    }
    return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(in), {});
}

void writeFile(const std::string& path, const std::vector<std::uint8_t>& bytes) {
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!out) {
        throw std::runtime_error("cannot write " + path);
    }
}

}

int main(int argc, char* argv[]) {
    const std::string mode = argc == 4 ? argv[1] : "";
    if (mode != "encode" && mode != "decode") {
        std::cerr << "usage: lac encode <in.wav> <out.lac>\n"
                     "       lac decode <in.lac> <out.wav>\n";
        return 2;
    }
    try {
        const std::vector<std::uint8_t> in = readFile(argv[2]);
        if (mode == "encode") {
            writeFile(argv[3], lac::encode(lac::parseWav(in)));
        } else {
            writeFile(argv[3], lac::serializeWav(lac::decode(in)));
        }
    } catch (const std::exception& e) {
        std::cerr << "lac: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
