"""Build tmp/func_80070188/mini/pre.c: src/text1b.c up to func_80070188's
INCLUDE_ASM line with every function BODY removed (declarations, typedefs and
externs kept), SRC_REPS applied. A candidate appended to it compiles in ~1 s;
sc.py --mini verifies it scores identically to the full TU."""
import sys, re, importlib.util
sys.path.insert(0, '.')
from pathlib import Path
_s = importlib.util.spec_from_file_location('sc_reps', 'tmp/func_80070188/sc_reps.py')
_m = importlib.util.module_from_spec(_s); _s.loader.exec_module(_m)

t = Path('src/text1b.c').read_text(encoding='utf-8')
marker = 'INCLUDE_ASM("asm/funcs", func_80070188);'
t = _m.apply(t)
t = t[:t.index(marker)]

out = []
i = 0
n = len(t)
depth = 0
last_top = 0  # index just after the last top-level ';' or '}'
res = []
seg_start = 0
while i < n:
    c = t[i]
    if c == '/' and t.startswith('/*', i):
        j = t.index('*/', i + 2) + 2
        i = j
        continue
    if c == '/' and t.startswith('//', i):
        i = t.index('\n', i)
        continue
    if c == '"' or c == "'":
        q = c
        i += 1
        while t[i] != q:
            i += 2 if t[i] == '\\' else 1
        i += 1
        continue
    if c == '#' and (i == 0 or t[i - 1] == '\n') and depth == 0:
        # preprocessor line (may continue)
        j = i
        while True:
            e = t.index('\n', j)
            if t[e - 1] == '\\':
                j = e + 1
                continue
            break
        i = e
        continue
    if c == '{':
        if depth == 0:
            k = i - 1
            while t[k].isspace():
                k -= 1
            if t[k] == ')':
                # function body: find matching brace
                d = 0
                j = i
                while True:
                    ch = t[j]
                    if ch == '/' and t.startswith('/*', j):
                        j = t.index('*/', j + 2) + 2; continue
                    if ch == '/' and t.startswith('//', j):
                        j = t.index(chr(10), j); continue
                    if ch == '#' and t[j - 1] == chr(10):
                        j = t.index(chr(10), j); continue
                    if ch == '"' or ch == "'":
                        q = ch; j += 1
                        while t[j] != q:
                            j += 2 if t[j] == '\\' else 1
                        j += 1; continue
                    if ch == '{': d += 1
                    elif ch == '}':
                        d -= 1
                        if d == 0: break
                    j += 1
                # replace "header { body }" with "header;"  (keep as prototype)
                res.append(t[seg_start:i].rstrip() + ';\n')
                seg_start = j + 1
                i = j + 1
                continue
        depth += 1
    elif c == '}':
        depth -= 1
    i += 1
res.append(t[seg_start:])
pre = ''.join(res)
# drop INCLUDE_ASM lines (they only emit other functions' asm)
pre = re.sub(r'^INCLUDE_ASM\([^\n]*\);\s*$', '', pre, flags=re.M)
Path('tmp/func_80070188/mini').mkdir(parents=True, exist_ok=True)
Path('tmp/func_80070188/mini/pre.c').write_bytes(pre.encode('utf-8'))
print(len(t), '->', len(pre))
