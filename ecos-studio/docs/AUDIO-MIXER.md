# Audio Mixer Foundation

This first audio slice is a deterministic software mixer, not an operating-system audio device or a production spatial-audio engine.

- Mixes mono or stereo interleaved floating-point PCM to stereo output.
- Supports per-voice gain, pan, one-shot playback, looping, and explicit stop.
- Uses equal-power panning for mono voices and balance attenuation for stereo voices.
- Clamps mixed samples to [-1, 1] and removes completed/stopped voices after a mix call.
- Validates finite samples, channel count, gain, pan, and sample-rate compatibility.

Resampling is not implemented; clip sample rate must equal the mixer rate. Audio decoding, device output, streaming, spatialization, effects, voice virtualization, and thread-safe real-time command submission remain future work. Do not call this a playable audio device yet.
