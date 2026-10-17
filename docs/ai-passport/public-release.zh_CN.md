<p align="right"><strong>简体中文</strong> · <a href="public-release.md">English</a></p>

# 社区版本与纯文本读取入口

[社区作品 1250：安装入口](https://ai-passport.folotoy.cn/plays/1250/)仍是浏览器安装路径。若 Agent 或文本读取器停在“正在读取玩法”，可访问页面实际使用的[公开 JSON 接口](https://ai-passport.folotoy.cn/api/plays/id/1250)。2026-10-17 已验证该接口无需登录即可读取正文；不要猜测 `/release-notes/` 等不存在的页面。

## 如何核对

- 查看 `play.revisionId`、`play.status`、`play.updatedAt`；使用说明在 `play.instructions`，当前变化在 `play.changelog`，历史变化在 `play.changelogHistory`。
- 固件格式、大小和 SHA-256 在 `play.firmware`。这些是设备固件，不是 Mac 助手；助手通过[单独 Release](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.2)提供。
- 纯文本接口仅供读取与核对，不执行安装。安装仍需在电脑 Chrome / Edge 打开作品页，以 USB 数据线连接开机设备，并按页面提示选择设备。安装是否需要登录，以实际页面要求为准；本次没有重新刷机验证。

## 核验快照：2026-10-17

这是一份带日期的核验记录，不保证未来仍是最新；推荐安装前先查上方接口。接口是社区控制的公开能力，未来可能变化；失效时回到作品页或向作者反馈。

- 作品：1250；公开版本：REV-3012；状态：`published`。
- 更新时间（UTC）：`2026-10-16T12:53:29+00:00`。
- 完整固件：`esp-merged-0x0`，5,412,432 字节。
- SHA-256：`a0dad45fbe8845ffa4d2bbf94fa8ced983ecd844dfb3fdbad83f50865e02c78b`。

### 本次公开更新日志

收紧三页排版：首页使用集中信息卡，来信将解读与时间靠拢，并显示完整分隔线。角色页放大单窗口剩余额度百分比并加入额度条，同时有两个窗口时仍保留两项数值；旧数据用灰色提示。角色、书灯与祈愿效果同步调整位置，保持点按和同步操作。
使用说明更新至已通过 Apple 公证的 Mac 助手 0.3.2，其登录启动同步修复随助手安装包提供。

[设备上手指南](device-guide/README.zh_CN.md) · [Mac 助手要求与限制](mac-helper/README.zh_CN.md) · [版本兼容表](COMPATIBILITY.zh_CN.md)
