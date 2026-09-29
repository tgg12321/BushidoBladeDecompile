#!/bin/bash
# Regenerate every body and measurement for the func_8008B488 Ruling 11 package.
# (WSL, repo root.) Output: tmp/f8b488s4/all.log and tmp/f8b488s4/dumps.txt
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" || exit 1
source .venv/bin/activate
D=tmp/f8b488s4
{
echo "== partitions of {AR,DR,SR,RR,SL} into shared locals (standalone scorer; counts include the +1 jtbl addend)"
python3 $D/gen_part.py $D/parts >/dev/null && bash $D/scorelist.sh $D/parts/list.txt
python3 $D/mkfinal.py >/dev/null
echo "== final / split"
python3 tmp/f8b488s2/score.py $D/final.c $D/split.c
python3 $D/mkfam.py >/dev/null; python3 $D/mkfam2.py >/dev/null; python3 $D/mkalt.py >/dev/null; python3 $D/mkfam3.py >/dev/null; python3 $D/mkfam4.py >/dev/null
echo "== sanctioned-family escapes, round 1 (fam/)"; bash $D/scorelist.sh $D/fam/list.txt
echo "== sanctioned-family escapes, round 2: non-constant-base chain-extenders (fam2/)"; bash $D/scorelist.sh $D/fam2/list.txt
echo "== sanctioned-family escapes, round 3: same-iteration FAKE write + chain-extender read in another block (fam3/)"; bash $D/scorelist.sh $D/fam3/list.txt
echo "== sanctioned-family escapes, round 4: FAKE write and chain-extender read in different basic blocks (fam4/)"; bash $D/scorelist.sh $D/fam4/list.txt
echo "== alternatives: other SL partners, types, scopes (alt/)"; bash $D/scorelist.sh $D/alt/list.txt
} > $D/all.log 2>&1
bash $D/dumps.sh >/dev/null 2>&1
SPLIT_SMODE=251 bash $D/dumps2.sh >/dev/null 2>&1
bash $D/dumpesc.sh > $D/dumpesc.log 2>&1
bash $D/dumpfam3.sh > $D/dumpfam3.log 2>&1
bash $D/dumpfam4.sh > $D/dumpfam4.log 2>&1
bash $D/collect.sh > $D/dumps.txt 2>&1
echo done
