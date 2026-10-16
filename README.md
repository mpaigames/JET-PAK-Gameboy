# Jet Pak - Game Boy and Master System ports

One shared game core in C (`common/`), two thin front ends (`gb/`, `sms/`).

| | Game Boy | Master System |
|---|---|---|
| Screen | 160x144 | 256x192 |
| Toolchain | GBDK-2020 | SDCC + devkitSMS |
| Output | `gb/jetpak.gb` | `sms/jetpak.sms` |
| Move | D-pad | D-pad |
| Thrust | A or Up | Button 2 or Up |
| Laser | B | Button 1 |
| Start | Start or B | Button 1 |

## Build
**Game Boy:** install GBDK-2020, then `cd gb && make GBDK_HOME=/path/to/gbdk`.
**Master System:** install SDCC and devkitSMS (build `ihx2sms`), then
`cd sms && make DEVKITSMS=/path/to/devkitSMS IHX2SMS=/path/to/ihx2sms`.
Run the ROMs in an emulator (mGBA, SameBoy, Emulicious, Mesen...) or on a flash cart.

## Status - please read
Not yet built or run on real ROM toolchains by the author (none were available).
Verified so far:
* `make test` - the shared core (`common/game.c`) compiles clean and a bot completes levels with
  enemies disabled, plus 400,000 frames of random-input fuzzing under AddressSanitizer/UBSan, for both screen sizes.
* The front ends (`gb/main.c`, `sms/main.c`) pass a syntax check against hand-written stub headers.
  The stubs are my guess at the GBDK / SMSlib API, so expect to fix small API mismatches at first build.
* `preview_*.png` are host-side renders of the same tile data and scene code, not emulator screenshots.
Untested: real timing/feel, sound (register values are approximations), sprite flicker limits on hardware.

## Layout
* `common/game.c` - all gameplay (physics, enemies, rocket, scoring); positions are pixels*16.
* `common/scene.c` - turns game state into sprites + background tiles for either console.
* `tools/gen_tiles.py` - draws all art (4 colours) and writes the tile headers; re-run with `make tiles`.
* High score lives in RAM only (no battery save yet).

## Enemies (4 types, mixed more as levels rise)
saucer (drifts with a wobble), spore (chases you), orb (bounces), bat (zigzags).

## Build the Game Boy ROM in the cloud (GitHub Actions)
1. Create a GitHub repo and push the contents of this folder to the `main` branch
   (keep the hidden `.github/workflows/build-gb.yml`).
2. Open the repo's **Actions** tab: "Build Game Boy ROM" runs on every push, or press **Run workflow**.
3. When it finishes (about 1-2 minutes) open the run and download **jetpak-gameboy-rom** from Artifacts.
   Unzip it to get `jetpak.gb` and open it in an emulator (mGBA, SameBoy, Emulicious).
The workflow runs `make test` first, downloads GBDK-2020 (version set at the top of the file), then builds the ROM.
