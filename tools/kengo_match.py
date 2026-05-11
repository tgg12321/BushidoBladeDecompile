#!/usr/bin/env python3
"""kengo_match.py — Match BB2 functions to their Kengo (PS2) equivalents.

Kengo (Master of Bushido, PS2) shares the "Marionation" engine with BB2
and ships with full debug symbols (function names + sizes + source paths).
This matcher cross-references every BB2 function against the Kengo symbol
table using three signals, in order of increasing strength:

  1. Size match (±tolerance %) — the weakest, used as a candidate filter
  2. Same-name match — when BB2 has a named symbol that exists in Kengo
  3. Call-graph overlap — Jaccard of named callees disambiguates the
     size-candidate set. Bootstrapped from (2) and iterated to fixpoint.

Writes `kengo_matches.csv` at the repo root with one row per BB2 function.

Inputs:
  Kengo/kengo_functions_full.txt   — Kengo names + sizes + source paths
  Kengo/disc/SLUS_200.21           — Kengo ELF (for callee extraction)
  asm/funcs/*.s                    — BB2 disassembly per function
  src/*.c                          — for inferring BB2 source-file mapping

Usage:
  python3 tools/kengo_match.py                       # refresh kengo_matches.csv
  python3 tools/kengo_match.py --tolerance 0.05      # tighter size band
  python3 tools/kengo_match.py --bb2 func_80023F08   # focused report
  python3 tools/kengo_match.py --queue-only          # only re-match queue funcs
"""
from __future__ import annotations

import argparse
import csv
import os
import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
KENGO_ELF = ROOT / "Kengo" / "disc" / "SLUS_200.21"
KENGO_FUNCS = ROOT / "Kengo" / "kengo_functions_full.txt"
ASM_FUNCS = ROOT / "asm" / "funcs"
CSV_OUT = ROOT / "kengo_matches.csv"
KENGO_DISASM_CACHE = ROOT / "tmp" / "kengo_disasm.txt"

OBJDUMP = "mipsel-linux-gnu-objdump"

# Kengo modules with no BB2 counterpart (PS2-specific). Filtered from the
# candidate pool because their size coincidences are pure noise.
PS2_ONLY_MODULES = frozenset({"mpc", "pack", "eecdvd", "tty", "init"})


# ---------------------------------------------------------------------------
# Kengo side
# ---------------------------------------------------------------------------

def parse_kengo_functions() -> list[dict]:
    """Parse kengo_functions_full.txt into a list of function records.

    Each record: {name, addr, size_insns, src_file, module}.
    Static functions are kept distinct (some Kengo names repeat across
    translation units; the caller disambiguates via size/callees).
    """
    funcs: list[dict] = []
    src_file = None
    in_game = False
    file_re = re.compile(r"// FILE -- (.+)")
    func_re = re.compile(r"/\*\s+([0-9a-f]+)\s+([0-9a-f]+)\s+\*/\s+(?:static\s+)?(\w+)\(\)")

    for line in KENGO_FUNCS.read_text(errors="replace").splitlines():
        m = file_re.match(line)
        if m:
            src_file = m.group(1).strip()
            in_game = src_file.startswith("src/")
            continue
        if not in_game:
            continue
        m = func_re.match(line)
        if m and src_file:
            size_b = int(m.group(2), 16)
            if size_b == 0:
                continue
            name = m.group(3)
            module = os.path.splitext(os.path.basename(src_file))[0]
            if module in PS2_ONLY_MODULES:
                continue
            funcs.append({
                "name": name,
                "addr": int(m.group(1), 16),
                "size_insns": size_b // 4,
                "src_file": src_file,
                "module": module,
                "callees": set(),  # filled later
            })
    return funcs


def disassemble_kengo() -> str:
    """Run objdump on the Kengo ELF once and cache the result. ~0.3s."""
    if (KENGO_DISASM_CACHE.exists()
            and KENGO_DISASM_CACHE.stat().st_mtime > KENGO_ELF.stat().st_mtime):
        return KENGO_DISASM_CACHE.read_text(errors="replace")
    print(f"  objdump {KENGO_ELF.name}...", file=sys.stderr, end="", flush=True)
    out = subprocess.run(
        [OBJDUMP, "-d", str(KENGO_ELF)],
        capture_output=True, text=True, errors="replace",
    )
    KENGO_DISASM_CACHE.parent.mkdir(parents=True, exist_ok=True)
    KENGO_DISASM_CACHE.write_text(out.stdout)
    print(" done", file=sys.stderr)
    return out.stdout


KENGO_FUNC_HEADER = re.compile(r"^([0-9a-f]+) <(\w+)>:")
# objdump emits `jal\t<hexaddr> <name>` for direct calls.
KENGO_JAL = re.compile(r"\bjal\s+[0-9a-f]+\s+<(\w+)>")


def populate_kengo_callees(funcs: list[dict]) -> None:
    """Walk the Kengo disassembly, assign each `jal <name>` to the
    enclosing function (keyed by address)."""
    by_addr = {f["addr"]: f for f in funcs}
    text = disassemble_kengo()
    cur: dict | None = None
    for line in text.splitlines():
        m = KENGO_FUNC_HEADER.match(line)
        if m:
            cur = by_addr.get(int(m.group(1), 16))
            continue
        if cur is None:
            continue
        j = KENGO_JAL.search(line)
        if j:
            cur["callees"].add(j.group(1))


# ---------------------------------------------------------------------------
# BB2 side
# ---------------------------------------------------------------------------

BB2_INSN = re.compile(r"^\s*/\*\s+[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s*\*/")
BB2_JAL = re.compile(r"\bjal\s+(\w+)")


def parse_bb2_functions() -> list[dict]:
    """Return [{name, size_insns, callees, src_file}] for every BB2 function
    with an `asm/funcs/<name>.s` entry. `src_file` is inferred by which
    src/*.c file references the function's name.
    """
    name_index: dict[str, set[str]] = defaultdict(set)
    name_re = re.compile(r"\b(func_[0-9A-Fa-f]{8}|[A-Za-z_][A-Za-z0-9_]+)\b")
    for c in sorted((ROOT / "src").glob("*.c")):
        text = c.read_text(errors="replace")
        for m in name_re.finditer(text):
            name_index[m.group(1)].add(c.name)

    funcs: list[dict] = []
    for p in sorted(ASM_FUNCS.glob("*.s")):
        name = p.stem
        # Skip data symbols (D_8001XXXX etc.) that landed in asm/funcs/
        # accidentally — splat sometimes emits them; they aren't functions.
        if name.startswith("D_"):
            continue
        size = 0
        callees: set[str] = set()
        for line in p.read_text(errors="replace").splitlines():
            if BB2_INSN.match(line):
                size += 1
            j = BB2_JAL.search(line)
            if j:
                callees.add(j.group(1))
        srcs = name_index.get(name, set())
        src = sorted(srcs)[0] if srcs else ""
        funcs.append({
            "name": name,
            "size_insns": size,
            "callees": callees,
            "src_file": src,
        })
    return funcs


# ---------------------------------------------------------------------------
# Matching
# ---------------------------------------------------------------------------

def size_candidates(bb2_size: int, kengo: list[dict], tol: float) -> list[dict]:
    if bb2_size <= 0:
        return []
    lo, hi = bb2_size * (1 - tol), bb2_size * (1 + tol)
    return [k for k in kengo if lo <= k["size_insns"] <= hi]


def jaccard(a: set[str], b: set[str]) -> float:
    if not a or not b:
        return 0.0
    u = a | b
    return len(a & b) / len(u) if u else 0.0


def translate_callees(callees: set[str], xlat: dict[str, str]) -> set[str]:
    """Map BB2 callee names through xlat (BB2 name → Kengo name) where
    known. Untranslated names stay as-is (BB2-named symbols frequently
    share their Kengo names already)."""
    return {xlat.get(c, c) for c in callees}


def match_all(bb2: list[dict], kengo: list[dict], tol: float,
              passes: int = 3, score_threshold: float = 0.2) -> dict[str, dict]:
    """Iterate to a fixpoint and return {bb2_name: result}."""
    kengo_by_name: dict[str, list[dict]] = defaultdict(list)
    for k in kengo:
        kengo_by_name[k["name"]].append(k)

    # Seed translation map with unique-name matches: BB2 name == Kengo name
    # and there is exactly one Kengo function with that name.
    xlat: dict[str, str] = {}
    for b in bb2:
        same = kengo_by_name.get(b["name"], [])
        if len(same) == 1:
            xlat[b["name"]] = b["name"]

    results: dict[str, dict] = {}
    for it in range(passes):
        new_xlat = dict(xlat)
        for b in bb2:
            bsize = b["size_insns"]
            bcallees_x = translate_callees(b["callees"], xlat)

            cands = size_candidates(bsize, kengo, tol)
            same_name = kengo_by_name.get(b["name"], [])
            if same_name:
                size_same = [k for k in same_name
                             if abs(k["size_insns"] - bsize) / max(bsize, 1) <= tol]
                if size_same:
                    cands = size_same

            scored = []
            for k in cands:
                s = jaccard(bcallees_x, k["callees"])
                scored.append((s, k))
            scored.sort(key=lambda x: (-x[0], abs(x[1]["size_insns"] - bsize)))

            best = scored[0][1] if scored else None
            best_score = scored[0][0] if scored else 0.0

            if not cands:
                conf = "no-match"
            elif same_name and len(cands) == 1:
                conf = "name-unique"
            elif same_name and best_score >= score_threshold:
                conf = "name-callgraph"
            elif best_score >= score_threshold and len(cands) > 1:
                conf = "callgraph"
            elif len(cands) == 1:
                conf = "size-only-unique"
            else:
                conf = "size-only-ambiguous"

            results[b["name"]] = {
                "bb2_func": b["name"],
                "bb2_file": b["src_file"] or "",
                "bb2_insns": bsize,
                "kengo_name": best["name"] if best else "",
                "kengo_insns": best["size_insns"] if best else "",
                "diff": (best["size_insns"] - bsize) if best else "",
                "kengo_module": best["module"] if best else "",
                "kengo_source_file": best["src_file"] if best else "",
                "confidence": conf,
                "n_candidates": len(cands),
                "callee_overlap": f"{best_score:.2f}",
                "_scored": scored,
            }

            if best and conf in ("name-unique", "name-callgraph", "callgraph"):
                new_xlat[b["name"]] = best["name"]

        if new_xlat == xlat:
            break
        xlat = new_xlat

    return results


# ---------------------------------------------------------------------------
# Output
# ---------------------------------------------------------------------------

CSV_COLS = [
    "bb2_func", "bb2_file", "bb2_insns",
    "kengo_name", "kengo_insns", "diff",
    "kengo_module", "kengo_source_file",
    "confidence", "n_candidates", "callee_overlap",
]


def write_csv(results: dict[str, dict], path: Path) -> None:
    rows = sorted(results.values(), key=lambda r: r["bb2_func"])
    with path.open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=CSV_COLS, extrasaction="ignore")
        w.writeheader()
        for r in rows:
            w.writerow(r)


def report_summary(results: dict[str, dict]) -> None:
    by_conf: dict[str, int] = defaultdict(int)
    for r in results.values():
        by_conf[r["confidence"]] += 1
    total = len(results)
    print()
    print(f"BB2 functions matched: {total}")
    order = ["name-unique", "name-callgraph", "callgraph",
             "size-only-unique", "size-only-ambiguous", "no-match"]
    for k in order:
        n = by_conf.get(k, 0)
        if n:
            print(f"  {k:<22s} {n:>5d}  ({n*100/total:5.1f}%)")


def print_focused(name: str, results: dict[str, dict]) -> None:
    r = results.get(name)
    if not r:
        print(f"{name}: not found in BB2 asm/funcs/")
        return
    print(f"BB2: {r['bb2_func']}  ({r['bb2_insns']} insns, {r['bb2_file'] or '?'})")
    print(f"Confidence: {r['confidence']}, candidates={r['n_candidates']}, "
          f"best Jaccard={r['callee_overlap']}")
    print()
    scored = r.get("_scored", [])[:10]
    print(f"{'score':>6s}  {'insns':>5s}  {'name':<40s}  {'module':<24s}  src")
    for s, k in scored:
        print(f"{s:>6.2f}  {k['size_insns']:>5d}  {k['name']:<40s}  "
              f"{k['module']:<24s}  {k['src_file']}")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--tolerance", type=float, default=0.10,
                    help="Fractional size tolerance (default 0.10 = ±10%%)")
    ap.add_argument("--bb2", help="Show focused match report for one BB2 function")
    ap.add_argument("--queue-only", action="store_true",
                    help="Filter summary/CSV to queue functions only")
    ap.add_argument("--no-write", action="store_true",
                    help="Don't write kengo_matches.csv; just print summary")
    ap.add_argument("--passes", type=int, default=3,
                    help="Iteration count for call-graph propagation (default 3)")
    args = ap.parse_args()

    print("Loading Kengo functions...", file=sys.stderr)
    kengo = parse_kengo_functions()
    print(f"  {len(kengo)} game-source Kengo functions", file=sys.stderr)

    print("Extracting Kengo call graph...", file=sys.stderr)
    populate_kengo_callees(kengo)
    with_callees = sum(1 for k in kengo if k["callees"])
    print(f"  {with_callees}/{len(kengo)} have ≥1 callee", file=sys.stderr)

    print("Loading BB2 functions...", file=sys.stderr)
    bb2 = parse_bb2_functions()
    print(f"  {len(bb2)} BB2 functions from asm/funcs/", file=sys.stderr)

    print("Matching (call-graph propagation)...", file=sys.stderr)
    results = match_all(bb2, kengo, tol=args.tolerance, passes=args.passes)

    if args.bb2:
        print_focused(args.bb2, results)
        return 0

    report_summary(results)

    if args.queue_only:
        queue_path = ROOT / "tmp" / "q_all.txt"
        if queue_path.exists():
            qfuncs = set(queue_path.read_text().split())
            queue_only = {k: v for k, v in results.items() if k in qfuncs}
            print(f"\n(queue-only filter: {len(queue_only)} functions)")
            report_summary(queue_only)
            results = queue_only

    if not args.no_write:
        write_csv(results, CSV_OUT)
        print(f"\nWrote {CSV_OUT.relative_to(ROOT)}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
