# Script to generate a test audio file for use in testing the audio system.
import numpy as np
from scipy.io.wavfile import write, read
import os
import tempfile
import pyttsx3
from scipy.signal import resample

FILES = {
    "F32": "TestAudio_F32.wav",
    "S16": "TestAudio_S16.wav",
    "U8": "TestAudio_U8.wav",
}

def save_tts_f32_wav(text: str, file_name: str, sample_rate: int = 44100) -> None:
    """Generate speech from text and save it as a float32 WAV file."""
    if not text or not text.strip():
        raise ValueError("text must be a non-empty string")
    if not file_name or not file_name.strip():
        raise ValueError("file_name must be a non-empty string")

    engine = pyttsx3.init()
    with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as tmp_file:
        tmp_path = tmp_file.name

    try:
        engine.save_to_file(text, tmp_path)
        engine.runAndWait()

        tts_rate, tts_data = read(tmp_path)

        # Convert integer PCM data to normalized float32.
        if np.issubdtype(tts_data.dtype, np.integer):
            max_val = max(abs(np.iinfo(tts_data.dtype).min), np.iinfo(tts_data.dtype).max)
            tts_data = tts_data.astype(np.float32) / float(max_val)
        else:
            tts_data = tts_data.astype(np.float32)

        # Downmix to mono if needed.
        if tts_data.ndim > 1:
            tts_data = tts_data.mean(axis=1)

        # Resample to target sample rate if needed.
        if tts_rate != sample_rate:
            target_len = int(round(len(tts_data) * sample_rate / tts_rate))
            tts_data = resample(tts_data, target_len).astype(np.float32)

        write(file_name, sample_rate, tts_data.astype(np.float32))
    finally:
        if os.path.exists(tmp_path):
            os.remove(tmp_path)

def main():
    # Generate a 1 second long 440Hz sine wave at a sample rate of 44100Hz.
    sample_rate = 44100
    duration = 1.0
    frequency = 440.0
    t = np.linspace(0, duration, int(sample_rate * duration), endpoint=False)
    audio_data = 0.5 * np.sin(2 * np.pi * frequency * t)

    # Save the audio data to WAV files with multiple sample formats.
    write("TestAudio_F32.wav", sample_rate, audio_data.astype(np.float32))
    write("TestAudio_S16.wav", sample_rate, (audio_data * np.iinfo(np.int16).max).astype(np.int16))
    write("TestAudio_U8.wav", sample_rate, ((audio_data + 1.0) * 127.5).clip(0, 255).astype(np.uint8)) # WAV 8-bit PCM is unsigned, so remap [-1.0, 1.0] into [0, 255].

    # Additional speech synthesis test for testing filtering and resampling.
    save_tts_f32_wav("This is a test of the text-to-speech system. It should generate a clear and intelligible audio file.", "TestAudio_TTS_F32.wav", sample_rate)

if __name__ == "__main__":
    main()