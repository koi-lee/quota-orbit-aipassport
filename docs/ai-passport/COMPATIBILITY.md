<p align="right"><a href="COMPATIBILITY.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Version and download compatibility

| Item | Public entry | What it is | Compatibility note |
|---|---|---|---|
| AI Passport community firmware | [Play 1250](https://ai-passport.folotoy.cn/plays/1250/) | Installable firmware for the device | Community REV-2886 and later include three-language UI, confirmation screens and `/discover` device address discovery. Consult the public release notes for the installed revision.
| Firmware source | [firmware application source](../../main/) | Editable source based on FoloToy AI Passport | This repository's source can be ahead of the community package. A source update does not publish or install firmware.
| Passport Mac Sync Assistant 0.3.3 | [GitHub pre-release](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.3) | Apple-silicon Mac companion installer | Pre-release; signed, notarized and stapled. Basic runtime checks passed on Apple silicon macOS 14.8.9, 15.7.9 and 27.0.1; 11–13 are untested and Intel unsupported. The author reports reboot and network recovery with the existing installation, not fresh acceptance of 0.3.3 or fresh-Mac pairing. Actual DHCP recovery remains untested and needs `/discover`. |
| Quota Orbit Mac desktop client | [Latest stable release](https://github.com/koi-lee/quota-orbit/releases/latest) | Separate Mac desktop app | Do not replace it with the Passport assistant.

For public announcements, install the community firmware; the Mac assistant is optional. To show personal Codex quota on the device, install the assistant and pair it once over USB.

The current helper provides Simplified Chinese, Traditional Chinese and English. See the [helper guide](mac-helper/README.md) for package requirements, verification and recovery limits; release installation notes are also available in all three languages. Device strings have three-language coverage, but not every settings state has been rechecked on hardware.
