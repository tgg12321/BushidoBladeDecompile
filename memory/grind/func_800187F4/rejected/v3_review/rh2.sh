#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate 2>/dev/null
python3 - <<'PY'
import json, sys
sys.path.insert(0,'.')
from engine import completion
t=open('tmp/rv3/staged_code6cac.c',encoding='utf-8').read()
h=completion.region_hashes(t,'func_800187F4')
g=json.load(open('tmp/rv3/staged_regions.json',encoding='utf-8'))
e=g['functions']['func_800187F4']['sha256']
print(len(h), len(e), h==e)
from engine import volatile_cheats as V
i=t.index('void func_800187F4(s16'); j=t.index('/* kengo:HIGH  |  nm_single_game/single_game_setModeRequest')
print(V.find_all_cheats(t[i-2600:j]))
PY
git diff --cached tools/canonical_asm_regions.json | grep '^[-+]' | grep -v sha | head -20
