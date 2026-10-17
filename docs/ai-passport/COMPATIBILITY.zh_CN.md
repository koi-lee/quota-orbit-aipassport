<p align="right"><strong>简体中文</strong> · <a href="COMPATIBILITY.md">English</a></p>

# 版本与下载兼容表

| 项目 | 公开入口 | 用途 | 兼容说明 |
|---|---|---|---|
| AI Passport 社区固件 | [社区作品 1250](https://ai-passport.folotoy.cn/plays/1250/) | 安装到设备的固件 | 社区 REV-2886 起包含三语界面、重要操作二次确认及 `/discover` 设备地址发现。请按安装页公开更新日志核对实际版本。 |
| 固件源码 | [设备应用源码](../../main/) | 基于 FoloToy AI Passport 的可构建源码 | 源码可能先于社区包更新；更新源码不会自动发布或安装固件。 |
| Passport Mac 同步助手 0.3.3 | [GitHub 预发布](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.3) | Apple 芯片 Mac 配套安装包 | 预发布；已签名、公证及装订。Apple 芯片 macOS 14.8.9、15.7.9、27.0.1 基础运行检查通过；11–13 未测，Intel 不支持。作者反馈现有安装重启与断网恢复正常；不等于 0.3.3 新包或全新电脑配对验收。DHCP 换址仍待实测，自动找回需 `/discover`。 |
| Quota Orbit Mac 主客户端 | [最新正式版](https://github.com/koi-lee/quota-orbit/releases/latest) | 独立 Mac 桌面客户端 | 不要用 Passport 助手替换主客户端。 |

只看公开公告时安装社区固件即可，Mac 助手是选装项。需要在设备查看个人 Codex 额度时，安装助手并通过 USB 配对一次。

当前助手提供简体中文、繁体中文和 English；安装包要求、验证与恢复限制以[助手指南](mac-helper/README.zh_CN.md)为准，Release 安装说明也有三语版本。设备文案已有三语覆盖，但不代表所有设置状态均已完整实机复验。
