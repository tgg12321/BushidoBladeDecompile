"""bodydiff.py <msgA> <msgB>: test clone - completed C bodies whose layer-2 key differs between two chain commits."""
import re, subprocess, sys
T = "/tmp/q56r2/t"
sys.path.insert(0, T)
from engine import layer2
DEF = re.compile(r"^[A-Za-z_][\w \t\*]*?\b([A-Za-z_]\w*)\s*\([^;{]*\)\s*\{?\s*$", re.M)


def commit(msg):
    return subprocess.run(["git", "-C", T, "log", "--format=%H", f"--grep=^{msg}$"], capture_output=True, text=True).stdout.split()[0]


def keys(c):
    out = {}
    files = [f for f in subprocess.run(["git", "-C", T, "ls-tree", "--name-only", f"{c}:src"], capture_output=True, text=True).stdout.split() if f.endswith(".c")]
    for f in files:
        t = subprocess.run(["git", "-C", T, "show", f"{c}:src/{f}"], capture_output=True, text=True).stdout
        for n in set(DEF.findall(t)):
            s = layer2.body_source(t, n, read=lambda p: None)
            if s and s[0] == "c":
                out[n] = (layer2._key(s[1]), f)
    return out


a, b = keys(commit(sys.argv[1])), keys(commit(sys.argv[2]))
for n in sorted(set(a) | set(b)):
    if a.get(n, (None,))[0] != b.get(n, (None,))[0]:
        print(f"{n}: {a.get(n)} -> {b.get(n)}")
