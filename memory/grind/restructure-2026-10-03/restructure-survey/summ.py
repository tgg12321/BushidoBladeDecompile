import re, collections, os
addr = {}
for ln in open("tmp/restructure-survey/elf_nm.txt"):
    p = ln.split()
    if len(p) >= 4: addr[p[3]] = int(p[0], 16)
    elif len(p) == 3: addr[p[2]] = int(p[0], 16)
objs = collections.OrderedDict()
for ln in open("tmp/restructure-survey/objfuncs.txt"):
    o, t, n = ln.split()
    objs.setdefault(o, []).append((t, n))
for o, syms in objs.items():
    fs = [(addr.get(n, 0), n) for t, n in syms if t in "Tt" and not n.startswith(".L") and not n.startswith("jtbl") and n != "gcc2_compiled."]
    fs.sort()
    if not os.path.exists(f"src/{o}.c"): continue
    src = open(f"src/{o}.c", encoding="utf-8", errors="replace").read()
    nlines = src.count("\n")
    nia = len(re.findall(r"^\s*INCLUDE_ASM\(", src, re.M))
    nex = len(re.findall(r"^\s*extern\b", src, re.M))
    nexb = len(re.findall(r"^\s+extern\b", src, re.M))
    named = [n for a, n in fs if not re.match(r"func_[0-9A-F]{8}$", n)]
    print(f"== {o}.c lines={nlines} funcs={len(fs)} named={len(named)} INCLUDE_ASM={nia} extern={nex} (block-scope {nexb})")
    if fs:
        print(f"   {fs[0][0]:08X} {fs[0][1]} .. {fs[-1][0]:08X} {fs[-1][1]}")
    print("   names:", " ".join(named[:40]) + (" ..." if len(named) > 40 else ""))
