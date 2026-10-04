#!/usr/bin/env python3
"""worklist.py OUTDIR : join tmp/p2/targets.tsv (refs.py) and tmp/p2/proto_trial.tsv (proto_trial.py) into
OUTDIR/decl_worklist.tsv, one row per function symbol:
  class symbol target target_src sotn def defcompat callers_same callers_diff callers_err
  arg_warnings verdict detail
verdict: ready (every caller compiles to the same asm under the target, no call-site warnings, the
definition is compatible or absent) | casts (same asm, but call sites need casts/typed args: the
warnings) | codegen (some caller's asm changes) | blocked (a caller fails to compile: arity, void
value, missing type) | def-retype (callers fine, the C definition's own spelling must change)."""
import collections, re, sys

out = sys.argv[1]
T = {}
for l in open("tmp/p2/targets.tsv", encoding="utf-8"):
    if l.startswith("#"):
        continue
    r = l.rstrip("\n").split("\t")
    T[(r[0], r[1])] = r
tr = collections.defaultdict(list)
for l in open("tmp/p2/proto_trial.tsv", encoding="utf-8"):
    if l.startswith("#"):
        continue
    r = l.rstrip("\n").split("\t")
    tr[(r[0], r[1])].append(r)
rows = []
for k, t in sorted(T.items()):
    cls, sym = k
    rs = tr.get(k, [])
    same = [r for r in rs if r[3] != "def" and r[4] == "same"]
    diff = [r for r in rs if r[3] != "def" and r[4].startswith("asm-diff")]
    err = [r for r in rs if r[3] != "def" and not (r[4] == "same" or r[4].startswith("asm-diff"))]
    dc = [r[4] for r in rs if r[3] == "def"]
    nwarn = 0
    for r in rs:
        if r[6]:
            parts = r[6].split("; ")
            m = re.match(r"\+(\d+) more$", parts[-1])
            nwarn += len(parts) - 1 + int(m.group(1)) if m else len(parts)
    if not t[6]:
        verdict, detail = "no-reference", "no SOTN/psyz/header spelling and no C definition"
    elif err:
        verdict = "blocked"
        detail = "; ".join(f"{r[2]}: {r[4]}" for r in err)
    elif diff:
        verdict = "codegen"
        detail = "; ".join(f"{r[2]}: {r[4]} in {r[5]}" for r in diff)
    elif any(d.startswith("defcompat no") or "type-missing" in d for d in dc):
        verdict = "def-retype"
        detail = "; ".join(dc)
    elif nwarn:
        verdict, detail = "casts", f"{nwarn} call-site warnings"
    else:
        verdict, detail = "ready", ""
    if verdict in ("blocked", "codegen") and any(d != "defcompat yes" for d in dc):
        detail += " | def: " + "; ".join(dc)
    rows.append([cls, sym, t[6], t[7], t[4], t[2], ";".join(dc) or "-",
                 ",".join(f"{r[2].replace('main/', '')}:{r[3]}" for r in same) or "-",
                 ",".join(f"{r[2].replace('main/', '')}:{r[3]}" for r in diff) or "-",
                 ",".join(f"{r[2].replace('main/', '')}:{r[3]}" for r in err) or "-",
                 str(nwarn), verdict, detail])
with open(f"{out}/decl_worklist.tsv", "w", encoding="utf-8", newline="\n") as fh:
    fh.write("# class\tsymbol\ttarget\ttarget_src\tsotn\tdef\tdefcompat\tcallers_same\tcallers_diff\tcallers_err"
             "\targ_warnings\tverdict\tdetail   (memory/grind/phase2-2026-10-03/tools/worklist.py)\n")
    for r in rows:
        fh.write("\t".join(x.replace("\t", " ") for x in r) + "\n")
print(collections.Counter((r[0], r[11]) for r in rows))
