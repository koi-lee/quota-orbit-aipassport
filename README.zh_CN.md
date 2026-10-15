<p align="right"><a href="README.md">English</a> · <strong>简体中文</strong></p>

# 额度星盘 · AI Passport

给经常使用 Codex 的你：在 AI Passport 小屏上查看公开重置消息；配合 Mac 同步助手，还能抬眼看自己的剩余额度，少一次切换窗口查询。

## 额度星盘是什么？

**额度星盘（Quota Orbit）是面向 Codex 用户的额度查看与重置消息工具。** Mac 主客户端通过桌宠和菜单栏，帮你了解「还剩多少额度、什么时候恢复」，查看本次会话中的额度变化，并关注公开重置公告，方便安排接下来的工作。

[了解额度星盘](https://www.starshoreai.com/quota-orbit/) · [Mac 主客户端与使用说明](https://github.com/koi-lee/quota-orbit)

## AI Passport 随身版能帮你做什么？

把 AI Passport 放在桌边或随身携带，你可以：

- **少刷消息页面，直接看重置进展。** 设备连上 Wi-Fi 后，独立读取公开公告，在首页和来信页查看最新消息、发布时间，以及来源明确提供的预计或完成时间。只看公告不需要 Mac，也不需要登录 Codex。
- **用声音留意新消息。** 开启提示音后，有效的新预告与完成公告会使用不同提示音；首次同步、重复和旧消息不会反复提醒。公开消息约每五分钟检查一次，不是即时推送。
- **抬眼看个人额度，减少切换窗口。** 选装 Passport Mac 同步助手并配对后，角色页用剩余数值和书灯显示 Mac 上已登录 Codex 的额度摘要，帮助你判断接下来如何安排使用。
- **想看详情时再用手机。** 在来信页按确定，用微信扫描屏幕上的小程序码，进入「星岸重置助手」继续查看。

**个人额度同步有使用条件：** 首次需通过 USB 配对，之后可以拔线，但 Mac 仍须保持运行、唤醒、登录 Codex，并与设备连接同一 Wi-Fi，运行同步助手。Mac 休眠或同步中断后，设备会标记旧数据；它不能离开 Mac 独立查询个人额度。

公开重置公告不代表你的个人额度已经恢复。额度星盘帮助你查看信息，不会增加额度或替你执行重置。

## 先选你要安装的内容

| 你要做什么 | 安装什么 | 下载入口 |
|---|---|---|
| 在 AI Passport 上查看额度星盘 | **AI Passport 社区固件**，安装到硬件，不是 Mac 安装包 | [社区作品 1250 · 安装固件](https://ai-passport.folotoy.cn/plays/1250/) |
| 把 Mac 上的 Codex 个人额度同步到设备 | **Passport Mac 同步助手**，选装；只用于个人额度 | [0.3.0 预发布（Apple 芯片）](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.0) · [简体中文说明](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.0/README.zh_CN.md) · [繁體中文](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.0/README.zh_TW.md) · [English](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.0/README.en.md) |
| 在 Mac 上使用桌宠、菜单栏和额度星盘 | **Quota Orbit Mac 主客户端**，不需要 AI Passport | [Mac 主客户端公开试用版](https://github.com/koi-lee/quota-orbit/releases/latest) |

只看公开公告时，不需要 Passport Mac 同步助手。社区安装会替换设备当前玩法，并可能清除原配置；请先阅读社区页面的安装提示。

## 上手与页面说明

- [设备页面、状态和配网／同步流程图](docs/ai-passport/device-guide/README.zh_CN.md)（[English](docs/ai-passport/device-guide/README.md)）
- [语言支持矩阵（简体中文）](docs/ai-passport/language-support.zh_CN.md)
- [版本兼容表](docs/ai-passport/COMPATIBILITY.zh_CN.md)
- [Passport Mac 同步助手源码](tools/lan-quota/)
- [AI Passport 固件源码与构建说明](main/)
- [Release 标签与分发边界](docs/development/releasing.zh_CN.md)

设备通过屏幕上的临时热点配网，网页入口为 `http://192.168.4.1`，仅使用 2.4 GHz Wi-Fi。个人额度同步须先用 USB 完成一次硬件配对；之后 Mac 和设备在同一 Wi-Fi 下自动同步。

## 版本与兼容范围

- 社区页面提供可直接安装的公开版本；仓库根目录的固件源码持续更新，不代表社区安装包已更新。源码变化不会自动更改社区版本。
- Mac 同步助手 `0.3.0` 是 Apple 芯片预发布包。登录后自动启动、Mac 重启后的恢复及网络换址能力仍有未验证范围；设备自动找回地址还需要包含 `/discover` 的固件。该接口当前不在社区公开固件中。
- 助手未使用 Developer ID 签名或 Apple 公证。首次打开时 macOS 可能提示确认。Intel Mac 尚未验证。
- 主客户端与助手是两个独立应用。主客户端公开试用版和 Latest 下载入口仍在 [koi-lee/quota-orbit](https://github.com/koi-lee/quota-orbit)；不要用 Passport 助手替换它。

## 本仓库包含什么

- 根目录 `main/`、`components/`、`assets/`：基于 [FoloToy/ai-passport](https://github.com/FoloToy/ai-passport) 的 AI Passport 设备玩法与源码；沿用上游目录结构构建，并保留许可证和致谢。
- `tools/lan-quota/`：Passport Mac 同步助手源码；配对密钥和额度数据保存在用户自己的 Mac，不放入仓库或安装包。
- `docs/ai-passport/`：安装、使用、页面状态、语言与流程说明。示意屏幕由固件 LVGL 主机渲染器生成，数据为示例值，不是实机照片或真实账户数据。
- GitHub 自动生成的 Source code 下载包对应 **本 AI Passport 仓库**；社区页面提供设备安装入口，Mac 助手安装包在 Releases。

## 许可与素材

Mac 同步助手源码遵循 [AGPL-3.0](tools/lan-quota/LICENSE)；固件源码遵循根目录 FoloToy 上游 MIT 许可证及各文件原有声明；字体遵循其随附许可证。月夜档案员角色和正式微信小程序码是额度星盘的产品素材，只授权作为本项目的一部分公开分发，不因此获得脱离本项目使用这些素材的许可；见 [角色素材说明](assets/character/README.md) 与 [小程序码说明](assets/wechat/README.md)。

Quota Orbit 是独立项目，不隶属于 OpenAI 或 FoloToy，也不代表其官方背书。
