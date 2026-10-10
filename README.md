# OBS Replay Duration Inline

![OBS](https://img.shields.io/badge/OBS-31.x-333) ![Windows](https://img.shields.io/badge/Windows-x64-blue) ![License](https://img.shields.io/badge/license-MIT-green) ![Release](https://img.shields.io/badge/release-1.0.0-brightgreen)

**Change the maximum replay-buffer duration right next to the original OBS Replay Buffer button.**

```text
Buffer stopped: [ Start Replay Buffer (fills remaining width) ][180]s
Buffer active:  [ Stop Replay Buffer  (fills remaining width) ][180]s [Save ↧]
```

The suffix `s` is fixed text. Only the numeric field is editable. No separate dock is needed.

## Install

1. Download the **v1.0.0** ZIP from [Releases](https://github.com/dmbai009/obs-replay-duration-inline/releases).
2. Exit OBS Studio completely.
3. Unzip the `obs-plugins` folder into your OBS Studio installation, usually `C:\Program Files\obs-studio\`.
4. Check that the DLL is at `C:\Program Files\obs-studio\obs-plugins\64bit\obs-replay-duration-inline.dll`.
5. Launch OBS; enter seconds, then press Enter or click outside.

To uninstall, close OBS and remove only this DLL.

> [!WARNING]
> Tested in **OBS Studio 31.0.2 on Windows x64**. Other OBS versions are not yet verified. Back up your profile before installing third-party native plugins.

## Behavior

- Enter 5–21600 seconds.
- Saves OBS's native `RecRBTime` profile setting (Simple and Advanced modes).
- Updates idle replay output without restarting the buffer.
- Disables input during replay buffer start, active buffering and stop.
- Tracks OBS profile and setting changes.
- The suffix `s` is outside the editable field.
- Native Replay Buffer button expands to fill the entire row; the native Save Replay button appears alongside the field while active.
- OBS memory caps can cause saved clips to be shorter than the configured time.

## Build

See [`src/plugin-main.cpp`](src/plugin-main.cpp), [`CMakeLists.txt`](CMakeLists.txt), and [the Windows workflow](.github/workflows/windows.yml). GitHub Actions obtains the official OBS Plugin Template and OBS/Qt dependencies.

## Limitations

The plugin integrates using OBS 31.x internal Qt object names (`controlsDock`, `replayBufferLayout`, `replayBufferButton`), which are not stable public API. Future OBS releases can require adjustments. This community plugin is not affiliated with OBS Project.

## License

MIT, see [LICENSE](LICENSE).

### v0.1.0-beta.2

Rebuilt with the **exact OBS 31.0.2 build dependencies** (including Qt) after OBS logs reported `obs-replay-duration-inline.dll not loaded` for beta.1. This is a loader compatibility fix; actual runtime still needs user testing.

### v0.1.0-beta.3

The native Replay Buffer button now stretches to occupy unused horizontal space; the compact `[180]s` input stays beside it. When recording to the replay buffer, the native Save Replay button appears at the right, without a large gap. The original button sizing policies are restored when the plugin unloads.

### v1.0.0

First stable release using the tested beta.3 plugin implementation, without changes to runtime logic. Verified on OBS 31.0.2 Windows x64: 30-second and 60-second replay saves, full-width layout and native Save Replay button, locked duration input while buffering, and settings persisting across restarts.

The v1.0.0 release is built and packaged by the Windows GitHub Actions workflow.
