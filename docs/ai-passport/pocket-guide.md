[简体中文](pocket-guide.zh_CN.md) · **English**

# Pocket Scenic Guide v0.5.0

An offline companion for visitors to Jiuzhaigou and Huanglong. Select a scenic spot, read a short introduction, then optionally try an observation prompt with a companion. The device does not run a live AI service. AI assists editorial preparation only; observation prompts are editorial suggestions.

## Start using it

1. No network setup is needed. Select Jiuzhaigou or Huanglong on the home screen, then choose a spot. Jiuzhaigou also has an optional area filter.
2. Use Up/Down to select items or switch between the introduction and observation pages. Press OK to enter or open the spot's action menu. Hold OK to return.
3. In the action menu, mark a spot as seen or add it to favorites. Find these lists in My Footprints. Continue Reading appears after a spot has been saved as the last read location.
4. Open Sound Settings on Home or in the spot menu to preview and adjust volume; hold OK to save and return.
5. The display dims after 30 seconds and turns its backlight off after 90 seconds without input. The first complete function-key gesture wakes the screen; press again to operate it. The separate hardware power button is not a screen-wake key.

## Included content

22 fixed entries, Simplified Chinese only in this version:

- Jiuzhaigou / Rize: Wuhua Lake, Arrow Bamboo Lake, Panda Lake, Pearl Shoal, Pearl Shoal Waterfall, Mirror Lake.
- Jiuzhaigou / Zechawa: Long Lake, Five-Color Pool.
- Jiuzhaigou / Nuorilang lookup group: Nuorilang Waterfall. This is a browsing group, not a fourth geographic valley.
- Jiuzhaigou / Shuzheng: Rhinoceros Lake, Shuzheng Waterfall, Shuzheng Lakes, Reed Lake, Bonsai Shoal.
- Huanglong Valley: Five-Color Pool, Huanglong Ancient Temple, Zhengyan Pools, Golden Sand Pavement, Bonsai Pools, Xishen Cave, Feipu Liuhui Waterfall, Yingbin Pools.

The two Five-Color Pools always show their park context. The catalog is not exhaustive, and list order is not a recommended travel route. No positioning, navigation, itinerary import, live opening times, queue data, weather, account, phone app or wireless initialization is included. Current operations must be checked against the park's notices and signage.

## Data and recovery

Seen/favorite bit sets, the last-read spot ID and the shared output volume preference are stored on the device. Stable IDs must not be reordered when expanding the catalog. Browsing and page turns do not write storage; opening a different spot or changing a record does. Both travelers share one set of records.

The separate `pocket_guide` NVS namespace uses two alternating, versioned 24-byte records, a sequence number and CRC-32. Loading selects the newest valid slot and can fall back to the older one. The format rejects unknown versions, invalid IDs, truncation and out-of-range bits. If neither existing slot validates, it preserves the stored evidence and disables writes. Failed saves restore the previous in-memory record and display failure instead of success. No automatic NVS erase, migration, network call or quota pairing access occurs.

My Footprints offers Clear Records, with Cancel selected by default; it only clears this application's records. A normal restart should restore the last successful save. Loss of power during a write may lose that latest action, and out-of-spec supply, damaged flash or a merged reflash can lose data. Host simulation is not physical power-loss acceptance; do not promise cross-firmware retention.

## Engineering and acceptance

- Entry point: `main/main.c`; application sources: `guide_data`, `guide_model`, `guide_record`, `guide_ui`.
- Reused generic board/input logic: BSP, boot-key guard, `orbit_input.h`, `orbit_power.h`. Legacy Quota Orbit sources remain as history but are not application target sources.
- Fonts: four Noto Sans CJK SC subsets; regenerate using `tools/generate-guide-fonts.py --font <NotoSansCJKsc-Regular.otf> --converter <lv_font_conv>`. The glyph inventory is `assets/fonts/guide-characters.txt`. Font licensing remains in the font asset directory.
- `./tools/validate.sh --static` includes record corruption/fallback, simulated write failure, clear cancellation, all 22 entries and navigation tests, alongside inherited host tests.
- `./tools/validate.sh --firmware` builds and archives a matching merged image, application image and debugging artifacts. The default partition table is unchanged. No USB flashing occurs in this gate.
- Native LVGL rendering: configure `tests/guide_render` with CMake, build `guide_render`, then run `<build>/guide_render <existing-output-directory> assets/fonts/guide-characters.txt`. It uses the same UI sources as firmware, audits all font glyphs, label safe areas/overlap and repeated redraw memory. It requires the pinned managed LVGL dependency to be installed.

Before device acceptance, test all keys/long holds, first-wake consumption, offline cold boot, both parks, the two identically named pools, record persistence across restarts, cancel/confirm clear, real power interruption during saving, and 30/90-second backlight behavior. RAM availability, response time, display rendering and battery runtime must be verified on hardware. Local preview battery values are examples, not live readings.

No firmware flash, Git commit/push or community publication is implied by a local build. This is a separate play; do not update community project 1250 as a guide release.

## Pixel edition — approved direction C

Version 0.2.0 implements the selected pixel-adventure handheld direction with a midnight header/footer, warm cream reading surface, mint stepped focus rows and two original park-specific pixel landscapes. Home uses a landscape strip, the introduction keeps only a small thumbnail, and the optional observation page uses a larger scene. Body text remains regular Simplified Chinese. Core navigation, persistence format and power behavior are unchanged; no points, game levels, animations or new wireless features were added.

Artwork is independent of real LVGL labels and state. See [asset provenance and reproducible conversion](../../assets/guide/README.md). The six Flash-resident RGB565 assets total 145,920 bytes; illustrations indicate the park, not a verified photograph of each spot. CJK line wrapping prevents common closing punctuation from starting a line; native rendering audits this as well as label safety, overlap, glyphs and redraw memory.

## On-demand narration — v0.5.0

Open a spot, press OK for More and select Listen to Introduction. It returns to the reading card and plays one offline Mandarin clip; entering a spot never starts sound automatically. During preparation or playback, OK cancels/stops directly; Up/Down still changes reading pages. Holding OK exits reading and cancels speech. End of a clip stops normally, with no automatic next spot or resume after reboot. Seen/favorite records remain independent of listening.

The worker handles preparation, bounded streaming decode, cancellation and failures without blocking UI callbacks. A newer request supersedes older ones; failed playback stays retryable and does not disable browsing. The ES8311 suspends after stopping/completing. Existing 30/90-second backlight timing and first-wake key consumption continue unchanged; audio does not force the display awake. The app never records or uploads audio. [Source, encoding and reproduction](../../assets/voice/README.md) describe the 22 AI-synthesized clips.

The voice preview uses tests/guide_render/voice-preview.html and the separately built build/guide-render-voice/guide_render. Pass --renderer, --template and --audio-dir to tools/guide-preview.py. Served WAVs must come from the actual C decoder via tools/decode-guide-voice.c and tests/test_guide_voice_assets.py --wav-dir; sound is played only after a user clicks Listen. It simulates playback state and cannot validate the board speaker, driver timing or sleep current.

Version 0.4.0 expands all 22 clips in the author-selected A voice (zf_001), using a relaxed young-traveler tone with factual explanations and original observation prompts. Longer narration does not expand screen text or change controls. See narration.json for sources and manifest.json for actual duration/storage. Major spots target 45–60 seconds; others 25–40 seconds. The new candidate still requires speaker and power checks on the device.

## Local interactive review

After building the native renderer, run `python3 tools/guide-preview.py --output <existing-or-new-output-directory>` with Pillow installed and open `http://127.0.0.1:8978/`. The server binds only to loopback, never touches USB, and replays the current session's button sequence through the real C navigation/UI. Up/Down/Enter/Escape and the visible buttons drive it. Records and successful-save messages in this preview are simulated; they do not validate NVS or the device button driver. Reset Preview clears only the preview session. Power timing and wake behavior are not simulated here.

## Editorial sources — checked 2026-10-25

1. [Jiuzhaigou official sightseeing structure](https://www.jiuzhai.com/intelligent-service/way-of-play) and [overview](https://www.jiuzhai.com/about): valley membership, names and stable landscape types. Do not copy fixed schedules or contradictory waterfall dimensions. The Reed Lake body text in the overview is mismatched, so it is not used.
2. [Huanglong official site](https://www.huanglong.com/): landscape categories and featured pools. Its live counters/weather and old URLs that fall back to the homepage are not spot-level evidence.
3. [China Association of National Parks historical Huanglong material](https://www.china-npa.org/uploads/1/file/public/201801/20180122101748_ggr18f3111.pdf): search excerpts attribute the material to Huanglong management; used conservatively for established landscape names/types, not current access. Direct PDF retrieval timed out, so details unavailable in verified excerpts are omitted.
4. [Hunan Forestry historical list](https://lyj.hunan.gov.cn/lyj/tslm_71206/lykp/syjs/201512/t20151227_2618033.html): cross-checks Bonsai Pools and Feipu Liuhui names only; suspect numerical/history claims are omitted.

Introductions are original short summaries. Observation prompts are our suggestions, not official directions. No photographs, medical guidance, emergency contacts, legends or unverified historical dates are bundled.

Version 0.5.0 completes the on-device volume workflow. Open Sound Settings from Home or the spot action menu. Up adds 5, Down subtracts 5 (0–100); OK starts/stops the current or last spot as a preview. Changes apply to the actual codec during playback without restarting the clip. Hold OK to stop, return and save after the worker is idle. The separate NVS volume key leaves PG01 records unchanged; invalid/missing settings use 95%, and failed saves are explicitly temporary. At zero, preview remains muted and playback stops.

Voice A and the B-denoised baseline are preserved. Offline peak-aware speech mastering reduces high peaks and narrows dynamics before IMA encoding, allowing more average speech level at a higher codec setting without simply scaling already high peaks. The default is 95%; the user can adjust it on the device instead of reflashing. All audio remains offline and on-demand. This is a new speaker adaptation, not the unchanged B waveform or the previous fixed +5dB experiment. Numerical level/headroom checks do not replace physical listening acceptance.

Idle behaviour is unchanged: 30 seconds to dim, 90 seconds to backlight-off, and the first complete function-key gesture only wakes. Audio does not force the display on. This is not deep sleep; real battery runtime is unmeasured.
