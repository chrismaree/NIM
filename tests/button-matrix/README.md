# ESP32 2x2 Button Matrix -> 4 LEDs (Wokwi + Cursor)

Proof of concept for a button matrix: 4 buttons on 4 GPIOs (2 rows + 2 columns), each toggling its own LED.

## Wiring

| Button | Row pin | Column pin | LED pin |
| ------ | ------- | ---------- | ------- |
| R0C0 (red, key `1`) | `GPIO25` | `GPIO32` | `GPIO18` |
| R0C1 (green, key `2`) | `GPIO25` | `GPIO33` | `GPIO19` |
| R1C0 (blue, key `3`) | `GPIO26` | `GPIO32` | `GPIO21` |
| R1C1 (yellow, key `4`) | `GPIO26` | `GPIO33` | `GPIO22` |

- Rows (`GPIO25`, `GPIO26`) are outputs, idle `HIGH`.
- Columns (`GPIO32`, `GPIO33`) use `INPUT_PULLUP` (pressed = `LOW` while its row is scanned).
- Each LED is driven through a `220` ohm resistor to `GND`.

## How the scan works

1. Pull one row `LOW`, leave the other `HIGH`.
2. Read both columns. A `LOW` column means the button at that row/column is pressed.
3. Restore the row to `HIGH` and repeat for the next row.

Each button is debounced independently and toggles its LED on press.

## Run in Cursor

1. Open `/Users/chris/projects/NIM/tests/button-matrix` in Cursor.
2. Command Palette -> `PlatformIO: Build` (creates `.pio/build/esp32/firmware.bin`).
3. Command Palette -> `Wokwi: Start Simulator`.
4. Click the buttons (or press `1`-`4`) to toggle the LEDs.

## Notes

- Pressing 3 buttons at once can cause ghosting (a phantom 4th press). Add a diode per button if multi-press matters.
