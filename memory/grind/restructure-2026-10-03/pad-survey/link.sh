#!/bin/bash
# usage: link.sh <ldscript> <outprefix>  (links existing objects; writes only <outprefix>.*)
set -eo pipefail
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
mipsel-linux-gnu-ld -nostdlib --no-check-sections -Map "$2.map" -T "$1" -T undefined_funcs_auto.txt -T undefined_syms_auto.txt -T named_syms.txt -o "$2.elf"
mipsel-linux-gnu-objcopy -O binary -j .main "$2.elf" "$2.bin"
python3 tools/make_psexe.py disc/SLUS_006.63 "$2.bin" "$2.exe" >/dev/null
sha1sum "$2.exe"
