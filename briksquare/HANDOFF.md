# Carciofo — build & bench handoff

**Module:** hand-built WGD **Carciofo** (6HP), a Daisy **Seed3** plugged inside.
Powered from the Eurorack rail (J4) or USB. Bench-verified 2026-09-27.

## Status: working module, one cosmetic fault
| Item | Result |
|---|---|
| MCU / flash / firmware boot | ✅ (onboard LED blinks, apps run) |
| P1–P4 pots | ✅ all four sweep |
| CV1 / CV2 | ✅ (CV1 tracked a Maths ramp 0.14→0.99) |
| B1 / B2 buttons | ✅ (drove octave −3..+3 in iotest) |
| Stereo audio IN | ✅ (input-monitor passes a source through) |
| Stereo audio OUT | ✅ (reverb washes; tones play) |
| USB serial telemetry | ✅ (COM port, VID_0483 PID_5740) |
| **WS2812 RGB LED (D5)** | ❌ **dark** — the only fault |

**LED diagnosis:** the app runs and the Seed's onboard LED blinks, so
firmware/flash/MCU are proven; only the panel RGB is dark → soldered wrong
(user's own read). WS2812B pins are **diagonal** (VDD↔GND one diagonal,
DIN↔DOUT the other), so a blind **180° flip reverse-powers it** — do NOT just
rotate 180°. Datasheet pin fn: 1=VDD, 2=DOUT, 3=VSS, 4=DIN. Next time: pull
WGD's Carciofo assembly art for the exact pin-1 corner before reflowing. LED is
cosmetic — no effect on the audio path.

## I/O map (from lib/carciofo.h)
- P1–P4 = ADC pins 15–18, `GetPot(POT_1..4)` 0..1.
- CV1/CV2 = ADC 19/20, **inverted** input stage → `GetCv = 1 - adc`; idle ≈ 0.001.
- B1=D6, B2=D7, to ground, internal pull-up (active-low).
- Stereo audio in + out; WS2812 RGB on D5 (bit-banged); onboard LED; USB serial.

## Flashing
```
cd SEED_READY/carciofo-official
./flash_carciofo.sh <app>        # reverb quattro doppio acciaio iotest hardware-test ...
```
Manual form: `dfu-util -a 0 -s 0x08000000:leave -D <app>.bin -d ,0483:df11`.
DFU = hold BOOT, tap RESET. After flashing, **tap RESET once** to launch
(the "abort request" message is normal). Recover from DFU = tap RESET alone.

## Verify hardware without a screen (serial)
Flash `iotest`, then read the Seed's COM port (any baud, USB CDC):
```powershell
$p = New-Object System.IO.Ports.SerialPort COM3,115200,None,8,one
$p.Open(); 1..40 | % { try { $p.ReadLine() } catch {} }; $p.Close()
```
It prints `P1..P4 CV1 CV2 B1 B2 oct mon` 4×/sec. Turn a pot / plug CV / press a
button and watch the matching field move. Hold BOTH buttons → `mon` flips to 1 =
audio-in monitor (plug a source into IN L/R to hear it pass through).

## Where things live
- Firmware source + apps: `hardware/daisy/wgd-ports/carciofo-firmware/src/`
  (reverb, quattro, doppio, acciaio, hardware-test, iotest, tonetest, blinktest, ledtest).
- Ready-to-flash bins: `hardware/daisy/SEED_READY/carciofo-official/`.
- Bench log: `hardware/daisy/wgd-ports/carciofo-firmware/BRIKSQUARE_LOG.md`.
- Batch-built "any Daisy firmware" bins: `hardware/daisy/SEED_READY/converted/` (INDEX.tsv).
