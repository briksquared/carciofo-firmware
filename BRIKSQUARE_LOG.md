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
