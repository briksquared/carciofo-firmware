#!/usr/bin/env bash
# True project-level inventory of Daisy Seed firmwares across both GitHub roots.
# Excludes .claude/worktrees/ (duplicate copies) and library-internal Makefiles.
set -u
RG="/c/Users/briks/AppData/Local/Microsoft/WinGet/Packages/BurntSushi.ripgrep.MSVC_Microsoft.Winget.Source_8wekyb3d8bbwe/ripgrep-15.1.0-x86_64-pc-windows-msvc/rg"
OUT="C:/Users/briks/Desktop/GitHub/hardware/daisy/SEED_READY"
TSV="$OUT/DAISY_FIRMWARE_INVENTORY.tsv"
MD="$OUT/DAISY_FIRMWARE_INVENTORY.md"
: > "$TSV"

for ROOT in /c/Users/briks/Desktop/GitHub /f/GitHub; do
  drive=$(echo "$ROOT" | grep -oE '^/[a-z]')
  "$RG" -l --no-messages -g 'Makefile' 'LIBDAISY_DIR|libDaisy' "$ROOT" 2>/dev/null \
    | grep -vE '/\.claude/worktrees/' \
    | grep -vE '/(libDaisy|DaisySP)/(core|src|tests|cmsis|Drivers|Middlewares)/' \
    | while IFS= read -r mk; do
        d=$(dirname "$mk")
        # must have a firmware entry point
        "$RG" -lq --no-messages -g '*.{cpp,cc,c}' 'int main' "$d" 2>/dev/null || continue
        rel=${d#"$ROOT"/}
        coll=$(echo "$rel" | awk -F/ '{if($1=="audio"||$1=="hardware") print $1"/"$2; else print $1}')
        printf "%s\t%s\t%s\n" "$drive" "$coll" "$rel" >> "$TSV"
      done
done

# Build the markdown summary
{
  echo "# Daisy Seed firmware inventory (both roots)"
  echo
  echo "Generated $(date '+%Y-%m-%d'). Unique buildable projects (worktree copies and"
  echo "library-internal Makefiles excluded). Each is a firmware that targets a Daisy"
  echo "(Seed or a board variant) and so can be built for the Carciofo's Seed."
  echo
  total=$(wc -l < "$TSV")
  echo "**Total unique projects: $total**"
  echo
  echo "## By drive"
  awk -F'\t' '{c[$1]++} END{for(k in c) printf "- %s: %d\n", k, c[k]}' "$TSV" | sort
  echo
  echo "## By collection (top 40)"
  awk -F'\t' '{c[$1" "$2]++} END{for(k in c) printf "%6d  %s\n", c[k], k}' "$TSV" | sort -rn | head -40
} > "$MD"

echo "=== wrote $TSV and $MD ==="
echo "total projects: $(wc -l < "$TSV")"
echo "--- by drive ---"; awk -F'\t' '{c[$1]++} END{for(k in c) printf "%s: %d\n", k, c[k]}' "$TSV" | sort
echo "--- top collections ---"; awk -F'\t' '{c[$1" "$2]++} END{for(k in c) printf "%6d  %s\n", c[k], k}' "$TSV" | sort -rn | head -25
