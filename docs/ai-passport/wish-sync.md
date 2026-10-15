<p align="right"><a href="wish-sync.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Website wish sync

Website wishes send Character-page taps to the Quota Orbit website through a dedicated HTTPS endpoint. Mac software is not needed. The receiver and website animation are deployed; community availability depends on the published firmware revision.

## User path

On first use, website wishes are on. Hold Confirm > Settings > Website wishes to disable them or review the explanation when re-enabling. Each valid short press on Character adds one wish locally and queues an anonymous event. Mac software is not needed. Returning to Settings shows pending/error state. Disabling stops new events and clears pending records; an already in-flight request can still complete, and publicly counted anonymous events cannot be retracted individually. Existing stars, quota, sound and book-light behavior remain separate.

The setting defaults on for first use: character-page taps are sent anonymously to the website. It can be disabled in Settings. Updates preserve saved on/off choices. When re-enabling, the explanation defaults to Cancel; a long press cancels. Missing endpoint configuration shows Service not ready rather than claiming sync success. No historic star count is uploaded. The global number is tap count, not people, personal quota or a reset probability.

## Persistence boundaries

A per-enable random seed and monotonic sequence derive stable event IDs across retries and restart. The seed stays on hardware. Acknowledgement removes only the exact sent prefix after saving succeeds. The seed/sequence state uses a separate NVS namespace; it never erases shared NVS or changes Wi-Fi/Mac keys. Recent clicks are checkpointed in batches; unexpected power failure before checkpoint, storage errors or input queue overflow can lose recent events. Pending capacity is 10,000, with a visible warning on overflow. Do not claim lossless or unlimited offline storage.

## Evidence and acceptance

Host tests drive the actual C queue model and a loopback HTTP ingress with a temporary shared website-style SQLite database: 20 taps add 20; a lost response/reopen/retry adds none; 3 offline taps after reopening add 3; opt-out clears pending and prevents additions. Additional tests cover concurrency, malformed payloads, schema refusal, limits, capacity and overflow. Native renders check the opt-in and settings states in Simplified Chinese, Traditional Chinese and English. A controlled original-device test confirmed 10 taps added exactly 10 website events. A later rapid-input repair processed all 108 observed taps with zero queue drops; the owner confirmed numbers, animation and sound. These observations do not establish long-duration, power-loss or other-device reliability.

The current private website source uses the counter table with nullable country and the live read-only API exposes total/cursor/recent fields. The candidate supports that known schema without migration; the production database schema, absence of triggers, ownership and service identity were inspected before deployment. An online SQLite backup passed integrity checks. Public method/invalid-payload checks and the existing read endpoint passed without adding test events. Subsequent explicitly authorized physical-device tests added real wishes to the website.

## Integration and rollback

See the [ingress protocol and runbook](../../tools/wish-service/README.md). Candidate files are the Python ingress, a new restricted service unit and one new HTTPS location. They preserve the existing browser endpoint and counter service. The deployed default `CONFIG_ORBIT_WISH_URL` is `https://www.starshoreai.com/quota-orbit/api/passport/wishes`. Flashing still requires authorization for the exact new application image.

Rollback removes only the new proxy route and service, then restores the previous device application if authorized. Existing shared counter rows are preserved. Do not automatically delete user clicks, withdraw a community revision, push this branch or modify the public release as part of local development.

Accepted UI tap actions are routed independently from the asynchronous audio-context hint. A later navigation or long press must not discard an earlier accepted tap. Host regression covers this ordering; the controlled ten-tap device test also covered immediate navigation after the final tap.
