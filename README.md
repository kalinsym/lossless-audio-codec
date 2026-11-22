# Lossless Audio Codec

A lossless audio compressor and decompressor written from scratch in C++. It takes an uncompressed WAV file, reduces its size by exploiting the statistical structure of audio, and reconstructs the original bit for bit.

COS4091 Senior Project, American University in Bulgaria, Fall 2026.
Author: Kalin Simeonov. Advisor: Prof. Vladimir Georgiev.

## Status

Version 0.3: each block of each channel is coded with the best of the fixed polynomial predictors (orders 0 to 4) and Rice coding of the residuals, or stored verbatim when prediction does not help. Stereo frames are coded as left/right, left/side, right/side or mid/side, whichever is smallest (78 tests).
A WAV file encoded and decoded again is identical bit for bit, and every single-bit error in an encoded file is detected.
Supported input: PCM WAV, mono or stereo, 8, 16 or 24 bits per sample.
Next: the first benchmark against reference FLAC.

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
- [x] Fixed polynomial predictors (orders 0-4)
- [x] Rice coding with adaptive parameter selection
- [x] Mid/side stereo decorrelation
- [ ] LPC via Levinson-Durbin
- [ ] Per-block strategy search
- [ ] CRC integrity checks and malformed-input handling
- [ ] Benchmark against reference FLAC
