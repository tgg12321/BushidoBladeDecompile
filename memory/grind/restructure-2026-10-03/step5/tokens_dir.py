"""tokens_dir.py DIR SNAP : token-stream hash of every linked TU compiled from DIR (a tree copy)
compared with snapshot SNAP's tokens.txt. Run in WSL from the repo root."""
import hashlib, os, subprocess, sys
sys.path.insert(0, ".")
from engine import tus
d, snap = sys.argv[1], sys.argv[2]
CPP = ["mipsel-linux-gnu-cpp", "-I" + os.path.join(d, "include"), "-undef", "-Wall", "-lang-c", "-fno-builtin", "-Dmips",
       "-D__GNUC__=2", "-D__OPTIMIZE__", "-D__mips__", "-D__mips", "-Dpsx", "-D__psx__", "-D__psx", "-D_PSYQ",
       "-D__EXTENSIONS__", "-D_MIPSEL", "-D_LANGUAGE_C", "-DLANGUAGE_C"]
ref = dict(l.split() for l in open(f"tmp/s5/snap/{snap}/tokens.txt") if l.strip())
diff = []
for t in tus.linked_tus():
    src = os.path.join(d, "src", t + ".c")
    pre = subprocess.run(CPP + [src], capture_output=True).stdout
    body = b"\n".join(l for l in pre.split(b"\n") if l.strip() and not l.startswith(b"# "))
    if hashlib.sha1(body).hexdigest() != ref.get(t):
        diff.append(t)
print(d, "vs", snap, ":", len(diff), "TUs with different token streams", diff[:10])
