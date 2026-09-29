"""Split src/text1b.c at a top-level boundary into a pre part (stays text1b.c)
and a post part (new TU). The post part is the byte-identical tail slice of
the original file; it gets a head block made of
  (a) the original include lines,
  (b) verbatim copies of the pre part's file-scope declaration items
      (typedef / struct / union / extern / prototype, #define still active at
      the split) whose declared names the post part uses, closed transitively,
  (c) for every function DEFINED in the pre part that the post part names, the
      declaration its definition provided there: the ANSI definition header
      verbatim + ';' (a prototype), or `<type> name();` for a K&R definition.
Function definitions, INCLUDE_ASM/__asm__ items and object definitions are
never copied.

usage: tusplit.py <split_before_func> <out_pre.c> <out_post.c> <report.txt>
"""
import re
import sys

SRC = 'src/text1b.c'
split_func = sys.argv[1]
out_pre, out_post, report = sys.argv[2:5]

text = open(SRC, encoding='utf-8', newline='').read()

def strip_comments(s):
    s = re.sub(r'/\*.*?\*/', ' ', s, flags=re.S)
    s = re.sub(r'//[^\n]*', ' ', s)
    s = re.sub(r'"(\\.|[^"\\])*"', '""', s)
    return s

KR_HEAD = re.compile(r'^[\w\s\*]*?\b(\w+)\s*\(\s*(\w+(\s*,\s*\w+)*)?\s*\)', re.S)

def is_kr_head(code):
    m = KR_HEAD.match(code)
    if not m or not m.group(2):
        return False
    params = [p.strip() for p in m.group(2).split(',')]
    return params != ['void'] and code[m.end():].strip() != ''

def items_of(txt):
    """Top-level items as (start, end, kind): pp | decl | func | asm."""
    i, n = 0, len(txt)
    depth = paren = 0
    start = 0
    out = []
    while i < n:
        c = txt[i]
        if txt.startswith('/*', i):
            i = txt.find('*/', i + 2) + 2
            continue
        if txt.startswith('//', i):
            j = txt.find('\n', i)
            i = j if j >= 0 else n
            continue
        if c in '"\'':
            i += 1
            while txt[i] != c:
                if txt[i] == '\\':
                    i += 1
                i += 1
            i += 1
            continue
        if depth == 0 and paren == 0 and c == '#' and (i == 0 or txt[i - 1] == '\n'):
            k = i
            while True:
                k2 = txt.find('\n', k)
                if k2 < 0:
                    k2 = n
                if txt[k2 - 1] == '\\':
                    k = k2 + 1
                    continue
                break
            out.append((i, k2, 'pp'))
            i = start = k2
            continue
        if c == '(':
            paren += 1
        elif c == ')':
            paren -= 1
        elif c == '{':
            if depth == 0 and paren == 0:
                code = strip_comments(txt[start:i]).strip()
                if re.search(r'\)\s*$', code) or is_kr_head(code):
                    # function body: find its matching brace
                    d = 0
                    j = i
                    while True:
                        if txt.startswith('/*', j):
                            j = txt.find('*/', j + 2) + 2
                            continue
                        if txt.startswith('//', j):
                            j = txt.find('\n', j)
                            continue
                        ch = txt[j]
                        if ch in '"\'':
                            j += 1
                            while txt[j] != ch:
                                if txt[j] == '\\':
                                    j += 1
                                j += 1
                            j += 1
                            continue
                        if ch == '{':
                            d += 1
                        elif ch == '}':
                            d -= 1
                            if d == 0:
                                break
                        j += 1
                    out.append((start, j + 1, 'func'))
                    i = start = j + 1
                    continue
            depth += 1
        elif c == '}':
            depth -= 1
        elif c == ';' and depth == 0 and paren == 0:
            code = strip_comments(txt[start:i]).strip()
            if is_kr_head(code) and not re.search(r'\)\s*;?\s*$', code):
                pass  # K&R parameter declaration inside a function head
            elif is_kr_head(code) and txt[i + 1:].lstrip()[:1] != '' and _next_is_kr(txt, i):
                pass
            else:
                kind = 'asm' if re.match(r'(INCLUDE_ASM|INCLUDE_RODATA|__asm__|PAD_NOPS_\d)\b', code) else 'decl'
                out.append((start, i + 1, kind))
                start = i + 1
        i += 1
    return out

def _next_is_kr(txt, i):
    """After `f(a, b)` + ';' ... is this a K&R head? Only if the item before the
    ';' is `name(ids)` with NO type words inside the parens AND a '{' follows
    the run of parameter declarations."""
    return False

items = items_of(text)

def func_name(code):
    head = strip_comments(code).split('{')[0]
    m = re.search(r'(\w+)\s*\([^()]*\)\s*(?:[^;{]*;)*\s*$', head.strip(), re.S)
    m = re.match(r'[\w\s\*]*?\b(\w+)\s*\(', head.strip(), re.S)
    return m.group(1) if m else None

target = None
for idx, (s, e, k) in enumerate(items):
    if k == 'func' and func_name(text[s:e]) == split_func:
        target = idx
        break
if target is None:
    sys.exit('split function not found')
j = target - 1
while j >= 0 and items[j][2] != 'func':
    j -= 1
split_at = text.index('\n', items[j][1]) + 1
pre_text, post_text = text[:split_at], text[split_at:]

IDENT = re.compile(r'[A-Za-z_]\w*')
KEYWORDS = set('''auto break case char const continue default do double else enum
extern float for goto if int long register return short signed sizeof static
struct switch typedef union unsigned void volatile while asm __asm__ inline'''.split())

def tokens(s):
    t = strip_comments(s)
    toks = set(IDENT.findall(t))
    for m in re.finditer(r'\b(struct|union|enum)\s+(\w+)', t):
        toks.add(m.group(1) + ' ' + m.group(2))
    return toks

def decl_names(st):
    names = []
    if st.startswith('typedef'):
        m = re.search(r'(\w+)\s*(\[[^\]]*\]\s*)*;\s*$', st)
        if m:
            names.append(m.group(1))
        m2 = re.search(r'\(\s*\*\s*(\w+)\s*\)', st)
        if m2:
            names.append(m2.group(1))
    m3 = re.match(r'(?:typedef\s+)?(struct|union|enum)\s+(\w+)', st)
    if m3:
        names.append(m3.group(1) + ' ' + m3.group(2))
    if not st.startswith('typedef'):
        body = re.sub(r'\{.*\}', ' ', st, flags=re.S)
        body = re.sub(r'\([^()]*\)', '()', body)  # drop prototype parameter lists
        body = re.sub(r'\([^()]*\)', '()', body)
        for mm in re.finditer(r'(\w+)\s*(?=[\(\[;,=)])', body):
            w = mm.group(1)
            if w not in KEYWORDS and not re.match(r'\d|0x', w):
                names.append(w)
    return names

decls = []          # (s, e, names, text)
macros = {}         # name -> (s, e)
func_defs = {}      # name -> (s, e)
objdefs = []
for s, e, k in items:
    if e > split_at:
        break
    t = text[s:e]
    st = strip_comments(t).strip()
    if k == 'pp':
        m = re.match(r'#\s*(define|undef)\s+(\w+)', st)
        if m and m.group(1) == 'define':
            macros[m.group(2)] = (s, e)
        elif m:
            macros.pop(m.group(2), None)
    elif k == 'func':
        n = func_name(t)
        if n:
            func_defs[n] = (s, e)
    elif k == 'decl':
        is_objdef = (not st.startswith(('extern', 'typedef'))
                     and not re.match(r'(struct|union|enum)\s+\w+\s*\{', st)
                     and ('=' in st.split('(')[0] or '(' not in st))
        if is_objdef:
            objdefs.append((s, st))
        else:
            decls.append((s, e, decl_names(st), st))

need = tokens(post_text)
chosen, chosen_macros, protos = {}, {}, {}
changed = True
while changed:
    changed = False
    for s, e, names, st in decls:
        if (s, e) not in chosen and any(n in need for n in names):
            chosen[(s, e)] = st
            need |= tokens(text[s:e])
            changed = True
    for name, span in macros.items():
        if name in need and span not in chosen_macros:
            chosen_macros[span] = name
            need |= tokens(text[span[0]:span[1]])
            changed = True
    for name, (s, e) in func_defs.items():
        if name in need and name not in protos:
            head = strip_comments(text[s:e]).split('{')[0].strip()
            if is_kr_head(head):
                m = re.match(r'([\w\s\*]*?)\b%s\s*\(' % name, head)
                protos[name] = (s, (m.group(1).strip() or 'int') + ' ' + name + '();')
            else:
                protos[name] = (s, re.sub(r'\s+', ' ', head) + ';')
            need |= tokens(protos[name][1])
            changed = True

def clean(seg):
    m = re.match(r'(\s|/\*.*?\*/|//[^\n]*)*', seg, re.S)
    return seg[m.end():].rstrip()

entries = [(s, clean(text[s:e])) for (s, e) in chosen]
entries += [(s, clean(text[s:e])) for (s, e) in chosen_macros]
entries += [(s, p) for (s, p) in protos.values()]
entries.sort()
seen = set()
uniq = []
for s_, p_ in entries:
    key = re.sub(r'\s+', ' ', p_)
    if key in seen:
        continue
    seen.add(key)
    uniq.append((s_, p_))
entries = uniq

head_lines = text.split('\n')[:6]
with open(out_pre, 'w', encoding='utf-8', newline='') as f:
    f.write(pre_text)
with open(out_post, 'w', encoding='utf-8', newline='') as f:
    f.write('\n'.join(head_lines) + '\n')
    f.write('\n/* File-scope declarations the functions below use, copied verbatim from\n'
            ' * src/text1b.c (declarations, typedefs, and the prototypes that the\n'
            ' * definitions of text1b.c functions provided); generated by\n'
            ' * memory/grind/func_800720FC/split/tusplit.py. */\n')
    f.write('\n'.join(p for _, p in entries) + '\n\n')
    f.write(post_text)
with open(report, 'w', encoding='utf-8', newline='') as f:
    f.write('split before %s at line %d\n' % (split_func, pre_text.count('\n') + 1))
    for s_, st_ in objdefs:
        names_ = decl_names(st_)
        if any(n in tokens(post_text) for n in names_):
            f.write('OBJECT DEFINITION used by post (not copied): %s\n' % st_[:100])
    f.write('decls %d, macros %d, prototypes %d\n' % (len(chosen), len(chosen_macros), len(protos)))
    for s, p in entries:
        f.write('%6d  %s\n' % (text.count('\n', 0, s) + 1, p.replace('\n', ' ')[:110]))
print(open(report).read()[:4000])
