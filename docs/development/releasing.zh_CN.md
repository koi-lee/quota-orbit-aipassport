<p align="right"><a href="releasing.md">English</a> · <strong>简体中文</strong></p>

# 发布与分发边界

本仓库包含两个独立分发的产品：AI Passport 固件与 Passport Mac 同步助手。标签使用不同前缀，避免助手发布时误附设备固件。

| 标签前缀 | 产品 | Release 内容 |
|---|---|---|
| `passport-helper-*` | Passport Mac 同步助手 | Apple 芯片 Mac 助手包、对应校验值、说明和更新日志；固件构建工作流不得对这些标签运行。 |
| `firmware-v*` | AI Passport 固件 | ESP-IDF 构建的设备完整镜像与固件更新说明。GitHub 固件 Release 不会更新社区作品。 |

当前已公开玩法的安装入口仍是 AI Passport 社区页。更新社区作品需要单独走创作者中心的更新流程；公开源码或发布 GitHub 固件都不会自动更改社区版本。不要把固件镜像附加到 Mac 助手 Release。
