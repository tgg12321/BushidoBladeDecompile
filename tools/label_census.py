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
            `glabel f` / `    /* 17E40 80027640 C0FFBD27 */  addiu  $sp, ...`,
            delay slots marked by ONE EXTRA leading space before the mnemonic,
            plus colon-less `jlabel`/`dlabel` (jump-table targets), `alabel`
            (data address) and `endlabel` directives).
  --build   cc1 dump, e.g. tmp/grind/<func>/dumps/<stem>.s, produced by
            `pwsh tools/grinder/dump.ps1 <func>` (`\\t.ent\\tf` ... `.L7:` ...
            `\\t.end\\tf`, NUMERIC registers, delay slots filled inside
            `.set noreorder` blocks). Numeric registers are normalised to ABI
            names so both sides compare directly.

LIMITATION -- indirect jumps: a `jr $reg` computed from a jump table carries no
textual edge, so its targets show up with zero predecessors. Those labels are
flagged `(jtbl target? ...)`; the project has 112 non-`$ra` `jr` sites, so a
zero-pred label is normal, not evidence of dead code.

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

# `.L8001961C:` (splat) / `.L7:` (cc1) / `$L3:` (other gcc spellings) / `foo:`
LABEL = re.compile(r"^\s*(\.L\w+|\$L\d+|[A-Za-z_]\w*):\s*$")
# .LC0 = rodata string, .Lfe1 = frame-end marker: never branch targets
NOT_A_BRANCH_TARGET = re.compile(r"^\.L(C|fe)\d+$")
# splat directives, no trailing colon
SPLAT_LABEL_DIR = re.compile(r"^\s*(jlabel|alabel|dlabel|endlabel)\s+(\S+)\s*$")
# group 1 = whitespace after the comment (3+ = delay slot), 2 = op, 3 = args
TARGET_INSN = re.compile(
    r"^\s*/\*\s*[0-9A-Fa-f]+\s+[0-9A-Fa-f]{8}\s+[0-9A-Fa-f]{8}\s*\*/(\s+)(\S+)\s*(.*)$"
)
BUILD_INSN = re.compile(r"^\s+([a-z][\w.]*)\s*(.*)$")

_NUMREG = re.compile(r"\$(\d+)\b")
_SREG = re.compile(r"\$(s[0-7])\b")
_PRED = re.compile(r"^(.*)@(\d+)$")

BRANCHES = {
    "b", "j", "beq", "bne", "beqz", "bnez", "blez", "bgtz", "bltz", "bgez",
    "bgezal", "bltzal", "beql", "bnel",
}
UNCOND = {"b", "j", "jr"}


def _norm_regs(args):
    """$16 -> $s0, $4 -> $a0, $0 -> $zero, $30 -> $fp ..."""
    def sub(m):
        n = int(m.group(1))
        return ABI[n] if 0 <= n < 32 else m.group(0)
    return _NUMREG.sub(sub, args)


def _span(lines, func):
    """Index range [start, end) of the body of `func`, or None if absent."""
    if not func:
        return 0, len(lines)
    start = None
    for i, ln in enumerate(lines):
        s = ln.strip()
        if s == "glabel " + func or re.match(r"^\.ent\s+" + re.escape(func) + r"\s*$", s):
            start = i + 1
            break
    if start is None:
        return None
    for j in range(start, len(lines)):
        s = lines[j].strip()
        if s.startswith("glabel ") or re.match(r"^\.end\s+" + re.escape(func) + r"\s*$", s):
            return start, j
    return start, len(lines)


def parse_asm(text, func=None):
    """-> (instructions, {label: index_of_next_instruction}).

    Each instruction is {"op", "args", "raw", "fmt", "in_noreorder",
    "delay_slot"}; registers normalised to ABI names. Returns ([], {}) when
    `func` is not present in `text`.
    """
    lines = text.splitlines()
    span = _span(lines, func)
    if span is None:
        return [], {}
    lo, hi = span
    body = lines[lo:hi]

    # Detect the format ONCE: a splat file must never be fed to BUILD_INSN
    # (its `jlabel`/`endlabel` lines look exactly like build instructions).
    is_target = any(TARGET_INSN.match(ln) for ln in body)

    ins = []
    labels = {}
    noreorder = False
    for raw in body:
        m = SPLAT_LABEL_DIR.match(raw)
        if m:
            kind, name = m.group(1), m.group(2)
            if kind in ("jlabel", "dlabel"):
                labels[name] = len(ins)
            continue                      # alabel = data address; endlabel = end marker
        m = LABEL.match(raw)
        if m:
            name = m.group(1)
            # the entry label of the function itself is not a branch-target label
            if (func and name == func) or NOT_A_BRANCH_TARGET.match(name):
                continue
            labels[name] = len(ins)
            continue
        if is_target:
            m = TARGET_INSN.match(raw)
            if m:
                ins.append({"op": m.group(2), "args": _norm_regs(m.group(3).strip()),
                            "raw": raw, "fmt": "target", "in_noreorder": False,
                            # splat marks a delay slot with one EXTRA space
                            "delay_slot": len(m.group(1)) > 2})
            continue
        s = raw.strip()
        if s.startswith(".set"):
            if "noreorder" in s:
                noreorder = True
            elif "reorder" in s:
                noreorder = False
            continue
        m = BUILD_INSN.match(raw)
        if m:
            ins.append({"op": m.group(1), "args": _norm_regs(m.group(2).strip()),
                        "raw": raw, "fmt": "build", "in_noreorder": noreorder,
                        # cc1 DOES fill delay slots, but only inside noreorder
                        "delay_slot": noreorder})

    # Fallback: if a splat file carries no explicit extra-space markers at all,
    # treat every target instruction as a possible delay slot (old heuristic).
    if is_target and not any(i["delay_slot"] for i in ins):
        for i in ins:
            i["delay_slot"] = True
    return ins, labels


def _sort_preds(preds):
    """(kind, numeric index) -- so beqz@2 sorts before beqz@14."""
    def key(p):
        m = _PRED.match(p)
        return (m.group(1), int(m.group(2))) if m else (p, 0)
    return sorted(preds, key=key)


def census(text, func=None):
    """-> {"n_insns", "labels": {L: {"at","preds","n_preds"}}, "reg_refs", "missing"}"""
    ins, labels = parse_asm(text, func)
    if func and not ins and not labels:
        return {"n_insns": 0, "labels": {},
                "reg_refs": {r: 0 for r in CALLEE_SAVED}, "missing": True}

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
        if at == 0 or at > len(ins):
            continue
        prev = ins[at - 1]
        prev2 = ins[at - 2] if at >= 2 else None
        if prev["op"] in UNCOND:
            continue
        # the insn before the label may itself be the FILLED DELAY SLOT of an
        # unconditional transfer -- splat marks those with an extra space, cc1
        # emits them inside `.set noreorder` blocks.
        if prev["delay_slot"] and prev2 is not None and prev2["op"] in UNCOND:
            continue
        preds[name].append("fallthrough@%d" % (at - 1))

    reg_refs = {r: 0 for r in CALLEE_SAVED}
    for i in ins:
        for m in _SREG.finditer(i["args"]):
            reg_refs["$" + m.group(1)] += 1

    return {
        "n_insns": len(ins),
        "labels": {n: {"at": labels[n], "preds": _sort_preds(preds[n]),
                       "n_preds": len(preds[n])} for n in labels},
        "reg_refs": reg_refs,
        "missing": False,
    }


def _print(side, c):
    print("=== %s ===" % side)
    if c.get("missing"):
        print("  (function not present in this file)")
        return
    print("  insns: %d   labels: %d" % (c["n_insns"], len(c["labels"])))
    for name, d in sorted(c["labels"].items(), key=lambda kv: kv[1]["at"]):
        note = ("  (jtbl target? -- indirect jr $reg edges are not resolvable "
                "from text)") if d["n_preds"] == 0 else ""
        print("    %-16s @%-4d preds=%d  [%s]%s"
              % (name, d["at"], d["n_preds"], ", ".join(d["preds"]) or "-", note))
    print("  callee-saved refs: "
          + "  ".join("%s=%d" % (r, c["reg_refs"][r]) for r in CALLEE_SAVED))


def _diff(t, b):
    print("=== DIFF (target vs build) ===")
    # 1. the ref counts -- what refuted the s16 mis-read
    diffs = [(r, t["reg_refs"][r], b["reg_refs"][r])
             for r in CALLEE_SAVED if t["reg_refs"][r] != b["reg_refs"][r]]
    if diffs:
        for r, a, bb in diffs:
            print("  %s refs: target=%d build=%d   <--" % (r, a, bb))
    else:
        print("  callee-saved refs: identical")

    # 2. predecessor counts in PROGRAM ORDER, zipped
    tl = [d for _, d in sorted(t["labels"].items(), key=lambda kv: kv[1]["at"])]
    bl = [d for _, d in sorted(b["labels"].items(), key=lambda kv: kv[1]["at"])]
    print("  label count: target=%d build=%d%s"
          % (len(tl), len(bl), "" if len(tl) == len(bl) else "   <--"))
    print("  pred counts, program order:")
    for k in range(max(len(tl), len(bl))):
        tv = tl[k]["n_preds"] if k < len(tl) else None
        bv = bl[k]["n_preds"] if k < len(bl) else None
        print("    pos %2d: t=%s b=%s%s"
              % (k, "-" if tv is None else tv, "-" if bv is None else bv,
                 "" if tv == bv else " <--"))
    tp = sorted(d["n_preds"] for d in tl)
    bp = sorted(d["n_preds"] for d in bl)
    print("  pred-count multiset: target=%s build=%s%s"
          % (tp, bp, "" if tp == bp else "   <-- DIFFERS"))
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
    if tc.get("missing"):
        sys.stderr.write("error: %s not found in %s\n" % (a.func, tpath))
        return 2

    bc = None
    if a.build:
        if not os.path.isfile(a.build):
            sys.stderr.write("error: build asm not found: %s\n" % a.build)
            return 2
        with open(a.build, "r", encoding="utf-8", errors="replace") as f:
            bc = census(f.read(), a.func)
        if bc.get("missing"):
            sys.stderr.write("warning: %s not found in %s (INCLUDE_ASM?)\n"
                             % (a.func, a.build))

    if a.json:
        print(json.dumps({"func": a.func, "target": tc, "build": bc}, indent=2))
        return 0

    _print("TARGET %s" % tpath, tc)
    if bc is not None:
        print()
        _print("BUILD %s" % a.build, bc)
        if not bc.get("missing"):
            print()
            _diff(tc, bc)
    return 0


if __name__ == "__main__":
    sys.exit(main())
