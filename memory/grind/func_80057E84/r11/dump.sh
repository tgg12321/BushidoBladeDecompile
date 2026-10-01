#!/bin/bash
# dump.sh <tag> <body.c>: allocation dumps of func_80057E84 for one body.
#  1. splice: engine.inlineasm.substitute_body(src/text1b.c, body) -> $D/text1b.c, plus the
#     0x360/0x361 header (prep_hdr.py's code6cac.h) beside it (quote-include lookup)
#  2. cpp with the build's CPP_FLAGS/CPP_DEFS -> t.i; strip_other_fns.py keeps func_80057E84 only
#  3. build cc1 (tools/gcc-2.7.2/build/cc1, the build's CC flags for text1b) -dr -dl -dg
#     -> <tag>.rtl / .lreg / .greg / .fn.s
#  4. instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_ALLOC_DEBUG=1 BB2_QTY_DEBUG=1 -> <tag>.alloc
#     (global.c ALLOCDBG lines + local-alloc.c QTYDBG lines); its asm must equal step 3's
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
tag=$1; body=$2
D=tmp/func_80057E84/r11/dumps/$tag; rm -rf $D; mkdir -p $D
python3 - "$body" "$D" <<'EOF'
import sys
from pathlib import Path
from engine import inlineasm
base = Path("src/text1b.c").read_text(encoding="utf-8")
Path(sys.argv[2], "text1b.c").write_text(inlineasm.substitute_body(base, "func_80057E84", Path(sys.argv[1]).read_text()))
EOF
python3 memory/grind/func_80057E84/r11/prep_hdr.py "$D" > /dev/null
read CPPF CPPD CCF <<<"$(python3 -c '
from engine import buildconfig as c
print(c.CPP_FLAGS.replace(" ", "~"), " ".join(c.CPP_DEFS.split()).replace(" ", "~"), (c.CC_FLAGS_GP if "text1b" in c.GP_FILES else c.CC_FLAGS).replace(" ", "~"))')"
CPPF=${CPPF//\~/ }; CPPD=${CPPD//\~/ }; CCF=${CCF//\~/ }
mipsel-linux-gnu-cpp $CPPF -Isrc $CPPD $D/text1b.c > $D/t.i
python3 tools/decomp-permuter/strip_other_fns.py $D/t.i func_80057E84
echo "$CCF" > $D/ccflags.txt
( cd $D && ../../../../../tools/gcc-2.7.2/build/cc1 $CCF -dr -dl -dg t.i -o t.s )
( cd $D && BB2_ALLOC_DEBUG=1 BB2_QTY_DEBUG=1 ../../../../../tools/gcc-2.7.2/cc1 $CCF t.i -o t.inst.s 2> t.alloc.raw )
grep -E "ALLOCDBG func=func_80057E84|QTYDBG" $D/t.alloc.raw > $D/$tag.alloc || true
for f in rtl lreg greg; do
  awk '/^;; Function func_80057E84/{p=1} /^;; Function /&&!/func_80057E84/{p=0} p' $D/t.i.$f > $D/$tag.$f
done
awk '/^func_80057E84:/{p=1} p{print} /\.end\tfunc_80057E84/{p=0}' $D/t.s > $D/$tag.fn.s
awk '/^func_80057E84:/{p=1} p{print} /\.end\tfunc_80057E84/{p=0}' $D/t.inst.s > $D/$tag.inst.fn.s
if cmp -s $D/$tag.fn.s $D/$tag.inst.fn.s; then echo "$tag IDENTITY OK"; else echo "$tag IDENTITY MISMATCH"; fi
rm -f $D/t.i.rtl $D/t.i.lreg $D/t.i.greg $D/t.inst.s
