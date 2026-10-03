"""restore2.py TREE LINE FILE... : re-insert the exact LINE (a column-0 declaration) into FILE at the
places it had in TREE, mapped through difflib (TREE text vs current text)."""
import difflib, subprocess, sys
tree, line, files = sys.argv[1], sys.argv[2], sys.argv[3:]
for f in files:
    old = subprocess.run(["git", "show", f"{tree}:{f}"], capture_output=True, text=True, encoding="utf-8").stdout.split("\n")
    cur = open(f, encoding="utf-8", newline="").read().split("\n")
    sm = difflib.SequenceMatcher(None, old, cur, autojunk=False)
    m = {}
    for a, b, n in sm.get_matching_blocks():
        for k in range(n):
            m[a + k] = b + k
    ins = []
    for i, l in enumerate(old):
        if l != line or i in m:
            continue
        j = i - 1
        while j >= 0 and j not in m:
            j -= 1
        ins.append(m[j] + 1 if j >= 0 else 0)
    for pos in sorted(ins, reverse=True):
        cur.insert(pos, line)
        print(f, "inserted at", pos + 1, line)
    open(f, "w", encoding="utf-8", newline="\n").write("\n".join(cur))
