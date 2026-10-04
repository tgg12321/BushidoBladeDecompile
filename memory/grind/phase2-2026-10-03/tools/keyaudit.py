import sys, re, subprocess
sys.path.insert(0, "."); sys.path.insert(0, "memory/grind/phase2-2026-10-03/tools")
from engine import layer2 as L2
from citems import items
def show(rev, f):
    r = subprocess.run(["git", "show", f"{rev}:{f}"], capture_output=True)
    return r.stdout.decode("utf-8") if r.returncode == 0 else None
commits = subprocess.run(["git", "rev-list", "--reverse", "1a40f4535..HEAD"], capture_output=True, text=True).stdout.split()
for c in commits:
    files = subprocess.run(["git", "diff", "--name-only", f"{c}^", c, "--", "src"], capture_output=True, text=True).stdout.split()
    for f in files:
        if not f.endswith(".c"): continue
        old, new = show(c + "^", f), show(c, f)
        if old is None or new is None: continue
        names = set()
        for t in (old, new):
            its, _ = items(t); names |= {i["name"] for i in its if i["kind"] in ("func","stub") and i["name"]}
        allf = set(re.findall(r'^(?:[A-Za-z_][\w \*]*?\s+\**)?([A-Za-z_]\w*)\s*\([^;]*\)\s*\{?\s*$', new, re.M))
        for fn in sorted(allf - names):
            ko, kn = L2.body_key(old, fn), L2.body_key(new, fn)
            if ko != kn and kn is not None: print(c[:9], f, fn, ko and ko[1], "->", kn[1])
