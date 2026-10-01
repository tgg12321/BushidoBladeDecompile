#!/usr/bin/env python3
"""model_setup.py — Q56 (A) proof of concept, applied to the scratch copy /tmp/q56/model (never main).

The per-file ("declaration-driven") gp model, as Sony ASPSX 2.34 behaves: a file gets gp for a small
(<= 8 byte) symbol only if the file DEFINES it (.comm / .lcomm / .sdata); an extern is never gp.

  1. Makefile: MASPSX_FLAGS / MASPSX_FLAGS_GP lose --sdata-syms, --sdata-funcs, --sdata-exclude and
     gain `-G8` (upstream maspsx's own switch: sdata_limit = 8, gp decided from the file's own
     .comm/.lcomm/.sdata directives) plus the POC stand-in list below.
  2. src/*.c: for every symbol S the oracle object reaches gp-relative (R_MIPS_GPREL16), the file gets
     a definition of S if it has none: `__typeof__(S) S;` (a tentative definition of S's own declared
     type -> `.comm S,size`, which GNU ld resolves to the symbol-file address without allocating,
     owner ruling Q62). Appended at the end of the file.
  3. POC stand-in (tools/maspsx patch, flag --poc-noncomm-syms=FILE): symbols whose original definition
     was NOT a tentative one — initialized (.sdata: address < 0x800A3308) or `static` (.lcomm) — and
     that the oracle reaches gp-relative at an offset. ASPSX gives those gp at every offset; a
     tentative stand-in would not. The real form would be the initialized / static definition in the
     file (a data move); the stand-in only removes such a symbol from maspsx's COMMON set.
Writes /tmp/q56/model_edits.json (every declaration added, per file).
"""
import json, os, re, subprocess
Q = "/tmp/q56"; M = Q + "/model"
A = json.load(open(Q + "/model_analysis.json"))

# --- 1. Makefile
mk = open(M + "/Makefile").read()
for var in ("MASPSX_FLAGS", "MASPSX_FLAGS_GP"):
    m = re.search(r"^%s :=(.*)$" % var, mk, re.M)
    v = m.group(1)
    for f in ("--sdata-syms=sdata_syms.txt", "--sdata-funcs=sdata_funcs.txt", "--sdata-exclude=sdata_exclude.txt"):
        assert f in v
        v = v.replace(" " + f, "")
    v += " -G8 --poc-noncomm-syms=poc_noncomm_syms.txt"
    mk = mk[:m.start(1)] + v + mk[m.end(1):]
open(M + "/Makefile", "w", newline="\n").write(mk)

# --- 3. maspsx stand-in flag
p = M + "/tools/maspsx/maspsx.py"
s = open(p).read()
s = s.replace('    parser.add_argument("--comm-syms",',
              '    parser.add_argument("--poc-noncomm-syms", type=str, default=None)\n'
              '    parser.add_argument("--comm-syms",', 1)
s = s.replace("    try:\n        out_lines = maspsx_processor.process_lines()",
              "    if args.poc_noncomm_syms:\n"
              "        with open(args.poc_noncomm_syms) as f:\n"
              "            maspsx_processor.poc_noncomm = set(l.split()[0] for l in f if l.strip() and not l.startswith('#'))\n"
              "    try:\n        out_lines = maspsx_processor.process_lines()", 1)
open(p, "w", newline="\n").write(s)
p = M + "/tools/maspsx/maspsx/__init__.py"
s = open(p).read()
old = "        return symbol in self.comm_symbols or symbol in self.comm_sym_map.get(self.current_func, ())"
assert old in s
s = s.replace(old, "        if symbol in getattr(self, 'poc_noncomm', ()):\n            return False\n" + old)
open(p, "w", newline="\n").write(s)

# --- 2. declarations
edits = {}
noncomm = []
for tu, rec in sorted(A["objects"].items()):
    if not rec["gp"]:
        continue
    defined = set()
    for ln in subprocess.run(f"mipsel-linux-gnu-nm --defined-only {Q}/refobj/{tu}.o", shell=True,
                             capture_output=True, text=True).stdout.splitlines():
        pp = ln.split()
        if len(pp) == 3:
            defined.add(pp[2])
    add = [S for S in sorted(rec["gp"]) if S not in defined]
    if add:
        with open(f"{M}/src/{tu}.c", "a", newline="\n") as f:
            f.write("\n/* Q56 POC: per-file gp model — this file defines the small data it reaches gp-relative */\n")
            for S in add:
                f.write(f"__typeof__({S}) {S};\n")
    edits[tu] = add
for S, v in A["symbols"].items():
    a = int(v["addr"], 16) if v["addr"] else None
    if v["offset_gp"]:
        noncomm.append((S, "initialized (.sdata)" if a is not None and a < 0x800A3308 else "static (.lcomm)", v["objects"]))
open(M + "/poc_noncomm_syms.txt", "w", newline="\n").write(
    "# Q56 POC stand-in: original definition was initialized/static, not tentative\n" +
    "".join(f"{S}  # {k}; gp at offset in {', '.join(o)}\n" for S, k, o in noncomm))
json.dump({"edits": edits, "noncomm": noncomm}, open(Q + "/model_edits.json", "w"), indent=1)
print("files edited:", sum(1 for v in edits.values() if v), "declarations:", sum(len(v) for v in edits.values()),
      "noncomm stand-ins:", len(noncomm))
