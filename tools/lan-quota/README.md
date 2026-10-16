# Passport Mac Sync Assistant source

<p align="right"><a href="README.zh_CN.md">简体中文</a> · <strong>English</strong></p>

This folder contains the Python source for the Passport companion that sends an encrypted quota summary to a paired AI Passport over the same Wi-Fi network. It is separate from the Quota Orbit Mac desktop client.

For installation and user steps, see the [0.3.2 instructions](../../docs/ai-passport/mac-helper/README.md) ([Chinese (Simplified)](../../docs/ai-passport/mac-helper/README.zh_CN.md), [Chinese (Traditional)](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.2/README.zh_TW.md)). The Apple-silicon pre-release package and checksum are attached to the [`passport-helper-v0.3.2` release](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.2).

## Development checks

Use Python 3 with `cryptography` and `pyserial`, then run from the repository root:

```sh
python3 -m unittest discover -s tools/lan-quota -p 'test_*.py'
```

Pairing keys and user quota data are local state. Never commit pairing files, credentials, real quota snapshots, or unsanitized device logs.

## Earlier fixes (0.3.1)

Version 0.3.1 introduced distinct states for missing or unreadable pairing records, an unreachable device, and a failed Codex quota read. A quota-read failure asks users to check installation and sign-in; it does not assert that they are signed out. Only fixed exit codes reach the UI, never raw worker errors or account data.

After a successful Quit request, the page stops polling, disables actions and shows an explicit exit message in all three languages. A disconnected helper shows a separate message. Duplicate launches recognize the existing helper page; an unrelated occupied port shows a local notice and does not overwrite the login item. Page recognition is a compatibility check, not authentication.

These fixes are included in the signed and notarized 0.3.2 package. The older 0.3.0 package remains available but does not contain them.

## Login-start fix

The helper resolves Codex in standard Homebrew and application locations and supplies Node paths when launched by macOS login services. This fixes a limited login PATH causing quota sync to fail while manual launch succeeds. Version 0.3.2 includes this fix. Login-service sync recovery was verified; a subsequent full Mac reboot remains untested.
