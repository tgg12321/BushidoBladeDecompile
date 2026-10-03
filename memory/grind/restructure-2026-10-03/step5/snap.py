#!/usr/bin/env python3
"""snap.py NAME : snapshot every linked TU's object, its token stream hash (cpp output without line
markers / blank lines) and its implicit-declaration set into tmp/s5/snap/NAME/. Run in WSL, repo root,
after a full build."""
import hashlib, os, re, shutil, subprocess, sys
sys.path.insert(0, ".")
from engine import tus
name = sys.argv[1]
d = f"tmp/s5/snap/{name}"
shutil.rmtree(d, ignore_errors=True)
os.makedirs(d)
CPP = ["mipsel-linux-gnu-cpp", "-Iinclude", "-undef", "-Wall", "-lang-c", "-fno-builtin", "-Dmips",
       "-D__GNUC__=2", "-D__OPTIMIZE__", "-D__mips__", "-D__mips", "-Dpsx", "-D__psx__", "-D__psx", "-D_PSYQ",
       "-D__EXTENSIONS__", "-D_MIPSEL", "-D_LANGUAGE_C", "-DLANGUAGE_C"]
CC1 = ["tools/gcc-2.7.2/build/cc1", "-O2", "-G0", "-funsigned-char", "-quiet", "-mcpu=3000", "-mips1",
       "-mno-abicalls", "-fno-builtin", "-Wimplicit", "-mel", "-msoft-float", "-o", "/dev/null"]
ids = tus.linked_tus()
toks, impl = [], []
for t in ids:
    o = f"build/src/{t}.o"
    os.makedirs(os.path.dirname(f"{d}/{t}"), exist_ok=True)
    shutil.copyfile(o, f"{d}/{t}.o")
    pre = subprocess.run(CPP + [f"src/{t}.c"], capture_output=True).stdout
    body = b"\n".join(l for l in pre.split(b"\n") if l.strip() and not l.startswith(b"# "))
    toks.append(f"{t} {hashlib.sha1(body).hexdigest()}")
    err = subprocess.run(CC1, input=pre, capture_output=True).stderr.decode("utf-8", "replace")
    fn = "<file scope>"
    for line in err.splitlines():
        m = re.search(r"In function `([^']*)'", line)
        if m:
            fn = m.group(1); continue
        if "At top level" in line:
            fn = "<file scope>"
        m = re.search(r"implicit declaration of function `([^']*)'", line)
        if m:
            impl.append(f"{t}\t{fn}\t{m.group(1)}")
open(f"{d}/tokens.txt", "w").write("\n".join(toks) + "\n")
open(f"{d}/implicit.txt", "w").write("\n".join(sorted(set(impl))) + "\n")
print(name, len(ids), "TUs;", len(set(impl)), "implicit pairs")
