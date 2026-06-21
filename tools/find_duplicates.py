#!/usr/bin/env python3
"""find_duplicates.py — port of SOTN's `tools/dups` to Bushido Blade 2.

Cross-function near-clone finder over the per-function `.s` files in
`asm/funcs/`. SOTN's algorithm, transliterated to Python:

  1. Parse each `.s` to its instruction word stream.
  2. Reduce each function to its MIPS opcode-class key — the top 6 bits
     of each instruction (`op >> 26`) packed as a byte string. Operand
     diffs (regs, immediates, branch targets) are normalised out.
  3. Pairwise Levenshtein similarity on the keys; pairs above
     --threshold are emitted.

We write two reports under `tmp/` (per root-cleanliness):

  tmp/duplicates.txt
      All near-clone pairs, sorted by descending similarity.

  tmp/duplicates_leads.txt
      The actionable view for BB2: for every INCOMPLETE function
      (member of engine/queue.json), the single highest-similarity
      ALREADY-COMPLETED-C candidate. The completed function's C body
      is a starting point — "here is a near-clone whose C is known."

Speed: a length-band filter (|len(a)-len(b)| / max < 1 - threshold)
prunes the 1435^2 ~= 2M pair grid down to ~thousands before each
Levenshtein. Tested ~15s wall on the full BB2 codebase.

This tool is read-only over `asm/`, `engine/queue.json`, and
`inline_asm_canonical.txt`. It writes only to `tmp/`.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional


REPO_ROOT = Path(__file__).resolve().parent.parent
ASM_FUNCS_DIR = REPO_ROOT / "asm" / "funcs"
QUEUE_JSON = REPO_ROOT / "engine" / "queue.json"
CANONICAL_ASM_TXT = REPO_ROOT / "inline_asm_canonical.txt"
TMP_DIR = REPO_ROOT / "tmp"

# Splat's per-function .s lines look like:
#   /* 7863C 80087E3C 21388000 */  addu       $a3, $a0, $zero
# We want the third hex word (the instruction, little-endian in the file).
INSN_RE = re.compile(
    r"^\s*/\*\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s+([0-9A-Fa-f]{8})\s+\*/"
)
# Falls back to splat's no-addr form when present:
#   /*  21388000 */  addu ...
INSN_RE_SHORT = re.compile(r"^\s*/\*\s+([0-9A-Fa-f]{8})\s+\*/")
GLABEL_RE = re.compile(r"^\s*glabel\s+(\S+)")


@dataclass
class Function:
    name: str
    path: Path
    # opcode-class key: one byte per instruction = (op >> 26) & 0x3F
    key: bytes = b""
    insn_count: int = 0


def parse_s_file(path: Path) -> Optional[Function]:
    """Return Function with opcode-class key, or None if the file is empty
    or trivial (e.g. just `jr $ra; nop`)."""
    name: Optional[str] = None
    ops: list[int] = []
    try:
        text = path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return None
    in_text = True  # default; BB2 per-function files are pure .text
    for line in text.splitlines():
        stripped = line.strip()
        if stripped.startswith(".section"):
            in_text = stripped.split()[1] == ".text"
            continue
        if not in_text:
            continue
        m_lbl = GLABEL_RE.match(line)
        if m_lbl and name is None:
            name = m_lbl.group(1)
            continue
        m = INSN_RE.match(line)
        word_hex: Optional[str] = None
        if m:
            # group(1)=vram, group(2)=insn word (LE in file)
            word_hex = m.group(2)
        else:
            m2 = INSN_RE_SHORT.match(line)
            if m2:
                word_hex = m2.group(1)
        if word_hex is None:
            continue
        try:
            raw = int(word_hex, 16)
        except ValueError:
            continue
        # Splat writes the bytes in file-order (little-endian). Reverse
        # to get the actual MIPS instruction word (big-endian-numeric).
        op = (
            ((raw >> 24) & 0xFF)
            | (((raw >> 16) & 0xFF) << 8)
            | (((raw >> 8) & 0xFF) << 16)
            | ((raw & 0xFF) << 24)
        )
        ops.append(op)
    if not ops or name is None:
        return None
    # SOTN skips `jr $ra; nop` (op == 0x03E00008, 0x00000000)
    if len(ops) == 2 and ops[0] == 0x03E00008 and ops[1] == 0x00000000:
        return None
    key = bytes((op >> 26) & 0x3F for op in ops)
    return Function(name=name, path=path, key=key, insn_count=len(ops))


def levenshtein_similarity(a: bytes, b: bytes, early_exit: int) -> float:
    """Damerau-free Levenshtein similarity in [0,1]; returns 0.0 if the
    edit distance exceeds `early_exit` (band cutoff for speed)."""
    la, lb = len(a), len(b)
    if la == 0 and lb == 0:
        return 1.0
    if la == 0 or lb == 0:
        return 0.0
    # Make `a` the shorter (cheaper inner loop).
    if la > lb:
        a, b = b, a
        la, lb = lb, la
    # If the length gap alone exceeds the budget, can't recover.
    if lb - la > early_exit:
        return 0.0
    prev = list(range(lb + 1))
    cur = [0] * (lb + 1)
    for i in range(1, la + 1):
        cur[0] = i
        ai = a[i - 1]
        row_min = cur[0]
        for j in range(1, lb + 1):
            cost = 0 if ai == b[j - 1] else 1
            cur[j] = min(prev[j] + 1, cur[j - 1] + 1, prev[j - 1] + cost)
            if cur[j] < row_min:
                row_min = cur[j]
        if row_min > early_exit:
            return 0.0
        prev, cur = cur, prev
    dist = prev[lb]
    max_len = max(la, lb)
    return (max_len - dist) / max_len


def collect_functions(asm_dir: Path) -> list[Function]:
    funcs: list[Function] = []
    for p in sorted(asm_dir.iterdir()):
        if p.suffix != ".s":
            continue
        f = parse_s_file(p)
        if f is not None:
            funcs.append(f)
    return funcs


def load_incomplete_names(queue_json: Path) -> set[str]:
    """Names of functions still in the queue (INCOMPLETE)."""
    if not queue_json.exists():
        return set()
    data = json.loads(queue_json.read_text(encoding="utf-8"))
    return {it["func"] for it in data.get("items", [])}


def load_canonical_asm_names(path: Path) -> set[str]:
    """Names listed in inline_asm_canonical.txt — these are
    COMPLETED-INLINE-ASM-CANONICAL, NOT pure-C, so they're not useful
    as leads."""
    names: set[str] = set()
    if not path.exists():
        return names
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.split("#", 1)[0].strip()
        if line:
            names.add(line.split()[0])
    return names


def find_all_pairs(funcs: list[Function], threshold: float) -> list[tuple[Function, Function, float]]:
    """All pairs whose similarity >= threshold."""
    # Length-banded sweep. For similarity >= t, max-len budget gives
    # min-len >= t * max-len, i.e. shorter/longer >= t.
    pairs: list[tuple[Function, Function, float]] = []
    # Pre-sort by key length so we can break early once length ratio drops.
    funcs_sorted = sorted(funcs, key=lambda f: len(f.key))
    n = len(funcs_sorted)
    for i in range(n):
        fi = funcs_sorted[i]
        li = len(fi.key)
        if li == 0:
            continue
        # Tightest the longer can be while still possibly hitting threshold:
        # li / lj >= threshold  =>  lj <= li / threshold
        max_lj = li / threshold if threshold > 0 else 1e18
        for j in range(i + 1, n):
            fj = funcs_sorted[j]
            lj = len(fj.key)
            if lj > max_lj:
                break  # sorted; everything past is too long
            # edit-distance budget at this pair
            max_len = max(li, lj)
            budget = int(max_len * (1.0 - threshold))
            sim = levenshtein_similarity(fi.key, fj.key, budget)
            if sim >= threshold:
                pairs.append((fi, fj, sim))
    return pairs


def find_leads(
    funcs: list[Function],
    incomplete: set[str],
    canonical_asm: set[str],
    threshold: float,
) -> list[tuple[Function, Function, float]]:
    """For each INCOMPLETE function, the single highest-similarity
    COMPLETED-C candidate (>= threshold)."""
    completed_c = [
        f for f in funcs
        if f.name not in incomplete and f.name not in canonical_asm
    ]
    completed_c_sorted = sorted(completed_c, key=lambda f: len(f.key))
    completed_lens = [len(f.key) for f in completed_c_sorted]

    incompletes = [f for f in funcs if f.name in incomplete]
    leads: list[tuple[Function, Function, float]] = []

    import bisect

    for fi in incompletes:
        li = len(fi.key)
        if li == 0:
            continue
        # candidate length window
        lo = int(li * threshold)
        hi = int(li / threshold) + 1
        left = bisect.bisect_left(completed_lens, lo)
        right = bisect.bisect_right(completed_lens, hi)
        best: Optional[tuple[Function, float]] = None
        for k in range(left, right):
            fj = completed_c_sorted[k]
            max_len = max(li, len(fj.key))
            budget = int(max_len * (1.0 - threshold))
            sim = levenshtein_similarity(fi.key, fj.key, budget)
            if sim >= threshold and (best is None or sim > best[1]):
                best = (fj, sim)
        if best is not None:
            leads.append((fi, best[0], best[1]))
    return leads


def write_report(
    path: Path,
    header: list[str],
    rows: list[tuple[Function, Function, float]],
    limit: int,
) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    rows = sorted(rows, key=lambda r: r[2], reverse=True)
    truncated = len(rows) > limit
    rows = rows[:limit]
    with path.open("w", encoding="utf-8", newline="\n") as fh:
        for h in header:
            fh.write(f"# {h}\n")
        if truncated:
            fh.write(f"# (output truncated to top {limit}; pass --limit to widen)\n")
        fh.write("#\n")
        for a, b, sim in rows:
            fh.write(
                f"{a.name:32s} ~= {b.name:32s} "
                f"(similarity {sim:.3f}, lens {a.insn_count}/{b.insn_count})\n"
            )


def main(argv: Optional[list[str]] = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--threshold", type=float, default=0.85,
                    help="similarity threshold in [0,1] (default 0.85)")
    ap.add_argument("--limit", type=int, default=200,
                    help="max rows per report (default 200)")
    ap.add_argument("--asm-dir", type=Path, default=ASM_FUNCS_DIR,
                    help=f"per-function asm dir (default {ASM_FUNCS_DIR})")
    ap.add_argument("--out-all", type=Path, default=TMP_DIR / "duplicates.txt",
                    help="report of all near-clone pairs")
    ap.add_argument("--out-leads", type=Path, default=TMP_DIR / "duplicates_leads.txt",
                    help="report of INCOMPLETE -> COMPLETED-C leads")
    ap.add_argument("--quiet", action="store_true", help="suppress progress output")
    args = ap.parse_args(argv)

    if not (0.0 < args.threshold <= 1.0):
        ap.error("--threshold must be in (0, 1]")
    if not args.asm_dir.is_dir():
        ap.error(f"--asm-dir does not exist: {args.asm_dir}")

    def log(msg: str) -> None:
        if not args.quiet:
            print(msg, file=sys.stderr, flush=True)

    t0 = time.time()
    log(f"[1/4] parsing .s files in {args.asm_dir}")
    funcs = collect_functions(args.asm_dir)
    log(f"      parsed {len(funcs)} functions ({time.time()-t0:.1f}s)")

    log("[2/4] loading queue.json + canonical-asm registry")
    incomplete = load_incomplete_names(QUEUE_JSON)
    canonical_asm = load_canonical_asm_names(CANONICAL_ASM_TXT)
    completed_c = sum(
        1 for f in funcs
        if f.name not in incomplete and f.name not in canonical_asm
    )
    log(f"      incomplete={len(incomplete)} canonical-asm={len(canonical_asm)} "
        f"completed-c={completed_c}")

    t1 = time.time()
    log(f"[3/4] scanning all pairs at threshold {args.threshold:.2f}")
    pairs = find_all_pairs(funcs, args.threshold)
    log(f"      found {len(pairs)} near-clone pairs ({time.time()-t1:.1f}s)")

    t2 = time.time()
    log("[4/4] computing leads (INCOMPLETE -> best COMPLETED-C)")
    leads = find_leads(funcs, incomplete, canonical_asm, args.threshold)
    log(f"      found {len(leads)} leads ({time.time()-t2:.1f}s)")

    header_common = [
        f"BB2 near-clone report — generated by tools/find_duplicates.py",
        f"threshold={args.threshold:.3f}  limit={args.limit}  "
        f"functions={len(funcs)}  asm_dir={args.asm_dir}",
        f"algorithm: SOTN-style opcode-class key (op>>26) + Levenshtein similarity",
    ]
    write_report(
        args.out_all,
        header_common + [f"section: ALL near-clone pairs ({len(pairs)} total)"],
        pairs,
        args.limit,
    )
    write_report(
        args.out_leads,
        header_common + [
            f"section: leads = INCOMPLETE -> highest-similarity COMPLETED-C "
            f"({len(leads)} total)",
            f"incomplete={len(incomplete)} completed_c={completed_c} "
            f"canonical_asm_excluded={len(canonical_asm)}",
        ],
        leads,
        args.limit,
    )
    log(f"wrote {args.out_all} ({min(len(pairs), args.limit)} rows)")
    log(f"wrote {args.out_leads} ({min(len(leads), args.limit)} rows)")
    log(f"done in {time.time()-t0:.1f}s")
    return 0


if __name__ == "__main__":
    sys.exit(main())
