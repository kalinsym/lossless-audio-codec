# Lossless Audio Codec

A lossless audio compressor and decompressor written from scratch in C++. It reads an uncompressed WAV file, compresses it by exploiting the statistical structure of audio, and reconstructs the original bit for bit.

COS4091 Senior Project, American University in Bulgaria, Fall 2026.
Author: Kalin Simeonov. Advisor: Prof. Vladimir Georgiev.

## Status

Early development. The roadmap below is worked through in order and each step is tagged when it is done.

## Building

Requirements: CMake 3.25 or newer and a C++20 compiler (Apple Clang 15, Clang 16 or GCC 12, or newer).

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure --timeout 60
```

Debug build with AddressSanitizer and UndefinedBehaviorSanitizer:

```sh
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug -DLAC_SANITIZE=ON
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure --timeout 120
```

## Repository layout

| Path | Contents |
|---|---|
| `include/lac/` | Public headers |
| `src/` | Codec implementation |
| `cli/` | Command-line interface (`lac`) |
| `tests/` | Unit, round-trip and fuzz tests |
| `bench/` | Benchmarking tool (`lac_bench`) |
| `docs/` | Design notes |
| `corpus/` | Local audio for benchmarking, not tracked |

## Dependencies

The codec uses only the C++ standard library. No third-party signal processing or compression libraries are used.

The test suite uses GoogleTest, which CMake downloads at configure time and links into the test executable only. The benchmark tool invokes a reference `flac` binary if one is on the PATH; it is never linked.

## Roadmap

- [ ] Bit-level stream writer and reader
- [ ] WAV parser and writer
- [ ] Round-trip verification harness
- [ ] Container format and verbatim coding
- [ ] Fixed polynomial predictors, orders 0–4
- [ ] Rice residual coding with adaptive parameters and partitioning
- [ ] Mid/side stereo decorrelation
- [ ] Linear prediction via Levinson–Durbin
- [ ] Per-block strategy search
- [ ] CRC integrity checks and malformed-input handling
- [ ] Benchmark against reference FLAC
