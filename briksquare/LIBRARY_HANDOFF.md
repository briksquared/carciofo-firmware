# Carciofo DSP library — handoff

## Vision
The Carciofo is one WGD module with a Daisy Seed inside. The product is not a
single firmware. It is a library of **hundreds** of flashes, each one a stand-in
for an expensive third-party Eurorack module (Mutable, Noise Engineering, Qu-Bit,
and the rest of the open DSP that actually runs on this chip).

Rule: a name goes in the catalog only when the DSP behind it is real (DaisySP,
an existing open port, or a port we just built). Do not ship an empty label.

Same panel every time: P1–P4, CV1/CV2, B1/B2, stereo in/out. See
`briksquare/PORTS.md` for the map. Bench: the built module works; only the
RGB LED is dark.

## What exists now
- Knob map: `briksquare/PORTS.md`. Detailed 2026-09-27 session record:
  `briksquare/SESSION_2026_09_27_LIBRARY.md`. Chronological log:
  `BRIKSQUARE_LOG.md`.
- **New this session, in `src/<name>/`:** 43 apps beyond the original 22.
  Panel count in PORTS.md is **59 ports plus `nimbus`**. Every one linked
  for internal flash (vector at `0x08000000`). None of them has been flashed
  onto the module.
- **Bins are not in this git repo.** Staged copies are
  `hardware/daisy/SEED_READY/carciofo-official/<name>.bin` on the desktop
  tree. Converted index: `hardware/daisy/SEED_READY/converted/INDEX.tsv`
  — **454 built, 1 FAIL**. The FAIL is `vendor/libdaisy-latest/core` (the
  library, not a firmware). Do not rerun `convert_daisy_firmwares.sh`; it
  overwrites the index. Append with Python UTF-8 only.
- **Reproducible generator:** `briksquare/tools/gen_ports.py`. Do not rerun
  it as written — it regenerates the original 20 apps and would overwrite
  them. Add a new directory instead.
- DaisySP in this tree **does** include ReverbSc and Compressor. They live
  in `lib/DaisySP/DaisySP-LGPL` and link only when the makefile sets
  `USE_DAISYSP_LGPL = 1` before the libDaisy include.

## Build recipe
Shared libs = this repo's `lib/libDaisy` + `lib/DaisySP`. Per app:
`cd src/<name> && make` → `build/<name>.bin`. On this Windows machine,
prepend Git `usr\bin`, the Arm GNU 14.2.1 toolchain, and ezwinports `make`
to PATH, and pass `LIBDAISY_DIR` / `DAISYSP_DIR` with forward slashes.
`make clean` fails here (no `rm`). GNU make command-line variables override
makefile assignments, which is how `BUILD_DIR` was isolated for Hothouse.
Apps whose DSP lives next door (`metal`, `ringsverb`, `cloudseed`) will
not link from a checkout that does not also have
`loewenzahnhonig-firmware` as a sibling of `carciofo-firmware` under
`wgd-ports/`.

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
`while(1){ hw.ProcessControls(); /* set params from pots/CV/buttons */ }`.
Add a new `src/<name>/` directory. Do not rerun `gen_ports.py` over the
existing apps.

## Status / caveat
The new apps in `SESSION_2026_09_27_LIBRARY.md` linked. None of this wave
has been heard on the module. Older likely tuning, still unflashed: autowah
wet/dry scaling, the one-pole tone normalization in overdrive/delay.

## Next (build order — from `audio/DSP_DREAMS/EURORACK_DSP.md`)
1. Clouds/Nimbus — **built**, not flashed. `nimbus.bin`.
2. Plaits-style macro voice — **built**, not flashed. `plaits.bin` (VA, FM, formant, VOSIM, ZOsc, additive).
3. Drum kit — **built**, not flashed. `drums.bin` (kick, 808 snare, 909 snare, hat).
3. West-coast chain — **built**, not flashed. `westcoast.bin` (OnePole follow, Wavefolder, Svf LPG, ReverbSc in SDRAM). `folder.bin` is the separate fuzz.
4. Drum kit — **built** as `drums.bin` and `kick.bin`. Grids is `grids.bin` (see item 7), not a new drum voice.
5. Deeper — **built**, not flashed: `vosim.bin`, `zosc.bin`, `grainlet.bin`, `particle.bin`, `dust.bin`, `modal.bin`. `drip` has Init and Process only (no setters), so it is not a panel port. Rings-Parasites (Oliverb/Resonestor) is not built.
6. Elements, Marbles-style clock, Stages envelope, Soap, organ bank, variable saw, fractal noise, compressor, stiff string, band-limited osc — **built**, not flashed. `elements.bin`, `marbles.bin`, `stages.bin`, `soap.bin`, `oscbank.bin`, `saw.bin`, `fractal.bin`, `compress.bin`, `stiff.bin`, `blosc.bin`.
7. Grids — **built**, not flashed. `grids.bin`. Drum mode interpolates the 25 Mutable node tables (32 steps × kick/snare/hat). B1 switches to the Euclidean LUT (1024 words). `moog.bin`, `comb.bin`, and `mode.bin` are the remaining LGPL filters with a panel map.
8. Looper, granular buffer, poly pluck, pluck, jitter, fold, bitcrush, allpass, nlfilt — **built**, not flashed. Rings-Parasites (Oliverb/Resonestor) is not in this source tree; the only local Parasites files are Clouds `.wav` flash dumps for the STM32F4 module, which do not run on the Seed.
9. Rings plate, metal distortion, splooge shimmer — **built**, not flashed. `ringsverb.bin` is `rings::Reverb` (Dattorro). `metal.bin` links `distortion_engine.cpp`. `splooge.bin` is the Loewenzahnhonig pitch/chorus/delay/reverb chain on the Carciofo pots.
10. Cloud Seed, smoodge, ringmod, dual VCA — **built**, not flashed. `cloudseed.bin` uses the Löwenzahnhonig engine and its ten programs, audio block 48, CPU boost left off. `smoodge.bin` is the 16-second delay. `ringmod.bin` and `vca.bin` follow those firmwares' pot maps.
Reference: `DSP_DREAMS/DAISY_HARVEST.md` + `firmware/community_daisy_catalog.json`.
