#!/usr/bin/env python3
"""mergec.py <out.c> <part.c>... : merge C translation units that the evidence shows were ONE original
file, in link order, into <out.c> (Q65 adoption, step 2). Moves only:
  - the merged file opens with the union of the parts' include blocks (INCLUDE_ASM_USE_MACRO_INC first
    when any part defines it; macro.inc is a superset of labels.inc);
  - each part follows VERBATIM (comments kept), except that a file-scope declaration statement
    (extern / prototype / typedef / struct-union-enum definition / #define) whose normalized text is
    IDENTICAL to one already emitted is dropped (a duplicate re-declaration; a typedef or struct
    repeated verbatim would be a compile error);
  - a declaration that declares an already-declared name with DIFFERENT text is reported (conflict),
    and kept; the compile check decides.
Parts other than <out.c> are deleted by the caller. Uses splitc.py's statement parser."""
import re, sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from splitc_lib import statements, declared, blank

out, parts = sys.argv[1], sys.argv[2:]
MACRO = "#define INCLUDE_ASM_USE_MACRO_INC 1"
incs, macro = [], False
bodies = []
for p in parts:
    raw = open(p).read()
    lines = raw.split("\n")
    n = 0
    while n < len(lines) and (lines[n].startswith("#include") or lines[n].startswith("#define INCLUDE_ASM_USE_MACRO_INC")
                              or (not lines[n].strip() and n + 1 < len(lines) and lines[n + 1].startswith("#"))):
        if lines[n].startswith("#include") and lines[n] not in incs:
            incs.append(lines[n])
        if lines[n].startswith("#define INCLUDE_ASM_USE_MACRO_INC"):
            macro = True
        n += 1
    bodies.append((p, raw, lines, n))

emitted = {}      # normalized declaration text -> first part
declnames = {}    # name -> normalized text
report = []
dropped = []   # (part, line, text) of each verbatim repeat dropped
chunks = []
for p, raw, lines, n in bodies:
    sts = statements(raw, n, len(lines))
    drop = set()
    for st in sts:
        t = st["text"]
        is_decl = st["kind"] == "pp" and re.match(r"#\s*define\b", t) or (
            st["kind"] == "stmt" and (re.match(r"(extern|typedef)\b", t) or re.match(r"(struct|union|enum)\s+\w+\s*\{", t)
                                      or (re.search(r"\)\s*;$", t) and "=" not in t and "{" not in t)))
        if not is_decl:
            continue
        # identity from the RAW lines: the parser blanks string literals, so two __asm__("glabel ...")
        # blocks would otherwise look identical
        norm = " ".join("\n".join(lines[st["a"]:st["b"] + 1]).split())
        if norm in emitted:
            drop.update(range(st["a"], st["b"] + 1))
            dropped.append((os.path.basename(p), st["a"] + 1, norm))
            continue
        emitted[norm] = p
        for nm in declared(st, set()):
            if nm in declnames and declnames[nm] != norm and not re.match(r"(extern\b|[^;{]*\)\s*;$)", norm):
                report.append(f"CONFLICT {nm}: {declnames[nm][:90]} | {norm[:90]} ({p})")
            declnames.setdefault(nm, norm)
    body = [l for i, l in enumerate(lines[n:], start=n) if i not in drop]
    chunks.append(f"\n/* ---- merged from {os.path.basename(p)} (owner ruling Q65: one original file) ---- */\n"
                  + "\n".join(body).strip("\n") + "\n")
head = ([MACRO] if macro else []) + incs
open(out, "w", newline="\n").write("\n".join(head) + "\n" + "".join(chunks))
print(f"merged {len(parts)} parts into {out}; conflicts: {len(report)}")
for r in report:
    print("  " + r)
for part, ln, norm in dropped:
    print(f"  dropped verbatim repeat ({part}:{ln}): {norm}")
