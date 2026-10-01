#!/bin/bash
f=/tmp/q56/model/build/src/text1a_c.p2.s
grep -E '^\s(lw|sw|lb|lbu|lh|lhu|sb|sh)\s+\$[0-9a-z]+,[A-Za-z_]' $f | head -5
grep -E '^\s\.(comm|lcomm|sdata|extern|local)' $f | head -5
