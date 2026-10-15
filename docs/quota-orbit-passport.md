<p align="right"><a href="quota-orbit-passport.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Quota Orbit Passport: release candidate

Current local candidate adds optional same-Wi-Fi personal quota through a Mac companion, dynamic book lights, responsive star taps with sound, readable navigation and bilingual branding. Public announcements remain separate from personal account quota. The original public-only prototype below is retained as implementation history; it does not describe the current full feature scope.

The companion requires a signed-in Codex Mac, first-time USB pairing confirmed on the hardware, then Start sync in the local browser page. Its Apple silicon bundle includes the runtime. See the project tools/lan-quota README for setup and unverified platform/network cases. Credentials stay on the Mac. No live quota, key or private recording belongs in a public package.

The normal sound toggle previews the actual click, forecast and completion cues. Notification replay is controlled by a local compile definition and is disabled in the release candidate; the synthetic device test passed, but natural new announcements are not fully verified. Current release candidate is not yet flashed or published. Existing device acceptance belongs to81134315.

## Historical first public-only candidate

# Quota Orbit Passport: first hardware candidate

This derivative replaces the BSP demo with a Chinese, three-button public reset
message display. It does not read personal quota or log into Codex. The user
approved the page structure on 2026-10-13. Firmware flashing remains a separate
approval and hardware acceptance step.

## Use

Connect a powered device with a USB data cable for authorized flashing. On first
boot, join the temporary `Starshore-…` WPA2 hotspot using the password shown on
the device. Open `http://192.168.4.1` and enter a 2.4 GHz Wi-Fi network. Submitting a network reconfigures only the station interface; it does not
stop/start the whole radio or deliberately tear down the setup hotspot.
Credentials are saved only after IP acquisition. The hotspot closes on success
or after ten minutes. A failed attempt preserves previous saved credentials.
Wi-Fi credentials are local NVS data, not encrypted Flash in this candidate.

Up/down cycles Home, Letter, Character. OK opens the letter/mini-program code, or lights
a local star on the character page. Long OK opens settings or returns from QR.
Settings contain Wi-Fi setup, sound (off by default), and return. Stars are
session-only and never change quota.

## Data contract

HTTPS GET `https://www.starshoreai.com/quota-orbit/api/monitor`, every five minutes,
with a one-minute retry after failure. Verify the TLS chain and hostname; wait
for network time instead of bypassing certificate validation. No cloud keys,
companion app, or Bluetooth connection is required.

The streaming parser retains only `publicInsights` schema 1 metadata and the
newest `type=reset` event. `completed` is a public completion message;
`scheduled`/`announced` is a forecast. Unknown types/states are not promoted to
completion. Source health is not verification of the announcement's truth.
Neither classification nor publish time establishes an individual's restored
quota. The screen says to verify the original scope and the user's own account.

The 2026-10-13 captured response was 167,481 bytes; parser state is 2,120 bytes on
the host. Total response size is capped at 512 KiB and nesting at 24 levels.
A response is applied only after HTTP success, complete receipt, valid structure,
schema, source timestamp and no publication more than five minutes in the future.
Partial/malformed responses leave the prior snapshot intact. Source checks older
than 30 minutes, unhealthy sources or events older than seven days are visibly
stale. Publish and device-read timestamps are displayed separately in UTC+8.

New-message sound requires an existing baseline, a new event ID/state, a healthy
fresh source, and a publication within the preceding 30 minutes. Initial boot
without a cache and unchanged events are silent. A small snapshot is saved on
record changes and half-hour checkpoints; no full feed is persisted.

## Architecture and validation

- `main/orbit_model.*`: bounded streaming parser, time/freshness/notification
  rules and form decoding, independent of ESP-IDF/LVGL.
- `main/orbit_net.*`: Wi-Fi owner task, temporary HTTP form, NVS and verified HTTPS.
- `main/orbit_ui.*`: independent LVGL pages and the existing Quota Orbit character.
- `main/main.c`: BSP initialization, queued button dispatch and locked rendering.
- [Font inventory and license](../assets/fonts/README.md).

Run the repository's complete `tools/validate.sh` gate with ESP-IDF 5.5.3. It
includes the application model tests alongside upstream checks. The optional
native renderer compiles the actual UI with pinned LVGL and a 24 KiB pool:

```sh
cmake -S tests/orbit_render -B build-host
cmake --build build-host -j 8
mkdir -p build-host/screens
build-host/orbit_render build-host/screens assets/fonts/characters.txt
```

It verifies glyph coverage, navigation and repeated rendering and writes PPM
screen captures using simulated data. Use the host's working SDK/compiler.
These are software rendering checks, not camera photographs of the device.

Before declaring hardware success, test correct/wrong Wi-Fi passwords, hotspot
expiry and recovery, HTTPS heap/stack high-water marks, all three keys, screen
corners and QR scanning, battery reading, mute/unmute, repeated reconnects and
power cycles. Brightness is 65%; this candidate is always awake and has no
verified battery-life claim. No release, upload, Git commit or hardware test is
implied by a successful build.

## Provisioning regression (2026-10-13)

A device screenshot showed HTTP 403 `Use the device hotspot` before the form.
The initial application accepted only AF_INET peers, while ESP-IDF 5.5.3's
IPv6-enabled HTTP server uses a dual-stack listener and lwIP maps IPv4 peers to
IPv6 socket addresses. Both form GET and POST now use one subnet check that
accepts IPv4 and IPv4-mapped IPv6, while rejecting other subnets and native IPv6.
`test_orbit_access.c` covers the previously rejected valid representation and
negative cases. Source/Host/Origin boundaries remain in place. The error text is
now Chinese. Passing host tests is separate from installing this correction and
successfully configuring Wi-Fi on the device.

## Wi-Fi diagnostics candidate (2026-10-13)

Phone screenshots verified form GET and POST after the subnet fix. Subsequent
connection failures and setup expiry are confirmed, but their Wi-Fi driver reason
was not captured. A shared 2.4/5 GHz SSID alone does not establish the cause.

The new candidate keeps the AP running during station configuration, enables
all-channel scanning and two driver retries, and allows 45 seconds for an attempt.
Each submission renews the ten-minute setup window. Failures retain the driver
reason or API error; E10000 means associated but no address before the deadline,
and E10001 means timeout without a captured driver reason. These codes describe
the observed stage, not a proven password or router defect. No credentials are
added to application diagnostics. A never-synced home screen now explicitly says
that public messages have not been synchronized.

The gate includes a stateful host regression for the actual station configure
function: AP preservation, retry settings and API error propagation. Native UI
checks cover failure/expiry states. Actual radio compatibility and first HTTPS
synchronization still require the new candidate's device test.

## Visual refresh (2026-10-13)

Use the existing purple-haired Moonlight Archivist artwork instead of the
geometric placeholder. The three-page structure and key actions remain the
approved contract. Plum, cream and warm gold follow the character and moon-book
brand icon. Text uses 24 px message titles, 16 px values, 14 px labels and 12 px auxiliary text;
the footer shows individual page labels. Failed first synchronization explicitly
says no synchronization yet, instead of claiming a saved prior record.

See [character provenance and encoding](../assets/character/README.md). The native
renderer uses actual firmware UI with simulated states; visual approval and
new-firmware device testing remain separate from host/build checks.

## Event times on the device (2026-10-13)

The home screen puts announcement status and event times first, with a small
Archivist beside the headline. The character page retains the full illustration.
The Letter page also shows when the device read the message. All visible event
and read timestamps use UTC+8, labeled Beijing time.

Retain optional `effective_at`, `expected_at`, `window_start`, `window_end` and
`time_kind` from the selected latest reset event. Publication is never copied
into an absent effective time. Completion with no effective time is labeled
"not separately announced". Forecasts distinguish expected time, start, deadline
and a start/end window. Past targets remain forecasts with an overdue indication;
missing or invalid fields do not create a countdown. A newer completion does not
inherit a historical forecast's target. No next reset is extrapolated from history.

The expanded public snapshot uses NVS key `snapshot2`; old `snapshot` bytes are
left intact and not reinterpreted. The first update refreshes the public cache
from the service. Wi-Fi credentials and sound settings retain their existing keys.
Revisions also trigger cache persistence. Live API parsing and simulated deadline,
window, absent-time and overdue states have separate rendering evidence.

## Moon archive layout (2026-10-13)

The home screen pairs a dark plum announcement cover with a flat cream information
area, replacing nested cards. Publication, completion and forecast values share
right-aligned columns with 32 px row spacing. The Letter page uses a full cream
paper surface and 36 px spacing, while the character page keeps its dark backdrop.
The same three pages, key actions and time semantics remain in place.

Community references were inspected visually: [City Radio](https://ai-passport.folotoy.cn/plays/939/)
for a dominant headline and limited accent color, and [Pocket Poetry](https://ai-passport.folotoy.cn/plays/315/)
for a continuous paper surface. No third-party images or UI code were copied;
the author screenshots do not establish hardware validation of this application.

## Website QR candidate (2026-10-13, not flashed; superseded by mini-program entry)

The QR screen always opens `https://www.starshoreai.com/quota-orbit/`, whether
the public snapshot contains an X source URL or no source. The Letter action
and QR title explicitly name the website. Users can read announcements and
follow original-source links on the website; this increment does not deep-link
to an individual event or open the WeChat mini-program directly.

## Current scan entry: WeChat mini-program (2026-10-13)

The user confirmed that the website opens but WeChat first displays a safety
interstitial, and requested the existing Starshore Reset Assistant mini-program
instead. OK shows its official 208×208 Flash-backed code with WeChat-specific
instructions. It does not use an ordinary URL QR code, X, or a website redirect.
The website candidate and archive remain available for a later user-requested
switch after the appeal is resolved; there is no automatic destination change.
See [asset provenance](../assets/wechat/README.md). Standard QR decoders cannot
validate this mini-program code; WeChat preview and actual-panel scans remain
separate acceptance steps.

## 2026-10-14 star feedback candidate

The supplied website recording shows orbit stars and immediate visual feedback; the old device
only changes its local count. OK now updates the count immediately, lifts the character for
420 ms and releases six gold/cream sparkles for 760 ms, staggered by 35 ms. Three orbit stars
light sequentially according to the local count. Repeated input replaces the current response
with at most eight active animations; deleting page objects cancels them on navigation,
settings or provisioning redraws. The count and lit stars remain legible without motion.
There is no continuous rotation, new sound or server-side counter.
Acceptance covers immediate feedback, completion in about one second, bounded rapid input,
navigation cancellation and unobscured text. Native previews use simulated data. Hardware
performance and visual acceptance await an authorized flash; the community release is unchanged.

Hardware feedback revision: the first candidate dropped frames during repeated input.
The fix keeps the character static and retains page objects, updating only the count,
orbit stars and six small sparkles. Sparkles appear immediately and fade over 640 ms
with 25 ms stagger. Rapid input replaces animations without rebuilding the page.
The same native 80-press sequence flushes 683125 pixels instead of 6705200, about
90 percent less; this is not a hardware frame-rate measurement.

Website-inspired celebration candidate: the supplied hardware recording motivates
a richer burst of original folded-hands wish particles and gold sparkles. Repeated
input increases density while continuing the active burst instead of rewinding it.
49 opaque 192x160 RGB565 frames combine the existing character with product-drawn
particles and occupy 3010560 bytes in Flash; no full animation framebuffer is
allocated at runtime. Each effect has 24 frames over 1120 ms, a scheduled cadence
and not a measured device frame rate. Retained objects keep the count immediate
and cancel the effect on navigation. This remains a local counter. The generator
is tools/render-star-celebration.py at the workspace root. Hardware acceptance is pending.

The next diagnostic candidate preserves the celebration with a static character.
A precomputed 4x4 change mask and custom LVGL drawing invalidate only changing
regions. Native samples compare partial updates with a full redraw pixel-for-pixel
to detect stale pixels. Temporary logs aggregate rendering duration, transfer wait
and active render intervals every 20 renders without user data. Hardware timings
and visual retesting are required before claiming the stutter resolved.

Sustained-input candidate: the previous one-shot burst increased existing particles
but did not keep emitting. Repeated clicks now extend the emission deadline while
the looping phase advances without reset. Continuous-mode particle births are
distributed over the cycle; the effect ends 1120 ms after input stops. Per-click
orbit opacity animations are removed, while count and lit-star feedback remain
immediate. A three-second sustained-input regression checks phase progression,
continued activity and completion after release. Hardware retesting is pending.

Rapid-input root-cause fix: the old app only accepted BSP_BTN_CLICK, a delayed
single-click event, and ignored double/multiple clicks. Rapid taps therefore
failed to update the count until input stopped. Physical PRESS/RELEASE edges
now queue each short release immediately; later single/double classifications
are ignored to prevent duplicates. Long OK still opens settings once and its
release cannot also light a star. Startup suppression remains intact. Tests
cover 30 rapid taps, classification/duplicate-release suppression, long-release
behavior and up/down mappings. Celebration appearance and continuous emission
remain unchanged; hardware input and smoothness acceptance are pending.

Battery display candidate: replace the top-right percentage with a battery outline
and SOC fill, a warm low-power color at 20 percent or below, and a question mark
for unavailable readings. This is device power, not remaining Codex quota. The BSP
has no charging-status API, so USB presence or SOC must not imply a charging bolt.
This version still reads public messages only; personal quota requires a separately
authorized future data-sync design.

Audio candidate: local star taps produce a 24 ms click; a new valid forecast uses
two tones and completion uses three rising tones. The existing sound toggle remains
default-off and preserves saved preference. Enabling it previews click, forecast
and completion without changing announcement data. First sync, duplicates, old
and stale messages remain silent; polling intervals are unchanged. A dedicated
worker retains up to four recent clicks with no 140 ms throttle, replaces old taps on overload, discards taps older than 120 ms and
prioritizes messages. Disabling clears pending cues and stops between PCM chunks;
the codec sleeps after 3000 ms idle and wakes before the next format setup. Format is 16 kHz mono at volume 70; the smooth PCM peak is 5400 (the 10800 candidate received hardware distortion feedback). Hardware
loudness and rapid-tap feedback still require acceptance. Extreme input beyond queue capacity does not accumulate without bound.

The three book-cover lamps follow the real quota windows returned by the paired Mac. One window is used directly; two windows use the lower remaining percentage. Changes transition smoothly: green at 40% or above, amber at 20–39%, red below 20%, and gray for missing or stale data. These colors are visual cues; star taps never change personal quota.
