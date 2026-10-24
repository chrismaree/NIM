# Wiring: ESP32-S3-DevKitC-1 Nim matrix

This is the exact wiring in `diagram.json` and `src/main.cpp`. All GND
connections go to an ESP32 `GND` pin. All resistors are 220 Ω.

## Button matrix (4 × 4)

Rows are driven LOW one at a time; columns use the internal pull-ups and read
LOW when a button in the active row is pressed.

|                    | **Col 1 — GPIO 6** | **Col 2 — GPIO 7** | **Col 3 — GPIO 8** | **Col 4 — GPIO 9** |
|---|---|---|---|---|
| **Row 1 — GPIO 1** | BTN 1  | BTN 2  | BTN 3  | BTN 4  |
| **Row 2 — GPIO 2** | BTN 5  | BTN 6  | BTN 7  | BTN 8  |
| **Row 3 — GPIO 4** | BTN 9  | BTN 10 | BTN 11 | BTN 12 |
| **Row 4 — GPIO 5** | BTN 13 | BTN 14 | BTN 15 | BTN 16 |

Each button has two contacts. In Wokwi, contact 1 is pin `1.l` and contact 2 is
pin `2.l`.

```
Row GPIO ──── contact 1 ┤ BTN n ├ contact 2 ──── Column GPIO
```

### Optional diode per button (real hardware)

Add one small signal diode (e.g. 1N4148) per button to stop ghost presses when
more than one button is held. The striped cathode goes to the row side:

```
Row GPIO ──|◄── contact 1 ┤ BTN n ├ contact 2 ──── Column GPIO
         cathode  anode
```

## Buttons, LEDs and pins

Every game LED is wired the same way:

```
LED GPIO ──[220 Ω]──►|── GND
                 anode  cathode
```

| Button | Matrix row (GPIO) | Matrix column (GPIO) | LED | LED GPIO | Nim heap |
|---|---|---|---|---|---|
| BTN 1  | Row 1 (1) | Col 1 (6) | LED 1  | 10 | Heap 1 (1 light) |
| BTN 2  | Row 1 (1) | Col 2 (7) | LED 2  | 11 | Heap 2 (3 lights) |
| BTN 3  | Row 1 (1) | Col 3 (8) | LED 3  | 12 | Heap 2 |
| BTN 4  | Row 1 (1) | Col 4 (9) | LED 4  | 13 | Heap 2 |
| BTN 5  | Row 2 (2) | Col 1 (6) | LED 5  | 14 | Heap 3 (5 lights) |
| BTN 6  | Row 2 (2) | Col 2 (7) | LED 6  | 15 | Heap 3 |
| BTN 7  | Row 2 (2) | Col 3 (8) | LED 7  | 16 | Heap 3 |
| BTN 8  | Row 2 (2) | Col 4 (9) | LED 8  | 17 | Heap 3 |
| BTN 9  | Row 3 (4) | Col 1 (6) | LED 9  | 18 | Heap 3 |
| BTN 10 | Row 3 (4) | Col 2 (7) | LED 10 | 21 | Heap 4 (7 lights) |
| BTN 11 | Row 3 (4) | Col 3 (8) | LED 11 | 19 | Heap 4 |
| BTN 12 | Row 3 (4) | Col 4 (9) | LED 12 | 20 | Heap 4 |
| BTN 13 | Row 4 (5) | Col 1 (6) | LED 13 | 35 | Heap 4 |
| BTN 14 | Row 4 (5) | Col 2 (7) | LED 14 | 36 | Heap 4 |
| BTN 15 | Row 4 (5) | Col 3 (8) | LED 15 | 46 | Heap 4 |
| BTN 16 | Row 4 (5) | Col 4 (9) | LED 16 | 48 | Heap 4 |

The electrical matrix rows don't match the Nim heaps. The heaps are the
physical layout of the board, as in the Wokwi diagram:

```
Heap 1:                   [1]
Heap 2:               [2] [3] [4]
Heap 3:           [5] [6] [7] [8] [9]
Heap 4:     [10] [11] [12] [13] [14] [15] [16]
```

Pressing a button removes that light and every lit light to its right in the
same heap.

## Indicators and mode switch

| Part | ESP32 pin | Other side |
|---|---|---|
| Player 1 turn LED (green) | GPIO 38 → 220 Ω → anode | cathode → GND |
| Player 2 turn LED (blue) | GPIO 39 → 220 Ω → anode | cathode → GND |
| Player 1 winner LED (green) | GPIO 41 → 220 Ω → anode | cathode → GND |
| Player 2 winner LED (yellow) | GPIO 42 → 220 Ω → anode | cathode → GND |
| Mode switch, middle pin | GPIO 40 | — |
| Mode switch, outer pin 1 | GND | Two-player mode |
| Mode switch, outer pin 3 | 3V3 | AI mode |

## Where the pins are on the DevKitC-1

USB ports at the bottom, viewed from the top. `·` marks a pin this project
doesn't use.

```
        Left header (J1)                       Right header (J3)
 3V3    ← mode switch pin 3              GND       ·
 3V3    ·                                TX  (43)  serial monitor
 RST    ·                                RX  (44)  serial monitor
 GPIO4  Row 3                            GPIO1     Row 1
 GPIO5  Row 4                            GPIO2     Row 2
 GPIO6  Col 1                            GPIO42    P2 winner LED
 GPIO7  Col 2                            GPIO41    P1 winner LED
 GPIO15 LED 6                            GPIO40    Mode switch middle
 GPIO16 LED 7                            GPIO39    P2 turn LED
 GPIO17 LED 8                            GPIO38    P1 turn LED
 GPIO18 LED 9                            GPIO37    ·
 GPIO8  Col 3                            GPIO36    LED 14
 GPIO3  ·                                GPIO35    LED 13
 GPIO46 LED 15                           GPIO0     · (BOOT)
 GPIO9  Col 4                            GPIO45    ·
 GPIO10 LED 1                            GPIO48    LED 16
 GPIO11 LED 2                            GPIO47    ·
 GPIO12 LED 3                            GPIO21    LED 10
 GPIO13 LED 4                            GPIO20    LED 12
 GPIO14 LED 5                            GPIO19    LED 11
 5V     ·                                GND       ·
 GND    ← all LED cathodes, switch pin 1 GND       ·
```
