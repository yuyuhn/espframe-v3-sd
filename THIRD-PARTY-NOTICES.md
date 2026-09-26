# Third-Party Notices

This repository is a fork of [Espframe](https://github.com/jtenniswood/espframe).

- The project-owned code and documentation are licensed under the
  **PolyForm Noncommercial License 1.0.0** — see [LICENSE](LICENSE).
- The required notice line is recorded in [NOTICE](NOTICE):
  `Required Notice: Copyright 2025-2026 Espframe contributors`.
- Third-party source is inventoried in [product/source-ownership.json](product/source-ownership.json)
  (upstream, revision, license, and local changes).

The components below are vendored third-party code. Each retains its own license, and the license
text is kept inside the named file — do not delete or alter those files. This document is a summary
index only; it is not a substitute for the license texts themselves.

## Vendored components

| Component | Path | Upstream | License | License text |
|---|---|---|---|---|
| ESPHome LEDC output component | `components/ledc` | [esphome/esphome](https://github.com/esphome/esphome) `2026.8.2` | MIT (see note below) | `components/ledc/LICENSE` |
| GSL3680 touchscreen component | `components/gsl3680` | [kvj/esphome](https://github.com/kvj/esphome) `dca6f3e` | GPL-3.0-or-later (C/C++), MIT (Python) | `components/gsl3680/LICENSE.md` |
| libjpeg-turbo ESP32 decode subset | `components/libjpeg-turbo-esp32` | [libjpeg-turbo](https://github.com/libjpeg-turbo/libjpeg-turbo) `3.1.1` | IJG + BSD-3-Clause | `components/libjpeg-turbo-esp32/LICENSE.md` |
| libwebp ESP32 decode subset | `components/libwebp-esp32` | [webm/libwebp](https://chromium.googlesource.com/webm/libwebp) `v1.6.0` | BSD-3-Clause | `components/libwebp-esp32/LICENSE` |
| Remote Image ESPHome component | `components/remote_image` | [esphome/esphome](https://github.com/esphome/esphome) `online_image` | MIT | `components/remote_image/LICENSE` |

## Build-time assets

| Asset | Upstream | License |
|---|---|---|
| Device text fonts (Noto) | [Google Fonts](https://fonts.google.com/noto) | OFL-1.1 |
| Device icon font (Material Design Icons) | [Templarian/MaterialDesign-Webfont](https://github.com/Templarian/MaterialDesign-Webfont) `v7.4.47` | Apache-2.0 |
| ArduinoJson parser test headers | [bblanchon/ArduinoJson](https://github.com/bblanchon/ArduinoJson) `v7.4.3` | MIT |

## Required attribution lines

- libjpeg-turbo, IJG License clause 2: when distributing binaries that statically link or otherwise
  incorporate libjpeg-turbo, your documentation must state:

  > This software is based in part on the work of the Independent JPEG Group.

## Distribution notes

- **PolyForm Noncommercial.** The project-owned code may be shared and modified only for
  non-commercial purposes. Anyone who receives a copy from you must also receive the PolyForm
  Noncommercial License 1.0.0 terms (or their URL) plus the `Required Notice:` line from
  [NOTICE](NOTICE). You may not relicense the project-owned code, and commercial use requires
  separate permission from the project owner.
- **GPLv3 (copyleft).** The C/C++ code of `components/gsl3680` is GPL-3.0-or-later. In addition,
  the bundled `components/ledc/LICENSE` is the ESPHome dual license (C/C++ = GPLv3, Python = MIT),
  even though [product/source-ownership.json](product/source-ownership.json) labels ledc as "MIT" —
  reconcile that label before distributing. If you distribute firmware binaries that compile this
  C/C++ code, GPLv3 requires you to provide the Corresponding Source for those parts under GPLv3.
