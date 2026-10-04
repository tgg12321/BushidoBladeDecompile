#!/usr/bin/env python3
"""snap.py NAME : snapshot the current build into tmp/p2/snap/NAME/ (run in WSL, repo root, after make).
  obj/<tu>.o      every linked TU's object (bb2.ld order, engine.tus)
  tokens.txt      <tu> <sha1 of its cpp output without line markers / blank lines>
  implicit.txt    <tu>\t<function>\t<callee> for every implicit declaration cc1 -Wimplicit reports
  src/            copies of src/**/*.c and include/**/*.h (l2diff.py keys bodies from these)
  meta.txt        exe sha1, git HEAD, dirty paths"""
import concurrent.futures as cf, glob, hashlib, os, re, shutil, subprocess, sys
sys.path.insert(0, ".")
from engine import tus

name = sys.argv[1]
d = f"tmp/p2/snap/{name}"
shutil.rmtree(d, ignore_errors=True)
os.makedirs(d)
CPP = ["mipsel-linux-gnu-cpp", "-Iinclude", "-undef", "-Wall", "-lang-c", "-fno-builtin", "-Dmips",
       "-D__GNUC__=2", "-D__OPTIMIZE__", "-D__mips__", "-D__mips", "-Dpsx", "-D__psx__", "-D__psx", "-D_PSYQ",
       "-D__EXTENSIONS__", "-D_MIPSEL", "-D_LANGUAGE_C", "-DLANGUAGE_C"]
CC1 = ["tools/gcc-2.7.2/build/cc1", "-O2", "-G0", "-funsigned-char", "-quiet", "-mcpu=3000", "-mips1",
       "-mno-abicalls", "-fno-builtin", "-Wimplicit", "-mel", "-msoft-float", "-o", "/dev/null"]
ids = tus.linked_tus()


def one(t):
    pre = subprocess.run(CPP + [f"src/{t}.c"], capture_output=True).stdout
    body = b"\n".join(l for l in pre.split(b"\n") if l.strip() and not l.startswith(b"# "))
    err = subprocess.run(CC1, input=pre, capture_output=True).stderr.decode("utf-8", "replace")
    fn, impl = "<file scope>", []
    for line in err.splitlines():
        m = re.search(r"In function `([^']*)'", line)
        if m:
            fn = m.group(1)
            continue
        if "At top level" in line:
            fn = "<file scope>"
        m = re.search(r"implicit declaration of function `([^']*)'", line)
        if m:
            impl.append(f"{t}\t{fn}\t{m.group(1)}")
    return f"{t} {hashlib.sha1(body).hexdigest()}", impl


for t in ids:
    os.makedirs(os.path.dirname(f"{d}/obj/{t}"), exist_ok=True)
    shutil.copyfile(f"build/src/{t}.o", f"{d}/obj/{t}.o")
with cf.ThreadPoolExecutor(os.cpu_count() or 4) as ex:
    res = list(ex.map(one, ids))
toks = [r[0] for r in res]
impl = sorted({x for r in res for x in r[1]})
for f in glob.glob("src/**/*.c", recursive=True) + glob.glob("include/**/*.h", recursive=True):
    os.makedirs(os.path.dirname(f"{d}/src/{f}"), exist_ok=True)
    shutil.copyfile(f, f"{d}/src/{f}")
with open(f"{d}/tokens.txt", "w", newline="\n") as fh:
    fh.write("\n".join(toks) + "\n")
with open(f"{d}/implicit.txt", "w", newline="\n") as fh:
    fh.write("\n".join(impl) + "\n")
sha = hashlib.sha1(open("build/bb2.exe", "rb").read()).hexdigest()
head = subprocess.run(["git", "rev-parse", "HEAD"], capture_output=True, text=True).stdout.strip()
dirty = subprocess.run(["git", "status", "--porcelain", "--", "src", "include"], capture_output=True,
                       text=True).stdout.split("\n")
with open(f"{d}/meta.txt", "w", newline="\n") as fh:
    fh.write(f"exe_sha1 {sha}\nhead {head}\n" + "".join(f"dirty {l}\n" for l in dirty if l.strip()))
print(f"snap {name}: {len(ids)} TUs, {len(impl)} implicit pairs, exe {sha[:12]}")
