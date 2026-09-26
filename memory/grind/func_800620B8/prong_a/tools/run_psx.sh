#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
for v in split_tex merged_tex; do
  bash tmp/func_800620B8/s3/psx.sh $v > /dev/null
  bash tmp/func_800620B8/s3/psx_obj.sh $v
  python3 tmp/func_800620B8/s3/cmp.py tmp/func_800620B8/s3/d/$v
done
