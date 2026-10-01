#!/bin/bash
# bank.sh: copy the Q65 series and the Q56 evidence into memory/grind/q65-adoption/ (text only, no binaries).
R="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
Q="$R/tmp/q56"; D="$R/memory/grind/q65-adoption"
rm -rf "$D"; mkdir -p "$D/q56/adopt" "$D/q56/probes"
cp "$Q/adopt/HANDOFF.md" "$D/HANDOFF.md"
cd "$Q" && for f in *; do
  [ -f "$f" ] || continue
  case "$f" in *.py|*.sh|*.md|*.txt|*.out|*.diff) cp "$f" "$D/q56/";; esac
done
cd "$Q/probes" && for f in *; do case "$f" in *.s|*.c|*.sh|*.py|*.txt|*.md|*.out) cp "$f" "$D/q56/probes/";; esac; done
cd "$Q/adopt" && for f in *; do
  [ -f "$f" ] || continue
  case "$f" in *.py|*.sh|*.md|*.txt|*.patch) cp "$f" "$D/q56/adopt/";; esac
done
rm -f "$D/q56/adopt/HANDOFF.md"
find "$D" -name "*.obj" -delete
du -sh "$D"; find "$D" -type f | wc -l
