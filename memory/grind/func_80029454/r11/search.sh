#!/bin/bash
# Q31 banked search: one-variable-per-value spellings of `rec` (loop-1 value, loop-2 value).
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
C=memory/grind/func_80029454/candidate.c
D=tmp/func_80029454/r11/s
mkdir -p $D
run() { echo "== $1"; bash tmp/func_80029454/q.sh "$D/$1.c" | head -1; }
L1=$(grep -n '^        rec = (u8 \*)&D_80101EC8 + i \* 0x44C;' $C | head -1 | cut -d: -f1)
L2=$(grep -n '^        rec = (u8 \*)&D_80101EC8 + i \* 0x44C;' $C | tail -1 | cut -d: -f1)
DECL=$(grep -n '^    u8 \*rec;$' $C | cut -d: -f1)
# s0: block-local per loop (the reviewer's split)
sed -e "${DECL}d" -e "${L1}s/        rec = /        u8 *rec = /" -e "${L2}s/        rec = /        u8 *rec = /" $C > $D/s0_blocklocal.c; run s0_blocklocal
# s1: two function-scope pointers
sed -e "${DECL}s/.*/    u8 *rec;\n    u8 *rec2;/" -e "$((L2)),\$s/\brec\b/rec2/g" $C > $D/s1_funcscope_rec2.c; run s1_funcscope_rec2
# s2: loop-2 walking pointer (function scope rec2, stepped by the loop)
sed -e "${DECL}s/.*/    u8 *rec;\n    u8 *rec2;/" -e "$((L2)),\$s/\brec\b/rec2/g" $C | sed -e "s/^    for (i = 0; i < 2; i++) {$/&/" > $D/tmp.c
awk -v l2="$((L2+1))" 'NR==l2 {sub(/rec2 = \(u8 \*\)&D_80101EC8 \+ i \* 0x44C;/, "")} {print}' $D/tmp.c > $D/tmp2.c
python3 - "$D/tmp2.c" "$D/s2_walking.c" <<'EOF'
import sys
s = open(sys.argv[1]).read()
marker = "    for (i = 0; i < 2; i++) {\n        LeafPos *dst = &ws[i * 8];\n"
assert marker in s
s = s.replace(marker, "    rec2 = (u8 *)&D_80101EC8;\n    for (i = 0; i < 2; i++, rec2 += 0x44C) {\n        LeafPos *dst = &ws[i * 8];\n", 1)
s = s.replace("        \n", "")
open(sys.argv[2], "w").write(s)
EOF
run s2_walking
# s3: loop-2 value never named (record address spelled at each use)
sed -e "$((L2))d" -e "$((L2)),\$s/(rec + /((u8 *)\&D_80101EC8 + i * 0x44C + /g" $C > $D/s3_inline.c; run s3_inline
# s4: block-local, loop 2 declares rec before dst
sed -e "${DECL}d" -e "${L1}s/        rec = /        u8 *rec = /" -e "$((L2))d" -e "$((L2-1))s/.*/        u8 *rec = (u8 *)\&D_80101EC8 + i * 0x44C;\n        LeafPos *dst = \&ws[i * 8];/" $C > $D/s4_blocklocal_first.c; run s4_blocklocal_first
# s5: block-local, index form of the record address
sed -e "${DECL}d" -e "${L1}s/        rec = .*/        u8 *rec = \&((u8 *)\&D_80101EC8)[i * 0x44C];/" -e "${L2}s/        rec = .*/        u8 *rec = \&((u8 *)\&D_80101EC8)[i * 0x44C];/" $C > $D/s5_blocklocal_index.c; run s5_blocklocal_index
# s6: two function-scope pointers declared in the other order
sed -e "${DECL}s/.*/    u8 *rec2;\n    u8 *rec;/" -e "$((L2)),\$s/\brec\b/rec2/g" $C > $D/s6_funcscope_rec2_first.c; run s6_funcscope_rec2_first
rm -f $D/tmp.c $D/tmp2.c
