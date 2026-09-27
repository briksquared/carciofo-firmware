# Carciofo firmwares — ready to flash

The Carciofo has a Daisy Seed inside. These `.bin`s flash to the Seed's internal
flash at `0x08000000`. All are the official WGD apps (plus a couple of my test
tools). Flash one with the helper:

```
./flash_carciofo.sh <app>
```

## Entering DFU / the reset dance
- **Enter DFU (to flash):** hold **BOOT**, tap **RESET**, release. (Rack power may
  stay connected when you use the RESET method.)
- **After a flash:** dfu-util prints `Error sending dfu abort request` — this is
  **normal** and means the Seed did *not* auto-reboot. **Tap RESET once** to launch.
- **Recover from DFU without flashing:** tap **RESET alone** (do NOT hold BOOT).

## The apps

### nimbus  — Clouds granular (Mutable / Electro-Smith DSP)
Fully wet. Audio in is recorded; audio out is the cloud. Not flashed yet.
| P1 (+CV1) position | P2 size | P3 pitch ±2 oct | P4 see mode |
| CV2 freeze gate | B1 cycle mode | B2 latch freeze |
B1 modes: **granular** (P4 = density; clockwise half opens texture + reverb),
**stretch** (P4 = filter), **looping delay** (P4 = feedback). LED colour follows
mode; white = frozen. Panel LED stays dark until the WS2812 is reflowed.

### reverb  — stereo reverb + freeze + shimmer
| P1 dry | P2 wet | P3(+CV1) decay | P4(+CV2) tone | B1 freeze | B2 shimmer(+oct) |

### quattro  — four fx in series: drive → chorus → delay → reverb
Buttons are shift layers (hold), soft-pickup so nothing jumps. Both held → B2 wins.
| Layer | P1 Drive | P2 Chorus | P3 Delay | P4 Reverb |
|---|---|---|---|---|
| Character (no button) | drive | rate | time | size |
| Motion (hold B1) | tone | depth | feedback | tone |
| Mix (hold B2) | wet | wet | wet | wet |
CV1 → delay time, CV2 → reverb size. Mix at 0 = true bypass.

### doppio  — two independent drum voices, one per output
Left = P1 tune / P3 decay / CV1 trigger in / B1 model (hold = 808↔909) / L out.
Right = P2 tune / P4 decay / CV2 trigger in / B2 model (hold = 808↔909) / R out.
Models: kick, snare, hat, clap — each in 808 and 909 flavours. Trigger height = velocity.
Short press steps model, long press toggles flavour; either fires once so you hear it.

### acciaio  — "Battito In Acciaio": percussive 6-osc synth voice
Trigger on CV1 (height = velocity). B1 steps mode: Skin / Liquid / Metal.
Hold B2 for the second layer (soft-pickup). CV2 = pitch (~1V/oct), summed with Pitch.
| Control | Default | Hold B2 |
|---|---|---|
| P1 | Pitch | Spread |
| P2 | Morph | Harmonics |
| P3 | Fold | Attack (noise·pop·slow) |
| P4 | Decay | Noise |

### hardware-test  — WGD factory bring-up (LED + USB serial driven)
Cycles the LED R/G/B/W on boot; B1 selects a control, B2 swaps tones/input; prints values.

### iotest  — audio-first full I/O test (briksquare)
OUT L osc pitch=P1(+CV1), tone=P2; OUT R pitch=P3(+CV2), tremolo=P4; B1/B2 = octave
±3; hold BOTH buttons = audio-in monitor (plug a source into IN L/R). Prints
`P1..P4 CV1 CV2 B1 B2 oct mon` over USB serial 4×/sec. Use this to verify hardware
without a screen (see HANDOFF.md for the serial-read one-liner).

### tonetest / blinktest / ledtest  — minimal diagnostics
Constant 440 Hz tone / onboard-LED blink / WS2812 drive. For isolating faults.

## Note on the RGB LED
The panel WS2812 (on D5) is currently dark on this build of the module — every
other I/O works (see HANDOFF.md). Apps that use the LED for readout (quattro,
acciaio, hardware-test) still work fully; you just won't see the colour cues until
the LED is reflowed.

## Library wave, 2026-09-27 — linked, not flashed

These are the apps added under `src/` after the original WGD set. Knob text
is `PORTS.md`. Sizes, SDRAM, and the things that must not be “cleaned up”
are `SESSION_2026_09_27_LIBRARY.md`. Same DFU address, `0x08000000`.

| bin | bytes | bin | bytes |
|---|---:|---|---:|
| westcoast | 98020 | vosim | 95376 |
| zosc | 96000 | grainlet | 95632 |
| particle | 96908 | dust | 96276 |
| modal | 97956 | elements | 99364 |
| marbles | 95824 | soap | 98376 |
| oscbank | 94656 | saw | 93884 |
| fractal | 99360 | compress | 93048 |
| stiff | 103036 | stages | 94776 |
| blosc | 95668 | grids | 112024 |
| moog | 96144 | comb | 93872 |
| mode | 93552 | looper | 94072 |
| grains | 96012 | polypluck | 94952 |
| pluck | 94764 | fold | 91700 |
| bits | 99972 | allpass | 92576 |
| nlfilt | 93020 | jitter | 95132 |
| ringsverb | 92748 | metal | 100408 |
| splooge | 101900 | cloudseed | 129028 |
| smoodge | 97596 | ringmod | 95220 |
| vca | 91532 | | |

`cloudseed` is 98.44% of the 128 KB bank and uses a 48-sample block.
`fold` is not `folder`. `bits` is not `crush`. `ringsverb` is the plate,
not `carciofo_rings`. Copies of the bins are in
`hardware/daisy/SEED_READY/carciofo-official/` and are not part of this
git repo.
