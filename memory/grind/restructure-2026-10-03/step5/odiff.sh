#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
mipsel-linux-gnu-objdump -dr tmp/s5/snap/$1/$3.o | tail -n +3 > /tmp/a.txt; mipsel-linux-gnu-objdump -dr tmp/s5/snap/$2/$3.o | tail -n +3 > /tmp/b.txt; diff /tmp/a.txt /tmp/b.txt | head -${4:-40}
