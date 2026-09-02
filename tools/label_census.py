#!/usr/bin/env python3
"""Pure-text census of branch targets, predecessor sets and callee-saved register
reference counts for ONE function -- on the splat target asm and, optionally, on
cc1 build output.

WHY (post-mortem 2026-09-01, func_800283D0 s27): twenty-six sessions of pass
forensics never asked where `.L80028488` pointed; session s16 mis-read a control-
flow arm as duplicated calls and only a `$s2`/`$s3` reference count refuted it,
five sessions later. Both facts are one read of the asm away. This tool makes
that read cheap, mechanical and diffable between target and build.

Usage:
    python3 tools/label_census.py --func <func> [--target <path>] [--build <path>] [--json]

  --target  defaults to asm/funcs/<func>.s (splat format:
            `glabel f` / `    /* 17E40 80027640 C0FFBD27 */  addiu  $sp, ...`
            with delay slots marked by an extra leading space).
  --build   cc1 dump, e.g. tmp/grind/<func>/dumps/<stem>.s, produced by
            `pwsh tools/grinder/dump.ps1 <func>` (`\\t.ent\\tf` ... `$L3:` ...
            `\\t.end\\tf`, NUMERIC registers). Numeric registers are normalised
            to ABI names so both sides compare directly.

Pure text: this NEVER builds, never invokes the toolchain, and never writes.
"""

import argparse
import json
import os
import re
import sys

# --- numeric -> ABI register names -------------------------------------------
ABI = [
    "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
    "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
    "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra",
]

CALLEE_SAVED = ["$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7"]

LABEL = re.compile(r"^\s*(\.L[0-9A-Fa-f]+|\$L\d+|[A-Za-z_]\w*):\s*$")
TARGET_INSN = re.compile(
    r"^\s*/\*\s*[0-9A-Fa-f]+\s+[0-9A-Fa-f]{8}\s+[0-9A-Fa-f]{8}\s*\*/\s+(\S+)\s*(.*)$"
)
BUILD_INSN = re.compile(r"^\s+([a-z][\w.]*)\s*(.*)$")

_NUMREG = re.compile(r"\$(\d+)\b")
_SREG = re.compile(r"\$(s[0-7])\b")

BRANCHES = {
    "b", "j", "beq", "bne", "beqz", "bnez", "blez", "bgtz", "bltz", "bgez",
    "bgezal", "bltzal", "beql", "bnel",
}
UNCOND = {"b", "j", "jr"}


def _norm_regs(args):
    """$16 -> $s0, $4 -> $a0, $0 -> $zero ..."""
    def sub(m):
        n = int(m.group(1))
        return ABI[n] if 0 <= n < 32 else m.group(0)
    return _NUMREG.sub(sub, args)


def _span(lines, func):
    """Index range [start, end) of the body of `func` within `lines`."""
    if not func:
        return 0, len(lines)
    start = None
    for i, ln in enumerate(lines):
        s = ln.strip()
        if s == "glabel " + func or re.match(r"^\.ent\s+" + re.escape(func) + r"\s*$", s):
            start = i + 1
            break
    if start is None:
        return 0, len(lines)
    for j in range(start, len(lines)):
        s = lines[j].strip()
        if s.startswith("glabel ") or re.match(r"^\.end\s+" + re.escape(func) + r"\s*$", s):
            return start, j
    return start, len(lines)


def parse_asm(text, func=None):
    """-> (instructions, {label: index_of_next_instruction}).

    Each instruction is {"op", "args", "raw"}; registers normalised to ABI names.
    """
    lines = text.splitlines()
    lo, hi = _span(lines, func)
    ins = []
    labels = {}
    for raw in lines[lo:hi]:
        m = LABEL.match(raw)
        if m:
            name = m.group(1)
            # the entry label of the function itself is not a branch-target label
            if func and name == func:
                continue
            labels[name] = len(ins)
            continue
        m = TARGET_INSN.match(raw)
        if m:
            ins.append({"op": m.group(1), "args": _norm_regs(m.group(2).strip()), "raw": raw})
            continue
        m = BUILD_INSN.match(raw)
        if m and not m.group(1).startswith("."):
            ins.append({"op": m.group(1), "args": _norm_regs(m.group(2).strip()), "raw": raw})
    return ins, labels


def census(text, func=None):
    """-> {"n_insns", "labels": {L: {"at","preds","n_preds"}}, "reg_refs": {$s0..$s7}}"""
    ins, labels = parse_asm(text, func)
    preds = {name: [] for name in labels}

    # explicit branch/jump edges
    for idx, i in enumerate(ins):
        if i["op"] not in BRANCHES:
            continue
        ops = [a.strip() for a in i["args"].split(",") if a.strip()]
        if ops and ops[-1] in labels:
            preds[ops[-1]].append("%s@%d" % (i["op"], idx))

    # fall-through edges
    for name, at in labels.items():
        if at == 0:
            continue
        prev = ins[at - 1]
        prev2 = ins[at - 2] if at >= 2 else None
        if prev["op"] in UNCOND:
            continue
        # splat/target format shows delay slots explicitly: the insn just before
        # the label may be the delay slot of an unconditional transfer.
        if TARGET_INSN.match(prev["raw"]) and prev2 is not None and prev2["op"] in UNCOND:
            continue
        preds[name].append("fallthrough@%d" % (at - 1))

    reg_refs = {r: 0 for r in CALLEE_SAVED}
    for i in ins:
        for m in _SREG.finditer(i["args"]):
            reg_refs["$" + m.group(1)] += 1

    return {
        "n_insns": len(ins),
        "labels": {n: {"at": labels[n], "preds": preds[n], "n_preds": len(preds[n])}
                   for n in labels},
        "reg_refs": reg_refs,
    }


def _print(side, c):
    print("=== %s ===" % side)
    print("  insns: %d   labels: %d" % (c["n_insns"], len(c["labels"])))
    for name, d in sorted(c["labels"].items(), key=lambda kv: kv[1]["at"]):
        print("    %-16s @%-4d preds=%d  [%s]"
              % (name, d["at"], d["n_preds"], ", ".join(sorted(d["preds"])) or "-"))
    print("  callee-saved refs: "
          + "  ".join("%s=%d" % (r, c["reg_refs"][r]) for r in CALLEE_SAVED))


def _diff(t, b):
    print("=== DIFF (target vs build) ===")
    if len(t["labels"]) != len(b["labels"]):
        print("  label count: target=%d build=%d" % (len(t["labels"]), len(b["labels"])))
    else:
        print("  label count: %d (same)" % len(t["labels"]))
    tp = sorted(d["n_preds"] for d in t["labels"].values())
    bp = sorted(d["n_preds"] for d in b["labels"].values())
    print("  pred-count multiset: target=%s build=%s%s"
          % (tp, bp, "" if tp == bp else "   <-- DIFFERS"))
    diffs = [(r, t["reg_refs"][r], b["reg_refs"][r])
             for r in CALLEE_SAVED if t["reg_refs"][r] != b["reg_refs"][r]]
    if diffs:
        for r, a, c in diffs:
            print("  %s refs: target=%d build=%d" % (r, a, c))
    else:
        print("  callee-saved refs: identical")
    if t["n_insns"] != b["n_insns"]:
        print("  insn count: target=%d build=%d" % (t["n_insns"], b["n_insns"]))


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--func", required=True)
    ap.add_argument("--target", help="default: asm/funcs/<func>.s")
    ap.add_argument("--build", help="cc1 dump .s (tmp/grind/<func>/dumps/<stem>.s)")
    ap.add_argument("--json", action="store_true")
    a = ap.parse_args(argv)

    tpath = a.target or os.path.join("asm", "funcs", a.func + ".s")
    if not os.path.isfile(tpath):
        sys.stderr.write("error: target asm not found: %s\n" % tpath)
        return 2
    with open(tpath, "r", encoding="utf-8", errors="replace") as f:
        tc = census(f.read(), a.func)

    bc = None
    if a.build:
        if not os.path.isfile(a.build):
            sys.stderr.write("error: build asm not found: %s\n" % a.build)
            return 2
        with open(a.build, "r", encoding="utf-8", errors="replace") as f:
            bc = census(f.read(), a.func)
        if not bc["n_insns"]:
            sys.stderr.write("warning: %s not present in %s (INCLUDE_ASM?)\n"
                             % (a.func, a.build))

    if a.json:
        print(json.dumps({"func": a.func, "target": tc, "build": bc}, indent=2))
        return 0

    _print("TARGET %s" % tpath, tc)
    if bc is not None:
        print()
        _print("BUILD %s" % a.build, bc)
        print()
        _diff(tc, bc)
    return 0


if __name__ == "__main__":
    sys.exit(main())
