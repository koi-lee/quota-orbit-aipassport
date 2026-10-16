<p align="right"><strong>简体中文</strong> · <a href="product-context.md">English</a></p>

# 额度星盘 AI Passport：产品上下文与源码导航

用于回答产品问题或定位实现，无需加载全部上游平台文档。开发与授权规则仍以 [AGENTS.zh_CN.md](../../AGENTS.zh_CN.md) 为准。

## 产品边界

- 本仓库包含 AI Passport 固件与选装的 Apple 芯片 Mac 助手，不要求安装独立的[额度星盘桌面客户端](https://github.com/koi-lee/quota-orbit)。
- 首页和来信通过设备 Wi-Fi 获取公开重置预告与完成公告，不需要 Mac 或 Codex 登录。公开完成公告不能证明个人额度已恢复，也不是 Codex 任务完成提醒。
- 个人额度来自已配对且保持唤醒的 Mac 上已登录的 Codex。首次 USB 配对后，助手通过同一 Wi-Fi 发送加密摘要；登录凭据留在 Mac。缺失窗口显示不可用，不补成零；两分钟没有更新后标记旧数据。
- 角色页短按确定会增加本次点亮星数；开启「官网祈愿」时，也会匿名计入官网祈愿并展示星光。祈愿不会重置或增加 Codex 额度，与个人额度同步是两条独立路径。
- 渲染或模拟的宣传素材不是实机录像，也不能证明当前账号数据。不能仅凭宣传动画推断功能已经实现。

## 按任务读取

| 任务 | 事实入口 |
|---|---|
| 介绍、安装或下载 | [用户首页](../../README.zh_CN.md)；[社区作品 1250](https://ai-passport.folotoy.cn/plays/1250/) |
| 按键、配网、语言与旧数据状态 | [设备指南](device-guide/README.zh_CN.md) |
| Mac 要求、配对、登录启动与恢复限制 | [助手指南](mac-helper/README.zh_CN.md) |
| 安装包兼容性 | [兼容表](COMPATIBILITY.zh_CN.md)；推荐具体安装包前核对线上社区版本与助手 Release |
| 界面与公开公告 | [界面](../../main/orbit_ui.c)、[状态模型](../../main/orbit_model.c)、[多语言文案](../../main/orbit_language.c) |
| 个人额度传输 | [设备额度接收](../../main/orbit_quota.c)、[Mac 助手源码与测试](../../tools/lan-quota/README.zh_CN.md) |
| 官网祈愿协议、重试与隐私 | [祈愿同步约定](wish-sync.zh_CN.md)、[发送实现](../../main/orbit_wish.c)、[接收服务](../../tools/wish-service/README.zh_CN.md) |
| 构建、测试或贡献 | [Agent 工程路由](../../AGENTS.zh_CN.md#按任务加载上下文) |

## 推荐操作前核对

新用户先看首页的两种安装路径，不要求其编译固件或下载 Source code 源码压缩包。实际按钮名、配网步骤以指南为准，不猜热点凭据或地址。

源码提交不会更新社区固件或助手安装包。固件以社区公开更新日志为准，助手以指南链接的 Release 为准。区分源码检查、CI、审核、公开可下载和真机验收。构建或 Apple 公证通过不能证明全部重启、恢复或全新电脑配对场景均已验证；相关证据与限制见助手指南。
