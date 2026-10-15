# Passport Mac Sync Assistant source

<p align="right"><a href="README.zh_CN.md">简体中文</a> · <strong>English</strong></p>

This folder contains the Python source for the Passport companion that sends an encrypted quota summary to a paired AI Passport over the same Wi-Fi network. It is separate from the Quota Orbit Mac desktop client.

For installation and user steps, see the [published 0.3.0 instructions](../../docs/ai-passport/mac-helper/README.md) ([Chinese (Simplified)](../../docs/ai-passport/mac-helper/README.zh_CN.md), [Chinese (Traditional)](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.0/README.zh_TW.md)). The Apple-silicon pre-release package and checksum are attached to the [`passport-helper-v0.3.0` release](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.0).

## Development checks

Use Python 3 with `cryptography` and `pyserial`, then run from the repository root:

```sh
python3 -m unittest discover -s tools/lan-quota -p 'test_*.py'
```

Pairing keys and user quota data are local state. Never commit pairing files, credentials, real quota snapshots, or unsanitized device logs.

## Unreleased assistant fixes

The source candidate distinguishes missing or unreadable pairing records, an unreachable device, and a failed Codex quota read. A quota-read failure asks users to check installation and sign-in; it does not assert that they are signed out. Only fixed exit codes reach the UI, never raw worker errors or account data.

After a successful Quit request, the page stops polling, disables actions and shows an explicit exit message in all three languages. A disconnected helper shows a separate message. Duplicate launches recognize the existing helper page; an unrelated occupied port shows a local notice and does not overwrite the login item. Page recognition is a compatibility check, not authentication.

These changes are not included in the published 0.3.0 package until a new package is built, signed, notarized and released.
