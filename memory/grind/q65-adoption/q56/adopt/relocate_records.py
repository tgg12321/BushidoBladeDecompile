#!/usr/bin/env python3
"""relocate_records.py <tree> <old-stem>...: a record that names a function's source file follows the function when a
step moved it. The file of every function is read from the tree itself (its C definition or its
INCLUDE_ASM line in src/*.c); a record is changed only when the file it names no longer holds the
function AND it names one of the <old-stem> files (the files a step split, merged or cut;
unrelated pre-existing drift is left alone). Touches: engine/queue.json items' "file", memory/grind/<func>/state.json "file",
tools/canonical_asm_regions.json functions' "file", and the function's own src/<file>.c token on its
tools/grinder/scope_allow.txt line. Text edits in place (no reformatting). Prints every change."""
import glob, json, os, re, sys
R = sys.argv[1]
OLD = set(sys.argv[2:])
os.chdir(R)
where = {}
for p in sorted(glob.glob("src/*.c")):
    stem = os.path.basename(p)[:-2]
    t = open(p).read()
    for m in re.finditer(r'INCLUDE_ASM\(\s*"[^"]*"\s*,\s*(\w+)\s*\)', t):
        where.setdefault(m.group(1), set()).add(stem)
    for m in re.finditer(r"^[A-Za-z_][\w\s\*]*?\b(\w+)\s*\([^;{]*\)\s*\{", t, re.M):
        where.setdefault(m.group(1), set()).add(stem)
    for m in re.finditer(r'glabel\s+(\w+)', t):
        where.setdefault(m.group(1), set()).add(stem)
moved = {}


def target(func, old):
    fs = where.get(func)
    if old not in OLD or not fs or old in fs or len(fs) != 1:
        return None
    return next(iter(fs))


# queue.json
p = "engine/queue.json"
t = open(p).read()
q = json.loads(t)
n = 0
for it in q.get("items", []):
    new = target(it.get("func"), it.get("file"))
    if new:
        print(f"queue: {it['func']}: {it['file']} -> {new}")
        moved[it["func"]] = (it["file"], new)
        it["file"] = new
        n += 1
if n:
    open(p, "w", newline="\n").write(json.dumps(q, indent=2) + "\n")
# grind state.json
for sp in sorted(glob.glob("memory/grind/*/state.json")):
    s = open(sp).read()
    fn = os.path.basename(os.path.dirname(sp))   # the ledger directory names the function
    f = re.search(r'"file":\s*"(\w+)"', s)
    if not f:
        continue
    new = target(fn, f.group(1))
    if new:
        print(f"state: {fn}: {f.group(1)} -> {new}")
        moved[fn] = (f.group(1), new)
        s = s[:f.start()] + s[f.start():f.end()].replace(f.group(1), new) + s[f.end():]
        open(sp, "w", newline="\n").write(s)
# canonical_asm_regions.json
p = "tools/canonical_asm_regions.json"
if os.path.exists(p):
    s = open(p).read()
    for m in list(re.finditer(r'"(\w+)":\s*\{\s*"file":\s*"(\w+)"', s)):
        new = target(m.group(1), m.group(2))
        if new:
            print(f"regions: {m.group(1)}: {m.group(2)} -> {new}")
            moved[m.group(1)] = (m.group(2), new)
    for func, (old, new) in moved.items():
        s = re.sub(r'("%s":\s*\{\s*"file":\s*)"%s"' % (re.escape(func), re.escape(old)), r'\1"%s"' % new, s)
    open(p, "w", newline="\n").write(s)
# scope_allow.txt
p = "tools/grinder/scope_allow.txt"
if os.path.exists(p):
    out, ch = [], 0
    for line in open(p).read().split("\n"):
        tk = line.split()
        if tk and tk[0] in where:
            for i, x in enumerate(tk[1:], 1):
                m = re.fullmatch(r"src/(\w+)\.c", x)
                if m and m.group(1) in OLD and m.group(1) not in where[tk[0]] and len(where[tk[0]]) == 1:
                    tk[i] = "src/%s.c" % next(iter(where[tk[0]]))
                    ch += 1
            line = " ".join(tk) if ch else line
        out.append(line)
    if ch:
        open(p, "w", newline="\n").write("\n".join(out))
        print(f"scope_allow: {ch} token(s) moved")
print(f"relocated {len(moved)} function record(s)")
