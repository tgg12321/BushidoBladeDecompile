#!/bin/bash
# pocbuild.sh [makefile]: build the scratch copy at /tmp/claude-0/poc and compare with the reference.
cd /tmp/claude-0/poc || exit 1
rm -rf build
make -f "${1:-Makefile}" -j16 build/bb2.bin > /tmp/claude-0/poc.log 2>&1
echo "make rc=$?"
tail -3 /tmp/claude-0/poc.log
cmp build/bb2.bin /tmp/claude-0/ref_main.bin && echo "IDENTICAL to reference" || { cmp -l build/bb2.bin /tmp/claude-0/ref_main.bin | wc -l; ls -l build/bb2.bin; }
