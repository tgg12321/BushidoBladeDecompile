#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate 2>/dev/null
git show :src/code6cac.c > tmp/rv3/staged_code6cac.c
git show :tools/canonical_asm_regions.json > tmp/rv3/staged_regions.json
python3 - <<'PY'
import json, sys
sys.path.insert(0,'.')
from engine import completion
t=open('tmp/rv3/staged_code6cac.c',encoding='utf-8').read()
h=completion.region_hashes(t,'func_800187F4')
g=json.load(open('tmp/rv3/staged_regions.json',encoding='utf-8'))
e=g['functions']['func_800187F4']
print(len(h), len(e), h==e)
PY
python3 -c "import sys; sys.path.insert(0,'engine'); from volatile_cheats import find_all_cheats; import re; t=open('tmp/rv3/staged_code6cac.c').read(); i=t.index('void func_800187F4(s16'); j=t.index('/* kengo:HIGH  |  nm_single_game/single_game_setModeRequest'); print(find_all_cheats(t[i-3000:j]))" 2>&1 | tail -3
