# Lossless Audio Codec

A lossless audio compressor and decompressor written from scratch in C++. It takes an uncompressed WAV file, reduces its size by exploiting the statistical structure of audio, and reconstructs the original bit for bit.

COS4091 Senior Project, American University in Bulgaria, Fall 2026.
Author: Kalin Simeonov. Advisor: Prof. Vladimir Georgiev.

## Status

Project setup. No codec components are implemented yet.

## Building

Requirements: CMake 3.25 or newer and a C++20 compiler.

```
cmake -S . -B build
cmake --build build
```

## Planned repository layout

```
include/lac/   public headers
src/           library implementation
cli/           command-line encoder and decoder
tests/         unit and round-trip tests
bench/         benchmark against reference FLAC
docs/          format specification and notes
corpus/        test audio (not tracked)
```

## Dependencies

The codec core uses only the C++ standard library. No third-party signal processing or compression libraries are used.

## Roadmap

- [ ] Bit-level stream writer and reader
- [ ] WAV parser and writer
- [ ] Round-trip test harness
- [ ] Container format with verbatim frames
- [ ] Fixed polynomial predictors (orders 0-4)
- [ ] Rice coding with adaptive parameter selection
- [ ] Mid/side stereo decorrelation
- [ ] LPC via Levinson-Durbin
- [ ] Per-block strategy search
- [ ] CRC integrity checks and malformed-input handling
- [ ] Benchmark against reference FLAC
