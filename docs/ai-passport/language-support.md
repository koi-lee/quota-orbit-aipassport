<p align="right"><a href="language-support.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Language support

| Product | Simplified Chinese | Traditional Chinese | English | Where to change it | Evidence and limits |
|---|---|---|---|---|---|
| AI Passport firmware source | Yes | Yes | Yes | Device Settings > Language; stored on device | Three-language UI source, font subsets and host rendering checks are included. Community firmware revision 2840 predates some source changes; installing source requires building firmware. Full physical glyph acceptance across every page is not claimed. |
| Passport Mac Sync Assistant source | Yes | Yes | Yes | Local helper page; choice is remembered in the browser profile | Current source includes three-language UI. The public 0.3.0 binary predates this language work; its runtime language menu behavior has not been verified. |
| Quota Orbit Mac desktop client | See the [main repository](https://github.com/koi-lee/quota-orbit) | See the [main repository](https://github.com/koi-lee/quota-orbit) | See the [main repository](https://github.com/koi-lee/quota-orbit) | Product-specific | This AI Passport repository does not modify the desktop client. |

The firmware translates fixed interface text. Public announcements and account quota values are shown as supplied by their sources; the device does not claim to translate them. The release includes Traditional Chinese Mac assistant setup instructions. See the [device setup and screen guide](device-guide/README.md).
