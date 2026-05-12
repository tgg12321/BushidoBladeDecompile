#!/usr/bin/env python3
"""Categorize a function's build-vs-target diff into a single-line
verdict per category. Lets agents pick the right tool in seconds
instead of reading 50+ lines of side-by-side diff output.

Reads the same data as `dc.sh verify <func>` but classifies each
differing instruction by failure mode:

  * `size`            : function length matches or not (in instructions)
  * `opcode-only`     : same operands, different mnemonic (e.g., lhu vs lbu)
                        -> typically fixable with a regfix `subst` rule
  * `register-rename` : same opcode + immediate, registers permuted
                        -> typically fixable with a regfix `swap` rule
  * `immediate`       : same opcode + registers, different immediate/offset
                        -> typically gp-rel offset drift or a constant mismatch
  * `branch-offset`   : branch instruction with target offset off by N
                        -> cascade or label-shift
  * `structural`     : different opcode AND different register field
                        -> usually a scheduling/reorder or missing instruction
  * `cascade`         : large run of consecutive diffs after a length mismatch
                        -> probably resolves when the length mismatch closes

Outputs a compact verdict + one suggested next action per category.

Usage:
    python3 tools/diff_summary.py <func>
    python3 tools/diff_summary.py <func> --json   # machine-readable
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# Parse a line from `regfix_verify.py` output. Example:
#   0x8006EC10 (+  4) [TEXT idx 1]:
#       ours:   0x978304AC  lhu $v1,1196($gp)
#       target: 0x938304AC  lbu $v1,1196($gp)
ADDR_RE = re.compile(r"^\s+0x([0-9A-Fa-f]+)\s+\(\+\s*(\d+)\)\s+\[TEXT idx (\d+)\]:")
OURS_RE = re.compile(r"^\s+ours:\s+0x([0-9A-Fa-f]+)\s+(.+)$")
TGT_RE = re.compile(r"^\s+target:\s+0x([0-9A-Fa-f]+)\s+(.+)$")

# Disassembly format from objdump: "lhu $v1,1196($gp)" — split into
# (mnemonic, operands).
DISASM_SPLIT = re.compile(r"^(\S+)\s*(.*)$")
REG_RE = re.compile(r"\$(?:\w+)")
IMM_RE = re.compile(r"-?\d+|-?0x[0-9A-Fa-f]+")
BRANCH_OPS = {"beq", "bne", "beqz", "bnez", "bltz", "bgtz", "blez", "bgez", "j", "jal", "b"}


def classify_diff(ours: str, tgt: str) -> str:
    """Return a category label for the pair of differing instructions."""
    m_o = DISASM_SPLIT.match(ours.strip())
    m_t = DISASM_SPLIT.match(tgt.strip())
    if not m_o or not m_t:
        return "structural"
    mnemonic_o, operands_o = m_o.groups()
    mnemonic_t, operands_t = m_t.groups()
    op_diff = mnemonic_o != mnemonic_t
    operands_diff = operands_o != operands_t

    # Strip registers & immediates separately to detect what differs.
    regs_o = REG_RE.findall(operands_o)
    regs_t = REG_RE.findall(operands_t)
    imms_o = IMM_RE.findall(operands_o)
    imms_t = IMM_RE.findall(operands_t)

    regs_diff = regs_o != regs_t
    imms_diff = imms_o != imms_t

    if op_diff and not operands_diff:
        return "opcode-only"
    if op_diff and operands_diff:
        # Both differ — probably structural
        return "structural"
    # Same opcode
    if mnemonic_o in BRANCH_OPS:
        # Branch offsets differ — usually cascade or label shift
        return "branch-offset"
    if regs_diff and not imms_diff:
        return "register-rename"
    if imms_diff and not regs_diff:
        return "immediate"
    return "structural"


def parse_verify_output(text: str) -> dict:
    """Parse `regfix_verify.py` text output into a dict.

    Returns:
      {
        "func": str,
        "address": int,
        "size_bytes": int,
        "size_insns": int,
        "match": bool,
        "diff_count": int,
        "diffs": [
          {"addr": int, "offset": int, "idx": int,
           "ours_word": int, "ours_dis": str,
           "tgt_word": int, "tgt_dis": str,
           "category": str},
          ...
        ]
      }
    """
    result = {"diffs": [], "match": False, "diff_count": 0}
    lines = text.splitlines()
    # Find header line: "func_X: MATCH (0 diffs in N bytes)" or
    # "func_X: K instruction(s) differ (function at 0xADDR, N bytes)"
    for line in lines:
        m = re.match(r"^(\w+):\s+MATCH\s+\(0 diffs in (\d+) bytes\)", line)
        if m:
            result["func"] = m.group(1)
            result["size_bytes"] = int(m.group(2))
            result["size_insns"] = int(m.group(2)) // 4
            result["match"] = True
            return result
        m = re.match(
            r"^(\w+):\s+(\d+) instruction\(s\) differ "
            r"\(function at 0x([0-9A-Fa-f]+),\s+(\d+) bytes\)",
            line,
        )
        if m:
            result["func"] = m.group(1)
            result["diff_count"] = int(m.group(2))
            result["address"] = int(m.group(3), 16)
            result["size_bytes"] = int(m.group(4))
            result["size_insns"] = int(m.group(4)) // 4
            break

    # Walk the diff entries.
    i = 0
    while i < len(lines):
        m = ADDR_RE.match(lines[i])
        if not m:
            i += 1
            continue
        addr, offset, idx = int(m.group(1), 16), int(m.group(2)), int(m.group(3))
        ours_word = ours_dis = tgt_word = tgt_dis = None
        for j in range(i + 1, min(i + 4, len(lines))):
            mo = OURS_RE.match(lines[j])
            if mo:
                ours_word = int(mo.group(1), 16)
                ours_dis = mo.group(2)
            mt = TGT_RE.match(lines[j])
            if mt:
                tgt_word = int(mt.group(1), 16)
                tgt_dis = mt.group(2)
        if ours_dis is not None and tgt_dis is not None:
            result["diffs"].append({
                "addr": addr, "offset": offset, "idx": idx,
                "ours_word": ours_word, "ours_dis": ours_dis,
                "tgt_word": tgt_word, "tgt_dis": tgt_dis,
                "category": classify_diff(ours_dis, tgt_dis),
            })
        i += 1
    return result


def summarize(parsed: dict) -> dict:
    """Produce a single-line-per-category summary + next-action hint."""
    if parsed.get("match"):
        return {"verdict": "MATCH", "size_bytes": parsed["size_bytes"],
                "size_insns": parsed["size_insns"], "categories": {}}

    categories: dict[str, list[dict]] = {}
    for d in parsed["diffs"]:
        categories.setdefault(d["category"], []).append(d)

    summary = {
        "verdict": "DIFF",
        "func": parsed.get("func", "?"),
        "size_bytes": parsed.get("size_bytes", 0),
        "size_insns": parsed.get("size_insns", 0),
        "diff_count": parsed.get("diff_count", len(parsed["diffs"])),
        "categories": {},
    }

    suggestions = {
        "opcode-only": "regfix `subst` to rewrite the opcode at this idx",
        "register-rename": "regfix `swap` to flip register names over a range",
        "immediate": "check gp-rel offset, sdata symbol decl, or constant in C source",
        "branch-offset": "check for label drift or cascade from a length mismatch",
        "structural": "C-level restructure or new regfix insert/delete/reorder",
        "cascade": "likely resolves automatically once a single root-cause is fixed",
    }

    for cat, items in categories.items():
        indices = sorted(d["idx"] for d in items)
        # Compress consecutive runs of indices like "3..7, 10..12"
        runs = []
        start = prev = indices[0]
        for idx in indices[1:]:
            if idx == prev + 1:
                prev = idx
                continue
            runs.append((start, prev))
            start = prev = idx
        runs.append((start, prev))
        run_strs = []
        for s, e in runs:
            run_strs.append(f"{s}" if s == e else f"{s}..{e}")
        summary["categories"][cat] = {
            "count": len(items),
            "indices": run_strs,
            "suggestion": suggestions.get(cat, "investigate"),
            "samples": [
                {"idx": items[0]["idx"], "ours": items[0]["ours_dis"],
                 "target": items[0]["tgt_dis"]}
            ],
        }
    return summary


def print_summary(s: dict) -> None:
    if s["verdict"] == "MATCH":
        print(f"MATCH ({s['size_insns']} insns, {s['size_bytes']} bytes)")
        return
    print(f"DIFF in {s['func']}: {s['diff_count']} diff(s) "
          f"across {s['size_insns']} insns ({s['size_bytes']} bytes)")
    for cat, info in s["categories"].items():
        idx_str = ",".join(info["indices"])
        print(f"  [{cat}] {info['count']} at idx {idx_str}")
        print(f"     suggest: {info['suggestion']}")
        sample = info["samples"][0]
        print(f"     ex idx {sample['idx']}: ours={sample['ours']!r}")
        print(f"                  target={sample['target']!r}")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func")
    ap.add_argument("--json", action="store_true",
                    help="Emit machine-readable JSON instead of text")
    args = ap.parse_args()

    # Run regfix_verify to get the raw diff text.
    res = subprocess.run(
        [sys.executable, str(ROOT / "tools" / "regfix_verify.py"), args.func],
        capture_output=True, text=True, cwd=str(ROOT),
    )
    parsed = parse_verify_output(res.stdout)
    summary = summarize(parsed)
    if args.json:
        print(json.dumps(summary, indent=2))
        return 0 if summary["verdict"] == "MATCH" else 1
    print_summary(summary)
    return 0 if summary["verdict"] == "MATCH" else 1


if __name__ == "__main__":
    sys.exit(main())
