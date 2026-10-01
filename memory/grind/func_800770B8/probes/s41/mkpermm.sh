#!/bin/bash
# mkpermm.sh <name> <cand.c>: permuter workspace /tmp/l770m/perm_<name> for func_800770B8 on the private MAIN
# clone (main's maspsx flags). base.c = the reduced TU (other function bodies stripped, top-level __asm__ and
# line markers dropped).
set -e
R="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
N="$1"; C="$2"; case "$C" in /*) ;; *) C="$R/$C";; esac
T=/tmp/l770m/tree
W=/tmp/l770m/perm_$N; rm -rf "$W"; mkdir -p "$W"
cd $T && source .venv/bin/activate
python3 - "$C" "$W/x.c" <<'PY'
import sys
src = open("src/text1b_tu2.c").read()
line = 'INCLUDE_ASM("asm/funcs", func_800770B8);'
assert src.count(line) == 1
open(sys.argv[2], "w").write(src.replace(line, open(sys.argv[1]).read()))
PY
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "$W/x.c" \
  | grep -v '^#' | grep -v '^__asm__' > "$W/full.c"
python3 - "$W/full.c" "$W/base.c" <<'PY'
import sys
sys.path.insert(0, "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tools/decomp-permuter")
from strip_other_fns import strip_other_fns
open(sys.argv[2], "w").write(strip_other_fns(open(sys.argv[1]).read(), "func_800770B8"))
PY
cat > "$W/compile.sh" <<'EOF'
#!/bin/bash
set -e
T=/tmp/l770m/tree
IN="$1"; OUT="$3"
TMPS=$(mktemp /tmp/l770mpXXXXXX.s)
trap "rm -f $TMPS" EXIT
cd $T
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float "$IN" -o /dev/stdout 2>/dev/null \
 | python3 tools/prologue_fix.py \
 | python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt --use-comm-section \
 | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt > "$TMPS"
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$OUT" "$TMPS"
EOF
chmod +x "$W/compile.sh"
printf 'func_name = "func_800770B8"\ncompiler_type = "gcc"\n' > "$W/settings.toml"
sed 's/^.set gp=64$//' "$R/tools/decomp-permuter/prelude.inc" > "$W/prelude_r3k.inc"
cat "$W/prelude_r3k.inc" "$T/asm/funcs/func_800770B8.s" > "$W/target.s"
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$W/target.o" "$W/target.s"
bash "$W/compile.sh" "$W/base.c" -o "$W/base.o"
f() { mipsel-linux-gnu-objdump -d "$1" | awk '/<func_800770B8>:/{p=1;next} /^[0-9a-f]+ </{p=0} p' | sed 's/^[^\t]*\t[^\t]*\t//' | grep -v '^$'; }
echo "diff lines vs target: $(diff <(f "$W/target.o") <(f "$W/base.o") | grep -c '^[<>]')"
echo "workspace $W ready"
