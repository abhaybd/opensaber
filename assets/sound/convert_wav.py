import wavio
import sys

"""
Extract the pcm data from a wav file. This allows us to copy/paste the audio into the code, so it can be played.
"""

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python convert_wav.py wav_file.wav")
        exit(1)

    audio = wavio.read(sys.argv[1])
    print(f"rate={audio.rate}")
    print(f"bits={8 * audio.sampwidth}")
    print(", ".join([str(x[0]) for x in audio.data]))
