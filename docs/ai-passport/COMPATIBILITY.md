<p align="right"><a href="COMPATIBILITY.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Version and download compatibility

| Item | Public entry | What it is | Compatibility note |
|---|---|---|---|
| AI Passport community firmware | [Play 1250](https://ai-passport.folotoy.cn/plays/1250/) | Installable firmware for the device | Community REV-2886 and later include three-language UI, confirmation screens and `/discover` device address discovery. Consult the public release notes for the installed revision.
| Firmware source | [firmware application source](../../main/) | Editable source based on FoloToy AI Passport | This repository's source can be ahead of the community package. A source update does not publish or install firmware.
| Passport Mac Sync Assistant 0.3.1 | [GitHub pre-release](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.1) | Apple-silicon Mac companion installer | Pre-release; the new `-notarized.zip` build is Developer ID signed and Apple-notarized. Login/restart startup and actual DHCP address recovery remain unverified. DHCP recovery needs matching device firmware with `/discover`.
| Quota Orbit Mac desktop client | [Latest stable release](https://github.com/koi-lee/quota-orbit/releases/latest) | Separate Mac desktop app | Do not replace it with the Passport assistant.

For public announcements, install the community firmware; the Mac assistant is optional. To show personal Codex quota on the device, install the assistant and pair it once over USB.

The assistant 0.3.0 package offers Simplified Chinese, Traditional Chinese and English; its preview-mode language selection and persistence were checked. The release's installation notes are available in all three languages. Device fixed UI strings have three-language source coverage; physical glyph rendering on all settings states has not been exhaustively re-verified for this public source snapshot.
