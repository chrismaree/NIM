# NIM on ESP32 + MCP23017 Expanders

This project implements a 2-player Nim game in a `1-3-5-7` layout.

Hardware model in Wokwi:
- 16 token buttons (`btn1..btn16`)
- 16 token LEDs (`led1..led16`)
- 2 side LEDs for turn indicator (`turn_p1`, `turn_p2`)
- 1 `START` button to reset/new game
- 4 MCP23017 custom chips on I2C

## Rules Implemented

- Standard Nim move shape: choose one row and remove one or more tokens.
- UI mapping: one click removes the clicked token and all tokens to its right in that row.
- Turn ends immediately after one click.
- Normal win rule: player who takes the last token wins.
- On win, the winner's side LED blinks slowly until `START` is pressed.
- Press `START` anytime to begin a new game.

## Pin/Address Use

- I2C: `SDA=GPIO21`, `SCL=GPIO22`
- Button expanders:
  - `0x20` => token buttons 1..16
  - `0x21` => `START` button (GPA0)
- LED expanders:
  - `0x22` => token LEDs 1..16
  - `0x23` => turn LEDs (`GPA0=P1`, `GPA1=P2`)
- Interrupt lines:
  - `mcp_btn_a:INTA -> GPIO34`
  - `mcp_btn_b:INTA -> GPIO35`

## Run in Cursor

1. Open `/Users/chris/projects/NIM/tests/port-expander`.
2. Ensure extensions:
   - `PlatformIO IDE`
   - `Wokwi Simulator`
3. Build once with `PlatformIO: Build`.
4. Start with `Wokwi: Start Simulator`.
