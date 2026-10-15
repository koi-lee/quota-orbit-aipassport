<p align="right"><a href="COMPATIBILITY.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Version and download compatibility

| Item | Public entry | What it is | Compatibility note |
|---|---|---|---|
| AI Passport community firmware | [Play 1250](https://ai-passport.folotoy.cn/plays/1250/) | Installable firmware for the device | Community revision 2840 is published. It does not include the `/discover` endpoint used by automatic DHCP address recovery in the current helper source.
| Firmware source | [firmware application source](../../main/) | Editable source based on FoloToy AI Passport | This repository's source can be ahead of the community package. A source update does not publish or install firmware.
| Passport Mac Sync Assistant 0.3.0 | [GitHub pre-release](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.0) | Apple-silicon Mac companion installer | Pre-release; not Developer ID signed or notarized. Login/restart startup and actual DHCP address recovery remain unverified. DHCP recovery needs matching device firmware with `/discover`.
| Quota Orbit Mac desktop client | [Latest stable release](https://github.com/koi-lee/quota-orbit/releases/latest) | Separate Mac desktop app | Do not replace it with the Passport assistant.

For public announcements, install the community firmware; the Mac assistant is optional. To show personal Codex quota on the device, install the assistant and pair it once over USB.

The assistant UI source has Simplified Chinese, Traditional Chinese and English. The 0.3.0 package was built before the source UI language work was completed; its runtime UI language behavior has not been verified. The release's installation notes are available in all three languages. Device fixed UI strings have three-language source coverage; physical glyph rendering on all settings states has not been exhaustively re-verified for this public source snapshot.
