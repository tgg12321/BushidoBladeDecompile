#!/bin/bash
# engine_verify.sh: the engine's own clean-driver build (pipeline.py + buildconfig.py, not the Makefile) in the
# SCRATCH clone, and verify-oracle --rebuild; proves the engine mirror of the step is exact.
cd "/tmp/q56/adopt tree" && source .venv/bin/activate
python3 -m engine.cli build > /tmp/q56/engine_build.log 2>&1; echo "engine build rc=$?"; tail -3 /tmp/q56/engine_build.log
python3 -m engine.cli verify-oracle --rebuild > /tmp/q56/engine_verify.log 2>&1; echo "verify-oracle rc=$?"; tail -6 /tmp/q56/engine_verify.log
