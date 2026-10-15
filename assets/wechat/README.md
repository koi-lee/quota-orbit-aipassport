<p align="right"><a href="README.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Official mini-program code

`miniapp-code-source.png` is the existing official Starshore Reset Assistant
mini-program code published by the user's Quota Orbit website, retrieved on
2026-10-13 from:
https://www.starshoreai.com/quota-orbit/wechat-miniapp-code.png?v=20261013-square

SHA256: `3a7102c979d8a2e33be20338037698bdd0c592d3913301b93e0b9afbb813b94c`.
This matches the website project's published artifact record. It is not a
preview/developer code or a newly generated ordinary QR code. The product owner authorized this asset for public distribution only as part
of `koi-lee/quota-orbit-aipassport` and its firmware package. This does not
grant a separate license to reuse the code image outside this project. Keep it
out of FoloToy upstream contributions.

The parent workspace `tools/prepare-passport-assets.py` retains the complete
800 × 800 code, composites transparent corners on white, and resamples it to
208 × 208. `orbit_miniapp.c` stores RGB565 pixels in Flash; the device does not
need a PNG decoder or download the image. No code dots or central logo are
redrawn, cropped, or replaced. `miniapp-code-208.png` is the preparation preview.

A normal QR decoder does not verify a WeChat mini-program code. Its identity is
based on the published source; recognition on the actual device screen must be
tested with WeChat. No website interstitial is bypassed: this opens the existing
mini-program through its own official code.
