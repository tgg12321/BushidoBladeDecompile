import json, subprocess, sys
sys.path.insert(0,'.')
from engine import completion
txt=subprocess.run(['git','show',':src/code6cac.c'],capture_output=True).stdout.decode('utf-8')
h=completion.region_hashes(txt,'func_800187F4')
g=json.loads(subprocess.run(['git','show',':tools/canonical_asm_regions.json'],capture_output=True).stdout.decode('utf-8'))
e=g["functions"]["func_800187F4"]; print(e["file"]); e=e["sha256"]
print(len(h), len(e), h==e)
wt=open('src/code6cac.c',encoding='utf-8').read()
print('worktree==index', wt==txt)
