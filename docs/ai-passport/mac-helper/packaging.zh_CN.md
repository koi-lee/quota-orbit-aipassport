<p align="right"><strong>简体中文</strong> · <a href="packaging.md">English</a></p>

# Mac 助手可复现打包与兼容检查

0.3.2 安装包误收集了以 macOS 26/27 为最低目标的 Homebrew 依赖。只改主程序目标或 Info.plist 无法解决。本候选管线改用官方 Python 3.13.16 运行库和锁定版本的二进制 wheel，再逐一检查包内 ARM64 Mach-O；源码配对与同步行为不变。

## 构建候选

在 macOS 上使用独立 Python 3.13.16 环境。[官方安装器](https://www.python.org/downloads/release/python-31316/) SHA-256 为 `30666509020b4da0dd8bc2e773255f34d76b7bb80b66960a928d5f6daa0192d7`。不用当前电脑 Homebrew 的解释器或依赖；新建不含系统 site-packages 的虚拟环境。

```sh
python -m pip install --only-binary=:all: -r tools/lan-quota/requirements-macos.txt
python -m unittest discover -s tools/lan-quota -p 'test_*.py'
python tools/lan-quota/build_macos.py --output /tmp/passport-helper-candidate --version 0.3.3 --minimum-macos 11.0
python tools/lan-quota/smoke_macos.py /tmp/passport-helper-candidate/dist/StarshorePassport.app
```

输出目录必须不存在。构建不会安装应用、覆盖旧候选、改配对或写登录项；会生成 ARM64 应用、依赖许可、准确的包清单与包内审计。默认临时签名只用于本地/CI 测试；可用 `--sign-identity` 指定 Developer ID。公开分发仍需公证、装订票据和最终签名/Gatekeeper 检查。

审计拒绝最低系统高于声明目标、缺少系统版本、依赖本机路径、符号链接越界或不含 ARM64 的二进制。读取的是最低系统命令，不把 SDK 版本当最低要求；也不会通过改写版本字段来掩盖不兼容依赖。

## 运行证据与限制

[macOS 兼容工作流](../../../.github/workflows/passport-helper-macos.yml)在标准 Apple 芯片 macOS 14、15 运行器构建并运行候选，检查冻结版桥接模块加载、禁用设备访问的预览启动、三语页面、重复启动、退出释放端口与再次打开。不使用真实账号或 USB；产物包含实际测试系统及清单。配置了工作流不代表已经运行通过。

最低二进制目标 macOS 11 只是打包检查，不能宣称 macOS 11-13 运行、全新电脑安装、登录重启、配对或额度同步已经通过。分别报告实际成功的工作流与真机测试；下载要求应绑定已验证的正式产物，不根据未公证候选改写。

[PyInstaller 兼容说明](https://pyinstaller.org/en/stable/usage.html#making-macos-apps-forward-compatible)建议在拟支持的最旧系统打包；当前工作流最旧运行系统是 macOS 14，说明支持范围时须保留更低版本未测的边界。[公开仓库的标准运行器](https://docs.github.com/en/actions/reference/runners/github-hosted-runners)免费。
