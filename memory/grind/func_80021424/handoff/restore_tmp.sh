#!/bin/bash
# restore_tmp.sh : put the banked PracticeMenuRec-series scripts and bodies back where they run from (tmp/ is
# gitignored). Every script resolves paths relative to the repo root and expects tmp/prc/ and tmp/func_80021424/.
set -e
cd "$(dirname "$0")/../../../.."
mkdir -p tmp/prc tmp/func_80021424
cp memory/grind/func_80021424/handoff/tmp_prc/* tmp/prc/
cp memory/grind/func_80021424/handoff/tmp_func_80021424/* tmp/func_80021424/
echo "restored into tmp/prc and tmp/func_80021424"
