<p align="right"><strong>简体中文</strong> · <a href="README.md">English</a></p>

# 额度星盘角色

复制现有额度星盘网页的 `web/pet-idle-poster.png` 为 `archivist-source.png`，
用于用户自己的随身配套产品。形象是紫发、月牙发饰、披风、持书的月夜档案员，
替换临时几何角色；没有另行生成新角色或动画。

父产品工作区的 `tools/prepare-passport-assets.py` 去除透明留边，保持比例适配
120 × 156 像素区域，并转换为 LVGL ARGB8888 的 BGRA 字节。实际大小为
86 × 156 像素，占 Flash 53,664 字节，无需 PNG 解码或运行时缩放，保留透明背景。

这是用户现有产品素材，不是 FoloToy 上游素材。本次复用属于已要求的产品开发；
产品所有者已授权将素材作为 `koi-lee/quota-orbit-aipassport` 仓库及其固件包的一部分公开分发；这不代表授权脱离本项目单独复用该形象。不要提交到 FoloToy 上游仓库。

信息优先的首页另使用 49 × 90 px 小图，占 Flash 17,640 字节；角色页保留
86 × 156 px 大图。

庆祝帧 orbit_celebration.c 使用同一已授权角色与本产品绘制的祈愿/星光粒子，
由工作区 tools/render-star-celebration.py 生成。49 张 RGB565 帧仅存闪存，
不包含用户数据；不是新角色图或实机截图。
