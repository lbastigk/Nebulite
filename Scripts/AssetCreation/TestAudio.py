# Script to generate a test audio file for use in testing the audio system.
import numpy as np

FILE="TestAudio.wav"

def main():
    # Generate a 1 second long 440Hz sine wave at a sample rate of 44100Hz.
    sample_rate = 44100
    duration = 1.0
    frequency = 440.0
    t = np.linspace(0, duration, int(sample_rate * duration), endpoint=False)
    audio_data = 0.5 * np.sin(2 * np.pi * frequency * t)

    # Save the audio data to a WAV file.
    from scipy.io.wavfile import write
    write(FILE, sample_rate, audio_data.astype(np.float32))

if __name__ == "__main__":
    main()