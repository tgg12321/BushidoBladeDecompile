#!/usr/bin/env python3
"""census.py : declaration census of the current tree (run in WSL, repo root).
Functions: cc1 -aux-info per TU (canonical prototype text, decl site file:line, kind NC/OC/IC/NF/OF).
Objects: a scan of every `extern ...;` (file and block scope) in src/**/*.c and include/**/*.h.
Writes tmp/p2/census.json: {"funcs": [{tu,file,line,kind,text,name}], "objs": [{file,line,block,text,name}],
"defs": {name: tu} (C definitions + INCLUDE_ASM functions)}."""
import json, re, subprocess, sys, glob
sys.path.insert(0, ".")
from engine import tus
CPP = ["mipsel-linux-gnu-cpp", "-Iinclude", "-undef", "-Wall", "-lang-c", "-fno-builtin", "-Dmips",
       "-D__GNUC__=2", "-D__OPTIMIZE__", "-D__mips__", "-D__mips", "-Dpsx", "-D__psx__", "-D__psx", "-D_PSYQ",
       "-D__EXTENSIONS__", "-D_MIPSEL", "-D_LANGUAGE_C", "-DLANGUAGE_C"]
CC1 = ["tools/gcc-2.7.2/build/cc1", "-O2", "-G0", "-funsigned-char", "-quiet", "-mcpu=3000", "-mips1",
       "-mno-abicalls", "-fno-builtin", "-w", "-mel", "-msoft-float", "-o", "/dev/null", "-aux-info", "/tmp/p2aux.txt"]
funcs, defs = [], {}
AUX = re.compile(r"/\* (\S+):(\d+):(\w\w) \*/ (.*)$")
for t in tus.linked_tus():
    pre = subprocess.run(CPP + [f"src/{t}.c"], capture_output=True).stdout
    subprocess.run(CC1, input=pre, capture_output=True)
    for l in open("/tmp/p2aux.txt", encoding="utf-8", errors="replace"):
        m = AUX.match(l.rstrip("\n"))
        if not m:
            continue
        f, ln, k, txt = m.groups()
        txt = re.sub(r"\s*/\*.*?\*/\s*$", "", txt).strip()
        nm = re.match(r"(?:extern |static )?.*?\b(\w+) \(", txt)
        name = nm.group(1) if nm else "?"
        funcs.append(dict(tu=t, file=f, line=int(ln), kind=k, text=txt, name=name))
        if k in ("NF", "OF") and f == f"src/{t}.c":
            defs[name] = t
    for m in re.finditer(r'INCLUDE_ASM\("asm/funcs",\s*(\w+)\)', open(f"src/{t}.c", encoding="utf-8").read()):
        defs.setdefault(m.group(1), t)
def strip_comments(s):
    return re.sub(r"/\*.*?\*/", lambda m: " " * 0 + "\n" * m.group(0).count("\n"), s, flags=re.S)
objs = []
for f in sorted(glob.glob("src/**/*.c", recursive=True)) + sorted(glob.glob("include/**/*.h", recursive=True)):
    if "m2c" in f:
        continue
    raw = open(f, encoding="utf-8").read()
    t = strip_comments(raw)
    t = re.sub(r"//[^\n]*", "", t)
    depth, line = 0, 1
    i = 0
    while i < len(t):
        c = t[i]
        if c == "\n":
            line += 1
        elif c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
        elif t.startswith("extern", i) and (i == 0 or not (t[i-1].isalnum() or t[i-1] == "_")):
            j = t.index(";", i)
            body = " ".join(t[i:j+1].split())
            if "(" in body and not re.search(r"\(\s*\*", body):
                pass  # function prototype: covered by aux-info
            else:
                b = re.sub(r"\[[^\]]*\]", "", body.split("=")[0])
                m2 = re.search(r"\(\s*\*\s*(\w+)\s*\)", b)
                name = m2.group(1) if m2 else re.findall(r"\w+", b)[-1]
                objs.append(dict(file=f, line=line, block=depth > 0, text=body, name=name))
            line += t[i:j+1].count("\n")
            i = j + 1
            continue
        i += 1
json.dump(dict(funcs=funcs, objs=objs, defs=defs), open("tmp/p2/census.json", "w"), indent=0)
print(len(funcs), "func decl rows;", len(objs), "object externs;", len(defs), "defined functions")
