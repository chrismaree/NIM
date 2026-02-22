# ESP32 Button -> Red LED Toggle (Wokwi + Cursor)

This is a minimal PlatformIO project for an ESP32 DevKit in Wokwi.

- Button on `GPIO5` uses `INPUT_PULLUP` (press = `LOW`)
- Red LED on `GPIO18` through a `220` ohm resistor
- Each button press toggles LED state

## Run in Cursor

1. Open `/Users/chris/projects/NIM/tests/button-test` in Cursor.
2. Install extensions if needed:
   - `PlatformIO IDE`
   - `Wokwi Simulator`
3. Build once so firmware artifacts exist:
   - Command Palette -> `PlatformIO: Build`
   - This should create `.pio/build/esp32/firmware.bin`
4. Start simulation:
   - Command Palette -> `Wokwi: Start Simulator`
5. Click the button (or press `Space`) to toggle the red LED.

## If you see "firmware.bin not found"

- Make sure the folder opened in Cursor is:
  - `/Users/chris/projects/NIM/tests/button-test`
- Run `PlatformIO: Build` first, then start Wokwi.
