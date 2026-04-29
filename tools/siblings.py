#!/usr/bin/env python3
"""Find structural siblings of a function in asm/funcs/.

Many functions in this project come in families with the same shape:
same prologue, same call sequence (different callees), same epilogue.
6 of the 12 matches in the 2026-04-29 session were siblings — porting
a matched function's C body to a sibling typically takes 10-15 minutes
vs 30-60 for a novel function.

Usage:
    python3 tools/siblings.py <func_name>
        Find unmatched functions that are structurally similar to
        <func_name> (which can itself be matched OR unmatched). Output
        is ranked by similarity score, with the reference's fingerprint
        printed first.

    python3 tools/siblings.py --all
        Cluster ALL still-inline-asm functions into similarity groups.
        Useful for finding family patterns.

Fingerprint captures:
  - Frame size (`addiu $sp,$sp,-N`)
  - Set of callee-saved registers (`sw $sN,offset($sp)` in prologue)
  - Sequence of `jal` call targets (function names)
  - Count of jalrs, branches, loads, stores
  - Total instruction count
  - Whether there's a loop (back-edge branch)

Two functions are "siblings" when their fingerprints match closely:
same prologue + same call sequence is the strongest signal.
"""
from __future__ import annotations

import argparse
import re
import sys
from collections import Counter
from pathlib import Path
from typing import Optional

ROOT = Path(__file__).resolve().parent.parent
ASM_FUNCS = ROOT / "asm" / "funcs"
SRC = ROOT / "src"

# Callee-saved registers in MIPS o32: $s0-$s7 ($16-$23), $fp ($30), $ra ($31)
CALLEE_SAVED = {
    "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", "$fp", "$ra",
    "$16", "$17", "$18", "$19", "$20", "$21", "$22", "$23", "$30", "$31",
}

NAMED_TO_NUM = {
    "$s0": "$16", "$s1": "$17", "$s2": "$18", "$s3": "$19",
    "$s4": "$20", "$s5": "$21", "$s6": "$22", "$s7": "$23",
    "$fp": "$30", "$ra": "$31",
}


def normalize_reg(r: str) -> str:
    """Normalize a register name to numeric form."""
    return NAMED_TO_NUM.get(r, r)


def parse_function(asm_path: Path) -> Optional[dict]:
    """Parse a splat asm/funcs/<func>.s file and return a fingerprint."""
    if not asm_path.exists():
        return None
    insns: list[tuple[str, str]] = []  # (mnemonic, operands)
    for line in asm_path.read_text(encoding="utf-8", errors="replace").splitlines():
        # Strip the `/* offset addr hex */` comment, keep the instruction
        m = re.match(r'\s*/\*[^*]*\*/\s*([a-zA-Z][a-zA-Z0-9.]*)\s*(.*?)\s*$', line)
        if not m:
            continue
        mnem = m.group(1)
        ops = m.group(2).strip()
        insns.append((mnem, ops))

    if not insns:
        return None

    # Frame size: first `addiu $sp,$sp,-N` (or subu)
    frame = 0
    for mnem, ops in insns[:6]:
        m = re.match(r'\$sp\s*,\s*\$sp\s*,\s*-?0?x?([\dA-Fa-f]+)', ops)
        if m and mnem in ("addiu", "subu"):
            try:
                frame = int(m.group(1), 16) if any(c in m.group(1) for c in "abcdefABCDEF") \
                    else int(m.group(1))
            except ValueError:
                frame = 0
            break

    # Callee-saved registers: `sw $sX, N($sp)` or `sw $ra, ...`
    saves = set()
    for mnem, ops in insns[:20]:
        if mnem != "sw":
            continue
        m = re.match(r'(\$\w+)\s*,\s*0?x?[\dA-Fa-f]+\s*\(\s*\$sp\s*\)', ops)
        if m:
            r = normalize_reg(m.group(1))
            if r in CALLEE_SAVED:
                saves.add(r)

    # JAL targets and jalr count
    jals: list[str] = []
    n_jalr = 0
    for mnem, ops in insns:
        if mnem == "jal":
            target = ops.strip()
            jals.append(target)
        elif mnem == "jalr":
            n_jalr += 1

    # Count loads, stores, branches; detect loops (backward branches)
    n_loads = 0
    n_stores = 0
    n_branches = 0
    has_loop = False
    branch_targets: list[str] = []
    for mnem, ops in insns:
        if mnem in ("lw", "lh", "lhu", "lb", "lbu"):
            n_loads += 1
        elif mnem in ("sw", "sh", "sb"):
            n_stores += 1
        elif mnem.startswith("b") and mnem != "break":
            n_branches += 1
            # Extract branch target label
            parts = ops.rsplit(",", 1)
            if len(parts) >= 1:
                tgt = parts[-1].strip()
                if tgt.startswith(".L"):
                    branch_targets.append(tgt)

    # Loop detection: a branch target that appears as a label EARLIER in the
    # function body indicates a back-edge.
    label_positions: dict[str, int] = {}
    for i, (mnem, ops) in enumerate(insns):
        # Won't catch labels-as-instruction-comments, but glabel parsing covers
        # the basic case; for inline labels we'd need source-level parsing.
        pass
    # Heuristic: if there are 2+ branches to the same `.L<N>` AND that label
    # likely appears mid-function, it's probably a loop.
    bt_counts = Counter(branch_targets)
    has_loop = any(c >= 2 for c in bt_counts.values()) or n_branches >= 3

    return {
        "name": asm_path.stem,
        "frame": frame,
        "saves": tuple(sorted(saves)),
        "jals": tuple(jals),
        "n_jalr": n_jalr,
        "n_loads": n_loads,
        "n_stores": n_stores,
        "n_branches": n_branches,
        "has_loop": has_loop,
        "n_insns": len(insns),
    }


def similarity(fp1: dict, fp2: dict) -> int:
    """Compute 0-100 similarity score between two fingerprints.

    Designed to award points for SPECIFIC structural matches, not for both
    happening to be "empty" (e.g., both having no calls and no saves —
    which is a weak signal). Empty-empty matches get small credit at most.
    """
    score = 0
    # Frame: identical (and non-zero) = +15, within 16 bytes = +5
    if fp1["frame"] == fp2["frame"]:
        score += 15 if fp1["frame"] > 0 else 5
    elif abs(fp1["frame"] - fp2["frame"]) <= 16:
        score += 5

    # Callee-save set: identical AND non-empty = +15.
    # Identical-empty-set is weak (means leaf function on both sides — generic).
    if fp1["saves"] == fp2["saves"]:
        score += 15 if fp1["saves"] else 3
    elif set(fp1["saves"]) & set(fp2["saves"]):
        common = len(set(fp1["saves"]) & set(fp2["saves"]))
        total = len(set(fp1["saves"]) | set(fp2["saves"]))
        score += int(8 * common / total) if total else 0

    # JAL sequence: identical-non-empty = +30, same SET = +20, same count = +10.
    # Identical-EMPTY = +5 (weak signal — both leaf functions).
    if fp1["jals"] == fp2["jals"]:
        score += 30 if fp1["jals"] else 5
    elif set(fp1["jals"]) == set(fp2["jals"]) and fp1["jals"]:
        score += 20
    elif len(fp1["jals"]) == len(fp2["jals"]) and fp1["jals"]:
        common = len(set(fp1["jals"]) & set(fp2["jals"]))
        if common:
            score += 5 + int(10 * common / max(len(fp1["jals"]), 1))
        else:
            score += 5

    # JALR count match (only credit if at least one has a jalr).
    if fp1["n_jalr"] == fp2["n_jalr"] and fp1["n_jalr"] > 0:
        score += 5

    # Instruction-count proximity, scaled by relative size — for big
    # functions, "within 5 insns" is a very weak signal. Use percent.
    avg = (fp1["n_insns"] + fp2["n_insns"]) / 2
    if avg > 0:
        rel_delta = abs(fp1["n_insns"] - fp2["n_insns"]) / avg
        if rel_delta <= 0.10:
            score += 10
        elif rel_delta <= 0.25:
            score += 5

    # Loop bool match (skip the empty-empty case — both False is uninteresting)
    if fp1["has_loop"] == fp2["has_loop"]:
        score += 5 if fp1["has_loop"] else 2

    # Loads / stores ballpark — only meaningful if at least one is non-zero
    if fp1["n_loads"] > 0 or fp2["n_loads"] > 0:
        if abs(fp1["n_loads"] - fp2["n_loads"]) <= 2:
            score += 5
    if fp1["n_stores"] > 0 or fp2["n_stores"] > 0:
        if abs(fp1["n_stores"] - fp2["n_stores"]) <= 2:
            score += 5

    # Branch count — skip for both-zero
    if fp1["n_branches"] > 0 or fp2["n_branches"] > 0:
        if abs(fp1["n_branches"] - fp2["n_branches"]) <= 1:
            score += 10

    return min(100, score)


def is_inline_asm(name: str, src_text_cache: dict) -> bool:
    """Is this function still inline asm in src/*.c?"""
    pattern = re.compile(rf'glabel\s+{re.escape(name)}\b')
    for src_file, text in src_text_cache.items():
        if pattern.search(text):
            return True
    return False


def load_src_cache() -> dict:
    """Read all src/*.c into memory once for the inline-asm check."""
    cache: dict[str, str] = {}
    for c_file in sorted(SRC.glob("*.c")):
        try:
            cache[c_file.name] = c_file.read_text(encoding="utf-8", errors="replace")
        except OSError:
            pass
    return cache


def load_blocked() -> set[str]:
    p = ROOT / "known_blocked.txt"
    out: set[str] = set()
    if not p.exists():
        return out
    for raw in p.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        first = line.split()[0] if line.split() else ""
        if first:
            out.add(first)
    return out


def cmd_one(reference: str) -> int:
    """Find siblings of a single reference function."""
    asm_path = ASM_FUNCS / f"{reference}.s"
    if not asm_path.exists():
        print(f"ERROR: {asm_path} not found", file=sys.stderr)
        return 1
    ref_fp = parse_function(asm_path)
    if ref_fp is None:
        print(f"ERROR: could not fingerprint {reference}", file=sys.stderr)
        return 1

    src_cache = load_src_cache()
    blocked = load_blocked()
    ref_is_inline = is_inline_asm(reference, src_cache)

    print(f"# Reference: {reference}")
    print(f"#   {'STILL inline asm' if ref_is_inline else 'already C'}")
    print(f"#   frame={ref_fp['frame']} insns={ref_fp['n_insns']} "
          f"jals={len(ref_fp['jals'])} branches={ref_fp['n_branches']} "
          f"loop={ref_fp['has_loop']}")
    print(f"#   saves: {', '.join(ref_fp['saves']) or '(none)'}")
    if ref_fp["jals"]:
        print(f"#   call sequence: {' -> '.join(ref_fp['jals'])}")
    print()

    # Score every other function in asm/funcs/
    scored: list[tuple[int, dict, bool]] = []
    for asm_file in sorted(ASM_FUNCS.glob("*.s")):
        name = asm_file.stem
        if name == reference or name in blocked:
            continue
        fp = parse_function(asm_file)
        if fp is None:
            continue
        score = similarity(ref_fp, fp)
        if score < 40:  # noise threshold
            continue
        inline = is_inline_asm(name, src_cache)
        scored.append((score, fp, inline))

    scored.sort(key=lambda x: -x[0])
    if not scored:
        print("# No similar functions found (threshold: score >= 40).")
        return 0

    # Print top 20 unmatched candidates first, then matched (for reference)
    unmatched = [s for s in scored if s[2]]
    matched = [s for s in scored if not s[2]]

    print(f"# Top unmatched siblings (score >= 40): {len(unmatched)} found")
    print(f"#   score  size  jals  saves                   func")
    for score, fp, _ in unmatched[:20]:
        saves = "{" + ",".join(fp["saves"]) + "}" if fp["saves"] else "{}"
        print(f"  {score:3d}    {fp['n_insns']:3d}   {len(fp['jals']):2d}    "
              f"{saves:24s} {fp['name']}")

    if matched:
        print()
        print(f"# Already-matched references (top 5): {len(matched)} total")
        for score, fp, _ in matched[:5]:
            saves = "{" + ",".join(fp["saves"]) + "}" if fp["saves"] else "{}"
            print(f"  {score:3d}    {fp['n_insns']:3d}   {len(fp['jals']):2d}    "
                  f"{saves:24s} {fp['name']}")

    return 0


def cmd_all() -> int:
    """Cluster all still-inline-asm functions into similarity groups."""
    src_cache = load_src_cache()
    blocked = load_blocked()

    fps: list[dict] = []
    for asm_file in sorted(ASM_FUNCS.glob("*.s")):
        name = asm_file.stem
        if name in blocked:
            continue
        if not is_inline_asm(name, src_cache):
            continue
        fp = parse_function(asm_file)
        if fp is None:
            continue
        fps.append(fp)

    print(f"# Clustering {len(fps)} still-inline-asm candidates")
    print()

    used = set()
    cluster_id = 0
    for i, ref in enumerate(fps):
        if ref["name"] in used:
            continue
        cluster = [ref]
        used.add(ref["name"])
        for other in fps[i + 1:]:
            if other["name"] in used:
                continue
            score = similarity(ref, other)
            if score >= 60:
                cluster.append(other)
                used.add(other["name"])
        if len(cluster) < 2:
            continue
        cluster_id += 1
        print(f"=== Cluster {cluster_id}: {len(cluster)} members "
              f"(seed: {ref['name']}, frame={ref['frame']}, "
              f"insns~{ref['n_insns']}, {len(ref['jals'])} jals) ===")
        for fp in cluster:
            print(f"  {fp['name']:25s}  frame={fp['frame']:4d}  "
                  f"insns={fp['n_insns']:3d}  jals={len(fp['jals']):2d}")
        print()

    if cluster_id == 0:
        print("# No clusters found at threshold score >= 60.")
    return 0


def main():
    parser = argparse.ArgumentParser(description="Find structural siblings of a function.")
    parser.add_argument("func", nargs="?", help="Reference function name")
    parser.add_argument("--all", action="store_true",
                        help="Cluster all unmatched functions instead")
    args = parser.parse_args()

    if args.all:
        return cmd_all()
    if not args.func:
        parser.print_help()
        return 1
    return cmd_one(args.func)


if __name__ == "__main__":
    sys.exit(main())
