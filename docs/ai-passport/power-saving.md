<p align="right"><a href="power-saving.zh_CN.md">简体中文</a></p>

# Idle screen-off

After 30 seconds without key activity, reduce backlight from 65% to 15%; after 90 seconds, turn it off. Any key wakes the display. The first complete short or long wake gesture only lights the screen: no navigation, wish or settings action. Dimmed keys work normally. Held keys inhibit timeout. Wi-Fi setup, Mac pairing and hardware confirmation keep the display lit and restart the idle interval on exit.

Wi-Fi, public news, personal quota reception, wish uploads and sound remain active while the backlight is off. Network updates do not wake the screen. Page rebuilds are deferred until visible; existing LVGL animations and background tasks may still run. This is connected screen blanking, not MCU deep sleep or LCD-controller power-off. Waking reads and redraws current data.

## Evidence and decision (2026-10-22)

- [FoloToy hardware guide](https://github.com/FoloToy/ai-passport/blob/main/docs/hardware-design/AI_HARDWARE_DEVELOPMENT_GUIDE.md): GPIO21 PWM controls backlight; upstream includes light/deep sleep examples. Terminal deep-sleep APIs release the button ADC and peripherals and cannot be reused for reversible blanking.
- [ESP-IDF 5.5.3 sleep documentation](https://docs.espressif.com/projects/esp-idf/en/v5.5.3/esp32c3/api-reference/system/sleep_modes.html): direct light/deep sleep does not preserve wireless connections. Keep current quota synchronization by avoiding direct sleep in this change.
- [ESP-IDF Wi-Fi documentation](https://github.com/espressif/esp-idf/blob/v5.5.3/docs/en/api-guides/wifi.rst): WIFI_PS_MIN_MODEM is already the default and this application does not disable it. Do not describe it as a new optimization. Existing audio suspend after three idle seconds remains unchanged.
- Goal: reduce unattended backlight use while retaining synchronization. No new persistent preference, pairing, network or partition changes.

## Acceptance

Host tests cover timeout boundaries, short/long wake suppression for all three keys, subsequent normal input, held keys, setup/pairing inhibition and overlapping wake keys. The complete gate checks the build and merged image.

Device checks: dim at 30 seconds, blank at 90 seconds; each key wakes without performing an action, subsequent input works; setup/pairing stays lit; Mac updates, sound and network recovery work while blanked; wake shows current data. Unplug USB and compare old/candidate battery change and runtime on the same Wi-Fi, volume and usage pattern. No saving percentage or standby duration is promised before device/current/runtime measurements.

On 2026-10-22 the tested application was flashed to the author’s original device. Startup, networking and idle-backlight logs passed. The author confirmed wake with all three function keys and reported about half a day of use without observed issues. The dedicated hardware power button is not a screen-wake input. Long-press wake, setup/pairing inhibition and power/runtime measurements have not been fully verified on hardware. This observation does not establish a standby duration or saving percentage.
