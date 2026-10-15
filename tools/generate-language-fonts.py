#!/usr/bin/env python3
"""Regenerate the fixed UI glyph inventories for Simplified and Traditional Chinese."""
from __future__ import annotations

import argparse
import json
import re
import subprocess
from pathlib import Path


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--sc-font", type=Path, required=True)
    parser.add_argument("--tc-font", type=Path, required=True)
    parser.add_argument("--converter", type=Path, default=Path("lv_font_conv"))
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    main_dir = root / "main"
    fonts = root / "assets/fonts"
    chars = set(chr(cp) for cp in range(32, 127))
    for source in [*main_dir.glob("orbit_*.c"), *main_dir.glob("orbit_*.h")]:
        for literal in re.findall(r'"([^"\\]*(?:\\.[^"\\]*)*)"', source.read_text()):
            try:
                value = json.loads('"' + literal + '"')
            except json.JSONDecodeError:
                continue
            chars.update(char for char in value if ord(char) > 127)
    inventory = "".join(sorted(chars, key=ord))
    (fonts / "characters.txt").write_text(inventory + "\n")

    if not args.sc_font.is_file() or not args.tc_font.is_file():
        parser.error("Both Noto Sans CJK SC and TC font files must exist")
    for variant, font in (("", args.sc_font), ("_tc", args.tc_font)):
        for size in (12, 14, 16, 20, 24):
            output = fonts / f"orbit_font{variant}_{size}.c"
            subprocess.run(
                [
                    str(args.converter), "--font", str(font), "--symbols", inventory,
                    "--size", str(size), "--bpp", "4", "--format", "lvgl",
                    "--no-compress", "--lv-font-name", f"orbit_font{variant}_{size}",
                    "--lv-include", "lvgl.h", "--output", str(output),
                ],
                check=True,
            )
    print(f"Generated ten font subsets with {len(chars)} code points")


if __name__ == "__main__":
    main()
