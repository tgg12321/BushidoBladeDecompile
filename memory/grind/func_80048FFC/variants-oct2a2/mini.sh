#!/bin/bash
# mini.sh <variant.c> <outname> [extra cc1 flags]: standalone TU (text1b's headers + the variant only),
# instrumented cc1 with RTL dumps and BB2_PRIO/RANK debug -> tmp/ffc2/<outname>/; prints the diff vs target.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
R="$(pwd)"; source .venv/bin/activate
T="tmp/ffc2/$2"; rm -rf "$T"; mkdir -p "$T"
{ sed -n 1,8p src/text1b.c; echo 'extern s32 D_800A36AC;'; cat "$1"; } > "$T/t.c"
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "$T/t.c" > "$T/t.i" 2>/dev/null
( cd "$T" && BB2_RANK_DEBUG=${RANK:-} BB2_PRIO_DEBUG=${PRIO:-} BB2_SCHED_DEBUG=${SCHED:-} "$R/tools/gcc-2.7.2/cc1" -O2 -G${G:-0} -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dr -dl -dg -dc -ds -dS -dR -dL -dj -dJ -df -dt $3 t.i -o t.cc1.s 2> err.txt
  for p in rtl jump cse loop cse2 flow combine lreg greg sched sched2 jump2 dbr; do
    [ -f t.i.$p ] && mv t.i.$p $p.txt
  done )
MF="--expand-div --aspsx-version=2.34 --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt --use-comm-section -G8"
python3 tools/prologue_fix.py < "$T/t.cc1.s" | python3 tools/maspsx/maspsx.py $MF | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt > "$T/t.s"
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$T/t.o" "$T/t.s" || exit 1
mipsel-linux-gnu-objdump -dr --no-show-raw-insn "$T/t.o" > "$T/t.dis"
[ -f tmp/ffc2/ref.dis ] || mipsel-linux-gnu-objdump -dr --no-show-raw-insn build/src/text1b.o > tmp/ffc2/ref.dis
python3 tmp/ffc2/fncmp.py tmp/ffc2/ref.dis "$T/t.dis" func_80048FFC | head -${NL:-30}
