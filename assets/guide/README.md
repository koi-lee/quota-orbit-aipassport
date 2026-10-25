[简体中文](README.zh_CN.md) · **English**

# Pixel landscape assets

Original AI-assisted Jiuzhaigou-inspired lake and Huanglong-inspired terrace-pool illustrations, generated on 2026-10-25 with the built-in imagegen tool after the author selected visual direction C. These are artistic interpretations, not spot photographs, maps or representations of current site conditions. Both travelers remain on a fenced boardwalk. No third-party artwork, logo or text is embedded.

The completed source PNGs and exact prompts are retained here. Both sources and all six converted files were opened for visual inspection. `tools/convert-guide-art.py` uses Pillow to resize with nearest-neighbor sampling and pack little-endian RGB565; it does not redraw the artwork. Run it from any directory to regenerate `guide_art.c` and the inspection PNGs.

Each park has a 240×80 home hero, 216×72 observation strip and 72×24 reading thumbnail. Together they contain 145,920 bytes of immutable pixel data in Flash. LVGL reads the constant RGB565 image descriptors directly; the application does not allocate a decoded full-screen image or initialize network downloads. Actual runtime RAM and physical display performance still require device validation.

The same two park illustrations provide context across the 22 entries. They are never labeled as individual spot portraits: the Five-Color Pool entry, for example, does not claim its thumbnail is an exact view of that pool. Interface text, focus, pages and saved states are real LVGL objects drawn separately from these pictures.
