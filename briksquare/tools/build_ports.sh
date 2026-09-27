#!/usr/bin/env bash
SRC="C:/Users/briks/Desktop/GitHub/hardware/daisy/wgd-ports/carciofo-firmware/src"
OUT="C:/Users/briks/Desktop/GitHub/hardware/daisy/SEED_READY/carciofo-ports"
PROG="$OUT/BUILD_PROGRESS.txt"; : > "$PROG"
apps="ladder svf chorus flanger phaser tremolo autowah overdrive folder crush pitch delay pingpong limiter va_osc fm formant additive karplus kick"
ok=0; fail=0
for a in $apps; do
  ( cd "$SRC/$a" && make >build.log 2>&1 )
  bin="$SRC/$a/build/$a.bin"
  if [ -s "$bin" ]; then cp "$bin" "$OUT/$a.bin"; sz=$(stat -c%s "$bin"); ok=$((ok+1)); echo "OK   $a  ${sz}B" >> "$PROG"; 
  else fail=$((fail+1)); echo "FAIL $a  -> $(grep -iE 'error:' "$SRC/$a/build.log" | head -1)" >> "$PROG"; fi
done
echo "DONE built=$ok failed=$fail" >> "$PROG"
