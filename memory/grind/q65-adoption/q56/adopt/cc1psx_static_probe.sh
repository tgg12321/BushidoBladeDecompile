#!/bin/bash
# cc1psx_static_probe.sh: Q65 A3 calibration with initialized statics (gp-model doc A.3b). cc1psx -G8 (calibration
# only, never a build path) vs our cc1 on cc1psx_static_probe.c; writes the two .s files beside this script.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
H="$(dirname "$(readlink -f "$0")")"
bash tools/cc1psx_wrapper.sh -O2 -G8 -funsigned-char -mcpu=3000 -mips1 -msoft-float -w < "$H/cc1psx_static_probe.c" \
  | grep -vE '^\s*#|^$' > "$H/cc1psx_static_probe_cc1psx.s"
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float \
  < "$H/cc1psx_static_probe.c" -o /dev/stdout | grep -vE '^\s*#' > "$H/cc1psx_static_probe_cc1.s"
