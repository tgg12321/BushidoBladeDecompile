#!/bin/bash
# bld.sh <dir> <G_head> [G_tail] : compile <dir>/text1b.c (cc1 -G<G_head>) and <dir>/text1b_tu1b.c (cc1 -G<G_tail>,
# default 0) through the Makefile's per-file pipeline (maspsx -G8, rodata align 4), link them in place of
# build/src/text1b.o (tail right after head in every section) with every other object from build/, and
# print the EXE SHA1 against the oracle. Nothing in the tree is touched.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
D=$1; GH=$2; GT=${3:-0}
O="$D/obj"; mkdir -p "$O"
MF="--expand-div --aspsx-version=2.34 --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt --use-comm-section -G8"
CPPF="-Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
comp() { # src G out
  set -o pipefail
  mipsel-linux-gnu-cpp $CPPF "$1" | tools/gcc-2.7.2/build/cc1 -O2 -G$2 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float \
   | python3 tools/prologue_fix.py | python3 tools/maspsx/maspsx.py $MF | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt \
   | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$3" || { echo "COMPILE FAILED $1"; exit 1; }
  mipsel-linux-gnu-objcopy --set-section-alignment .rodata=4 "$3"
}
comp "$D/text1b.c" $GH "$O/text1b.o"
[ -f "$D/text1b_tu1b.c" ] && comp "$D/text1b_tu1b.c" $GT "$O/text1b_tu1b.o"
[ -f "$D/text1b_ro.c" ] && comp "$D/text1b_ro.c" 0 "$O/text1b_ro.o"
python3 tmp/cam/mkld.py "$O" > "$O/bb2.ld" || exit 1
mipsel-linux-gnu-ld -nostdlib --no-check-sections -Map "$O/bb2.map" -T "$O/bb2.ld" -T undefined_funcs_auto.txt -T undefined_syms_auto.txt -T named_syms.txt -o "$O/bb2.elf" || { echo LINK FAILED; exit 1; }
mipsel-linux-gnu-objcopy -O binary -j .main "$O/bb2.elf" "$O/bb2.bin"
python3 tools/make_psexe.py disc/SLUS_006.63 "$O/bb2.bin" "$O/bb2.exe" >/dev/null
S=$(sha1sum "$O/bb2.exe" | cut -d' ' -f1)
if [ "$S" = 62efab4f73f992798c43e8c730aa43baa10bb4fa ]; then echo "SHA1 $S == ORACLE"; else echo "SHA1 $S MISMATCH"; cmp "$O/bb2.exe" disc/SLUS_006.63 | head -3; cmp -l "$O/bb2.exe" disc/SLUS_006.63 | wc -l; fi
