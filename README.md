# OBS Replay Duration Inline

![OBS](https://img.shields.io/badge/OBS-31.x-333) ![Windows](https://img.shields.io/badge/Windows-x64-blue) ![License](https://img.shields.io/badge/license-MIT-green) ![Status](https://img.shields.io/badge/status-beta-orange)

**Change the maximum replay-buffer duration right next to the original OBS Replay Buffer button.**

```text
[ Start Replay Buffer ][180]s
```

The suffix `s` is fixed text. Only the numeric field is editable. No separate dock is needed.

## Install

1. Download the ZIP from [Releases](https://github.com/dmbai009/obs-replay-duration-inline/releases).
2. Exit OBS Studio completely.
3. Unzip the `obs-plugins` folder into your OBS Studio installation, usually `C:\Program Files\obs-studio\`.
4. Check that the DLL is at `C:\Program Files\obs-studio\obs-plugins\64bit\obs-replay-duration-inline.dll`.
5. Launch OBS; enter seconds, then press Enter or click outside.

To uninstall, close OBS and remove only this DLL.

> [!WARNING]
> Experimental beta for OBS Studio **31.x Windows x64**. Compiled using OBS 31.1.1 development headers and **not yet verified on OBS 31.0.2**. Back up your profile before testing.

## Behavior

- Enter 5–21600 seconds.
- Saves OBS's native `RecRBTime` profile setting (Simple and Advanced modes).
- Updates idle replay output without restarting the buffer.
- Disables input during replay buffer start, active buffering and stop.
- Tracks OBS profile and setting changes.
- The suffix `s` is outside the editable field.
- OBS memory caps can cause saved clips to be shorter than the configured time.

## Build

See [`src/plugin-main.cpp`](src/plugin-main.cpp), [`CMakeLists.txt`](CMakeLists.txt), and [the Windows workflow](.github/workflows/windows.yml). GitHub Actions obtains the official OBS Plugin Template and OBS/Qt dependencies.

## Limitations

The plugin integrates using OBS 31.x internal Qt object names (`controlsDock`, `replayBufferLayout`, `replayBufferButton`), which are not stable public API. Future OBS releases can require adjustments. This community plugin is not affiliated with OBS Project.

## License

MIT, see [LICENSE](LICENSE).
