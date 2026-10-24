# ESP32 Resistor Encoder -> Two LEDs (Wokwi + Cursor)

Two buttons share **one analog pin** through a resistor network. Each button
combination produces a different voltage, and the firmware decodes it by range.

- Encoder input on `GPIO34` (ADC1, input-only), `10k` pull-up to `3V3`
- Button A pulls the node to `GND` through `20k`
- Button B pulls the node to `GND` through `10k`
- Red LED on `GPIO18` lights while A is held; green LED on `GPIO19` while B is held
- Both LEDs light when both buttons are held

## Voltage levels

The button resistors are binary-weighted (B conducts twice as much as A), so all
four combinations stay distinct:

| Pressed | Node voltage | Decoded when |
|---|---:|---|
| none | 3.30 V | `>= 2750 mV` |
| A | 2.20 V | `1925–2749 mV` |
| B | 1.65 V | `1485–1924 mV` |
| A + B | 1.32 V | `< 1485 mV` |

Thresholds are the midpoints between levels, so resistor tolerance and ADC noise
have headroom. A state must hold for `35 ms` before the LEDs change.

## Simulation vs real hardware

Wokwi cannot simulate resistor dividers (its resistors only work as pull-ups or
pull-downs), so the network is modelled by a custom chip,
`chip-resistor-encoder`. It takes the two buttons on `A`/`B` and outputs the same
voltage the real network would on `OUT`, using the `vcc`, `r_pullup`, `r_a` and
`r_b` attributes set in `diagram.json`.

On real hardware, skip the chip and wire the resistors directly as described above.
The firmware stays the same.

Rebuild the chip after editing `chip/resistor-encoder.chip.c`:

```sh
/opt/wasi-sdk/bin/clang --target=wasm32-unknown-wasi -O2 -nostartfiles \
  -Wl,--import-memory -Wl,--export-table -Wl,--no-entry -Werror \
  -o dist/resistor-encoder.chip.wasm chip/resistor-encoder.chip.c
```

## Run in Cursor

1. Open `/Users/chris/projects/NIM/tests/encoder-test` in Cursor.
2. Install extensions if needed:
   - `PlatformIO IDE`
   - `Wokwi Simulator`
3. Build once so firmware artifacts exist:
   - Command Palette -> `PlatformIO: Build`
   - This should create `.pio/build/esp32/firmware.bin`
4. Start simulation:
   - Command Palette -> `Wokwi: Start Simulator`
5. Hold button A (or press `a`) for red, B (or press `b`) for green.
   The serial monitor prints the decoded state and measured millivolts.

## If you see "firmware.bin not found"

- Make sure the folder opened in Cursor is:
  - `/Users/chris/projects/NIM/tests/encoder-test`
- Run `PlatformIO: Build` first, then start Wokwi.
