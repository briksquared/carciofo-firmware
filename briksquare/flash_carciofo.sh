#!/usr/bin/env bash
# Flash a Carciofo firmware onto the Seed inside the module.
# Usage:  ./flash_carciofo.sh <app>      e.g.  ./flash_carciofo.sh quattro
# It waits for the Seed to show up in DFU, flashes, and reminds you to tap RESET.
set -u
DIR="$(cd "$(dirname "$0")" && pwd)"
app="${1:-}"
avail=$(ls "$DIR"/*.bin 2>/dev/null | xargs -n1 basename | sed 's/\.bin$//' | tr '\n' ' ')
if [ -z "$app" ] || [ ! -f "$DIR/$app.bin" ]; then
  echo "Usage: $0 <app>"
  echo "Available: $avail"
  exit 1
fi
echo ">> $app.bin ($(stat -c%s "$DIR/$app.bin") bytes)"
echo ">> Put the Carciofo in DFU:  hold BOOT, tap RESET, release."
printf ">> waiting for DFU"
found=0
for i in $(seq 1 90); do
  if dfu-util -l 2>/dev/null | grep -q "Internal Flash"; then found=1; echo " - found"; break; fi
  printf "."; sleep 1
done
[ "$found" = 1 ] || { echo; echo "!! No DFU device after 90s. Aborting."; exit 1; }
dfu-util -a 0 -s 0x08000000:leave -D "$DIR/$app.bin" -d ,0483:df11 2>&1 | grep -iE "download done|success|error"
echo
echo ">> Flashed $app.  NOW TAP RESET ONCE on the module to launch it."
echo "   ('Error sending dfu abort request' just above is normal on this board.)"
