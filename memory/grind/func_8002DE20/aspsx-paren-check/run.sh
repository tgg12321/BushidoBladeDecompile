#!/bin/bash
# Assemble bare.s and explicit.s with real Psy-Q ASPSX 2.34 (psyq3.5) via dosemu2.
# Harness: memory/grind/func_80036140/research-common-gp/{aspsx.sh,asm_obj.py,crlf.py,run_asm_tests.py}
# copied to tmp/research36140/ with psyq3.5 untarred to tmp/research36140/psyq/.
# usage (WSL, repo root): bash memory/grind/func_8002DE20/aspsx-paren-check/run.sh
cd "$(dirname "$0")/../../../.."
D=memory/grind/func_8002DE20/aspsx-paren-check
for f in bare explicit; do
  echo "== $f.s  (cmd: bash tmp/research36140/aspsx.sh $D/$f.s $D/$f.obj -G8)"
  python3 tmp/research36140/asm_obj.py $D/$f.s -G8
done
