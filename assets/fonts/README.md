<p align="right"><a href="README.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Quota Orbit font

`orbit_font_{12,14,16,20,24}.c` and `orbit_font_tc_{12,14,16,20,24}.c` are 4 bpp, uncompressed LVGL subsets generated with lv_font_conv 1.5.3 from Noto Sans CJK SC Regular and TC Regular respectively. `characters.txt` is the printable ASCII and fixed firmware UI inventory for all supported languages. The host renderer checks every code point at all ten sizes; adding fixed UI text requires regenerating this inventory and font family.

Sources: https://github.com/notofonts/noto-cjk/tree/main/Sans/OTF/SimplifiedChinese and https://github.com/notofonts/noto-cjk/tree/main/Sans/OTF/TraditionalChinese. Source SHA256 values used for this local build: SC `2c76254f6fc379fddfce0a7e84fb5385bb135d3e399294f6eeb6680d0365b74b`; TC `dce08bd4fd91aa8aa76ed8fea4b694c2dfb8550f67871e326843212ddbeb88b`.
License: [SIL Open Font License](OFL.txt). No system fonts are redistributed.

Regenerate all ten subsets from the firmware repository root. Provide licensed source font files; raw OTF files are not redistributed:

```sh
python3 tools/generate-language-fonts.py \
  --sc-font /path/to/NotoSansCJKsc-Regular.otf \
  --tc-font /path/to/NotoSansCJKtc-Regular.otf \
  --converter /path/to/lv_font_conv-1.5.3
```

The generated font sources total about 3.5 MB before firmware linking. The firmware build and size report must confirm they fit the configured app partition. Real panel legibility, Traditional Chinese visual style, and color remain device acceptance items.
