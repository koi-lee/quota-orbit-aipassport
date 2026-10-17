<p align="right"><a href="README.zh_CN.md">简体中文</a> · <a href="docs/ai-passport/device-guide/README.zh_TW.md">Traditional Chinese</a> · <strong>English</strong></p>

# Quota Orbit · Codex quota and reset reminders for AI Passport

**Keep Codex quota and public reset news on a small screen beside your desk, with fewer window switches.** This play runs on AI Passport hardware. It is not a Mac desktop app.


## Install here

**1. [Install the device play](https://ai-passport.folotoy.cn/plays/1250/)** — Connect AI Passport by USB and use Chrome or Edge on your computer. This is all you need for public notices.

**2. [Get the Mac quota sync assistant](docs/ai-passport/mac-helper/README.md)** — Add this only to see your own quota on Role. The notarized 0.3.3 preview passed basic runtime checks on Apple silicon macOS 14.8.9, 15.7.9 and 27.0.1; macOS 11–13 are untested. The separate Quota Orbit desktop app is not required.


- **Follow announcements:** Wi-Fi lets the device read public forecasts and completion notices independently, with distinct optional sounds. No Mac or Codex sign-in is needed for public news.
- **See your own quota:** add the optional Mac Sync Assistant to show remaining quota and matching lights on the Moonlight Archivist's book.
- **Interact:** press Confirm for stars and a prayer sound, or scan the message screen's code in WeChat to open the Starshore Reset Assistant.

## New here? Start with your goal

Public notices only: **install the device play → set up Wi-Fi with your phone → see “Synced · public notices” on Home**. No Mac is needed. For personal Codex quota, add the Mac helper and complete one USB pairing using the four steps below; unplug after quota appears on Character.

- **Home** shows current status, **Letter** explains the notice and its times, and **Character** shows personal quota and star interactions.
- **Website wishes is on by default on first use.** Character taps upload anonymous event IDs to trigger [website starlight](https://www.starshoreai.com/quota-orbit/), without sending your account or personal quota. Hold Confirm → Settings → Website wishes to turn it off: this stops new events and clears pending ones; already-counted anonymous events cannot be withdrawn per person. [Data and retry details](docs/ai-passport/wish-sync.md). Wishes do not increase quota.
- No source build or Python installation is needed. [Setup steps](#first-use-in-four-steps) · [Helper troubleshooting](docs/ai-passport/mac-helper/README.md).

## See it in motion

<p align="center"><img src="docs/ai-passport/media/wish-sync-v1.gif" width="600" alt="Simulated AI Passport star taps and the website starlight response"></p>

A silent 6.4-second preview of Role taps and the website wish effect. The shell is an AI illustration; screens and linkage use simulated data, not device footage. Wishes do not increase Codex quota.

[Download the full 40-second V1 video with music](docs/ai-passport/media/demo-rev3012.mp4) · [Visual setup guide (Chinese)](docs/ai-passport/media/quick-start-rev3012.png)

| Public announcements: Home / Letter | Personal quota: Character / book lights |
|---|---|
| ![Example public reset notice](docs/ai-passport/device-guide/images/home.png) | ![Example personal quota](docs/ai-passport/device-guide/images/quota-week-only.png) |

These are code-rendered examples with sample data, not device photos; some screenshots retain earlier wording. **A public reset notice does not confirm your account has recovered. Star taps do not increase quota.**

[View the character quota and prayer interaction example](docs/ai-passport/device-guide/images/role-prayer.jpg)

## What do I install?

You need an AI Passport device. Choose the first item for public news; add the second for personal quota. Without hardware, start with the [official FoloToy AI Passport introduction](https://ai-passport.folotoy.cn/) before deciding whether to buy.

| Purpose | Install on | Entry |
|---|---|---|
| Required: Quota Orbit firmware | AI Passport hardware | [Community play 1250 → Install latest](https://ai-passport.folotoy.cn/plays/1250/) |
| Optional: Passport Mac Sync Assistant 0.3.3 | Apple silicon Mac | [Download and setup guide](docs/ai-passport/mac-helper/README.md) · [Release assets](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.3) |

Firmware does not install on a Mac. **Source code archives are not installers.** The helper installer is `StarshorePassport-AppleSilicon-0.3.3-notarized.zip`; Python is not required.

The new `-notarized.zip` helper is Developer ID signed and Apple-notarized, and remains a pre-release. If macOS blocks normal opening, stop and report the message rather than disabling system protection. No verified helper installer is provided for Intel Macs or Windows.

## First use in four steps

1. **Install and connect Wi-Fi.** Open the community play in Chrome or Edge on your computer, connect the powered-on device with a USB data cable, choose Install latest and select your device. Keep it connected until installation finishes. This replaces the current play and may clear settings. On your phone, join the device's displayed `Starshore-…` hotspot using its displayed password. Open [http://192.168.4.1](http://192.168.4.1) in the phone browser, enter your home **2.4 GHz Wi-Fi**, and select Connect and save. “Synced · public notices” confirms success. Retry incorrect entries; after timeout or for a new network, hold Confirm → Settings → Wi-Fi setup, then confirm.
2. **Read messages.** Up/Down switches Home, Letter and Character. Confirm on Letter opens the mini-program code; scan with WeChat for detail, then press Confirm to return. Hold Confirm for Settings to enable sound or select Simplified Chinese, Traditional Chinese or English.
3. **Add personal quota if wanted.** Sign in to Codex CLI on the Mac, or use the Codex desktop app in Applications. Keep the Mac and device on the same Wi-Fi. Install and open the [helper](docs/ai-passport/mac-helper/README.md), connect USB and click Connect device on the Mac first. Wait for hardware startup, then select Connect Mac in hardware Settings and approve with Confirm.
4. **Unplug after quota appears.** Sync starts automatically after pairing. Quota on Character confirms success; you can remove USB. Keep the Mac awake and the helper running. If you paused manually, click Resume sync. Tap Confirm on Character for stars; revoke pairing through hardware Settings when no longer wanted.

Public news is checked about every **5 minutes**; personal quota syncs about every **30 seconds**. Letter's “Last sync” is the latest successful public fetch, not a clock. Mac sleep or two minutes without a quota update marks old data and greys the book lights. Missing windows are not filled with zero.

## More help

- [Device screens, controls, Wi-Fi setup and sync diagrams](docs/ai-passport/device-guide/README.md)
- [Community revision and text-only access](docs/ai-passport/public-release.md)
- [Version compatibility](docs/ai-passport/COMPATIBILITY.md) · [Language support](docs/ai-passport/language-support.md)
- [Mac helper source](tools/lan-quota/) · [Firmware source](main/) · [Build and release notes](docs/development/releasing.md)

Consult the community release notes and [compatibility table](docs/ai-passport/COMPATIBILITY.md) for the installed version; repository updates do not publish community firmware. Current helper verification and recovery limits are recorded in the [helper guide](docs/ai-passport/mac-helper/README.md). Codex credentials stay on the Mac; the device receives only an encrypted quota summary.

## Looking for the Mac desktop app?

The independent desktop-pet and menu-bar app is in the [Quota Orbit Mac repository](https://github.com/koi-lee/quota-orbit). **It is not required to use Passport or its sync helper.** See the [Quota Orbit website](https://www.starshoreai.com/quota-orbit/) for the wider product.

## For agents and developers

Read [AGENTS.md](AGENTS.md), then follow the [product context and source map](docs/ai-passport/product-context.md). Installation, personal quota and website wishes have separate entry points; upstream hardware examples do not describe this play’s current UI.

## Repository contents

`main/`, `components/` and `assets/` contain the device play based on [FoloToy/ai-passport](https://github.com/FoloToy/ai-passport); `tools/lan-quota/` contains the Mac helper; `docs/ai-passport/` contains setup, screen and language guides. Users should use the community and helper links above; source is for development.

## License and assets

The Passport assistant source uses [AGPL-3.0](tools/lan-quota/LICENSE). Firmware source retains FoloToy's MIT license and original notices in the root `LICENSE`; fonts retain their accompanying licenses. The Moonlight Archivist character and official WeChat mini-program code are Quota Orbit product assets authorized for public distribution as part of this project only. That does not grant a separate license to use them outside this project; see the [character notice](assets/character/README.md) and [mini-program code notice](assets/wechat/README.md).

Quota Orbit is an independent project and is not affiliated with or endorsed by OpenAI or FoloToy.
