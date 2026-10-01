#!/bin/bash
# run_all.sh: regenerate g0proof.out (camera_CalcAngles, 2026-10-01). Probes are compiled by our cc1 and, for
# calibration only, by the original cc1psx. The text1b -G0/-G8 comparison needs the candidate spliced into a
# scratch copy first: python3 g0proof/splice.py memory/grind/camera_CalcAngles/candidate.c tmp/camera_CalcAngles/text1b_cand.c
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
P=memory/grind/camera_CalcAngles/g0proof
tail1() { sed -n '/jal/,/j\t\$31/p'; }
{
echo "## 1. p0.c (static short D_pair[2]; [0], [1], return D_pair): our cc1 -G0, with the expand (rtl) / cse / cse2 / combine / greg tail"
bash $P/probe.sh $P/p0.c 0 "-dr -ds -dt -dc -dg" cse | tail1
for e in rtl cse cse2 combine greg; do echo "--- $e"; ls /tmp/camCA/p.c.$e >/dev/null && grep -v '^$' /tmp/camCA/p.c.$e | sed -n '/^(insn 2[4-9]\|^(insn 3[0-9]/,+3p'; done
echo; echo "## 2. p0.c under the original cc1psx -G0 (calibration): the same extra move"
bash $P/psx.sh $P/p0.c 0 | sed -n '/^f:/,$p'
echo; echo "## 3. respellings, our cc1 -G0 (tail after the second call)"
for v in v1 v2 v3 v4 v5 v6; do echo "--- $v.c"; sed -n '4,$p' $P/$v.c; bash $P/probe.sh $P/$v.c 0 | tail1; done
echo; echo "## 4. file-scope asm between two functions (fasm.c): order of the output"
for g in 0 8; do bash $P/cmp.sh $P/fasm.c $g | grep -E "===|^[a-z]+:"; done
echo; echo "## 5. externs / statics under -G8, cc1psx vs our cc1 (ext.c): identical"
bash $P/cmp.sh $P/ext.c 8 | grep -vE '\$sp'
} > $P/g0proof.out 2>&1
echo "## 6. text1b with the candidate, cc1 -G0 vs -G8 (order-free per-function diff)" >> $P/g0proof.out
bash $P/g8cmp.sh tmp/camera_CalcAngles/text1b_cand.c >> $P/g0proof.out 2>&1
