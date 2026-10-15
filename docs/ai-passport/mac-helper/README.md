# Quota Orbit · Passport Mac Sync Assistant 0.3.0 (public preview)

<p align="right"><a href="README.zh_CN.md">简体中文</a> · <a href="README.zh_TW.md">Traditional Chinese</a> · <a href="README.md">English</a></p>

Send the personal quota available from your signed-in Codex account on a Mac to a paired AI Passport on the same Wi-Fi, using encrypted summaries. The assistant is optional: public reset notices work without it.

## Download

[Download for Apple silicon](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.0/StarshorePassport-AppleSilicon-0.3.0.zip) · [Checksum file](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.0/SHA256SUMS.txt) · [Release and all assets](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.0)

This GitHub Pre-release is ad-hoc signed locally, not Developer ID signed or Apple-notarized. If macOS blocks normal opening, stop installation and report the system message. Do not disable system security protections.

## Requirements

Use an Apple silicon Mac with Codex CLI installed and signed in, or the Codex desktop app installed in Applications. Keep the Mac and device on the same Wi-Fi, with the Mac awake, running and online. First pairing needs a USB data cable.

## First use

1. Extract the ZIP and drag `StarshorePassport.app` into Applications. Quit an older running copy before replacing it. The assistant opens its browser control page; if it does not appear, visit http://127.0.0.1:8942/ .
2. Connect the powered-on device by USB and click Connect device on the Mac first. The device may restart. After its home screen returns, hold Confirm, open Settings > Connect Mac, then press Confirm when asked to allow syncing.
3. Sync starts automatically after pairing. Once personal quota appears on Character, unplug USB. The assistant continues syncing when the device starts again. If you manually paused it, click Resume sync on the Mac.
4. On its first normal launch, the assistant registers a login item for the current user. Disable it in System Settings > General > Login Items. Quitting does not unpair the device. To revoke access, select Unpair Mac in device Settings and follow its confirmation prompt.

## Scope and limitations

- Data refreshes about every 30 seconds. Missing Codex quota windows are not converted into zero. When offline or the Mac sleeps, the device keeps the last snapshot and marks its update age.
- Periodic sync after a device restart was checked on one device. Actual Mac logout/login or reboot startup, and recovery after a real DHCP address change, remain untested.
- Address recovery needs a paired-device discovery endpoint in device firmware. Older community REV-2840 lacks it but can still use the saved address. A newer candidate includes it; check the [community changelog](https://ai-passport.folotoy.cn/plays/1250/) to see whether it is public. Some large network subnets are not scanned automatically and may require reconnecting after an address change.
- Codex credentials and pairing keys stay on the Mac; the device receives only encrypted quota summaries. A public reset announcement does not establish that your own quota recovered.
- When reporting an issue, include assistant, firmware and macOS versions and the steps taken. Never send sign-in credentials, pairing files or Wi-Fi passwords.

This is the AI Passport companion, with a separate installer from the Quota Orbit Mac desktop client. This independent project is not officially endorsed by OpenAI or FoloToy.
