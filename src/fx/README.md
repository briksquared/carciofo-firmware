# fx — Alia-style multi-effect (19 effects)

A switchable multi-effect for the WGD boards, companion to `iteritas`. It processes the **stereo
audio input** through one of 14 effects, chosen live. Clean-room DSP + DaisySP effects/filters. The
input-processing counterparts to the voice bank — this is where the local `metal`/`soap`/`grains`-
style engines belong (they process audio in, they aren't voices).

## Effects (P4 is always dry/wet mix)

| # | Effect | A | B | C |
|---|---|---|---|---|
| 0 | CHORUS | depth | rate | feedback |
| 1 | FLANGER | depth | rate | feedback |
| 2 | PHASER | centre | rate | feedback |
| 3 | TREMOLO | rate | depth | waveform |
| 4 | AUTOWAH | wah | – | level |
| 5 | DRIVE | drive | – | – |
| 6 | FOLD | gain | offset | – |
| 7 | CRUSH | downsample | bit depth | – |
| 8 | LADDER | cutoff | resonance | drive |
| 9 | SVFILT | cutoff | resonance | LP/BP/HP |
| 10 | REVERB | size | damping | – |
| 11 | DELAY | time | feedback | tone |
| 12 | PITCH | transpose ±12 | grain | – |
| 13 | RING | carrier freq | – | – |
| 14 | COMB | comb resonator | length | feedback | – |
| 15 | LOFI | bitcrush + low-pass | downsample | bit depth | tone |
| 16 | GATE | rhythmic gate | rate | width | smooth |
| 17 | WSHAPE | waveshaper | drive | asymmetry | tone |
| 18 | OCTAVE | octave-up rectifier | tone | – | gain |

REVERB is a clean-room freeverb-style network (4 combs + 2 allpass); DELAY is a 1 s feedback line;
COMB is a short tuned feedback line; RING/OCTAVE/WSHAPE are clean-room. The rest wrap DaisySP.

## Controls

**Carciofo** (buttons; LED optional): `P1` A (+CV1) · `P2` B · `P3` C (+CV2) · `P4` mix ·
`B1` next effect · `B2` previous · `IN L/R → OUT L/R`.

**Löwenzahnhonig** (4 knobs): `P1` **effect select** (14) · `P2` A (+CV1) · `P3` B (+CV2) ·
`P4` mix.

The signal is summed to mono, processed, and sent to both outputs.

## Build & flash — BOOT_QSPI

Runs from QSPI (holds the delay/reverb buffers in SDRAM). `APP_TYPE = BOOT_QSPI`.

```
make -C src/fx
```

Carciofo ≈ 98 KB, Löwenzahnhonig ≈ 85 KB. Flash is the two-step QSPI method (bootloader to
`0x08000000`, cold replug, app to `0x90040000`) — same as `iteritas`; the Daisy Web Programmer does
both steps. See `../iteritas/README.md` for the full recipe.

## Adding effects
Add a member/case in `alia_fx.h` (`FxBank`) + a `kFx[]` row. Effects that need buffers (delay,
reverb, pitch) work because the host places the whole `FxBank` in `DSY_SDRAM_BSS`.
