<p align="right"><a href="packaging.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Reproducible Mac helper packaging and compatibility checks

The 0.3.2 package accidentally collected Homebrew libraries targeting macOS 26/27. Changing only the launcher's target or Info.plist cannot fix this. The candidate pipeline uses the official Python 3.13.16 runtime and pinned binary wheels, then checks every bundled ARM64 Mach-O. Source pairing and sync behavior are unchanged.

## Build a candidate

Use macOS with an isolated Python 3.13.16 environment. The [official installer](https://www.python.org/downloads/release/python-31316/) SHA-256 is `30666509020b4da0dd8bc2e773255f34d76b7bb80b66960a928d5f6daa0192d7`. Do not use an interpreter or libraries copied from the current host's Homebrew installation. A new virtual environment is required; it must not include system site-packages.

```sh
python -m pip install --only-binary=:all: -r tools/lan-quota/requirements-macos.txt
python -m unittest discover -s tools/lan-quota -p 'test_*.py'
python tools/lan-quota/build_macos.py --output /tmp/passport-helper-candidate --version 0.3.3 --minimum-macos 11.0
python tools/lan-quota/smoke_macos.py /tmp/passport-helper-candidate/dist/StarshorePassport.app
```

The output path must not exist. The build never installs the app, overwrites another candidate, changes pairing, or writes a login item. It generates an ARM64 app, dependency licenses, an exact package manifest and a bundle audit. The default ad-hoc signature is only for local/CI testing. A Developer ID identity can be supplied with `--sign-identity`; public distribution still requires notarization, stapling and a final signature/Gatekeeper check.

The audit rejects any ARM64 binary whose minimum OS exceeds the declared target, unknown deployment versions, host-specific library paths, escaping symlinks or missing ARM64 code. It reads the minimum deployment command, not the SDK version. It does not rewrite binary deployment versions to conceal incompatible dependencies.

## Runtime evidence and limits

The [macOS compatibility workflow](../../../.github/workflows/passport-helper-macos.yml) builds and runs the candidate on standard Apple silicon macOS 14 and 15 runners. It imports the frozen bridge, starts preview mode with device access disabled, checks the three-language page, duplicates a launch, quits, verifies port release and reopens. It never uses real account data or USB. Artifacts include the actual tested OS and manifests; a configured workflow is not evidence that a run passed.

A minimum binary target of macOS 11 is a packaging check, not a claim that macOS 11-13 runtime, a fresh Mac install, login reboot, pairing or quota sync passed. Report actual successful workflow runs and hardware tests separately. Keep user download requirements tied to verified release artifacts, not an unnotarized candidate.

[PyInstaller's compatibility guidance](https://pyinstaller.org/en/stable/usage.html#making-macos-apps-forward-compatible) recommends freezing on the oldest supported OS. The oldest runtime available in this workflow is macOS 14; support claims must retain the lower-version testing boundary. Standard runners are free for this [public repository](https://docs.github.com/en/actions/reference/runners/github-hosted-runners).
