<p align="right"><a href="public-release.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Community revision and text-only access

[Community play 1250](https://ai-passport.folotoy.cn/plays/1250/) remains the browser installation entry. If an agent or text reader stops at the loading placeholder, read the [public JSON endpoint used by that page](https://ai-passport.folotoy.cn/api/plays/id/1250). Unauthenticated content access was verified on 2026-10-17. Do not invent a separate `/release-notes/` page.

## What to check

- Inspect `play.revisionId`, `play.status` and `play.updatedAt`. Instructions are in `play.instructions`, current changes in `play.changelog`, and previous changes in `play.changelogHistory`.
- Firmware format, size and SHA-256 are in `play.firmware`. This is device firmware, not the Mac helper; the latter has a [separate Release](https://github.com/koi-lee/quota-orbit-aipassport/releases/tag/passport-helper-v0.3.3).
- The endpoint is for reading and verification, not installation. Install through the play page in computer Chrome / Edge with a USB data cable and the powered-on device, following the device picker. Follow the actual page if login is requested; this audit did not repeat a firmware installation.

## Verification snapshot: 2026-10-17

This dated record is not a promise that the revision remains latest. Check the endpoint before recommending installation. The community controls this public interface and may change it; fall back to the play page or report a failure to the author.

- Project: 1250; public revision: REV-3012; status: `published`.
- Updated (UTC): `2026-10-16T12:53:29+00:00`.
- Full firmware: `esp-merged-0x0`, 5,412,432 bytes.
- SHA-256: `a0dad45fbe8845ffa4d2bbf94fa8ced983ecd844dfb3fdbad83f50865e02c78b`.

### Published changes

Compact all three screens: group Home information in a card, bring Letter explanations and times closer together, and extend its divider. Enlarge the remaining percentage for a single quota window and add a quota bar; keep both values when two windows are available and gray out stale data. Move character artwork, book lights and wish effects together without changing controls or sync behavior.
Updated setup instructions link to the Apple-notarized Mac assistant 0.3.2; its login-start sync fix is delivered in the separate helper package.

[Device setup guide](device-guide/README.md) · [Mac helper requirements and limits](mac-helper/README.md) · [Compatibility table](COMPATIBILITY.md)
