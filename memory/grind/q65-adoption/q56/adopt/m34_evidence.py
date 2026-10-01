#!/usr/bin/env python3
"""m34_evidence.py <tree>: the merge-test evidence for the Q67 groups M3 / M4, recorded like rodata-align
section 7 (run on the tree at the pre-merge tag, built). Sections:
  1. link order: the members are contiguous in every bb2.ld section list they appear in;
  2. small-data gp reach per member (defs_plan.json of this build): every sdata / static-region / COMMON
     object each member reaches gp, and the objects reached gp from two or more members (the merge test's
     evidence: a K2 or K3 object reached gp from each part);
  3. mergecheck2 on the merged groups: no symbol is reached gp in one part and by a direct lui/%lo load or
     store in another (a contradiction would mean no single definition serves the merged file);
  4. the jump-table phase check (rodata-object-alignment condition 1): every member's compiled tables share
     one phase, so the merge removes no rodata-proven boundary;
  5. the PSYLINK probe results it rests on (tmp/q56/psylink_probe2_results.txt)."""
import json, os, re, subprocess, sys
T = sys.argv[1]
os.chdir(T)
H = os.path.dirname(os.path.abspath(__file__))
Q = os.path.dirname(H)
GROUPS = {"M3": ["text1a_c2", "text1a_b", "text1a_b_pre_rodata", "sound", "text1b"],
          "M4": ["text1b_tu2", "text1b_b"]}
TAIL = "text1a_c_tu2"   # the text1a_c tail: not in M3 (rule: borderline unless the record shows a shared object)


def sh(c):
    return subprocess.run(c, shell=True, capture_output=True, text=True).stdout


out = []
w = out.append
w(f"# M3 / M4 merge evidence (tree {sh('git rev-parse --short HEAD').strip()}, {sh('date -u +%Y-%m-%d').strip()})\n")
ld = open("bb2.ld").read().split("\n")
w("## 1. Link order\n")
for g, ms in GROUPS.items():
    for sec in ("rodata", "text", "data", "bss"):
        idx = [i for i, l in enumerate(ld) if re.match(r"^\s*build/src/(\w+)\.o\(\.%s\);" % sec, l)]
        names = [re.match(r"^\s*build/src/(\w+)\.o", ld[i]).group(1) for i in idx]
        pos = [names.index(m) for m in ms if m in names]
        if not pos:
            continue
        span = names[min(pos):max(pos) + 1]
        ok = all(n in ms for n in span)
        w(f"- {g} .{sec}: {' '.join(span)} -> {'contiguous' if ok else 'NOT contiguous'}")
w("")
subprocess.run([sys.executable, f"{H}/defs_plan.py", T], capture_output=True)
P = json.load(open("/tmp/q56/defs_plan.json"))
w("## 2. Small-data objects reached gp, per member\n")
for g, ms in list(GROUPS.items()) + [("text1a_c tail", [TAIL])]:
    reach = {}
    for m in ms:
        for nm, u in P.get(m, {}).items():
            if u["gp"]:
                reach.setdefault((u["addr"], nm, u["region"]), set()).add(m)
    w(f"### {g}\n")
    for m in ms:
        objs = sorted((a, nm, r) for (a, nm, r), fs in reach.items() if m in fs)
        w(f"- {m}: " + (", ".join(f"{nm} ({r})" for a, nm, r in objs) or "none"))
    shared = sorted((a, nm, r, sorted(fs)) for (a, nm, r), fs in reach.items() if len(fs) > 1)
    w(f"- reached gp from two or more members: " +
      (", ".join(f"{nm} 0x{a:08X} ({r}) by {'+'.join(fs)}" for a, nm, r, fs in shared) or "none"))
    w("")
# objects the tail shares with M3 members
tail = {(u["addr"]) for nm, u in P.get(TAIL, {}).items() if u["gp"] and u["region"] != "common"}
m3 = {(u["addr"]) for m in GROUPS["M3"] for nm, u in P.get(m, {}).items() if u["gp"] and u["region"] != "common"}
w(f"text1a_c tail and M3 share {len(tail & m3)} non-COMMON gp object(s): {', '.join(hex(a) for a in sorted(tail & m3)) or 'none'}\n")
w("## 3. mergecheck2 on the merged groups\n")
src = open(f"{Q}/mergecheck2.py").read()
src = re.sub(r"FILES = \{.*?\n\}\n", "FILES = " + repr({f"{g}: " + " + ".join(ms): [m for m in ms if os.path.exists(f'build/src/{m}.o')]
                                                     for g, ms in GROUPS.items()}) + "\n", src, flags=re.S)
r = subprocess.run([sys.executable, "-c", src], capture_output=True, text=True)
w("```\n" + (r.stdout + r.stderr).strip() + "\n```\n")
w("## 4. Jump-table phases (rodata-object-alignment condition 1)\n")
r = subprocess.run([sys.executable, f"{H}/phases.py", T] + [m for ms in GROUPS.values() for m in ms],
                   capture_output=True, text=True)
w("```\n" + (r.stdout + r.stderr).strip() + "\n```\n")
w("## 5. PSYLINK probe (Sony PSYLINK 2.37, per-file .lcomm blocks in link order, then .comm)\n")
w("```\n" + open(f"{Q}/psylink_probe2_results.txt").read().strip() + "\n```\n")
open(f"{H}/m34_evidence.md", "w", newline="\n").write("\n".join(out) + "\n")
print("wrote m34_evidence.md")
