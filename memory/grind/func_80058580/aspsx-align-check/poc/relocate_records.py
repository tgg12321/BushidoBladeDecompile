"""Relocate records that name a moved function's source file to its new TU part: the queue item's
"file" field, the grind state.json "file" field, and the function's own src path token on its
grinder scope_allow.txt line. Nothing else (line-numbered citations, historical measurements and
verdict text stay as written). Needs tmp/moved.json: {func: new_stem}."""
import json, os
R = "/home/user/BushidoBladeDecompile/"
moved = json.load(open(R + "tmp/moved.json"))
parent = lambda stem: stem.rsplit("_tu", 1)[0]
p = R + "engine/queue.json"
q = json.load(open(p))
for it in q["items"]:
    if it["func"] in moved and it.get("file") == parent(moved[it["func"]]):
        it["file"] = moved[it["func"]]
open(p, "w", newline="\n").write(json.dumps(q, indent=2, ensure_ascii=True) + "\n")
for f, stem in moved.items():
    sp = R + "memory/grind/%s/state.json" % f
    if os.path.exists(sp):
        s = open(sp).read()
        open(sp, "w", newline="\n").write(s.replace('"file": "%s"' % parent(stem), '"file": "%s"' % stem, 1))
p = R + "tools/grinder/scope_allow.txt"
out = []
for line in open(p).read().split("\n"):
    t = line.split()
    if t and t[0] in moved:
        line = " ".join([t[0]] + ["src/%s.c" % moved[t[0]] if x == "src/%s.c" % parent(moved[t[0]]) else x for x in t[1:]])
    out.append(line)
open(p, "w", newline="\n").write("\n".join(out))
