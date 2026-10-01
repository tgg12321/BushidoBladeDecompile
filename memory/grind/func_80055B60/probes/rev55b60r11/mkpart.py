# Two-way partitions of temp's six values: subset S moves to temp_b (function scope), rest stay in temp.
import re, itertools, sys, types
src = open('../../memory/grind/func_80055B60/probes/r11/abl2.py').read()
src = src.split("made = []")[0].replace("body = open(sys.argv[1]).read()", "body = open('F_staged_body.c').read()").replace("prefix = sys.argv[2]", "prefix='x'")
ns = {}
exec(src, ns)
V = ns['V']; body = ns['body']
vals = ['dist', 'n', 'add', 'mask', 'da', 'ret']
def gen(S):
    t = body
    for nm in S:
        for o, new in V[nm]:
            new2 = re.sub(r'\n\s*s32 %s;\n(\n)?' % nm, '\n', new)
            new2 = re.sub(r'^\s*s32 %s;\n' % nm, '', new2) if new2.strip().startswith('s32 %s;' % nm) else new2
            new2 = re.sub(r'\b%s\b' % nm, 'temp_b', new2)
            if o.endswith("s32 i;\n") and new2 == o:
                continue
            assert t.count(o) >= 1, (nm, o)
            t = t.replace(o, new2, 1)
    t = t.replace("    s32 temp;\n", "    s32 temp;\n    s32 temp_b;\n", 1)
    return t
out = []
for k in (1, 2, 3):
    for S in itertools.combinations(vals, k):
        if k == 3 and 'dist' not in S:
            continue  # 3-3 splits: count each once
        name = 'P_' + '_'.join(S)
        open('part/%s.c' % name, 'w', newline='\n').write(gen(S))
        out.append(name)
print(' '.join(out))
