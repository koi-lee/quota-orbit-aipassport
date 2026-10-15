# Passport Mac Sync Assistant 0.3.0 (Pre-release)

<p align="right"><a href="README.zh_CN.md">简体中文</a> · <strong>English</strong></p>

This companion app encrypts and syncs personal quota data readable by a Codex-signed-in Mac to a paired AI Passport on the same Wi-Fi network. It is not the Quota Orbit desktop app and is not device firmware; it does not replace or update the Mac desktop app.

## Download and requirements

- Package: `StarshorePassport-AppleSilicon-0.3.0.zip` (Apple silicon Macs only).
- Verify it with `SHA256SUMS.txt` from the same release page: run `shasum -a 256 StarshorePassport-AppleSilicon-0.3.0.zip` and compare the result.
- Codex CLI must be installed and signed in on the Mac. The Mac and Passport must use the same Wi-Fi. Keep the Mac powered on, awake, and online.
- First-time pairing requires a USB data cable and confirmation on the device. After pairing, the assistant resumes periodic sync when the device starts; you do not need to click “Start sync” each time.

## Install and pair

1. Download and unzip the package. Move `StarshorePassport.app` to Applications. If an older helper is running, quit it before replacing it.
2. Open the helper; its control page opens in your browser (if it does not, visit `http://127.0.0.1:8942/`). Connect USB and click “Connect device” on the Mac page. The device may restart; wait for its home screen.
3. On the device, hold Confirm, open “Settings → Connect Mac,” and press Confirm to allow pairing. Sync starts automatically after the Mac page reports success. Unplug USB after personal quota appears on the device’s Character page.
4. On first launch, the app registers a login item for the current macOS user. At later logins it is intended to start quietly and resume sync. Disable it in “System Settings → General → Login Items.”

## Sync and compatibility

- The helper reads and sends a quota summary about every 30 seconds. It cannot refresh while the Mac is shut down, asleep, or offline; the device keeps the last snapshot and marks it old.
- Version 0.3.0 first checks the saved device address, then tries to find the paired device on supported private networks if that address stops responding. Automatic address recovery also requires matching new device firmware with the discovery endpoint.
- The firmware currently published in the community does not include this new discovery endpoint, so automatic IP-change recovery is not fully available with the current public firmware. Recovery after an actual DHCP address change has not yet been tested on a device.
- Automatic sync after a Passport restart was confirmed on one device. Automatic launch after Mac logout/login or restart has not been tested; feedback is welcome.
- This Apple silicon preview is ad-hoc signed and has not been Developer ID signed or notarized by Apple. macOS may display a security warning. Do not disable system security protections. If macOS will not open the app through its normal approval flow, stop and report the message.

## Privacy and feedback

Codex sign-in credentials and the pairing key stay on the user’s Mac; only an encrypted quota summary is sent to the device. Do not share account credentials, Wi-Fi passwords, pairing files, or access tokens. For feedback, include the helper version, macOS version, Passport firmware version, steps, and screenshots without private data.

To unpair, confirm “Settings → Unpair Mac” on the device. This revokes authorization and clears personal quota data from the device. Quitting the helper does not unpair it.

This is an independent community project and is not officially endorsed by OpenAI or FoloToy.
