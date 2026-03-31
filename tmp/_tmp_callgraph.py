#!/usr/bin/env python3
"""
Call graph propagation: find BB2 stubs that are called ONLY by named
(semantic) functions. A stub called exclusively from a named parent can
be cross-referenced against Kengo's call graph for that parent function.

Output: for each named BB2 function, list the func_XXXXXXXX stubs it
calls, grouped with the Kengo parent's callee list for comparison.
"""
import re, glob, os, sys

ROOT = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"

# ── Load named functions (semantic names currently in INCLUDE_ASM) ──────────
# A "named stub" has a semantic name (not func_XXXXXXXX) in INCLUDE_ASM
FUNC_RE = re.compile(r"func_[0-9A-Fa-f]{8}")
INCLUDE_RE = re.compile(r'INCLUDE_ASM\s*\(\s*"[^"]+"\s*,\s*(\w+)\s*\)')

named_stubs = set()   # semantic names still as INCLUDE_ASM stubs
unnamed_stubs = set() # func_XXXXXXXX stubs still as INCLUDE_ASM stubs

for c_file in sorted(glob.glob(os.path.join(ROOT, "src", "*.c"))):
    txt = open(c_file, encoding="utf-8", errors="replace").read()
    for m in INCLUDE_RE.finditer(txt):
        name = m.group(1)
        if FUNC_RE.match(name):
            unnamed_stubs.add(name)
        else:
            named_stubs.add(name)

print(f"Named stubs:   {len(named_stubs)}")
print(f"Unnamed stubs: {len(unnamed_stubs)}")

# ── Load Kengo function body lines for named functions ───────────────────────
# kengo_functions_full.txt format:
#   /* addr size */ funcname() { ... }
# (empty bodies — no actual call graph in this file)
# We'll parse the Kengo debug symbol file for size info only.

# ── Build BB2 call graph from ASM files ────────────────────────────────────
# Each named stub has a .s file in asm/funcs/<name>.s
# JAL instructions within that file reveal what it calls.

JAL_RE = re.compile(r"jal\s+(\w+)")

callers_of = {}  # callee -> set of callers
calls_of = {}    # caller -> set of callees

funcs_dir = os.path.join(ROOT, "asm", "funcs")

all_stubs = named_stubs | unnamed_stubs

for name in sorted(named_stubs):
    s_file = os.path.join(funcs_dir, f"{name}.s")
    if not os.path.exists(s_file):
        continue
    txt = open(s_file, encoding="utf-8", errors="replace").read()
    callees = set()
    for m in JAL_RE.finditer(txt):
        callee = m.group(1)
        if callee in unnamed_stubs:
            callees.add(callee)
            callers_of.setdefault(callee, set()).add(name)
    if callees:
        calls_of[name] = callees

# ── Find unnamed stubs called by ONLY ONE named parent ──────────────────────
print("\n=== Unnamed stubs called by exactly ONE named parent ===")
count = 0
for callee, parents in sorted(callers_of.items()):
    if len(parents) == 1:
        parent = next(iter(parents))
        # Get callee size
        s_file = os.path.join(funcs_dir, f"{callee}.s")
        insn_count = 0
        if os.path.exists(s_file):
            insn_re = re.compile(r"^\s*/\* [0-9A-Fa-f]+ [0-9A-Fa-f]+ [0-9A-Fa-f]+ \*/")
            insn_count = sum(1 for line in open(s_file) if insn_re.match(line))
        print(f"  {callee:<32} {insn_count:4d}i  ← only called by {parent}")
        count += 1
print(f"\nTotal: {count} exclusively-owned callees")

# ── Find unnamed stubs called ONLY by named parents ─────────────────────────
print("\n=== Named parents and their exclusive unnamed callees ===")
for parent, callees in sorted(calls_of.items()):
    exclusive = [c for c in callees if len(callers_of.get(c, set())) == 1]
    if exclusive:
        print(f"\n  {parent}:")
        for c in sorted(exclusive):
            s_file = os.path.join(funcs_dir, f"{c}.s")
            insn_count = 0
            if os.path.exists(s_file):
                insn_re = re.compile(r"^\s*/\* [0-9A-Fa-f]+ [0-9A-Fa-f]+ [0-9A-Fa-f]+ \*/")
                insn_count = sum(1 for line in open(s_file) if insn_re.match(line))
            print(f"    {c}  {insn_count}i")
