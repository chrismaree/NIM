# Fusion 360 NIM Enclosure Generator

This folder contains a Fusion 360 Python script that generates a box/enclosure sized for the Nim control layout used in this project.

## File

- `/Users/chris/projects/NIM/tests/port-expander/cad/nim_box_fusion360.py`

## What it builds

- Outer box: `230 x 160 x 50 mm`
- Wall thickness: `3 mm`
- Top thickness: `3 mm`
- Floor rim with bottom service opening for wiring/battery access
- Top panel holes:
  - `7 mm` button holes (16 token buttons + 1 start/reset)
  - `3 mm` LED holes (16 token LEDs + 2 turn LEDs)
- Nim pattern: `1-3-5-7`
- Raised text:
  - `PLAYER 1`
  - `PLAYER 2`
  - `NIM` on the side

## Run in Fusion 360

1. Open Fusion 360.
2. Go to `Utilities` -> `Scripts and Add-Ins`.
3. Add or copy the script into your Fusion scripts folder.
4. Run `nim_box_fusion360.py`.
5. Save the generated design as `.f3d`.

## Notes

- This is a generated "best fit" enclosure and may need minor tuning after first print:
  - button bezel diameter/clearance
  - LED holder style (flush vs recessed)
  - battery holder mounting pattern

