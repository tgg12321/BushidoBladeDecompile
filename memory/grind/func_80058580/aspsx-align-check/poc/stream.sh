#!/bin/bash
# stream.sh <stem>: the assembler input for src/<stem>.c (pipeline minus the rodata sed), to stdout.
cd /tmp/claude-0/poc || exit 1
make -s -f /tmp/claude-0/poc/Makefile -n build/src/$1.o -B | grep "cc1" | sed -e 's/| *sed "s\/\\.align\\t3\/.align\\t2\/" *|/|/' -e 's/| *mipsel-linux-gnu-as .*$//' > /tmp/claude-0/stream_cmd.sh
bash /tmp/claude-0/stream_cmd.sh
