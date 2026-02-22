# ESP32 + 4x MCP23017 + 27 Buttons/LEDs

This project uses four MCP23017 I2C port expanders:

- `0x20`, `0x21`: button inputs (27 total, active-low with pull-ups)
- `0x22`, `0x23`: LED outputs (27 total)

Behavior:
- Press any button and the matching LED turns ON.
- Release the button and the matching LED turns OFF.
- Button expanders signal `INTA` to ESP32 pins `GPIO34` and `GPIO35`, so reads only happen on changes.

## Run in Cursor

1. Open `/Users/chris/projects/NIM/tests/port-expander`.
2. Ensure extensions:
   - `PlatformIO IDE`
   - `Wokwi Simulator`
3. Build once with `PlatformIO: Build`.
4. Start with `Wokwi: Start Simulator`.

## Notes

- The simulation uses a custom chip binary:
  - `/Users/chris/projects/NIM/tests/port-expander/dist/mcp23017.chip.wasm`
- I2C pins:
  - SDA = `GPIO21`
  - SCL = `GPIO22`
- Interrupt pins:
  - `mcp_btn_a:INTA` -> `GPIO34`
  - `mcp_btn_b:INTA` -> `GPIO35`
