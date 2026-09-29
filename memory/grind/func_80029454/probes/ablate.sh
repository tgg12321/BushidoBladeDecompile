#!/bin/bash
# Ablations of the final candidate; each prints the harness score line.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
C=memory/grind/func_80029454/candidate.c
A=tmp/func_80029454/abl
mkdir -p $A
run() { echo "== $1"; bash tmp/func_80029454/q.sh "$A/$1.c" | head -1; }

sed 's/    u32 mask;/    s32 mask;/' $C > $A/a1_s32mask.c; run a1_s32mask
sed 's/p = (s32 \*)((u8 \*)ws + (i \* 0x60 + j \* 0x18));/p = (s32 *)((u8 *)ws + i * 0x60 + j * 0x18);/' $C > $A/a2_noparen.c; run a2_noparen
sed 's/p = (s32 \*)((u8 \*)ws + (i \* 0x60 + j \* 0x18));/p = (s32 *)\&ws[i * 8 + j * 2];/' $C > $A/a3_index.c; run a3_index
# direct condition instead of the inline helper
awk '{ if ($0 ~ /if \(box_overlap\(scr\)$/) { print "            if (*(s32 *)(scr + 0x78) <= *(s32 *)(scr + 0x9C) && *(s32 *)(scr + 0x84) >= *(s32 *)(scr + 0x90)"; print "                && *(s32 *)(scr + 0x7C) <= *(s32 *)(scr + 0xA0) && *(s32 *)(scr + 0x88) >= *(s32 *)(scr + 0x94)"; print "                && *(s32 *)(scr + 0x80) <= *(s32 *)(scr + 0xA4) && *(s32 *)(scr + 0x8C) >= *(s32 *)(scr + 0x98)" } else print }' $C > $A/a4_direct.c; run a4_direct
# ws as a plain constant pointer
sed 's/    LeafPos \*ws = SPAD->unkA8\[0\];/    LeafPos *ws = (LeafPos *)0x1F8000A8;/' $C > $A/a5_wsconst.c; run a5_wsconst
# save/restore through ws instead of SPAD
sed -e 's/saved\[k\] = SPAD->unkA8\[0\]\[k\];/saved[k] = ws[k];/' -e 's/SPAD->unkA8\[0\]\[k\] = saved\[k\];/ws[k] = saved[k];/' $C > $A/a6_saverestore_ws.c; run a6_saverestore_ws
# constant-cast scratch points instead of SPAD members
sed -e 's/SPAD->unk00\[i\]\[\([012]\)\]/((LeafPos *)0x1F800000)[i * 3 + \1]/g' -e 's/SPAD->unk48\[i\]\[\([01]\)\]/((LeafPos *)0x1F800048)[i * 2 + \1]/g' $C > $A/a7_constcast.c; run a7_constcast
# separate variable for the halving inner loop
sed -e 's/^    s32 n;$/    s32 n;\n    s32 m;/' -e 's/        for (j = 0; j < 4; j++) {/        for (m = 0; m < 4; m++) {/' -e 's/p = (s32 \*)((u8 \*)ws + (i \* 0x60 + j \* 0x18));/p = (s32 *)((u8 *)ws + (i * 0x60 + m * 0x18));/' $C > $A/a8_halving_own_var.c; run a8_halving_own_var
# save loop on i
sed -e '0,/    for (k = 0; k < 16; k++) {/s//    for (i = 0; i < 16; i++) {/' -e '0,/        saved\[k\] = SPAD->unkA8\[0\]\[k\];/s//        saved[i] = SPAD->unkA8[0][i];/' $C > $A/a9_save_on_i.c; run a9_save_on_i
# overlap operand order as (max >= min)
sed -e 's/return \*(s32 \*)(scr + 0x78) <= \*(s32 \*)(scr + 0x9C)/return *(s32 *)(scr + 0x9C) >= *(s32 *)(scr + 0x78)/' -e 's/&& \*(s32 \*)(scr + 0x7C) <= \*(s32 \*)(scr + 0xA0)/\&\& *(s32 *)(scr + 0xA0) >= *(s32 *)(scr + 0x7C)/' -e 's/&& \*(s32 \*)(scr + 0x80) <= \*(s32 \*)(scr + 0xA4)/\&\& *(s32 *)(scr + 0xA4) >= *(s32 *)(scr + 0x80)/' $C > $A/a10_overlap_order.c; run a10_overlap_order
# cached triangle-point pointer in the bounds loops
sed -e 's/((LeafPos \*\*)(scr + 0x60))\[k\]->/v->/g' -e 's/((LeafPos \*\*)(scr + 0x6C))\[k\]->/w->/g' -e 's/^    u8 \*rec;$/    u8 *rec;\n    LeafPos *v;\n    LeafPos *w;/' $C | awk '{print} /for \(k = 1; k < 3; k\+\+\) \{/{getline nx; if (nx ~ /v->/) print "            v = ((LeafPos **)(scr + 0x60))[k];"; else if (nx ~ /w->/) print "                w = ((LeafPos **)(scr + 0x6C))[k];"; print nx}' > $A/a11_cached_ptr.c; run a11_cached_ptr
# 0x286 as if/else
sed 's/            \*(s16 \*)(rec + 0x286) = \*(s16 \*)(rec + 0x8C) != 0 ? 0x19 : 0xB;/            if (*(s16 *)(rec + 0x8C) != 0) {\n                *(s16 *)(rec + 0x286) = 0x19;\n            } else {\n                *(s16 *)(rec + 0x286) = 0xB;\n            }/' $C > $A/a12_ifelse286.c; run a12_ifelse286
