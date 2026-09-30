set -e
source .venv/bin/activate
SRC=$1; OUT=$2; MASPSX=${3:-tools/maspsx/maspsx.py}; EXTRA=${4:-}
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "$SRC" 2>/dev/null \
 | tools/gcc-2.7.2/build/cc1 -O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float \
 | python3 tools/prologue_fix.py \
 | python3 $MASPSX --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt $EXTRA \
 | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt \
 | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$OUT"
mipsel-linux-gnu-objcopy --set-section-alignment .rodata=4 "$OUT"
python3 - "$OUT" <<'PY'
import sys; sys.path.insert(0, ".")
from engine import score
for f in ("cdrom_SetMix", "func_80035F78"):
    r = score.score_func(sys.argv[1], "build/src/code6cac_b4.o", f)
    print(f, r["score"], r["target_insns"], r["build_insns"])
PY
mipsel-linux-gnu-nm "$OUT" | grep -i "atv\|36B8"
