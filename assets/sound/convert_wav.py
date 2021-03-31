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
    sound_str = ", ".join([str(x[0]) for x in audio.data])
    with open(dest, "w") as f:
        f.write("#pragma once\n\n")
        f.write("constexpr struct {\n"
                f"    const uint freq = {audio.rate}; // Hz\n"
                f"    const uint8_t sound[{len(audio.data)}] = {{{sound_str}}};\n"
                f"}} {name};\n")
