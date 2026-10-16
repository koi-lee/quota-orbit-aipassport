# Passport Mac 同步助手源码

[English](README.md)

此目录包含 Passport 配套助手的 Python 源码，用于将个人额度摘要加密发送到同一 Wi-Fi 下已配对的 AI Passport。它与 Quota Orbit Mac 主客户端相互独立。

安装和上手步骤请看[0.3.1 安装说明](../../docs/ai-passport/mac-helper/README.zh_CN.md)（[繁體中文](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.1/README.zh_TW.md) · [English](../../docs/ai-passport/mac-helper/README.md)）。Apple 芯片预发布安装包和校验值位于 [`passport-helper-v0.3.1` Release](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.1)。

## 开发检查

使用 Python 3，并安装 `cryptography` 和 `pyserial`；在仓库根目录运行：

```sh
python3 -m unittest discover -s tools/lan-quota -p 'test_*.py'
```

配对密钥和用户额度数据属于本机状态。不要提交配对文件、凭据、真实额度快照或未经清理的设备日志。

## 助手 0.3.1 修复

0.3.1 区分尚未配对、配对记录无法读取、设备暂时不可达和 Codex 额度读取失败。读取失败提示检查安装及登录，不直接断言用户已退出登录。界面只接收固定退出码，不显示后台原始错误或账号数据。

退出请求成功后，页面停止轮询、禁用操作，并以三语明确显示已退出；无法连接助手另行提示。重复打开时识别已有助手页面；其他程序占用端口时显示本地提示，不覆盖登录启动项。页面识别仅用于兼容判断，不作为身份认证。

这些修复已包含在签名并公证的 0.3.1 安装包中。旧版 0.3.0 仍保留下载，但不含这些修复。

## 待发布的登录启动修复

下一版助手会从标准 Homebrew 和应用目录查找 Codex，并为登录启动补齐 Node 路径，修复手动运行正常、Mac 登录启动后却无法同步的问题。本地 0.3.2 候选已签名，尚未公证或公开发布；公开 0.3.1 不含此修复。
