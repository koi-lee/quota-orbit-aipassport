<p align="right"><a href="language-support.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Language support

| Product | Simplified Chinese | Traditional Chinese | English | Where to change it | Evidence and limits |
|---|---|---|---|---|---|
| AI Passport firmware source | Yes | Yes | Yes | Device Settings > Language; stored on device | Three-language UI source, font subsets and host rendering checks are included. Community REV-2886 includes these language controls; newer source changes may still be ahead of the public package. Full physical glyph acceptance across every page is not claimed. |
| Passport Mac Sync Assistant source | Yes | Yes | Yes | Local helper page; choice is remembered in the browser profile | Current source includes three-language UI. The 0.3.0 binary language menu and remembered choice were verified in preview mode; this is not a claim of full physical device acceptance. |
| Quota Orbit Mac desktop client | See the [main repository](https://github.com/koi-lee/quota-orbit) | See the [main repository](https://github.com/koi-lee/quota-orbit) | See the [main repository](https://github.com/koi-lee/quota-orbit) | Product-specific | This AI Passport repository does not modify the desktop client. |

The firmware translates fixed interface text. Public announcements and account quota values are shown as supplied by their sources; the device does not claim to translate them. The release includes Traditional Chinese Mac assistant setup instructions. See the [device setup and screen guide](device-guide/README.md).
