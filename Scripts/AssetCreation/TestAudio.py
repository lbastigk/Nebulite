# Script to generate a test audio file for use in testing the audio system.
import numpy as np
from scipy.io.wavfile import write

FILES = {
    "F32": "TestAudio_F32.wav",
    "S16": "TestAudio_S16.wav",
    "U8": "TestAudio_U8.wav",
}

def main():
    # Generate a 1 second long 440Hz sine wave at a sample rate of 44100Hz.
    sample_rate = 44100
    duration = 1.0
    frequency = 440.0
    t = np.linspace(0, duration, int(sample_rate * duration), endpoint=False)
    audio_data = 0.5 * np.sin(2 * np.pi * frequency * t)

    # Save the audio data to WAV files with multiple sample formats.
    write(FILES["F32"], sample_rate, audio_data.astype(np.float32))
    write(FILES["S16"], sample_rate, (audio_data * np.iinfo(np.int16).max).astype(np.int16))

    # WAV 8-bit PCM is unsigned, so remap [-1.0, 1.0] into [0, 255].
    u8_data = ((audio_data + 1.0) * 127.5).clip(0, 255).astype(np.uint8)
    write(FILES["U8"], sample_rate, u8_data)

if __name__ == "__main__":
    main()