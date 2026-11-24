# Lossless Audio Codec

A lossless audio compressor and decompressor written from scratch in C++. It takes an uncompressed WAV file, reduces its size by exploiting the statistical structure of audio, and reconstructs the original bit for bit.

COS4091 Senior Project, American University in Bulgaria, Fall 2026.
Author: Kalin Simeonov. Advisor: Prof. Vladimir Georgiev.

## Status

Version 0.4: each block of each channel is coded with the smallest of verbatim samples, the fixed polynomial predictors (orders 0 to 4) and linear prediction (LPC, orders 1 to 12, coefficients from the Levinson-Durbin recursion quantized to 15 bits), with Rice coding of the residuals. Stereo frames are coded as left/right, left/side, right/side or mid/side, whichever is smallest (87 tests).
A WAV file encoded and decoded again is identical bit for bit, and every single-bit error in an encoded file is detected.
Supported input: PCM WAV, mono or stereo, 8, 16 or 24 bits per sample.
Next: a faster encoder search and hardening.

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

## Benchmark

Eleven tracks of the EBU SQAM test set (EBU Tech 3253), 16-bit, 44.1 kHz stereo, compared with reference FLAC 1.5.0. Size is the encoded file as a percentage of the WAV; smaller is better.

| Track | lac | flac -5 | flac -8 |
|---|---|---|---|
| 27 Castanets | 39.29% | 37.21% | 34.54% |
| 35 Glockenspiel | 25.95% | 23.32% | 21.80% |
| 40 Harpsichord | 36.25% | 34.40% | 34.05% |
| 48 Vocal quartet | 38.90% | 36.84% | 36.44% |
| 50 Male speech, English | 32.12% | 29.70% | 29.30% |
| 58 Guitar | 33.13% | 30.01% | 29.59% |
| 60 Piano | 22.85% | 21.56% | 21.42% |
| 64 Choir | 51.23% | 49.23% | 49.05% |
| 65 Orchestra | 37.82% | 36.40% | 36.32% |
| 69 Pop (ABBA) | 43.71% | 41.12% | 40.60% |
| 70 Pop (Eddie Rabbitt) | 38.87% | 35.96% | 35.52% |
| All files | 34.39% | 32.46% | 31.96% |

Every track decodes bit for bit. lac is about 2 percentage points behind flac -5 overall; linear prediction closed half of the earlier gap, most clearly on the glockenspiel (33.66% with fixed predictors only, 25.95% with LPC). The exhaustive search over predictors and stereo modes makes encoding about 20 times slower than flac -5; decoding is about 2 times slower, measured on an Apple MacBook Pro.

To reproduce, with the WAV files in a folder outside the repository:

```
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
python3 bench/compare.py path/to/*.wav
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
- [x] LPC via Levinson-Durbin
- [ ] Per-block strategy search
- [ ] CRC integrity checks and malformed-input handling
- [x] Benchmark against reference FLAC
