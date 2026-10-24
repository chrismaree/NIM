# 4x4 button-matrix Nim example

This standalone example keeps the existing project unchanged. It uses heaps
of 1, 3, 5, and 7 lights. Pressing a button removes that light and every lit
light to its right in the same heap. Taking the last light loses.

The slide switch selects two-player mode or AI mode, like the original example.
The turn LEDs show whose turn it is. At game end, the winner LED blinks for
three seconds, then the game restarts with a brief light blink. The Serial
Monitor reports the mode, moves, turns, and winner.

## Pin use

| Purpose | ESP32-S3 GPIOs |
|---|---|
| Matrix scan rows | 1, 2, 4, 5 |
| Matrix scan columns | 6, 7, 8, 9 |
| Individual game LED outputs (lights 1–16) | 10–18, 21, 37, 47, 35, 36, 3, 45 |
| Player turn LEDs | 38, 39 |
| Mode slide switch | 40 |
| Winner LEDs | 41, 42 |

The 16 buttons are wired as a 4x4 electrical matrix, so their inputs use eight
GPIOs instead of sixteen. The 16 game LEDs remain independently controlled and
each still has its own GPIO and series resistor.

The mode switch center pin connects to GPIO 40; its two outside pins connect
to GND and 3.3 V. GND selects two-player mode and 3.3 V selects AI mode. Each
indicator LED also has its own GPIO and 220-ohm series resistor to its anode,
with its cathode connected to GND.

In the physical matrix, button/light indexes 1–4 occupy matrix row 1, indexes
5–8 row 2, 9–12 row 3, and 13–16 row 4; columns advance left to right. This
electrical mapping is independent of the Nim heap layout shown in the diagram.
For reliable real-hardware scans, add one small signal diode in series with
each button to prevent ghost readings if multiple buttons are pressed at once.
With this firmware's scan direction, connect each diode's anode to the column
side and its striped cathode to the row side. Connect the matrix and LED grounds
to ESP32 GND. Check that GPIO 47 is brought out on the exact ESP32-S3 board
before building the physical version.

The Serial Monitor uses UART0 (GPIO 43/44, the DevKitC-1 `TX`/`RX` pins and
its UART USB port), so no game LED uses those pins. Don't enable
`ARDUINO_USB_CDC_ON_BOOT`: with it, the firmware hangs in Wokwi before any
LED lights or any button is read. GPIO 33 and 34 are not broken out on the
DevKitC-1, so lights 11 and 12 use GPIO 37 and 47. Lights 15 and 16 use the
strapping pins GPIO 3 and 45; their LED and resistor to GND hold them low,
which matches the default boot configuration.

In Wokwi, the slide switch starts on the GND side, so the game begins in
two-player mode. Click the switch to change to AI mode; the game restarts and
the agent then answers each of Player 1's moves about half a second later.

## Run in Wokwi

Open this folder as a separate PlatformIO project, build it, then start Wokwi
using this folder's `diagram.json` and `wokwi.toml`. The diagram uses the
16-button matrix, separate game LEDs, four indicator LEDs, and a mode switch.
