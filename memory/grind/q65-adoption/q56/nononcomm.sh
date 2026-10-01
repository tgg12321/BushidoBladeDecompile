#!/bin/bash
# (2) the model without the stand-in list: which functions need an initialized/static definition?
D=/tmp/q56/model_nn; rm -rf $D; cp -a /tmp/q56/model $D
cd $D; : > poc_noncomm_syms.txt; source .venv/bin/activate
rm -rf build; make -j16 build/bb2.exe > build.log 2>&1; echo "nononcomm make_rc=$? exe_sha1=$(sha1sum build/bb2.exe | cut -d' ' -f1)"
python3 "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/model_diff.py" $D/build/src | tail -12
