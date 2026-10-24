# 4x4 button matrix serial test

Tests the 16-button matrix from `../matrix-4x4` without any LEDs. Each press and
release is printed over serial, along with the row and column GPIO and a grid of
the buttons currently held down.

The wiring matches the game. See [`../matrix-4x4/wiring.md`](../matrix-4x4/wiring.md).

|                    | Col 1 — GPIO 6 | Col 2 — GPIO 7 | Col 3 — GPIO 8 | Col 4 — GPIO 9 |
|---|---|---|---|---|
| **Row 1 — GPIO 1** | BTN 1  | BTN 2  | BTN 3  | BTN 4  |
| **Row 2 — GPIO 2** | BTN 5  | BTN 6  | BTN 7  | BTN 8  |
| **Row 3 — GPIO 4** | BTN 9  | BTN 10 | BTN 11 | BTN 12 |
| **Row 4 — GPIO 5** | BTN 13 | BTN 14 | BTN 15 | BTN 16 |

## Run on the board over USB

The DevKitC-1 has two USB ports. Pick the build that matches the port you plug
into:

| Port | Command |
|---|---|
| `UART` (recommended) | `pio run -e uart -t upload -t monitor` |
| `USB` (native USB) | `pio run -e usb -t upload -t monitor` |

Open the serial monitor at 115200 baud. Example output:

```
PRESS   BTN6   row 2 (GPIO 2)  col 2 (GPIO 7)  t=5123ms
Held: BTN6
        C1(6) C2(7) C3(8) C4(9)
R1(1)      .     .     .     .
R2(2)      .   [ 6]    .     .
R3(4)      .     .     .     .
R4(5)      .     .     .     .
```

## What to look for

- **Nothing prints for one button:** that button, or one of its two wires, is
  open.
- **A whole row or column never responds:** that row or column wire to the
  ESP32 is broken or on the wrong pin.
- **The wrong button number appears:** two row wires or two column wires are
  swapped. The printed GPIOs show which ones.
- **A warning at startup that a column reads LOW:** that column is shorted to
  GND.
- **Holding three buttons that form an L shape also shows a fourth:** that's
  ghosting. Add the diodes described in `wiring.md`.

## Run in Wokwi

Build the default `uart` environment (`pio run`), then start Wokwi from this
folder. Each button is labelled with its number, row and column. The `usb`
build hangs in Wokwi, so don't use it there.
