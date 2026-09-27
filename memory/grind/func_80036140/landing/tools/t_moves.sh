#!/bin/bash
# Move records for commit B (split) and commit D (second split), from the t_steps.sh scratch trees.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
H=tmp/func_80036140; REV=${1:-$(git rev-parse HEAD)}
echo "== commit B: code6cac_b2_post.c @ $REV -> b2_post | b4 | b4_post"
git show $REV:src/code6cac_b2_post.c > $H/pre_B.c
python3 $H/movecheck2.py $H/pre_B.c $H/stB/src/code6cac_b2_post.c $H/stB/src/code6cac_b4.c $H/stB/src/code6cac_b4_post.c
echo; echo "== commit D: code6cac_b4_post.c (commit C state + the g_cd_result respelling) -> b4_post | b5 | b5_post"
rm -rf $H/stCr; cp -r $H/stC $H/stCr 2>/dev/null || true
python3 $H/apply2.py $H/stCr match --respell-only
diff <(cat $H/stC/src/code6cac_b4_post.c) <(cat $H/stCr/src/code6cac_b4_post.c) | sed 's/^/   respelling: /'
python3 $H/movecheck2.py $H/stCr/src/code6cac_b4_post.c $H/stD/src/code6cac_b4_post.c $H/stD/src/code6cac_b5.c $H/stD/src/code6cac_b5_post.c
