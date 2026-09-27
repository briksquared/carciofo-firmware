# Carciofo DSP library — handoff

## Vision
The Carciofo is a generic Daisy Seed DSP box: 4 pots, 2 CV, stereo audio in/out
(audio ins retaskable as CV), 2 buttons, WS2812. Goal: replace $$$ hardware
modules with firmware — build the DSP once, assign the knobs. See
[[carciofo-module-bringup]] memory for the bench state (module works; only the
RGB LED is dark).

## What exists now
- **22 module ports** in `src/<name>/` — knob maps in `briksquare/PORTS.md`.
  Built bins live (outside git, under the ignored hardware/ tree) at
  `hardware/daisy/SEED_READY/carciofo-ports/*.bin`.
- **Reproducible generator:** `briksquare/tools/gen_ports.py` (edit the `apps{}`
  dict to add a port) + `build_ports.sh` (batch build + stage).
- **Whole-repo conversion:** `briksquare/tools/convert_daisy_firmwares.sh` built
  188 existing Daisy projects → `SEED_READY/converted/` (INDEX.tsv).
- **Inventory:** `briksquare/DAISY_FIRMWARE_INVENTORY.md` — 668 Daisy projects
  across both GitHub roots (built with `tools/daisy_inventory.sh`).

## Build recipe
Shared prebuilt libs = this repo's `lib/libDaisy` + `lib/DaisySP` (⚠ trimmed —
**no ReverbSc, no Compressor**; verify any class in `lib/DaisySP/Source` before
using). Per app: `cd src/<name> && make` → `build/<name>.bin`.

## Flash
DFU = hold BOOT, tap RESET. `dfu-util -a 0 -s 0x08000000:leave -D <bin> -d ,0483:df11`.
The "Error sending dfu abort request" line is normal → **tap RESET to launch**.
Recover from DFU without flashing = tap RESET alone (no BOOT).

## Hardware layer (`lib/carciofo.h`)
`GetPot(POT_1..4)`, `GetCv(CV_1/2)` (inverted; idle ~0.001), `button[BUTTON_1/2]`,
`StartAudio`, `SampleRate`; stereo `in[0/1]` / `out[0/1]`; `led` (WS2812).

## Adding a port (the pattern)
Include `../../lib/carciofo.h` + `daisysp.h`; hold a DaisySP DSP object; in
`AudioCallback` read `in[]` / write `out[]`; in `main()` Init + StartAudio, then
`while(1){ hw.ProcessControls(); /* set params from pots/CV/buttons */ }`. Fastest
path: add an entry to `gen_ports.py` `apps{}` and rerun the generator + builder.

## Status / caveat
All 22 **compile clean**; they are **not yet sonically bench-validated** (library
is being built ahead of flashing). Likely tuning once flashed: autowah wet/dry
scaling, the one-pole tone normalization in overdrive/delay.

## Next (build order — from `audio/DSP_DREAMS/EURORACK_DSP.md`)
1. Clouds/Nimbus granular (needs the SDRAM buffer + a grain engine — a real port).
2. Plaits multi-engine macro-voice (fold va/fm/formant/additive into engine-select).
3. West-coast chain (env-follow → wavefolder → vactrol LPG → space) — unbuyable.
4. Drum kit (snare + hats to join kick) + a Grids rhythm LUT.
5. Deeper: vosim, zosc, grainlet/particle/dust noise voices, drip, modal, and
   Rings-Parasites (Oliverb/Resonestor) ideas.
Reference: `DSP_DREAMS/DAISY_HARVEST.md` + `firmware/community_daisy_catalog.json`.
