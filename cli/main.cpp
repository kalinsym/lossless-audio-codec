#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

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
    if (argc != 4 || std::string(argv[1]) != "encode") {
        std::cerr << "usage: lac encode <in.wav> <out.lac>\n";
        return 2;
    }
    try {
        writeFile(argv[3], lac::encode(lac::parseWav(readFile(argv[2]))));
    } catch (const std::exception& e) {
        std::cerr << "lac: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
