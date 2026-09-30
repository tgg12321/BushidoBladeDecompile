#!/bin/bash
# mktree.sh: fresh scratch tree tmp/c8dc/tree from HEAD (read-only export), tools/disc/engine symlinked.
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
S=tmp/c8dc/tree
rm -rf $S
mkdir -p $S
git archive HEAD src include asm bb2.ld Makefile undefined_funcs_auto.txt undefined_syms_auto.txt named_syms.txt \
  sdata_syms.txt sdata_funcs.txt sdata_exclude.txt expand_lb_funcs.txt multu_funcs.txt multu_pad_funcs.txt \
  expand_dest_funcs.txt maspsx_prefill_label_funcs.txt maspsx_comm_syms.txt | tar -x -C $S
ln -s "$PWD/tools" $S/tools
ln -s "$PWD/disc" $S/disc
ln -s "$PWD/engine" $S/engine
echo "tree at $S ($(git rev-parse --short HEAD))"
