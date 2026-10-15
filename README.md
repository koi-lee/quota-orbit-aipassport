<p align="right"><a href="README.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Quota Orbit · AI Passport

For people who use Codex regularly: check public reset announcements on a small AI Passport screen. Add the Mac sync assistant to see your own remaining quota at a glance, with fewer window switches.

## What is Quota Orbit?

**Quota Orbit helps Codex users check their quota and follow reset announcements.** Its Mac desktop app uses a desktop companion and menu bar to show how much quota remains, when it recovers, and quota changes during the current session. Public reset announcements help you follow updates and plan your work.

[Explore Quota Orbit](https://www.starshoreai.com/quota-orbit/) · [Mac desktop app and guide](https://github.com/koi-lee/quota-orbit)

## What can AI Passport do for you?

Keep AI Passport on your desk or carry it with you to:

- **Check reset progress without repeatedly opening a news page.** Over Wi-Fi, the device independently reads public announcements. Its home and message screens show the latest message, publication time, and estimated or completion times explicitly provided by the source. Public announcements require neither a Mac nor a Codex login.
- **Hear when a new announcement arrives.** Enable sounds for distinct alerts on valid new forecasts and completion announcements. Initial sync, duplicates and old messages do not trigger repeated alerts. Public messages are checked about every five minutes, not pushed instantly.
- **Glance at personal quota with fewer window switches.** With the optional Passport Mac Sync Assistant installed and paired, the character screen shows remaining quota values and a book light based on the quota summary from Codex signed in on your Mac, helping you plan your usage.
- **Open more detail on your phone when needed.** Press confirm on the message screen and scan the mini-program code with WeChat to continue in the Starshore Reset Assistant.

**Personal quota sync has requirements:** pair once over USB, then unplug the cable. Your Mac must still be running and awake, signed in to Codex, on the same Wi-Fi as the device, and running the sync assistant. If the Mac sleeps or sync stops, the device marks the data as stale; it cannot independently query personal quota away from the Mac.

A public reset announcement does not mean your personal quota has recovered. Quota Orbit displays information; it does not increase quota or perform resets for you.

## Choose what you need

| Goal | Install | Download |
|---|---|---|
| Use Quota Orbit on AI Passport | **AI Passport community firmware**, installed on the hardware; it is not a Mac installer | [Community play 1250 · install firmware](https://ai-passport.folotoy.cn/plays/1250/) |
| Sync personal Codex quota from Mac to Passport | **Passport Mac Sync Assistant**, optional and only for personal quota | [0.3.0 pre-release (Apple silicon)](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.0) · [Chinese (Simplified)](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.0/README.zh_CN.md) · [Chinese (Traditional)](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.0/README.zh_TW.md) · [English instructions](https://github.com/koi-lee/quota-orbit-aipassport/releases/download/passport-helper-v0.3.0/README.en.md) |
| Use the Mac desktop pet, menu bar and quota orbit | **Quota Orbit Mac desktop app**; AI Passport is not required | [Mac desktop public trial](https://github.com/koi-lee/quota-orbit/releases/latest) |

The Mac assistant is not needed to read public announcements. Installing from the community replaces the device's current play and may clear existing settings; read the community installation notes first.

## Setup and screen guide

- [Device screens, states, Wi-Fi setup and sync flows](docs/ai-passport/device-guide/README.md) ([Chinese (Simplified)](docs/ai-passport/device-guide/README.zh_CN.md))
- [Language support matrix](docs/ai-passport/language-support.md)
- [Passport Mac Sync Assistant source](tools/lan-quota/)
- [AI Passport firmware source](main/)
- [Release naming and distribution boundaries](docs/development/releasing.md)

Use the temporary hotspot shown on the device to configure Wi-Fi at `http://192.168.4.1`; only 2.4 GHz Wi-Fi is supported. Personal quota sync requires one initial USB pairing confirmed on the device. After pairing, the Mac and Passport sync automatically while on the same Wi-Fi.

## Versions and compatibility

- The community page provides the installable public firmware. The firmware source at this repository root is evolving, not a claim that the community package has been updated. Source changes do not update the community release automatically.
- Passport Mac Sync Assistant `0.3.0` is an Apple silicon pre-release. Some behavior around login startup, Mac restart and network address changes has not been fully verified. Automatic device rediscovery also requires firmware with the `/discover` endpoint, which is not in the current public community firmware.
- The assistant is not Developer ID signed or Apple notarized; macOS may ask you to confirm on first launch. Intel Macs are unverified.
- The desktop app and Passport assistant are separate products. The desktop app's public trial and Latest download remains in [koi-lee/quota-orbit](https://github.com/koi-lee/quota-orbit). Do not replace the desktop app with the Passport assistant.

## Contents

- Root `main/`, `components/` and `assets/`: AI Passport application derived from [FoloToy/ai-passport](https://github.com/FoloToy/ai-passport), with the upstream license and notices retained.
- `tools/lan-quota/`: Passport Mac Sync Assistant source. Pairing keys and quota data stay on the user's Mac and are not included in this repository or installer.
- `docs/`: setup, usage, screen states, language support and flows. Example screens are generated by the firmware's native LVGL host renderer with sample values; they are not device photos or real account data.
- GitHub's automatic Source code archives contain **this AI Passport repository**. The community page is the device install path; the Mac assistant installer is in Releases.

## License and assets

The Passport assistant source uses [AGPL-3.0](tools/lan-quota/LICENSE). Firmware source retains FoloToy's MIT license and original notices in the root `LICENSE`; fonts retain their accompanying licenses. The Moonlight Archivist character and official WeChat mini-program code are Quota Orbit product assets authorized for public distribution as part of this project only. That does not grant a separate license to use them outside this project; see the [character notice](assets/character/README.md) and [mini-program code notice](assets/wechat/README.md).

Quota Orbit is an independent project and is not affiliated with or endorsed by OpenAI or FoloToy.
