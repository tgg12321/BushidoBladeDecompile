#!/bin/bash
# Fresh scratch copy of the build inputs at a git rev (default HEAD) in tmp/func_80036140/<name>.
# usage (WSL, repo root): bash tmp/func_80036140/mktree.sh <name> [rev]
set -eo pipefail
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
NAME="$1"; REV="${2:-HEAD}"
T="tmp/func_80036140/$NAME"
rm -rf "$T"; mkdir -p "$T/tools"
git archive "$REV" Makefile bb2.ld src include asm engine tools/maspsx tools/prologue_fix.py \
  tools/multu_pad.py tools/prologue_config.json tools/make_psexe.py \
  $(git ls-tree --name-only "$REV" | grep -E '\.txt$') | tar -x -C "$T"
ln -s "$PWD/tools/gcc-2.7.2" "$T/tools/gcc-2.7.2"
ln -s "$PWD/disc" "$T/disc"
echo "tree $T at $(git rev-parse --short "$REV")"
