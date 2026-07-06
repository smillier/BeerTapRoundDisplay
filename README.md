# BeerTapRoundDisplay V2 - Remote Scale (ESP32-C6 + BLE)

Fork of [BeerTapRoundDisplay](https://github.com/smillier/BeerTapRoundDisplay) that separates the scale (HX711 + weight sensors) from the round GC9A01 display. The two boards communicate over Bluetooth Low Energy instead of being wired together on a single ESP32.
─────────────────────────┐        BLE (GATT)        ┌──────────────────────────┐
│   scale-esp32c6          │  ────────────────────►   │   display-esp32          │
│   ESP32-C6 + HX711       │   notify weight (g)       │   ESP32 + round GC9A01   │
│   under the keg           │  ◄────────────────────   │   Wifi + WebServer + OTA │
│                          │   tare command             │   computes volume        │
└─────────────────────────┘                            └──────────────────────────┘

## Why two boards?

- The scale (HX711 + load cells) stays as close as possible to the keg, under its foot, far from the display, which is mounted remotely on the cabinet/fridge front panel.
- The ESP32-C6 is a natural fit for this node: small, cheap, native BLE, no need for Wifi/display/large flash on the scale side.
- The display keeps the existing Wifi/OTA/web server (beer config, IBU, ABV, SG...) and shows a status dot (green/orange/red) indicating how fresh the BLE link is.

## Repo structure
scale-esp32c6/     Scale firmware (HX711 -> BLE GATT server)
display-esp32/     Display firmware (BLE GATT client -> display + Wifi/webserver)
These are two independent PlatformIO projects, flashed separately (two `platformio.ini` files).

## BLE Protocol

The scale (`scale-esp32c6`) is the **BLE server (peripheral)**; the display (`display-esp32`) is the **BLE client (central)**. There's no pairing/bonding (no sensitive data is exchanged) — the connection is made by filtering on the service UUID.

| Element | UUID | Type | Content |
|---|---|---|---|
| Scale service | `a1f4e100-3b7a-4f56-9d1b-2f6a9c5e10a0` | - | - |
| Weight characteristic | `a1f4e101-3b7a-4f56-9d1b-2f6a9c5e11a0` | READ + NOTIFY | `float32` weight in grams |
| Command characteristic | `a1f4e102-3b7a-4f56-9d1b-2f6a9c5e12a0` | WRITE | `uint8`: `0x01` = tare |

These values are defined in `scale-esp32c6/include/config.h` and `display-esp32/include/config.h` (must stay identical on both sides).

Behavior:

- The scale measures weight every `WEIGHT_UPDATE_PERIOD_MS` (2s by default) and notifies the value if a client is connected.
- The display scans, connects to the first device advertising the service above, and automatically resubscribes on disconnection (`BLE_RECONNECT_PERIOD_MS`).
- Volume (weight / configured SG density on the display) is still computed on the display side, as before — the scale only reports raw weight.
- A "Tare" tap in the display's web UI (`POST /tare`) sends the command to the scale, which performs `scale.tare()`.
- `GET /scale_status` returns `{"connected":bool,"stale":bool}` for external monitoring.
- On the display, a dot at the top of the dial shows status: green = fresh data, orange = connected but stale data (> `BLE_WEIGHT_STALE_MS` = 15s), red = scale not connected.

## scale-esp32c6/ - HX711 wiring

| HX711 | ESP32-C6 |
|---|---|
| DOUT | GPIO2 |
| SCK | GPIO3 |
| VCC | 3.3V (or 5V depending on HX711 module) |
| GND | GND |

GPIO2/3 were chosen because GPIO4, 5, 8, 9, 15 are boot strapping pins on the ESP32-C6 and must be avoided. Adjust `SCALE_CALIBRATION_FACTOR` in `scale-esp32c6/include/config.h` for your own load cell (same calibration procedure as the original: weigh a known weight and adjust the factor).

## display-esp32/ - Display

GC9A01 wiring unchanged from the original repo (see `display-esp32/include/config.h`). The HX711 is no longer wired on this board.

## Flashing both boards

Each folder is a standalone PlatformIO project:
cd scale-esp32c6
pio run -t upload         # ESP32-C6 over USB
cd ../display-esp32
pio run -t upload         # regular ESP32, over USB the first time, then OTA
`display-esp32` keeps OTA (ArduinoOTA + AsyncElegantOTA) as in the original project; `scale-esp32c6` is flashed over USB (no OTA, no Wifi on this board).

## Limitations / TODO

- The `Case/` and `PCB/` files from the original repo (3D enclosure, Eagle PCB) were designed for the combined display+HX711 board: they'll need to be redesigned/split into two enclosures (not addressed in this version, out of software scope).
- `data/config.json` (demo content from the original repo) contains a dummy SSID and Wifi key in plain text — check/clean this up before publishing if that wasn't intentional.
- No BLE pairing (LE Secure Connections): fine for home use where only weight is transmitted, but worth keeping in mind if the protocol evolves.
