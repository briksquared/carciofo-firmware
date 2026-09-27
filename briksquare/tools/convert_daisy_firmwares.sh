#!/usr/bin/env bash
# Convert (build) every Daisy firmware project in the repo into a flashable Seed .bin.
# Builds against the shared prebuilt libDaisy/DaisySP so per-repo submodules don't matter.
# Harvests build/*.bin into SEED_READY/converted/, logs status per project.
set -u
GH="C:/Users/briks/Desktop/GitHub"
LIB="$GH/hardware/daisy/wgd-ports/carciofo-firmware/lib/libDaisy"
DSP="$GH/hardware/daisy/wgd-ports/carciofo-firmware/lib/DaisySP"
OUT="$GH/hardware/daisy/SEED_READY/converted"
mkdir -p "$OUT"
IDX="$OUT/INDEX.tsv"; printf "status\tbin\tbytes\tsource\n" > "$IDX"
LOG="$OUT/BUILD_LOG.txt"; : > "$LOG"
PROG="$OUT/PROGRESS.txt"

# Discover firmware Makefiles (exclude the library source trees themselves).
mapfile -t MKS < <(grep -rliE "LIBDAISY_DIR" --include=Makefile \
  "$GH/hardware/daisy/wgd-ports" "$GH/hardware/daisy/briksquare-daisy" \
  "$GH/hardware/daisy/vendor" "$GH/audio/electrosmith" 2>/dev/null \
  | grep -vE "/(libDaisy|DaisySP)/(core|src|Drivers|Middlewares|cmsis|tests)/" \
  | sort -u )

total=${#MKS[@]}; ok=0; fail=0; n=0
echo "discovered $total candidate firmware Makefiles" | tee "$PROG"
for mk in "${MKS[@]}"; do
  n=$((n+1))
  d=$(dirname "$mk")
  # needs a real firmware entry point
  grep -rqlE "int main|daisy_seed\.h|DaisySeed|DaisyPod|DaisyPatch" "$d"/*.cpp "$d"/*.c 2>/dev/null || { echo "[$n/$total] skip (no main): $d" > "$PROG"; continue; }
  name=$(echo "$d" | sed -E "s#$GH/##; s#/#__#g")
  echo "===== [$n/$total] $name =====" >> "$LOG"
  ( cd "$d" && timeout 150 make LIBDAISY_DIR="$LIB" DAISYSP_DIR="$DSP" >>"$LOG" 2>&1 )
  bin=$(ls -S "$d"/build/*.bin 2>/dev/null | head -1)
  if [ -n "$bin" ] && [ -s "$bin" ]; then
    sz=$(stat -c%s "$bin"); cp "$bin" "$OUT/$name.bin"
    printf "built\t%s.bin\t%s\t%s\n" "$name" "$sz" "$d" >> "$IDX"; ok=$((ok+1))
  else
    printf "FAIL\t%s\t0\t%s\n" "$name" "$d" >> "$IDX"; fail=$((fail+1))
  fi
  echo "[$n/$total] ok=$ok fail=$fail  last=$name" > "$PROG"
done
echo "DONE: built=$ok failed=$fail of $total processed" | tee -a "$PROG"
echo "bins now in SEED_READY/converted: $(ls "$OUT"/*.bin 2>/dev/null | wc -l)" | tee -a "$PROG"
