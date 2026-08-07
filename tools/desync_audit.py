#!/usr/bin/env python3
"""Do the name-keyed pipeline surfaces still agree with the glabels? (read-only)

A function name in this tree is a KEY, not a label. A rename that moves one surface but
not another can still LINK and still hit the oracle SHA1 while quietly dropping a regfix
rule or desyncing the worklist — this failure mode is silent by construction, which is
why it needs its own gate. Run it as a standing post-wave check alongside the
SHA1 / `engine test` / residual battery:

    python3 tools/desync_audit.py           # exit 0 = no NEW desync

Four sections, two of which are hard gates:

  [1] INCLUDE_ASM targets with no matching glabel AND .s file  -> HARD, must be 0.
      This is a build break: INCLUDE_ASM expands to a literal `.include`.
  [2] .s filename != its own glabel                            -> DEBT, baselined.
  [3] name-keyed rule/gate entries naming no live function     -> DEBT, baselined.
  [4] engine/queue.json entries naming no live function        -> HARD, must be 0.
      Not a build break, but a desynced worklist.

Sections [2] and [3] carry PRE-EXISTING debt that no wave introduced, so failing on any
non-zero count would make the gate useless. They are compared against a recorded
baseline instead: the gate fails only when a run makes them WORSE.

Baseline recorded 2026-08-07 at c733c1a6, before the libscan naming wave:
  [2] 33 — stale split artifacts (`func_8003F1C8.s` holding `glabel game_GetMode`, the
           `cdrom_*` cluster, the two duplicate-glabel pairs docs/naming/README.md
           already documents).
  [3]  3 — `bios_FileReadRaw`, `func_8007F2DC`, `setjmp` in inline_asm_canonical.txt.
           `setjmp` is keyed to a name nothing defines yet; the libscan wave FIXES it by
           renaming func_80083220 to setjmp, so expect [3] to drop to 2 afterwards.
Re-baseline deliberately, with the reason, never to make a red run go green.
"""
import glob
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)

BASELINE_MISMATCH = 33
BASELINE_DANGLING = 3

glabels = set()
for f in glob.glob("asm/funcs/*.s"):
    for ln in open(f, encoding="utf-8", errors="replace"):
        m = re.match(r"^glabel\s+(\S+)", ln)
        if m:
            glabels.add(m.group(1))
            break
print("glabels:", len(glabels))

# C-level definitions / INCLUDE_ASM targets. The macro's OWN definition line matches the
# call pattern and yields its parameter name `NAME`, so `#define` lines are dropped first.
csrc = "".join(open(f, encoding="utf-8", errors="replace").read()
               for f in sorted(glob.glob("src/*.c")) + sorted(glob.glob("include/*.h")))
csrc_calls = "\n".join(ln for ln in csrc.split("\n") if not ln.lstrip().startswith("#define"))
incasm = set(re.findall(r"INCLUDE_ASM\([^,]+,\s*([A-Za-z_]\w*)\s*\)", csrc_calls))

# [1] an INCLUDE_ASM target needs BOTH a glabel and a file of that exact name
bad_inc = sorted(n for n in incasm
                 if n not in glabels or not os.path.exists("asm/funcs/%s.s" % n))
print("\n[1] INCLUDE_ASM targets with no matching glabel/.s file:", len(bad_inc))
for n in bad_inc[:20]:
    print("   ", n)

# [2] glabel must match its own filename (a rename that moved one but not the other)
mismatch = []
for f in sorted(glob.glob("asm/funcs/*.s")):
    stem = os.path.basename(f)[:-2]
    for ln in open(f, encoding="utf-8", errors="replace"):
        m = re.match(r"^glabel\s+(\S+)", ln)
        if m:
            if m.group(1) != stem:
                mismatch.append((stem, m.group(1)))
            break
print("\n[2] .s filename != its glabel: %d  (baseline %d)" % (len(mismatch), BASELINE_MISMATCH))
for a, b in mismatch[:20]:
    print("    %s.s -> glabel %s" % (a, b))

# [3] name-keyed rule/gate files referencing a function name that no longer exists
KEYED = {
    "regfix.txt": r"^\s*([A-Za-z_]\w*)\s*:",
    "asmfix.txt": r"^\s*([A-Za-z_]\w*)\s*:",
    "regfix_stage2.txt": r"^\s*([A-Za-z_]\w*)\s*:",
    "inline_asm_canonical.txt": r"^\s*([A-Za-z_]\w*)\b",
    "maspsx_label_nop_funcs.txt": r"^\s*([A-Za-z_]\w*)\s*$",
    "expand_lb_funcs.txt": r"^\s*([A-Za-z_]\w*)\s*$",
    "expand_dest_funcs.txt": r"^\s*([A-Za-z_]\w*)\s*$",
    "multu_funcs.txt": r"^\s*([A-Za-z_]\w*)\s*$",
    "multu_pad_funcs.txt": r"^\s*([A-Za-z_]\w*)\s*$",
    "sdata_funcs.txt": r"^\s*([A-Za-z_]\w*)\s*$",
    "sdata_exclude.txt": r"^\s*([A-Za-z_]\w*)\s*$",
}
known = glabels | set(re.findall(r"^\s*([A-Za-z_]\w*)\s*\(", csrc, re.M)) | incasm
known |= set(re.findall(r"\b([A-Za-z_]\w*)\s*\([^;{]*\)\s*\{", csrc))
print("\n[3] name-keyed entries with no glabel and no C definition:")
dangling_total = 0
for fn, pat in KEYED.items():
    if not os.path.exists(fn):
        continue
    dangling = []
    for ln in open(fn, encoding="utf-8", errors="replace"):
        s = ln.strip()
        if not s or s.startswith("#"):
            continue
        m = re.match(pat, ln)
        if not m:
            continue
        nm = m.group(1)
        if nm in ("glabel", "endlabel"):
            continue
        if nm not in known:
            dangling.append(nm)
    if dangling:
        dangling_total += len(dangling)
        print("    %s: %d -> %s" % (fn, len(dangling), sorted(set(dangling))[:8]))
print("    TOTAL dangling: %d  (baseline %d)" % (dangling_total, BASELINE_DANGLING))

# [4] queue.json entries
q = json.load(open("engine/queue.json", encoding="utf-8"))
items = q if isinstance(q, list) else q.get("items", q.get("functions", []))
qbad = sorted({it.get("func") or it.get("name") for it in items
               if isinstance(it, dict) and (it.get("func") or it.get("name")) not in known}
              - {None})
print("\n[4] engine/queue.json entries naming an unknown function:", len(qbad), qbad[:10])

fails = []
if bad_inc:
    fails.append("[1] %d INCLUDE_ASM target(s) with no .s file — this is a BUILD BREAK" % len(bad_inc))
if qbad:
    fails.append("[4] %d queue entr(ies) naming no live function — desynced worklist" % len(qbad))
if len(mismatch) > BASELINE_MISMATCH:
    fails.append("[2] filename/glabel mismatches rose %d -> %d"
                 % (BASELINE_MISMATCH, len(mismatch)))
if dangling_total > BASELINE_DANGLING:
    fails.append("[3] dangling name-keyed entries rose %d -> %d"
                 % (BASELINE_DANGLING, dangling_total))

print()
if fails:
    print("DESYNC AUDIT: FAIL")
    for f in fails:
        print("  X", f)
    sys.exit(1)
print("DESYNC AUDIT: PASS — no new desync (sections [2]/[3] at or below baseline)")
