"""Print, for func_800233AC / func_80023648, the insns (in emission order) between the
a0/a1 index computation and the row load: compact one-line form per insn."""
import re
import sys

def insns(path, func):
    t = open(path).read()
    i = t.index(f';; Function {func}')
    j = t.find(';; Function ', i + 10)
    body = t[i:j if j > 0 else None]
    out = []
    for m in re.finditer(r'\n\((insn|jump_insn|call_insn|code_label|note) (\d+) .*?(?=\n\((?:insn|jump_insn|call_insn|code_label|note|barrier) |\Z)', body, re.S):
        kind, uid, txt = m.group(1), m.group(2), ' '.join(m.group(0).split())
        if kind == 'note':
            continue
        out.append((uid, kind, txt))
    return out

for path in sys.argv[1:]:
    for func in ('func_800233AC', 'func_80023648'):
        rows = insns(path, func)
        k = next(n for n, r in enumerate(rows) if 'D_8008EB40' in r[2])
        print(f'== {path} {func}')
        for uid, kind, txt in rows[max(0, k - 8):k + 4]:
            s = re.sub(r'\(nil\)|\[[^\]]*\]', '', txt)
            s = re.sub(r'^\((insn|jump_insn) \d+ \d+ \d+ ', '', s)
            print(f'  {uid:>5} {kind[:4]} {s[:150]}')
