# Session handoff — 2026-09-27 Carciofo library wave

This is the record of the firmware written after the catalog of existing
Daisy Makefiles was finished (INDEX 417 built / 1 FAIL). Nothing in this
wave was written to the module. The RGB LED being dark is still the only
accepted hardware fault (solder on the WS2812, not firmware).

Repo: `https://github.com/wgd-modular/carciofo-firmware`
Branch these files belong on: `briksquare/carciofo-tools`.
Sibling tree the external DSP comes from:
`hardware/daisy/wgd-ports/loewenzahnhonig-firmware/` (not a submodule of
this repo).

## Hardware the bins assume

Carciofo, WGD 6HP, Daisy Seed3 already built and on the desk. `lib/carciofo.h`:

| Control | Where |
|---|---|
| P1–P4 | ADC 15–18, `GetPot(POT_1..4)` |
| CV1, CV2 | ADC 19/20, inverted (`GetCv = 1 - adc`), idle about 0.001 |
| B1, B2 | D6, D7, active-low pull-up, `button[BUTTON_n].RisingEdge()` / `Pressed()` |
| Audio | stereo in/out, `in[0/1]`, `out[0/1]` |
| LED | WS2812 on D5. Dark until reflow. Do not drive it as a bring-up test in a loop. |
| Default audio block | 16 samples (`Carciofo::Init`). Cloud Seed overrides this. |

DFU for every bin in the size table below: hold BOOT, tap RESET,
`dfu-util -a 0 -s 0x08000000:leave -D <bin> -d ,0483:df11`, then tap RESET.
The line `Error sending dfu abort request` is the normal `:leave` abort.
Recover from DFU with no write: tap RESET, do not hold BOOT.

`.text` in these new apps sits after a vector table whose load address is
`0x08000000` (readelf: `.isr_vector` PROGBITS `08000000`). That is an
internal-flash image. Do not apply that recipe to SRAM-boot or QSPI images
from other trees (Jupiter, Torus, GuitarPedal, Hothouse, DSP Dreams
`dsp_dreams.bin`). Those are listed in `BRIKSQUARE_LOG.md` and are not part
of this wave.

## What “linked” means

`make` exited 0 and printed a FLASH usage under 128 KB. A `.bin` on disk
from a failed link was not indexed. Bytes below are the file size of
`build/<name>.bin`, which matched the FLASH line.

Staged copies (not committed; the path is outside this git root):

`C:/Users/briks/Desktop/GitHub/hardware/daisy/SEED_READY/carciofo-official/`

Index rows appended with Python in UTF-8 (no BOM). Key shape:

`hardware__daisy__wgd-ports__carciofo-firmware__src__<name>.bin`

Source column is the absolute forward-slash path of `src/<name>`.

## Size table — this wave

All of these are internal flash. SDRAM figures are `.sdram_bss` NOBITS at
`0xC0000000` and are not in the `.bin`.

| bin | bytes | flash % | SDRAM | class / source |
|---|---:|---:|---:|---|
| westcoast | 98020 | 74.78 | ReverbSc object (about 396 KB) | OnePole, Wavefolder, Svf, ReverbSc |
| vosim | 95376 | 72.77 | — | VosimOscillator |
| zosc | 96000 | 73.24 | — | ZOscillator |
| grainlet | 95632 | 72.96 | — | GrainletOscillator |
| particle | 96908 | 73.93 | — | Particle |
| dust | 96276 | 73.45 | — | Dust into Svf |
| modal | 97956 | 74.73 | — | ModalVoice |
| elements | 99364 | 75.81 | — | Particle + Dust + ClockedNoise into Resonator (resolution 24) |
| marbles | 95824 | 73.11 | — | Metro, SmoothRandomGenerator, SampleHold, Maytrig, AdEnv, VariableSawOscillator |
| soap | 98376 | 75.05 | — | Soap |
| oscbank | 94656 | 72.22 | — | OscillatorBank, 7 amplitudes renormalized to sum 1 |
| saw | 93884 | 71.63 | — | VariableSawOscillator |
| fractal | 99360 | 75.81 | — | FractalRandomGenerator<ClockedNoise, 4> into Soap |
| compress | 93048 | 70.99 | — | two Compressor instances, L and R |
| stiff | 103036 | 78.61 | — | daisysp::String (input-excited Karplus) |
| stages | 94776 | 72.31 | — | Phasor retriggers AdEnv on VariableSawOscillator |
| blosc | 95668 | 72.99 | — | BlOsc |
| grids | 112024 | 85.47 | — | copied Grids nodes + Euclidean LUT, AnalogBassDrum, AnalogSnareDrum, HiHat |
| moog | 96144 | 73.35 | — | MoogLadder |
| comb | 93872 | 71.62 | 8192 samples | Comb |
| mode | 93552 | 71.37 | — | Mode |
| looper | 94072 | 71.77 | 96000 samples (2 s) | Looper |
| grains | 96012 | 73.25 | 48000 samples | GranularPlayer over a live write |
| polypluck | 94952 | 72.44 | — | PolyPluck<4> |
| pluck | 94764 | 72.30 | 4096 samples | Pluck |
| fold | 91700 | 69.96 | — | Fold (sample-hold), not Wavefolder |
| bits | 99972 | 76.27 | — | Bitcrush, not Decimator |
| allpass | 92576 | 70.63 | 4096 samples | Allpass. SetFreq argument is loop time in seconds |
| nlfilt | 93020 | 70.97 | — | NlFilt, ProcessBlock of one sample |
| jitter | 95132 | 72.58 | — | Jitter into VariableSawOscillator |
| ringsverb | 92748 | 70.76 | 32768 × uint16 = 65536 B | rings::Reverb |
| metal | 100408 | 76.61 | — | DistortionEngine (unchanged .cpp) |
| splooge | 101900 | 77.74 | 1487380 B | PitchShifter, Chorus, two DelayLine<float,120000>, ReverbSc, two Svf |
| cloudseed | 129028 | 98.44 | 15990784 B | cloudseed_daisy::Engine, 10 programs |
| smoodge | 97596 | (linked, under 128 KB) | 6144024 B | two DelayLine<float, 48000*16>, Chorus, two Tremolo |
| ringmod | 95220 | (linked, under 128 KB) | — | three Oscillator |
| vca | 91532 | (linked, under 128 KB) | — | multiply only |

`smoodge`, `ringmod`, and `vca` did not reprint a `FLASH:` summary line in
the captured log because the filter looked for a line starting at column 0.
The linker wrote the bins above and `readelf` shows `.isr_vector` at
`0x08000000`.

Index after the last append: **454 rows starting with `built`**, **1 row
starting with `fail`**. Header makes the file longer than 455.

## Panel behavior that is easy to get wrong

`briksquare/PORTS.md` is the knob table. These notes are the ones a later
edit will break if they are “cleaned up”:

- `folder.bin` is DaisySP `Wavefolder` (the fuzz). `fold.bin` is DaisySP
  `Fold`, which holds the last input sample for `incr` counts. They are not
  substitutes.
- `crush.bin` is `Decimator`. `bits.bin` is `Bitcrush` (bit depth 1–16 and
  a crush rate in Hz).
- `karplus.bin` is `StringVoice` (self-excited, trigger). `stiff.bin` is
  `daisysp::String` (`Process(const float in)`), so the audio input is the
  string’s exciter. `pluck.bin` is the LGPL `Pluck` with its own buffer.
- `westcoast` follower coefficient is `OnePole::SetFrequency(0.004f)`. That
  argument is normalized, not Hz. The LPG cutoff is set in the audio
  callback: `80 + envelope * open * 8000` Hz. Reverb feedback is
  `0.55 + wet * 0.4`.
- `fractal` must be `FractalRandomGenerator<ClockedNoise, 4>`. The class is
  a template. `ClockedNoise` is the type because `Process()` calls
  `SetFreq` on each generator.
- `oscbank` amplitudes are forced to sum to 1 every control frame. The
  header says they must.
- `grids` drum mode: step 0..31, three instruments, level from bilinear
  mix of the 5×5 node map. Trigger when `level > (uint8_t)(~density)`.
  Accent when `level > 192`. Chaos is added only when `step == 0`, and the
  added amount is `(random_byte * (chaos >> 2)) >> 8`. Euclidean mode (B1)
  uses `kGridsEuclidean[(length-1)*32 + (density>>3)]`. Density at idle CV
  is about 80 and rises with CV1. Tempo is `bpm * 8 / 60` Hz because the
  original step clock is 8 steps per quarter note. Kick/snare/hat pitches
  are fixed in `main` (49 Hz, 180 Hz, 7000 Hz). There is no free pot left
  for pitch.
- `grids_lut.h` is the bytes from
  `loewenzahnhonig-firmware/lib/eurorack/grids/resources.cc`, nodes 0–24
  (96 bytes each) and `lut_res_euclidean` (1024 uint32). The U8Mix used is
  `(a * (255-balance) + b * balance) >> 8` with the sum widened to 16 bits.
  The portable `avrlib` one-liner without parentheses is not the same,
  because `>>` binds tighter than `+`. Mutable Grids is GPL-3.0-or-later.
  That header carries the copyright. Do not drop the notice.
- `ringsverb` does not replace `carciofo_rings`. Rings-the-resonator is
  already `carciofo_rings` (Resonator / StringVoice / ModalVoice, resolution
  32). `ringsverb` is only `rings::Reverb` from `rings/dsp/fx/reverb.h`,
  the Dattorro plate (four input allpasses, then two tanks). Buffer length
  matches `FxEngine<32768, FORMAT_16_BIT>`: 32768 uint16.
- `metal` compiles
  `loewenzahnhonig-firmware/src/distortion/distortion_engine.cpp` as a
  second translation unit. Do not edit that file to fit the panel. Pot
  order is the comment in `distortion.cpp`: distortion, low shelf, high
  shelf, level. CV1 adds to distortion, CV2 adds to the high shelf, each
  with weight 0.5, then clamped to 0..1.
- `splooge` follows `sploodge-reverb.cpp` (the source filename is spelled
  sploodge). One `PitchShifter` is transpositioned to +24 and then +12 on
  successive calls in the same sample. Delay feedback and the shimmer mix
  both follow pot 4, matching the original. Reverb feedback follows pot 1.
  The highpass after the reverb is `Svf::High()` with cutoff `500 * tone`.
- `cloudseed` uses audio block **48** (`kMaxBlockSize`). `Carciofo::Init`
  defaults to 16 so the bit-banged LED has time; this app calls `Init(48)`
  because the engine returns false for a block size that is not 48 or a
  divisor/multiple it was given in `Config::block_size`. `CLOUDSEED_BOOST`
  is 0. Do not turn the 480 MHz option on from a Carciofo makefile without
  being asked. Programs, in pot-1 order, are the nine factory presets plus
  `kDarkPlate`, with the line counts in `src/cloudseed`’s neighbor
  `programs.h` (that header is included from the Löwenzahnhonig tree, not
  copied). `OnProgramLoaded` forces `DryOut = 0` and `CutoffEnabled = 1`
  because this file mixes dry itself and the tone pot is the post cutoff.
  CV2 freeze thresholds are 0.55 on and 0.45 off. Main must call
  `engine.Service()`; the callback only `RequestProgram` / `Process`.
  FLASH is 98.44%. There is almost no room to add another translation unit
  to this binary.
- `smoodge` divisor steps are copied from `smoodge-delay.cpp` (8, 4, 3, 2,
  1.5, 1, 0.5, 0.33, 0.25, 0.2, 0.125 as the pot crosses each tenth). CV1
  ≥ 0.5 is a clock edge. Period is the average of the last three intervals,
  in seconds, then converted with `* 48000 / divisor`. The original
  `SetDelay` calls are swapped: the right line gets `currentDelayL` and the
  left line gets `currentDelayR`. That was kept. Wow is a sine whose
  amplitude is `9 * pot4`, added as `(20 * osc) * pot4²` samples. Tremolo
  rate steps with the clock (`(1 + (n % 10)) / 10`) instead of `std::time`,
  because the original `srand(time)` call is not a bare-metal function we
  should depend on. Both delay targets are clamped to `[1, kMaxDelay-2]`
  inside the audio callback.
- `ringmod`: left in is the modulator, right out is the carrier. Shape
  crossfades sine→triangle and then that result toward square, which is
  what `ringmod.cpp` does (the square is mixed in with the same `shape`
  knob, so the sine never fully leaves). Frequency is exponential from
  1 Hz to 16 kHz.
- `vca`: `left = clamp(P1 + P3*CV1)`, `right = clamp(P2 + P4*CV2)`. No
  audio is generated.

## Makefiles that are not self-contained

| app | extra |
|---|---|
| westcoast, compress, blosc, moog, comb, mode, polypluck, pluck, fold, bits, allpass, nlfilt, jitter, splooge | `USE_DAISYSP_LGPL = 1` before the include |
| metal | `CPP_SOURCES` adds `../../../loewenzahnhonig-firmware/src/distortion/distortion_engine.cpp` |
| ringsverb | `-I../../../loewenzahnhonig-firmware/lib/eurorack` |
| cloudseed | includes `cloudseed-daisy/cloudseed.mk`. `CLOUDSEED_DAISY_DIR` points at the sibling tree. `C_INCLUDES` adds `loewenzahnhonig-firmware/src/cloudseed` for `programs.h` and `mix.h` |

`lib/DaisySP` here has a nested `DaisySP-LGPL/DaisySP-LGPL` copy of the same
headers. The makefile flag links `libdaisysp-lgpl`. Include `daisysp.h`;
the LGPL headers come in only when `USE_DAISYSP_LGPL` is defined. Do not
include the nested duplicate path by hand.

## What was deliberately not built

- `Drip`: header exposes `Init(sample_rate, dettack)` and `Process(bool)`.
  No setters. A trigger-only drip with no knob was not given a name.
- Oliverb and Resonestor. Searched the trees. The only Parasites artifacts
  are Clouds `.wav` / `.hex` dumps for the STM32F4 module under
  `audio/eurorack/briksquare-rack/firmwares/`. They do not run on the Seed.
  Do not rename Nimbus and call it Parasites.
- Full `PatternGenerator` / `avrlib` / AVR EEPROM. The drum-map bytes and
  the Euclidean table were extracted instead.
- `vendor/libdaisy-latest/core`, `tests`, and `DaisyExamples/utils/Template`.
  Not product firmware. The core row stays FAIL.
- NeuralSeed still has commented-out `setupWeights()`. The earlier bin is
  real and the models are empty. Do not invent weights.
- A second copy of the Hothouse / DSP Dreams QSPI images. Already indexed.
  Do not DFU them at `0x08000000`.
- `gen_ports.py` was not rerun. `convert_daisy_firmwares.sh` was not rerun.

## Build environment that actually worked

PATH prefix, in this order:

1. `C:\Program Files\Git\usr\bin` (GNU `mkdir`, because Windows `mkdir` rejects `mkdir -p`)
2. `C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\14.2 rel1\bin`
3. ezwinports `make` from WinGet

`LIBDAISY_DIR=C:/Users/briks/Desktop/GitHub/hardware/daisy/wgd-ports/carciofo-firmware/lib/libDaisy`
`DAISYSP_DIR=C:/Users/briks/Desktop/GitHub/hardware/daisy/wgd-ports/carciofo-firmware/lib/DaisySP`

PowerShell `Start-Process` strips backslashes. Use the call operator and
forward slashes. Do not set `OPT` on the make command line for these small
apps; the libDaisy makefile’s default is enough and they all fit.

## Checks not run

- No DFU. No listening test. No serial dump of pots after these new bins.
- No ERC/DRC. This is not a PCB change.
- `smoodge` / `ringmod` / `vca` flash percentage was not re-printed. Sizes
  and the vector address were.
- Cloud Seed was not run at 48-sample blocks on the hardware, so the CPU
  budget (`cpu_budget = 0.9` inside the engine) is unmeasured on the
  Carciofo clock with boost off.

## Next real gap

DaisySP’s effect, filter, drum, and oscillator headers that have a panel
mapping are now in `src/`. Another “keep going” should be an engine that
already exists as source (another Löwenzahnhonig or eurorack port), not a
new label. Oliverb still has no source to compile.
