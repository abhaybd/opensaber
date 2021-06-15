import wavio
import sys

"""
Extract the pcm data from a wav file and write to a header file, which can be included in the c++ code.
"""

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python convert_wav.py soundName wav_file.wav dest_header.h")
        exit(1)

    name, wav, dest = sys.argv[1:]

    audio = wavio.read(wav)
    data = [x[0] for x in audio.data]
    bits = 8 * audio.sampwidth
    # 8 bit pcm is unsigned, but 16 and 24 are signed, so make unsigned by adding bias
    if bits != 8:
        bias = 1 << (bits - 1)
        data = [x + bias for x in data]
    sound_str = ", ".join([str(x) for x in data])
    with open(dest, "w") as f:
        f.write("#pragma once\n\n")
        f.write("constexpr struct {\n"
                f"    const uint freq = {audio.rate}; // Hz\n"
                f"    const uint precision = {bits};\n"
                f"    const uint{bits}_t sound[{len(audio.data)}] = {{{sound_str}}};\n"
                f"}} {name};\n")
