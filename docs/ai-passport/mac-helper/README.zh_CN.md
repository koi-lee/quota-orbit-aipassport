# 额度星盘 · Passport Mac 同步助手 0.3.0（公开试用版）

<p align="right"><a href="README.zh_CN.md">简体中文</a> · <a href="README.zh_TW.md">繁體中文</a> · <a href="README.md">English</a></p>

把已登录 Codex 的 Mac 上可读取的个人剩余额度，加密同步到同一 Wi-Fi 下已配对的 AI Passport。只看公开重置消息不需要此助手。

## 下载

[下载 Apple 芯片 Mac 安装包](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.0/StarshorePassport-AppleSilicon-0.3.0.zip) · [校验文件](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.0/SHA256SUMS.txt) · [版本与全部附件](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.0)

这是 GitHub Pre-release，当前为本地 ad-hoc 签名，尚未 Developer ID 签名或 Apple 公证。若 macOS 阻止正常打开，请停止安装并反馈系统提示，不要关闭系统安全保护。

## 准备

需要 Apple 芯片 Mac；安装并登录 Codex CLI，或使用安装在“应用程序”中的 Codex 桌面应用。Mac 与设备连接同一 Wi-Fi，Mac 保持开机、唤醒和联网。首次配对用 USB 数据线。

## 首次使用

1. 解压 ZIP，将 `StarshorePassport.app` 拖入“应用程序”。旧助手正在运行时先退出再替换。首次打开后，浏览器显示「随身星盘 · Mac同步助手」；若未打开，可访问 http://127.0.0.1:8942/ 。
2. USB 连接开机设备，先点电脑的「连接设备」。设备可能重启；等它回到首页后，再长按硬件确定→设置→连接 Mac，出现允许提示后按确定。
3. 电脑显示配对成功后自动同步。角色页出现个人额度，即可拔掉 USB。以后设备开机，助手会自动继续同步；若曾手动暂停，在电脑点「恢复同步」。
4. 助手首次正常运行时登记当前用户的登录启动项，可在“系统设置→通用→登录项”关闭。关闭助手不会解除配对；需要撤销授权时，在设备设置选择「解除 Mac 配对」并按设备提示确认。

## 范围与限制

- 约每 30 秒更新一次。Codex 未提供的额度窗口不会被补成零；离线或 Mac 休眠时，设备保留旧数据并标记更新时间。
- 设备重启后的周期同步已在一台设备验证；Mac 实际注销／登录或系统重启后的自动启动，以及真实 DHCP 换址恢复仍未完成实测。
- 自动换地址需要设备固件提供配对设备发现接口。较旧的社区 REV-2840 不包含该接口，仍可使用已保存地址同步；新候选固件包含接口，是否已公开请核对[社区作品](https://ai-passport.folotoy.cn/plays/1250/)的更新日志。某些较大网段会跳过自动扫描，换址后可能需要重新连接。
- Codex 登录凭据与配对密钥留在 Mac，设备只接收加密额度摘要。公开重置公告不能证明个人额度已恢复。
- 反馈请提供助手版本、固件版本、macOS 版本和操作步骤；不要发送登录凭据、配对文件或 Wi-Fi 密码。

这是额度星盘 AI Passport 的配套工具；Quota Orbit Mac 桌面主客户端另有独立安装包。本项目与 OpenAI、FoloToy 无官方背书关系。
