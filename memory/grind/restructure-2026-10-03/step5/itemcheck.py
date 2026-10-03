"""itemcheck.py OLDREV NEWREV : the multiset of top-level items (each with its attached comments,
whitespace-normalized) of the old game headers vs the new game.h + bb2.h. Prints the items only on
one side."""
import collections, re, subprocess, sys
sys.path.insert(0, "tmp/s5")
from citems import items
old_rev, new_rev = sys.argv[1], sys.argv[2]
def show(rev, p):
    return subprocess.run(["git", "show", f"{rev}:{p}"], capture_output=True, text=True, encoding="utf-8").stdout
def bag(texts):
    b = collections.Counter()
    for t in texts:
        its, lines = items(t)
        for it in its:
            own = " ".join("\n".join(lines[it["start"] - 1:it["end"]]).split())
            lead = " ".join("\n".join(lines[it["lead"] - 1:it["start"] - 1]).split())
            b[("ITEM", own)] += 1
            if lead:
                for c in re.findall(r"/\*.*?\*/", lead):
                    b[("COMMENT", c)] += 1
    return b
old = bag(show(old_rev, f"include/{h}.h") for h in ("system", "gpu", "game", "code6cac"))
new = bag(show(new_rev, f"include/{h}.h") for h in ("game", "bb2"))
for k in sorted(set(old) | set(new)):
    if old[k] != new[k]:
        print(f"{'OLD' if old[k] > new[k] else 'NEW'} x{abs(old[k]-new[k])} {k[0]}: {k[1][:200]}")
