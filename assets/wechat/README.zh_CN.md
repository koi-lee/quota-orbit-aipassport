<p align="right"><strong>简体中文</strong> · <a href="README.md">English</a></p>

# 正式小程序码

`miniapp-code-source.png` 是用户额度星盘官网现有的「星岸重置助手」正式小程序码，
2026-10-13 从以下地址取得：
https://www.starshoreai.com/quota-orbit/wechat-miniapp-code.png?v=20261013-square

SHA256：`3a7102c979d8a2e33be20338037698bdd0c592d3913301b93e0b9afbb813b94c`。
与官网项目的已发布产物记录一致。不是开发预览码，也不是重新生成的普通二维码。
产品所有者已授权将此素材作为 `koi-lee/quota-orbit-aipassport` 仓库及其固件包的一部分公开分发；不代表授权脱离本项目单独复用小程序码。不要提交至 FoloToy 上游。

父工作区 `tools/prepare-passport-assets.py` 保留完整 800 × 800 码图，透明角合成
白底后缩放为 208 × 208；`orbit_miniapp.c` 将 RGB565 像素存储于 Flash，设备无需
PNG 解码或联网下载图片。不重绘、裁切或替换码点和中心标识。
`miniapp-code-208.png` 是预处理预览。

普通二维码解码器不能验证微信小程序码。身份依据为已发布原图，实体屏幕的识别率
仍须用微信实测。不绕过官网安全提示，使用正式码直接打开既有小程序。
