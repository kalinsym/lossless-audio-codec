import os
import subprocess
import sys
import tempfile
import time

LAC = os.environ.get("LAC", "build-release/lac")


def timed(command):
    start = time.perf_counter()
    subprocess.run(command, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return time.perf_counter() - start


def codecs(wav, packed, restored):
    return {
        "lac": ([LAC, "encode", wav, packed], [LAC, "decode", packed, restored]),
        "flac -5": (["flac", "-5", "-f", "-o", packed, wav], ["flac", "-d", "-f", "-o", restored, packed]),
        "flac -8": (["flac", "-8", "-f", "-o", packed, wav], ["flac", "-d", "-f", "-o", restored, packed]),
    }


def main(paths):
    totals = {}
    print(f"{'file':<32} {'codec':<8} {'size':>8} {'encode s':>9} {'decode s':>9}")
    with tempfile.TemporaryDirectory() as tmp:
        packed = os.path.join(tmp, "packed")
        restored = os.path.join(tmp, "restored.wav")
        for wav in paths:
            for name, (encode, decode) in codecs(wav, packed, restored).items():
                encode_time = timed(encode)
                size = os.path.getsize(packed)
                decode_time = timed(decode)
                total = totals.setdefault(name, [0, 0])
                total[0] += size
                total[1] += os.path.getsize(wav)
                print(f"{os.path.basename(wav):<32} {name:<8} {100 * size / os.path.getsize(wav):7.2f}% "
                      f"{encode_time:9.3f} {decode_time:9.3f}")
    for name, (packed_bytes, wav_bytes) in totals.items():
        print(f"{'all files':<32} {name:<8} {100 * packed_bytes / wav_bytes:7.2f}%")


if __name__ == "__main__":
    main(sys.argv[1:])
