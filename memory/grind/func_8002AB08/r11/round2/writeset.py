# writeset.py <root>... : transitive callee closure over asm/funcs/*.s (jal targets) and every store that
# could hit a PracticeMenuRec +0x0C..+0x0F: base+offset stores with offset 0xC..0xF (any base), and stores
# through %lo(sym) with sym inside g_practice_menu_table's two records (0x80101EC8..0x80102713) at +0xC..+0xF.
import re, sys, glob, os
syms = {}
for f in ['undefined_syms_auto.txt', 'symbol_addrs.txt', 'named_syms.txt']:
    if os.path.exists(f):
        for l in open(f, errors='replace'):
            m = re.match(r'\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)', l)
            if m: syms[m.group(1)] = int(m.group(2), 16)
def body(fn):
    p = f'asm/funcs/{fn}.s'
    return open(p, errors='replace').read() if os.path.exists(p) else None
seen, todo, missing = set(), list(sys.argv[1:]), []
while todo:
    fn = todo.pop()
    if fn in seen: continue
    seen.add(fn)
    b = body(fn)
    if b is None: missing.append(fn); continue
    for t in re.findall(r'\bjal\s+(\w+)', b):
        todo.append(t)
    if re.search(r'\bjalr\b', b): print(f"NOTE {fn}: indirect call (jalr)")
print("closure:", len(seen), "functions; not asm (C or lib):", sorted(missing))
REC0, REC1 = 0x80101EC8, 0x80101EC8 + 0x44C
for fn in sorted(seen):
    b = body(fn)
    if b is None: continue
    for l in b.splitlines():
        m = re.search(r'\b(sw|sh|sb|swl|swr)\s+\$\w+, (.*)$', l)
        if not m: continue
        op, addr = m.group(1), m.group(2).strip()
        m2 = re.match(r'(-?0x[0-9A-Fa-f]+|-?\d+)\(\$(\w+)\)', addr)
        if m2:
            off = int(m2.group(1), 0); base = m2.group(2)
            if base != 'sp' and 0xC <= off <= 0xF:
                print(f"OFF  {fn}: {l.strip()}")
            continue
        m3 = re.match(r'%lo\((\w+)(?:\s*\+\s*(0x[0-9A-Fa-f]+|\d+))?\)', addr)
        if m3:
            a = syms.get(m3.group(1))
            if a is None: continue
            a += int(m3.group(2), 0) if m3.group(2) else 0
            for r in (REC0, REC1):
                if r + 0xC <= a <= r + 0xF:
                    print(f"SYM  {fn}: {l.strip()}")
