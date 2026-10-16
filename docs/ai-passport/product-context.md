<p align="right"><a href="product-context.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Quota Orbit for AI Passport: product context and source map

Use this map for product questions and implementation lookup without loading every upstream platform document. Development and authorization rules remain in [AGENTS.md](../../AGENTS.md).

## Product boundaries

- This repository contains AI Passport firmware and an optional Apple silicon Mac helper. The separate [Quota Orbit desktop app](https://github.com/koi-lee/quota-orbit) is not required.
- Home and Letter fetch public reset forecasts and completion notices over device Wi-Fi, without a Mac or Codex login. A public completion notice does not establish personal quota recovery or completion of a Codex task.
- Personal quota comes from signed-in Codex on a paired, awake Mac. After initial USB pairing, the helper sends encrypted summaries over the same Wi-Fi. Credentials stay on the Mac. Missing windows are unavailable, not zero; two minutes without updates marks old data.
- Confirm taps on Character update the session star count. With Website wishes enabled, they also anonymously contribute to the website wish count and starlight. Wishes never reset or increase Codex quota; this path is separate from personal quota sync.
- Rendered or simulated media is illustrative, not device footage or proof of current account data. Do not infer implemented behavior from a promotional animation alone.

## Task routes

| Task | Authoritative entry |
|---|---|
| Explain, install or download | [User README](../../README.md); [community play 1250](https://ai-passport.folotoy.cn/plays/1250/) |
| Buttons, Wi-Fi setup, languages and stale states | [Device guide](device-guide/README.md) |
| Mac requirements, pairing, login startup and recovery limits | [Helper guide](mac-helper/README.md) |
| Package compatibility | [Compatibility table](COMPATIBILITY.md); check the live community revision and helper Release before recommending a specific package |
| UI and public notices | [UI](../../main/orbit_ui.c), [model](../../main/orbit_model.c), [language strings](../../main/orbit_language.c) |
| Personal quota transport | [Device receiver](../../main/orbit_quota.c), [Mac helper source and tests](../../tools/lan-quota/README.md) |
| Website wish protocol, retries and privacy | [Wish sync contract](wish-sync.md), [sender](../../main/orbit_wish.c), [service](../../tools/wish-service/README.md) |
| Build, tests or contributions | [Agent engineering routes](../../AGENTS.md#task-specific-context-routing) |

## Before recommending an action

Start first-time users with the two installation paths in the root README, not a source build or Source code archive. Follow the guides for actual button names and Wi-Fi steps; never invent hotspot credentials or addresses.

Source commits do not update community firmware or helper installers. Use public community release notes for firmware and the helper guide's linked Release for its installer. Keep source checks, CI, review, public availability and device acceptance distinct. A successful build or Apple notarization does not establish all reboot, recovery or fresh-computer pairing scenarios; consult the helper guide's evidence and limits.
