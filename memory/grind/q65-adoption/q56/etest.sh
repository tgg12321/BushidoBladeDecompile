#!/bin/bash
# etest.sh: the engine regression suite, run in the SCRATCH adopt clone ("/tmp/q56/adopt tree"), not main.
cd "/tmp/q56/adopt tree" && mkdir -p tmp && source .venv/bin/activate
python3 -m engine.cli test > /tmp/q56/etest.log 2>&1; rc=$?
echo "engine test rc=$rc"; grep "FAIL" /tmp/q56/etest.log | head; tail -1 /tmp/q56/etest.log
