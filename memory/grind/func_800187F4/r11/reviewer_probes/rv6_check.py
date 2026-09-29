import sys, json, subprocess, re
sys.path.insert(0, '.')

from engine import completion, gtemacro, inlineasm
F = 'func_800187F4'
text = subprocess.run(['git', 'show', ':src/code6cac.c'], capture_output=True, check=True).stdout.decode('utf-8')
assert '\r' not in text, 'CRLF in staged src'
js = json.loads(subprocess.run(['git', 'show', ':tools/canonical_asm_regions.json'], capture_output=True, check=True).stdout.decode('utf-8'))
exp = js['functions'][F]
got = completion.region_hashes(text, F)
print('region count', len(got), 'json count', len(exp['sha256']), 'file', exp['file'])
print('hashes equal:', got == exp['sha256'])
span = inlineasm._func_body_span(text, F)
units = [u for u in gtemacro.unit_spans(text) if span[0] <= u[0] < span[1]]
print('units in body:', len(units))
from collections import Counter
print(Counter(u[2] for u in units))
# check every asm block lies in some unit
blocks = completion.blocks(text, F)
cov = 0
for s, e in blocks:
    if any(us <= s and e <= ue for us, ue, _ in units):
        cov += 1
print('asm blocks', len(blocks), 'covered by units', cov)
# words
body = text[span[0]:span[1]]
print('words', Counter(re.findall(r'\.word 0x[0-9A-Fa-f]+', body)))
from engine.volatile_cheats import find_all_cheats
func_text = text[span[0]-200:span[1]]
print('volatile_cheats (function slice):', find_all_cheats(text[text.index('void func_800187F4(s16 *arg0, s32 *arg1) {'):span[1]+1]))
# cheat count outside islands
t2 = text
for s, e in reversed(blocks):
    t2 = t2[:s] + ''.join('\n' if c == '\n' else ' ' for c in t2[s:e]) + t2[e:]
print('cheat count outside islands', inlineasm.func_cheat_asm_count(t2, F))
