<p align="right"><a href="README.md">English</a> · <strong>简体中文</strong></p>

# 额度星盘 · AI Passport

这是 Quota Orbit 的 AI Passport 随身版：设备通过 Wi-Fi 查看公开重置消息；需要查看个人 Codex 额度时，再选装 Passport Mac 同步助手。

## 先选你要安装的内容

| 你要做什么 | 安装什么 | 下载入口 |
|---|---|---|
| 在 AI Passport 上查看额度星盘 | **AI Passport 社区固件**，安装到硬件，不是 Mac 安装包 | [社区作品 1250 · 安装固件](https://ai-passport.folotoy.cn/plays/1250/) |
| 把 Mac 上的 Codex 个人额度同步到设备 | **Passport Mac 同步助手**，选装；只用于个人额度 | [0.3.0 预发布（Apple 芯片）](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.0) · [简体中文说明](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.0/README.zh_CN.md) · [繁體中文](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.0/README.zh_TW.md) · [English](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.0/README.en.md) |
| 在 Mac 上使用桌宠、菜单栏和额度星盘 | **Quota Orbit Mac 主客户端**，不需要 AI Passport | [主客户端正式版本](https://github.com/koi-lee/quota-orbit/releases/latest) |

只看公开公告时，不需要 Passport Mac 同步助手。社区安装会替换设备当前玩法，并可能清除原配置；请先阅读社区页面的安装提示。

## 上手与页面说明

- [设备页面、状态和配网／同步流程图](docs/ai-passport/device-guide/README.zh_CN.md)（[English](docs/ai-passport/device-guide/README.md)）
- [语言支持矩阵（简体中文）](docs/ai-passport/language-support.zh_CN.md)
- [版本兼容表](docs/ai-passport/COMPATIBILITY.md)
- [Passport Mac 同步助手源码](tools/lan-quota/)
- [AI Passport 固件源码与构建说明](main/)

设备通过屏幕上的临时热点配网，网页入口为 `http://192.168.4.1`，仅使用 2.4 GHz Wi-Fi。个人额度同步须先用 USB 完成一次硬件配对；之后 Mac 和设备在同一 Wi-Fi 下自动同步。

## 版本与兼容范围

- 社区页面提供可直接安装的公开版本；仓库根目录的固件源码持续更新，不代表社区安装包已更新。源码变化不会自动更改社区版本。
- Mac 同步助手 `0.3.0` 是 Apple 芯片预发布包。登录后自动启动、Mac 重启后的恢复及网络换址能力仍有未验证范围；设备自动找回地址还需要包含 `/discover` 的固件。该接口当前不在社区公开固件中。
- 助手未使用 Developer ID 签名或 Apple 公证。首次打开时 macOS 可能提示确认。Intel Mac 尚未验证。
- 主客户端与助手是两个独立应用。主客户端正式版和 Latest 下载入口仍在 [koi-lee/quota-orbit](https://github.com/koi-lee/quota-orbit)；不要用 Passport 助手替换它。

## 本仓库包含什么

- 根目录 `main/`、`components/`、`assets/`：基于 [FoloToy/ai-passport](https://github.com/FoloToy/ai-passport) 的 AI Passport 设备玩法与源码；沿用上游目录结构构建，并保留许可证和致谢。
- `tools/lan-quota/`：Passport Mac 同步助手源码；配对密钥和额度数据保存在用户自己的 Mac，不放入仓库或安装包。
- `docs/ai-passport/`：安装、使用、页面状态、语言与流程说明。示意屏幕由固件 LVGL 主机渲染器生成，数据为示例值，不是实机照片或真实账户数据。
- GitHub 自动生成的 Source code 下载包对应 **本 AI Passport 仓库**；社区页面提供设备安装入口，Mac 助手安装包在 Releases。

## 许可与素材

Mac 同步助手源码遵循 [AGPL-3.0](tools/lan-quota/LICENSE)；固件源码遵循根目录 FoloToy 上游 MIT 许可证及各文件原有声明；字体遵循其随附许可证。月夜档案员角色和正式微信小程序码是额度星盘的产品素材，只授权作为本项目的一部分公开分发，不因此获得脱离本项目使用这些素材的许可；见 [角色素材说明](assets/character/README.md) 与 [小程序码说明](assets/wechat/README.md)。

Quota Orbit 是独立项目，不隶属于 OpenAI 或 FoloToy，也不代表其官方背书。
