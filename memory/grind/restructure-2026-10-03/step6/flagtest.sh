#!/bin/bash
# Extract cc1_flags from build_oracle_cc1.sh and print the flags it assigns per GP/non-GP TU.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
F="-G0"; FG8="-G8"
eval "$(sed -n '/^GP_IDS=/,/^}/p' tools/build_oracle_cc1.sh)"
for s in main/309CC main/31548 main/31CFC main/31D3C main/24F08 main/26730 main/26940 main/5ED34 main/368E4 main/87A0 main/psxsdk/libgpu/sys; do
  echo "$s $(cc1_flags "$s")"
done
