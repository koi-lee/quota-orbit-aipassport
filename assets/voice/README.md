[简体中文](README.zh_CN.md) · **English**

# Offline scenic narration

The 22 original, sourced scripts in narration.json use the author-selected A voice (Kokoro v1.1-zh zf_001) at normal speed. Major spots last 45–60 seconds, others 25–40 seconds, with a 780-second total budget. The device plays prepared audio; no TTS model, recording, network or upload is active on it.

The [model card](https://huggingface.co/hexgrad/Kokoro-82M-v1.1-zh) declares Apache-2.0. The development runtime [kokoro-onnx](https://github.com/thewh1teagle/kokoro-onnx) is MIT. Keep KOKORO-LICENSE.txt. Model weights, environments and original WAVs remain outside the checkout; Mac system voice trials are excluded.

Reproduction starts from immutable synthesis cache. First reproduce the approved B baseline: 85Hz high-pass, 6.2kHz low-pass, 8dB FFT denoising with -42dB floor, PCM16 intermediate and separate +0.11dB trim. The original B audition ADPCM/text fingerprints remain strict before speaker adaptation. Version 0.5.0 then uses documented peak-aware speech compression/limiting and a -20 LUFS/-9dBTP target. Processing is offline, preserves sample counts and text, and does not stack filtering across reruns. manifest.json records all parameters and baseline hashes. This supersedes the fixed +5dB experiment.

Assets are 16kHz mono continuous 4-bit IMA/DVI, low nibble first, zero initial predictor/index and explicit sample counts. Firmware decodes 160 samples at a time, checks codec volume write/readback and suspends after completion/cancellation. Sound Settings provides 0–100% live control, default95%, with one separate persisted volume key; Flash saves wait until playback stops.

Checks cover all hashes, duration, peak/tail, C decoding, optional independent FFmpeg comparison and SNR against mastered reference WAVs. Optional baseline references verify B identity and voiced-frame coverage after mastering. Per-clip decoded loudness/true-peak checks and producer-gap logs complement these. They do not prove subjective naturalness, physical speaker SPL or battery runtime. Preview WAVs come from the actual firmware C decoder.
