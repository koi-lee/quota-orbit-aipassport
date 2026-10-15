<p align="right"><a href="releasing.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Release and distribution boundaries

This repository contains two separately distributed products: AI Passport firmware and the Passport Mac Sync Assistant. Use distinct tag prefixes so an assistant release cannot accidentally receive a firmware image.

| Tag prefix | Product | Release contents |
|---|---|---|
| `passport-helper-*` | Passport Mac Sync Assistant | Apple silicon Mac helper package and its own checksum, guides and notes. The firmware build workflow must not run for these tags. |
| `firmware-v*` | AI Passport firmware | ESP-IDF-built full device image and firmware release notes. A firmware GitHub release does not update the community play. |

The AI Passport community page remains the documented installation path for the currently published play. Updating that play requires its separate creator-center update process; publishing source or a GitHub firmware release does not change the community version. Never attach a firmware image to a Mac helper release.
