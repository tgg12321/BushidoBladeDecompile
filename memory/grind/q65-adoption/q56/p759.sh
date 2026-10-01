#!/bin/bash
# p759.sh: try func_800759D0's banked landing.patch on a scratch copy of the pinned commit (never main).
D=/tmp/q56/cand/p759; rm -rf $D; mkdir -p $D
REPO="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
cd "$REPO" && git archive "$(cat /tmp/q56/commit.txt)" | tar -x -C $D
cd $D && patch -p1 --forward < "$REPO/memory/grind/func_800759D0/landing.patch"
head -40 src/text1b_tu2.c.rej
grep -n "D_8009BCE4" src/text1b_tu2.c | head
