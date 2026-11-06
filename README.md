# Lossless Audio Codec

A lossless audio compressor and decompressor written from scratch in C++. It takes an uncompressed WAV file, reduces its size by exploiting the statistical structure of audio, and reconstructs the original bit for bit.

COS4091 Senior Project, American University in Bulgaria, Fall 2026.
Author: Kalin Simeonov. Advisor: Prof. Vladimir Georgiev.

## Status

Version 0.1: the first full round trip. The bit-level writer and reader, the WAV parser and writer, CRC-8 and CRC-16, and the container format with its encoder, decoder and command-line tool are implemented (56 tests).
A WAV file encoded and decoded again is identical bit for bit, and every single-bit error in an encoded file is detected.
Frames are stored verbatim for now, so encoded files are slightly larger than the input.
Supported input: PCM WAV, mono or stereo, 8, 16 or 24 bits per sample.
Next: fixed predictors and Rice coding, the first real compression.

## Building

Requirements: CMake 3.25 or newer and a C++20 compiler.

```
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

## Usage

```
./build/lac encode input.wav output.lac
./build/lac decode output.lac restored.wav
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

- [x] Bit-level stream writer and reader
- [x] WAV parser and writer
- [x] Round-trip test harness
- [x] Container format with verbatim frames
- [ ] Fixed polynomial predictors (orders 0-4)
- [ ] Rice coding with adaptive parameter selection
- [ ] Mid/side stereo decorrelation
- [ ] LPC via Levinson-Durbin
- [ ] Per-block strategy search
- [ ] CRC integrity checks and malformed-input handling
- [ ] Benchmark against reference FLAC
