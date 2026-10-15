# Passport Mac 同步助手源码

[English](README.md)

此目录包含 Passport 配套助手的 Python 源码，用于将个人额度摘要加密发送到同一 Wi-Fi 下已配对的 AI Passport。它与 Quota Orbit Mac 主客户端相互独立。

安装和上手步骤请看[已发布的 0.3.0 说明](../../docs/ai-passport/mac-helper/README.zh_CN.md)（[繁體中文](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.0/README.zh_TW.md) · [English](../../docs/ai-passport/mac-helper/README.md)）。Apple 芯片预发布安装包和校验值位于 [`passport-helper-v0.3.0` Release](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.0)。

## 开发检查

使用 Python 3，并安装 `cryptography` 和 `pyserial`；在仓库根目录运行：

```sh
python3 -m unittest discover -s tools/lan-quota -p 'test_*.py'
```

配对密钥和用户额度数据属于本机状态。不要提交配对文件、凭据、真实额度快照或未经清理的设备日志。

## 尚未发布的助手修复

源码候选区分尚未配对、配对记录无法读取、设备暂时不可达和 Codex 额度读取失败。读取失败提示检查安装及登录，不直接断言用户已退出登录。界面只接收固定退出码，不显示后台原始错误或账号数据。

退出请求成功后，页面停止轮询、禁用操作，并以三语明确显示已退出；无法连接助手另行提示。重复打开时识别已有助手页面；其他程序占用端口时显示本地提示，不覆盖登录启动项。页面识别仅用于兼容判断，不作为身份认证。

这些变更尚未包含在公开的0.3.0安装包中，需新包完成构建、签名、公证和发布后才对下载用户生效。
