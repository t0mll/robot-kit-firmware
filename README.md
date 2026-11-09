# Robot Kit Shop Firmware

Open-source (MIT) Firmware for [Robot Kit Shop](https://robotkit.shop) robots. Every Robot runs the **Robot Shell** on the shared **Brain**, a LilyGO T-Display-S3 (ESP32-S3 with a 1.9" screen):

- the **Face**: animated eyes with 8 Expressions;
- a **menu** of Modes on the 3 buttons (prev / select / next; long-press select to go back);
- a buzzer, settings in NVS (they survive re-flashing), and battery level.

Each Robot adds its own **Modes**.

| Folder | What |
|---|---|
| `lib/RobotShell/` | The Robot Shell library (Arduino). Your own sketches use it too. |
| `robots/rover/` | The Rover, the entry Robot (wheels, line sensor, distance sensor). |
| `robots/biped/` | The Biped, the learning walker. |
| `scripts/` | Version stamping and packaging for browser flashing. |

**Status:** skeleton. The Face, menu, buttons, buzzer and placeholder Modes work. Motors, sensors, the phone remote, the walking policy and block coding (MicroBlocks) arrive next.

## Build

Requires [PlatformIO](https://platformio.org). The Arduino-ESP32 core is pinned to **2.0.17** (`espressif32@6.10.0`) so that MicroBlocks can run inside the Shell.

```bash
pio run                 # build every Robot
pio run -e rover -t upload
```

## Flash from the browser

CI builds a merged binary and an [ESP Web Tools](https://esphome.github.io/esp-web-tools/) manifest for every Robot (`python scripts/package.py <version>`). Tagged releases (`v*`) attach them, and the shop's install page flashes them in Chrome or Edge on a desktop.

## Rules

- **Never reconfigure GPIO19/20** (native USB), or browser re-flashing breaks.
- **Drive GPIO15 high** for the screen to light on battery (the Shell does it).
- **No GPL dependencies**: everything here stays MIT-compatible.

## Licence

MIT, see [LICENSE](LICENSE). Third-party libraries keep their own licences: LovyanGFX (FreeBSD).
