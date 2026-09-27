# Carciofo bench log

## 2026-09-27 — bench bring-up of the hand-built module

Module = WGD Carciofo, Daisy Seed3 inside. Verified full I/O without a screen by
reading the USB-serial telemetry from a purpose-built test app.

- Wrote `src/iotest/` (audio-first full I/O test): pots -> osc pitch/tone, CV ->
  pitch, buttons -> octave, both-buttons -> audio-in monitor, RGB + onboard LED
  driven, all values printed over USB serial 4x/sec. Built ~98 KB, staged at
  `../../SEED_READY/carciofo-official/iotest.bin`.
- Serial verify (COM port, VID_0483 PID_5740): P1-P4 all sweep; B1/B2 move oct
  -3..+3; CV1 tracked a Make Noise Maths ramp 0.14 -> 0.99 cleanly; stereo audio
  in/out pass. No spontaneous reboots (boot banner never re-appeared) = stable.
- ONLY fault: WS2812 RGB (D5) stays dark. App runs + onboard LED blinks + all I/O
  works => firmware/flash/MCU proven; LED is soldered wrong (user's read). Pins
  diagonal, so 180deg flip reverse-powers it — do not blind-rotate. Cosmetic only.
- Flash quirk logged: dfu-util `:leave` prints "Error sending dfu abort request"
  and does NOT auto-reboot — tap RESET once to launch. Recover from DFU = tap
  RESET alone (no BOOT). This was the "keep hitting reset" friction.
- Left the module on the intended WGD **reverb** firmware (P1 dry / P2 wet /
  P3+CV1 decay / P4+CV2 tone / B1 freeze / B2 shimmer). Confirmed washing.

## 2026-09-27 — nimbus (Clouds) firmware built, not flashed
- New app `src/nimbus/`. DSP objects come from the existing Löwenzahnhonig
  Nimbus port (`loewenzahnhonig-firmware/src/nimbus/dsp`); Carciofo `nimbus.cpp`
  only maps the panel. Links against this tree's libDaisy / DaisySP.
- Panel: P1+CV1 position, P2 size, P3 pitch (±24 st, centre dead-zone),
  P4 density (granular) / filter (stretch) / feedback (looping delay).
  CV2 freeze gate. B1 cycles the three Clouds playback modes. B2 latches freeze.
- `-O2` overflowed internal flash by 884 B. Makefile locks `OPT = -Os`.
  Linked: FLASH 121,924 / 131,072 (93%), SRAM 262,544 / 512 KB. Bin staged at
  `SEED_READY/carciofo-official/nimbus.bin`.
- Not flashed. Module is still on reverb.

## 2026-09-27 — catalog direction + two more mimics
- The library is the product: hundreds of Carciofo flashes, each standing in for
  an expensive third-party module. A name ships only with real DSP behind it.
- Built `plaits` (100,396 B): B1 steps VA, 2-op FM, formant, VOSIM, ZOsc, additive.
- Built `drums` (106,252 B): B1 steps kick / analog snare / synthetic snare / hat.
  CV2 or B2 triggers.
- Staged next to nimbus in `SEED_READY/carciofo-official/`. Not flashed.

## 2026-09-27 — conversion retry
- Restored DaisySP-LGPL into `carciofo-firmware/lib/DaisySP/DaisySP-LGPL` (`libdaisysp-lgpl.a` 40,202 B) so `daisysp-lgpl.h` resolves.
- Rebuilt the LGPL misses: Carciofo reverb 88,860 B and quattro 90,032 B (staged in `SEED_READY/carciofo-official/`), pitch 96,520 B (`carciofo-ports/pitch.bin`, `SetTransposition`), briksquare-daisy distortion / splooge-reverb / vca, AudreyTouch 114,888 B, TouchBass 126,584 B.
- Cloned `cloudseed-daisy` into the empty Löwenzahnhonig submodule and junctioned `lib/eurorack` to the local briksquare rings tree. Rebuilt vca, ringmod, splooge-reverb, distortion, smoodge-delay, rings-reverb, and cloudseed (113,916 B FLASH, 86.91%). Cloudseed's `image-crc` rule still wants `python3` on Windows; the bin is written before that rule.
- `SEED_READY/converted/INDEX.tsv`: 202 built, 6 still FAIL. Still failing: Spotykach (`Storage` template arity), official spotykach (`std::function`), official audrey-ii (missing `arm_common_tables.o`), simple arpeggiator and looper (`Pin` vs `dsy_gpio_port` against this libDaisy), and `vendor/libdaisy-latest/core` (library, not a firmware). Not flashed.

## 2026-09-27 — last conversion fails
- simple-arpeggiator-touch 105,328 B and simple-looper-touch 90,452 B. ADC pin list in `simple-daisy-touch.h` is `Pin`, matching this libDaisy `InitSingle`.
- official spotykach 118,916 B. Added `<functional>` in `i.generator.h`. Toggle pulls are `GPIO::Pull::PULLUP`.
- official audrey-ii (FeedbackSynth) 102,968 B. CMSIS tables come from carciofo `Drivers/CMSIS-DSP/Source`. Delay switch init takes `Pin` and `GPIO::Pull::PULLUP`. `USE_DAISYSP_LGPL=1` for `Tone` and `ReverbSc`.
- Spotykach (Synthux) 204,864 B. Built on bleeptools libDaisy branch `sk` (`PersistentStorage<T, slug, version>`), not the carciofo lib. SRAM boot image (`APP_TYPE=BOOT_SRAM`): FLASH used 0, code is in `SRAM_EXEC` 204,864 / 256 KB. Do not flash this bin at `0x08000000`.
- INDEX is 207 built, 1 FAIL. The last row is `vendor/libdaisy-latest/core`, the library itself, not a firmware. Not flashed.

## 2026-09-27 — daisy-pedals conversion wave
- Built the Daisy projects the first pass never scanned (`audio/firmwares`, `audio/pedals`, `audio/DSP_DREAMS`) against this libDaisy/DaisySP with `USE_DAISYSP_LGPL=1`.
- `SEED_READY/converted/INDEX.tsv`: **361 built, 26 FAIL**. This wave added 152 flash-sized bins plus two DSP Dreams images that linked into QSPI (`.text` at `0x90040000`, bins 4,905,920 B and 5,285,136 B). Those two are not an internal-flash DFU.
- 26 still FAIL. Cluster: Funbox petal apps call `DaisyPetal::InitMidi`, which this libDaisy petal does not have. Others: missing `stmlib`, `RTNeural`, `terrarium.h`, `q/fx/envelope.hpp`, old `dsy_gpio_write`, CMSIS `arm_fir_f32`, `WavPlayer` template, seed Osc link. `vendor/libdaisy-latest/core` is still not a firmware. Not flashed.

## 2026-09-27 — remaining FAIL pass
- `DaisyPetal::InitMidi` added from the Funbox petal mod (UART RX D30, TX D29). libDaisy archive rebuilt.
- Linked: Balnea, Cassini, FmSynth, SamplerTest, Pluto, Saturn, Venus, Gate_Output, HardwareTest, Sequencer, fir, Osc, Neptune. INDEX **374 built, 13 FAIL**.
- Neptune's `.text` is at `0x24000000` (SRAM), 138,080 B. Jupiter still overflows internal flash by 2,432 B after CloudSeed `-Os` (133,504 / 128 KB).
- Still failing, and why: Nimbus flash +7,496 B, Torus flash +2,568 B, ChorusVerb SRAM +208,240 B, MidiKeys missing `delayline_2tap.h`, Mars missing `RTNeural`, Uranus missing `q/fx/envelope.hpp`, NeuralSeed missing `terrarium.h`, WavPlayer method names do not match this libDaisy, GuitarPedal makefile requires gcc 10 (this machine is 14.2.1), the two `PEDAL_FAMILY` DSP Dreams makefiles point at `C:\Users\briks\Desktop\libDaisy`, and `vendor/libdaisy-latest/core` is not a firmware. Not flashed.

## 2026-09-27 — FAIL pass continued
- Linked and staged: Nimbus 123,848 B (`OPT = -Os` before the core Makefile include), MidiKeys 119,180 B (`-I../../Mars` for `delayline_2tap.h`), ChorusVerb 100,104 B (the two `PitchShifter`s, `Chorus`, and `ReverbSc` are `DSY_SDRAM_BSS`; FLASH 100,104 / 128 KB, SRAM 54,624), Uranus 113,204 B (cycfi `q` + `infra` cloned under Funbox), Mars 130,592 B (RTNeural + Eigen already on disk). Mars is `APP_TYPE = BOOT_SRAM`: FLASH used 0, code is in SRAM 130,592 / 480 KB. Do not flash that bin at `0x08000000`.
- GuitarPedal 407,640 B. Makefile now accepts gcc 14.2.1 as well as the upstream 10.x gate. `q` and `infra` are junctions to the Funbox clones; `gcem` is a depth-1 clone; CloudSeed's own `libcloudseed.a` was built after dropping `-Werror`. `frequency_detector_q.h` includes `q/fx/signal_conditioner.hpp` because current q has `signal_conditioner` as a using-alias, not a class. `APP_TYPE = BOOT_SRAM`: FLASH 0, SRAM 407,640 / 480 KB (82.93%). Do not flash at `0x08000000`.
- PEDAL_FAMILY DSP Dreams `firmware/daisy` 5,285,136 B and `firmware/daisy/main` 4,904,940 B. Stale `build/*.d` entries pointed at `Desktop\libDaisy`; deleted those, then `LIBDAISY_DIR` / `DAISYSP_DIR` were the carciofo trees. Both are `BOOT_QSPI` (FLASH 0, `.text` in QSPI). Do not DFU them at `0x08000000`.
- Torus still overflows internal flash by 456 B after `OPT = -Oz -flto` (131,528 / 128 KB). Jupiter still overflows by 2,432 B after CloudSeed and the app at `-Oz` (133,504 / 128 KB).
- Still FAIL: NeuralSeed (`terrarium.h`, `all_model_data.h`, and its own `../RTNeural` are not in this tree), WavPlayer method names do not match this libDaisy, `vendor/libdaisy-latest/core` is the library, not a firmware.
- INDEX **382 built, 5 FAIL**. Not flashed.

## 2026-09-27 — WavPlayer and NeuralSeed
- WavPlayer 102,704 B (FLASH 78.36%). The daisy-pedals example called the old index API (`GetCurrentFile`, `GetNumberFiles`, `Open(size_t)`, `Stream()`). This libDaisy is `WavPlayer<16384>::Init(const char*)` and `Stream(float*, channels)`, matching `libDaisy/examples/WavPlayer`. The Pod example now opens `loop.wav`, loops, and writes both channels. The encoder no longer steps through a directory.
- NeuralSeed 129,396 B (FLASH 98.72%). Pin map is PedalPCB `terrarium.h` (fetched into `NeuralSeed/Terrarium/`). `RTNeural` is a junction to the Funbox clone of jatinchowdhury18/RTNeural. `all_model_data.h` is the stock file: `setupWeights()` contains only comments, so the bin has the empty `Model1`–`Model13` shells and no trained weights.
- Jupiter LTO (`-Oz -flto`, CloudSeed archived with `gcc-ar`) cut the overflow from 2,432 B to 1,404 B (132,476 / 128 KB). Still over. Torus remains 456 B over at `-Oz -flto` (131,528 / 128 KB). `vendor/libdaisy-latest/core` is still the library.
- INDEX **384 built, 3 FAIL**. Not flashed.

## 2026-09-27 — Torus and Jupiter off the internal bank
- Internal flash is 128 KB. Torus at `-Oz -flto` was 456 B over (131,528). Jupiter after the same LTO pass was 1,404 B over (132,476).
- Torus `APP_TYPE = BOOT_QSPI`. Resonator BSS does not fit the SRAM-boot DTCM region (overflowed DTCM by 127,376 B). QSPI link: QSPIFLASH 131,368 B, SRAM 258,448 / 512 KB. Bin 131,368 B. Daisy bootloader address `0x90040000`. Do not DFU at `0x08000000`.
- Jupiter `APP_TYPE = BOOT_SRAM` (the makefile already had that line, commented). Link: FLASH 0, code in SRAM 132,304 / 480 KB, DTCM 55,004 / 128 KB. Bin 132,304 B. Same rule: do not DFU at `0x08000000`.
- INDEX **386 built, 1 FAIL**. The last row is `vendor/libdaisy-latest/core`, the library itself. Not flashed.

## 2026-09-27 — Carciofo effect apps harvested
- Twenty-two Carciofo apps under `src/` were already linked (make had nothing left to do) and had never been copied into `SEED_READY`. Each `.text` load address checked is internal flash (`0x080000A0`).
- Staged in `carciofo-official/` and appended to `converted/INDEX.tsv`: additive 95,432, autowah 95,568, carciofo_rings 109,484, carciofo_warps 97,572, chorus 92,924, crush 91,728, delay 96,136, flanger 92,160, fm 96,076, folder 91,688, formant 95,256, karplus 104,820, kick 100,776, ladder 96,236, limiter 91,740, overdrive 96,468, phaser 92,972, pingpong 91,992, pitch 96,520, svf 95,632, tremolo 94,368, va_osc 96,824. All under the 128 KB bank.
- INDEX **408 built, 1 FAIL**. The remaining FAIL is still `vendor/libdaisy-latest/core`. Not flashed.

## 2026-09-27 — Nimbus variants and the three unlisted Carciofo bins
- Löwenzahnhonig Nimbus linked four distinct images, all on internal flash. Hashes differ. Default 123,932 B (94.55%, SRAM 250,424). Lofi 123,932 B. Stretch 123,844 B (94.49%). Looping delay 123,844 B.
- Carciofo `nimbus` 121,924 B, `drums` 106,252 B, and `plaits` 100,396 B were already in `carciofo-official/` with `.text` at `0x080000A0` and were missing from the converted index. Appended.
- INDEX **415 built, 1 FAIL**. Not flashed.

## 2026-09-27 — cloudseed example and Hothouse
- Every Daisy app Makefile under `hardware/daisy` and `audio` is now in the index except `vendor/libdaisy-latest/core` (the library), its `tests`, and `DaisyExamples/utils/Template`.
- cloudseed-daisy Seed example 110,004 B, internal flash 83.93%.
- DSP Dreams `make HOTHOUSE=1` linked `hothouse_seed.bin` 5,089,676 B into `build-hothouse` so the Feather `dsp_dreams` objects stayed put. `BOOT_QSPI`: FLASH 0, QSPIFLASH 5,089,676 B (62.63%), SRAM 150,436 / 512 KB. Bootloader address `0x90040000`. Do not DFU at `0x08000000`. The PEDAL_FAMILY `hothouse_seed.cpp` hash matches this tree.
- Host tests in `firmware/daisy/test`: curve_map 25 pass, rungler 6 pass.
- INDEX **417 built, 1 FAIL**. Not flashed.

## 2026-09-27 — west-coast chain and the leftover DaisySP voices
- `westcoast.bin` 98,020 B (FLASH 74.78%). Input goes envelope-follower (OnePole, coeff 0.004) → Wavefolder → Svf lowpass opened by the follower → ReverbSc. The reverb object is `DSY_SDRAM_BSS` (396,100 B at `0xC0000000`). Vector table at `0x08000000`. DFU at `0x08000000`.
- Standalone engines, same bank: vosim 95,376, zosc 96,000, grainlet 95,632, particle 96,908, dust 96,276 (Dust into Svf), modal 97,956. `Drip` exposes Init and Process only, so it was not given a knob map.
- Staged in `carciofo-official/` and appended to `converted/INDEX.tsv`.
- INDEX **424 built, 1 FAIL**. The remaining FAIL is `vendor/libdaisy-latest/core`. Not flashed.

## 2026-09-27 — Elements, Marbles, Stages, and the unused DaisySP classes
- These classes were in DaisySP and had no `src/` app: `Resonator` fed by `Particle`+`Dust`+`ClockedNoise`, `SmoothRandomGenerator`+`SampleHold`+`Metro`+`Maytrig`+`AdEnv`, `Phasor`+`AdEnv`, `Soap`, `OscillatorBank`, `VariableSawOscillator`, `FractalRandomGenerator<ClockedNoise,4>`, `String` (input-excited Karplus), `Compressor`, `BlOsc`.
- Internal flash, vector at `0x08000000`: elements 99,364 (75.81%), marbles 95,824, soap 98,376, oscbank 94,656, saw 93,884, fractal 99,360, compress 93,048, stiff 103,036, stages 94,776, blosc 95,668. Fractal uses four clocked-noise generators because the class is a template. Compress and blosc set `USE_DAISYSP_LGPL=1`.
- Staged in `carciofo-official/` and appended to `converted/INDEX.tsv`.
- Grids `pattern_generator` still depends on `avrlib` and was not linked.
- INDEX **434 built, 1 FAIL**. Not flashed.

## 2026-09-27 — Grids map and the leftover LGPL filters
- Grids `pattern_generator.cc` stays in the eurorack tree (it includes `avrlib` and AVR EEPROM). The drum nodes and Euclidean LUT were copied out of `grids/resources.cc` into `src/grids/grids_lut.h` (25 × 96 bytes, 1024 uint32s). The step evaluator matches `ReadDrumMap`: index `x>>6` / `y>>6`, mix with `x<<2` and `y<<2`, threshold `~density`, chaos added at step 0. Euclidean uses `(length-1)*32 + (density>>3)`.
- `grids.bin` 112,024 B (FLASH 85.47%). Vector at `0x08000000`. Voices are `AnalogBassDrum`, `AnalogSnareDrum`, `HiHat`. B1 selects the map or the Euclidean table. Tempo is one step per eighth of a beat.
- `moog.bin` 96,144 (MoogLadder), `comb.bin` 93,872 (Comb, 8192 samples in SDRAM), `mode.bin` 93,552 (Mode). All three set `USE_DAISYSP_LGPL=1`.
- INDEX **438 built, 1 FAIL**. Not flashed.

## 2026-09-27 — looper, grains, and the remaining LGPL classes
- Oliverb / Resonestor source is not in the GitHub trees. The Parasites files on disk are Clouds audio flash dumps for the STM32F4, so they were not linked for the Seed.
- Internal flash, vector at `0x08000000`: looper 94,072 (96,000-sample loop in SDRAM), grains 96,012 (48,000-sample table), polypluck 94,952, pluck 94,764, fold 91,700, bits 99,972, allpass 92,576, nlfilt 93,020, jitter 95,132. `fold` is the DaisySP sample-hold Fold, not `Wavefolder` (`folder.bin`). `bits` is `Bitcrush`; `crush.bin` remains the decimator.
- INDEX **447 built, 1 FAIL**. Not flashed.

## 2026-09-27 — Rings plate, metal, splooge
- `ringsverb.bin` 92,748 B (FLASH 70.76%). DSP is the Mutable Rings header `rings/dsp/fx/reverb.h` (Dattorro topology). Buffer is 32,768 uint16 in `.sdram_bss` at `0xC0000000` (65,536 B). Vector at `0x08000000`.
- `metal.bin` 100,408 B. Links `loewenzahnhonig-firmware/src/distortion/distortion_engine.cpp` unchanged (4× oversampled shelves and clip). Pots match that firmware: distortion, low, high, level.
- `splooge.bin` 101,900 B. Same signal path as `sploodge-reverb.cpp`: +12/+24 pitch into chorus, into a 150 ms stereo delay, into `ReverbSc`, then a highpass. `.sdram_bss` is 1,487,380 B. `USE_DAISYSP_LGPL=1` for the reverb.
- INDEX **450 built, 1 FAIL**. Not flashed.

## 2026-09-27 — Cloud Seed, smoodge, ringmod, VCA
- `cloudseed.bin` 129,028 B (FLASH 98.44%). Same engine and ten programs as `loewenzahnhonig-firmware/src/cloudseed`. Audio block is 48. `CLOUDSEED_BOOST` is off, so the Seed clock is unchanged. Delay memory is `.sdram_bss` at `0xC0000000`, 15,990,784 B. Vector at `0x08000000`.
- `smoodge.bin` 97,596 B. Chorus into two 16-second delay lines (6,144,024 B in SDRAM), tremolo, and the clock-divisor steps from the original. CV1 is the clock. The original writes the right-channel delay time into the left line and the reverse; that swap is kept.
- `ringmod.bin` 95,220 B. Left input times a sine/triangle/square carrier. Right output is the carrier. `vca.bin` 91,532 B. Left level is P1 + P3×CV1, right is P2 + P4×CV2.
- INDEX **454 built, 1 FAIL**. Not flashed.

## 2026-09-27 — saved
- Detailed handoff: `briksquare/SESSION_2026_09_27_LIBRARY.md` (every bin
  size, SDRAM section, panel traps, sibling-tree makefiles, Grids GPL
  notice, what was not built, what was not flashed).
- `LIBRARY_HANDOFF.md` no longer says ReverbSc is missing or that
  `gen_ports.py` should be rerun. `FLASHING.md` lists this wave’s sizes.
- Bins stay in `SEED_READY/carciofo-official/` outside this git root.
  Source and docs are what get committed.






