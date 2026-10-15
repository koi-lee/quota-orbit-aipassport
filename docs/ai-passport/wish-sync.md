<p align="right"><a href="wish-sync.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Website wish sync: development candidate

This feature is isolated on `feature/passport-wish-sync`, based on `bd409e3`. It is not part of the current community release. The local candidate implements the approved optional setting, default-cancel consent, a bounded anonymous event queue, persistence/retry, and a separate website ingress candidate. The production ingress was deployed on 2026-10-15; firmware now defaults to that HTTPS endpoint. No device has been flashed with this feature.

## User path

Hold Confirm > Settings > Website wishes: off > review the data explanation > Enable. Each valid short press on Character adds one wish locally and queues an anonymous event. Mac software is not needed. Returning to Settings shows pending/error state. Disabling stops new events and clears pending records; an already in-flight request can still complete, and publicly counted anonymous events cannot be retracted individually. Existing stars, quota, sound and book-light behavior remain separate.

The setting defaults off. Consent defaults Cancel; a long press cancels. Missing endpoint configuration shows Service not ready rather than claiming sync success. No historic star count is uploaded. The global number is tap count, not people, personal quota or a reset probability.

## Persistence boundaries

A per-enable random seed and monotonic sequence derive stable event IDs across retries and restart. The seed stays on hardware. Acknowledgement removes only the exact sent prefix after saving succeeds. The seed/sequence state uses a separate NVS namespace; it never erases shared NVS or changes Wi-Fi/Mac keys. Recent clicks are checkpointed in batches; unexpected power failure before checkpoint, storage errors or input queue overflow can lose recent events. Pending capacity is 10,000, with a visible warning on overflow. Do not claim lossless or unlimited offline storage.

## Evidence and acceptance

Host tests drive the actual C queue model and a loopback HTTP ingress with a temporary shared website-style SQLite database: 20 taps add 20; a lost response/reopen/retry adds none; 3 offline taps after reopening add 3; opt-out clears pending and prevents additions. Additional tests cover concurrency, malformed payloads, schema refusal, limits, capacity and overflow. Native renders check the opt-in and settings states in Simplified Chinese, Traditional Chinese and English. Firmware build, archive integrity and physical testing are separate gates; physical sound/animation/network concurrency and real power failures remain unverified.

The current private website source uses the counter table with nullable country and the live read-only API exposes total/cursor/recent fields. The candidate supports that known schema without migration; the production database schema, absence of triggers, ownership and service identity were inspected before deployment. An online SQLite backup passed integrity checks. Public method/invalid-payload checks and the existing read endpoint passed without adding test events. No production test clicks were submitted.

## Integration and rollback

See the [ingress protocol and runbook](../../tools/wish-service/README.md). Candidate files are the Python ingress, a new restricted service unit and one new HTTPS location. They preserve the existing browser endpoint and counter service. The deployed default `CONFIG_ORBIT_WISH_URL` is `https://www.starshoreai.com/quota-orbit/api/passport/wishes`. Flashing still requires authorization for the exact new application image.

Rollback removes only the new proxy route and service, then restores the previous device application if authorized. Existing shared counter rows are preserved. Do not automatically delete user clicks, withdraw a community revision, push this branch or modify the public release as part of local development.

Accepted UI tap actions are routed independently from the asynchronous audio-context hint. A later navigation or long press must not discard an earlier accepted tap. Host regression covers this ordering; physical count accuracy after this fix still needs retesting.
