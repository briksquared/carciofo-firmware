#!/usr/bin/env bash
set -u
DFU="/c/Users/briks/.platformio/packages/tool-dfuutil/bin/dfu-util.exe"
BOOT="/c/Users/briks/Desktop/GitHub/hardware/daisy/wgd-ports/carciofo-firmware/lib/libDaisy/core/dsy_bootloader_v6_4-intdfu-2000ms.bin"
APP="/c/Users/briks/Desktop/GitHub/hardware/daisy/wgd-ports/carciofo-firmware/src/fx/build/fx.bin"
echo "[1/2] flashing Daisy bootloader to internal flash (0x08000000)..."
O1=$("$DFU" -a 0 -s 0x08000000:leave -D "$BOOT" -d ,0483:df11 2>&1)
echo "$O1" | grep -E "downloaded successfully|dfuERROR|No DFU capable"
echo "$O1" | grep -q "File downloaded successfully" || { echo "bootloader write NOT confirmed"; exit 1; }
echo ""
echo ">>> TAP RESET ONCE on the Carciofo now to open the 2s bootloader window <<<"
echo "[2/2] catching QSPI DFU window (0x90040000) and writing fx (64 effects)..."
for i in $(seq 1 120); do
  OUT=$("$DFU" -a 0 -s 0x90040000:leave -D "$APP" -d ,0483:df11 2>&1)
  if echo "$OUT" | grep -q "File downloaded successfully"; then
    echo "OK: fx written to QSPI. Tap RESET once more to launch it."; exit 0
  fi
  sleep 0.5
done
echo "ERROR: never caught the window (tap RESET while this runs; data USB cable)."; exit 1
