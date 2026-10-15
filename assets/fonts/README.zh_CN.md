<p align="right"><strong>简体中文</strong> · <a href="README.md">English</a></p>

# 额度星盘字体

`orbit_font_{12,14,16,20,24}.c` 和 `orbit_font_tc_{12,14,16,20,24}.c` 是由 lv_font_conv 1.5.3 分别使用 Noto Sans CJK SC Regular 与 TC Regular 生成的 4 bpp、不压缩 LVGL 子集。`characters.txt` 是所有语言固定界面的可打印 ASCII 与码点清单。主机渲染器会通过 LVGL 检查全部十种字号的每个码点；新增固定界面文案后必须重新生成字表和字体。

来源：https://github.com/notofonts/noto-cjk/tree/main/Sans/OTF/SimplifiedChinese 与 https://github.com/notofonts/noto-cjk/tree/main/Sans/OTF/TraditionalChinese。本地构建所用源文件 SHA256：SC `2c76254f6fc379fddfce0a7e84fb5385bb135d3e399294f6eeb6680d0365b74b`；TC `dce08bd4fd91aa8aa76ed8fea4b694c2dfb8550f67871e326843212ddbeb88b`。
许可证：[SIL Open Font License](OFL.txt)。不分发系统字体。

在固件仓库根目录重新生成全部十个子集。请自行提供有授权的字库文件；仓库不分发原始 OTF：

```sh
python3 tools/generate-language-fonts.py \
  --sc-font /path/to/NotoSansCJKsc-Regular.otf \
  --tc-font /path/to/NotoSansCJKtc-Regular.otf \
  --converter /path/to/lv_font_conv-1.5.3
```

生成的字体 C 源码合计约 3.5 MB，链接后的固件构建及尺寸报告必须确认仍小于应用分区。实体屏幕可读性、繁体字形观感和颜色仍需真机验收。
