<p align="right"><a href="README.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Passport wish ingress

This service adds anonymous hardware events to the **existing website counter database**, not a separate public total. The ingress was deployed on 2026-10-15 at `https://www.starshoreai.com/quota-orbit/api/passport/wishes`; this development branch defaults to that address. Physical end-to-end testing is pending, and this feature is not in the community release.

## Protocol

`POST /quota-orbit/api/passport/wishes`, JSON `{"events":["canonical-UUID-v4", "..."]}`; 1–32 unique events, at most 2048 bytes. Success returns `{"total":123,"accepted":["..."]}`. Repeated event IDs do not increase the total. The firmware requires an exact complete acknowledgement before removing a durable prefix. It does not forge a browser Origin or send account/quota data. No device MAC, seed or serial number is sent.

Firmware derives unlinkable UUID-shaped event IDs from a random 32-byte local seed and an unsigned sequence using SHA-256. The version and variant bits match UUID v4 validation; these are hash-derived identifiers rather than independent random UUID draws. Switching off clears unsent records; switching back on uses a new seed. Records already counted publicly cannot be retracted per user. A request already in flight may finish after switching off.

## Local checks

Run `CC=<host-C-compiler> python3 tools/wish-service/test_server.py` from the repository root. Tests create temporary databases and use loopback only. They drive the firmware C queue model through a compiled bridge, simulate a lost response/restart, and check the same SQL total used by the website. They cover 20 initial taps, deduplicated retries, 3 offline taps after reopening, opt-out, malformed requests, concurrent duplicates, schema refusal and rate limits. No real website writes occur.

The ingress accepts an existing `clicks(id TEXT PRIMARY KEY, received_at INTEGER NOT NULL, version INTEGER NOT NULL)` table, with an optional nullable `country TEXT` column. It refuses missing databases, unknown columns or incompatible country constraints; it performs no schema migration. Hardware records use version 2 and no country, preserving website history and cursor ordering. Production schema, absence of triggers, ownership and service identity were checked before deployment. The online backup passed integrity checks. Public rejection checks preserved the counter; no test events were submitted.

## Deployment boundary

The candidate binds only to `127.0.0.1:8970` and requires `--db <existing-shared-db>`. Deployment must first back up and verify the **actual** shared database and check concurrent website reads/writes. Do not point it at a new empty database and call that synchronization. Use the same restricted service identity and database access policy as the existing counter service.

An approved HTTPS reverse proxy must map the exact protocol route to this loopback service, enforce the 2 KiB body limit, and apply a per-client network limit of two requests/second with a bounded burst. The local service also limits two requests/second with burst six per direct peer; it does not trust forwarded client headers and therefore shares that fallback bucket behind a proxy. No CORS permission is provided. There is no claim of authenticated devices or fraud-proof counting: like the current web interaction, this is an anonymous click counter, not unique users or a reset probability.

For a deployed endpoint, configure `CONFIG_ORBIT_WISH_URL` to its approved HTTPS URL and build a new exact candidate. Redirects and certificate bypasses are disabled. Empty configuration shows Service not ready and retains events locally. Live deployment, firmware flashing, source publication and community submission require their corresponding authorization.

## Persistence and performance

The device stores a bounded seed/sequence prefix in its own `orbit_wish` NVS namespace. Capacity is 10,000 pending events; overflow is visible rather than silently claiming a successful sync. Network requests run in a worker and time out after six seconds. Retry backoff reaches 32 seconds. Pending taps are coalesced into checkpoints, not written to Flash in the key callback. A sudden power loss before checkpointing, an overflowing RAM input queue or storage failure can lose recent taps. The 64-entry RAM queue also covers clicks arriving during an in-flight request; this is not a promise of unlimited tap rate or lossless power failure. These cases and simultaneous sound/display/network behavior still need physical device testing. Existing Wi-Fi, Mac pairing and sound preferences use different NVS namespaces and keys.
