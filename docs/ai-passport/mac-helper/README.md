# Quota Orbit · Passport Mac Sync Assistant 0.3.1 (public preview)

<p align="right"><a href="README.zh_CN.md">简体中文</a> · <a href="README.zh_TW.md">Traditional Chinese</a> · <a href="README.md">English</a></p>

Send the personal quota available from your signed-in Codex account on a Mac to a paired AI Passport on the same Wi-Fi, using encrypted summaries. The assistant is optional: public reset notices work without it.

## Download

[Download for Apple silicon](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.1/StarshorePassport-AppleSilicon-0.3.1-notarized.zip) · [Checksum file](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.1/SHA256SUMS.txt) · [Release and all assets](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.1)

This package is Developer ID signed and Apple-notarized, with its notarization ticket stapled and Gatekeeper assessment passed. It remains a public preview; functional test limits are listed below.

## Requirements

Use an Apple silicon Mac with Codex CLI installed and signed in, or the Codex desktop app installed in Applications. Keep the Mac and device on the same Wi-Fi, with the Mac awake, running and online. First pairing needs a USB data cable.

## First use

1. Extract the ZIP and drag `StarshorePassport.app` into Applications. Quit an older running copy before replacing it. The assistant opens its browser control page; if it does not appear, visit http://127.0.0.1:8942/ .
2. Connect the powered-on device by USB and click Connect device on the Mac first. The device may restart. After its home screen returns, hold Confirm, open Settings > Connect Mac, then press Confirm when asked to allow syncing.
3. Sync starts automatically after pairing. Once personal quota appears on Character, unplug USB. The assistant continues syncing when the device starts again. If you manually paused it, click Resume sync on the Mac.
4. On its first normal launch, the assistant registers a login item for the current user. Disable it in System Settings > General > Login Items. Quitting does not unpair the device. To revoke access, select Unpair Mac in device Settings and follow its confirmation prompt.

## If something goes wrong

| What you see | What to do next |
| --- | --- |
| Waiting for device / first sync fails | Complete the USB pairing steps above. Do not remove an existing pairing first. |
| Pairing incomplete | Check the data cable and device power; connect only one Passport. Click Connect on the Mac, wait for the device to boot, then choose Connect Mac in device Settings and confirm permission. |
| Personal quota sync fails | Check that Codex is signed in, then check that the Mac and powered-on device share the same Wi-Fi. The assistant retries automatically; click Resume sync if you paused it manually. |
| The browser page does not open, or shows another program | Confirm the assistant is running and reopen it. If another program still occupies local port 8942, report it; do not enter credentials on an unfamiliar page. |
| After Quit assistant | The page confirms the assistant has exited and stops refreshing. Close that page; reopen the app to run it again. |
| macOS blocks the app | Check that you downloaded `-notarized.zip`. Report the exact system message; do not disable system security. |

The assistant distinguishes missing or unreadable pairing records, an unreachable device, and Codex quota-read failures. A quota-read failure asks you to check installation and sign-in; it does not necessarily mean you are signed out. Closing a browser tab does not quit the assistant; click Quit assistant to stop it.

## Report a problem

[Open a usage question](https://github.com/koi-lee/quota-orbit-aipassport/issues/new?template=passport_helper.yml). Copy these details; use “unknown” for versions you cannot identify:

```text
Assistant version / package filename:
macOS version / Apple silicon model:
Community firmware version:
Steps to reproduce:
Expected result:
Actual result / exact error message:
Does it happen every time:
```

Redact personal quota and account details from screenshots. Do not upload pairing files, sign-in credentials, Wi-Fi passwords, or unreviewed full logs.

## Scope and limitations

- Data refreshes about every 30 seconds. Missing Codex quota windows are not converted into zero. When offline or the Mac sleeps, the device keeps the last snapshot and marks its update age.
- Periodic sync after a device restart was checked on one device. Actual Mac logout/login or reboot startup, and recovery after a real DHCP address change, remain untested.
- Community REV-2886 and later include device address discovery; older REV-2840 does not. Check the [public release notes](https://ai-passport.folotoy.cn/plays/1250/). Actual DHCP address-change recovery remains unverified. Some larger subnets skip scanning; reconnect the device if needed without first revoking pairing.
- Codex credentials and pairing keys stay on the Mac; the device receives only encrypted quota summaries. A public reset announcement does not establish that your own quota recovered.
- When reporting an issue, include assistant, firmware and macOS versions and the steps taken. Never send sign-in credentials, pairing files or Wi-Fi passwords.

This is the AI Passport companion, with a separate installer from the Quota Orbit Mac desktop client. This independent project is not officially endorsed by OpenAI or FoloToy.
