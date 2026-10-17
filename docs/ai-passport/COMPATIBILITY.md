<p align="right"><a href="COMPATIBILITY.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Version and download compatibility

| Item | Public entry | What it is | Compatibility note |
|---|---|---|---|
| AI Passport community firmware | [Play 1250](https://ai-passport.folotoy.cn/plays/1250/) | Installable firmware for the device | Community REV-2886 and later include three-language UI, confirmation screens and `/discover` device address discovery. Consult the public release notes for the installed revision.
| Firmware source | [firmware application source](../../main/) | Editable source based on FoloToy AI Passport | This repository's source can be ahead of the community package. A source update does not publish or install firmware.
| Passport Mac Sync Assistant 0.3.2 | [GitHub pre-release](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.2) | Apple-silicon Mac companion installer | Pre-release; the new `-notarized.zip` build is Developer ID signed and Apple-notarized. Only macOS 27.0.1 / Apple silicon was verified; older OS versions are not listed as supported. Login-service recovery was verified, but a subsequent full reboot and actual DHCP recovery remain untested. DHCP recovery needs matching device firmware with `/discover`.
| Quota Orbit Mac desktop client | [Latest stable release](https://github.com/koi-lee/quota-orbit/releases/latest) | Separate Mac desktop app | Do not replace it with the Passport assistant.

For public announcements, install the community firmware; the Mac assistant is optional. To show personal Codex quota on the device, install the assistant and pair it once over USB.

The current helper provides Simplified Chinese, Traditional Chinese and English. See the [helper guide](mac-helper/README.md) for package requirements, verification and recovery limits; release installation notes are also available in all three languages. Device strings have three-language coverage, but not every settings state has been rechecked on hardware.
