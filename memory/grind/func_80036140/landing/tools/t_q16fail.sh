#!/bin/bash
# Q16 (1): bank the FAILING builds of each post-split respelling applied before its split.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
H=tmp/func_80036140; REV=$(head -1 $H/t_steps.log | cut -d" " -f2); OUT=memory/grind/func_80036140/landing/steps/q16_failing_builds.txt
{
echo "# Q16 (1): each respelling that lands after a split, applied to the tree BEFORE that split (rev $REV)"
echo; echo "## CdlATV merge (commit C) on the unsplit tree: HEAD + gate + the CdlATV respellings, code6cac_b2_post.c whole, -G0"
bash $H/mktree.sh q16_atv $REV >/dev/null
python3 $H/gate.py $H/q16_atv && python3 $H/apply_model.py $H/q16_atv --split none --merge --ext split
bash $H/fullbuild.sh q16_atv -j8; python3 $H/scoretree.py q16_atv
python3 $H/scoretree.py q16_atv --diff cdrom_SetMix | grep -v not-scored | tail -4
echo; echo "## g_cd_result u8[8] (commit E) before commit D's split: commit-C state + the respelling of func_80036940 in code6cac_b4_post.c (-G0)"
rm -rf $H/q16_cdres; cp -r $H/stC $H/q16_cdres; rm -rf $H/q16_cdres/build
python3 - <<'EOF'
from pathlib import Path
R = Path('tmp/func_80036140/q16_cdres')
h = (R / 'include/code6cac.h').read_text()
h = h.replace('extern u8 g_cd_result_plus_0x4;\n', Path('tmp/func_80036140/hdr_cdres.txt').read_text(), 1)
(R / 'include/code6cac.h').write_bytes(h.encode())
p = (R / 'src/code6cac_b4_post.c').read_text()
assert p.count('CdSync(1, &g_cd_result)') == 5 and p.count('if (g_cd_result & 0x10)') == 2
p = p.replace('extern u8 g_cd_result;\n', '', 1).replace('CdSync(1, &g_cd_result)', 'CdSync(1, g_cd_result)')
p = p.replace('if (g_cd_result & 0x10)', 'if (g_cd_result[0] & 0x10)')
(R / 'src/code6cac_b4_post.c').write_bytes(p.encode())
EOF
bash $H/fullbuild.sh q16_cdres -j8; python3 $H/scoretree.py q16_cdres
python3 $H/scoretree.py q16_cdres --diff func_80036940 | grep -v not-scored | tail -4
} > $OUT 2>&1
cat $OUT
