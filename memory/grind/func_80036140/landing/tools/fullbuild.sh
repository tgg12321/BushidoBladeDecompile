#!/bin/bash
# Full clean build of a scratch tree (make runs INSIDE the scratch copy only) + SHA1.
# usage (WSL, repo root): bash tmp/func_80036140/fullbuild.sh <name> [-jN]
set -o pipefail
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
T="tmp/func_80036140/$1"; J="${2:--j6}"
rm -rf "$T/build"
make -C "$T" $J build/bb2.exe > "$T/build.log" 2>&1
rc=$?
if [ $rc -ne 0 ]; then grep -iE "error|fail" "$T/build.log" | grep -v "^mipsel.*warning" | tail -20; echo "BUILD FAILED rc=$rc"; exit 1; fi
sha=$(sha1sum "$T/build/bb2.exe" | cut -d' ' -f1)
if [ "$sha" = "62efab4f73f992798c43e8c730aa43baa10bb4fa" ]; then echo "SHA1 $sha == ORACLE"; else echo "SHA1 $sha != oracle"; fi
