<p align="right"><strong>简体中文</strong> · <a href="COMPATIBILITY.md">English</a></p>

# 版本与下载兼容表

| 项目 | 公开入口 | 用途 | 兼容说明 |
|---|---|---|---|
| AI Passport 社区固件 | [社区作品 1250](https://ai-passport.folotoy.cn/plays/1250/) | 安装到设备的固件 | 社区 REV-2886 起包含三语界面、重要操作二次确认及 `/discover` 设备地址发现。请按安装页公开更新日志核对实际版本。 |
| 固件源码 | [设备应用源码](../../main/) | 基于 FoloToy AI Passport 的可构建源码 | 源码可能先于社区包更新；更新源码不会自动发布或安装固件。 |
| Passport Mac 同步助手 0.3.2 | [GitHub 预发布](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.2) | Apple 芯片 Mac 配套安装包 | 预发布；新版 `-notarized.zip` 已通过 Developer ID 签名与 Apple 公证。Mac 登录／重启启动及真实 DHCP 换址恢复仍未验证；自动找回需要设备固件提供 `/discover`。 |
| Quota Orbit Mac 主客户端 | [最新正式版](https://github.com/koi-lee/quota-orbit/releases/latest) | 独立 Mac 桌面客户端 | 不要用 Passport 助手替换主客户端。 |

只看公开公告时安装社区固件即可，Mac 助手是选装项。需要在设备查看个人 Codex 额度时，安装助手并通过 USB 配对一次。

助手 0.3.0 安装包提供简体中文、繁体中文和 English，已在预览模式核验语言切换及选择记忆；Release 安装说明也提供三语版本。设备固定界面已有三语文案与字体覆盖检查，但不代表所有设置状态的字形均已完整实机复验。
