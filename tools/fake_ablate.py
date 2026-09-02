#!/usr/bin/env python3
"""FAKE-ablation sweep — score one candidate form across the grid of its
FAKE-annotated constructs removed.

WHY (post-mortem 2026-09-01): func_8002EA24 recorded its winning lever
"inert" in s8 because a /* FAKE */ carrier occupied the pseudo the lever
targeted; the same lever applied to the no-carrier control scores 0.
func_800324D0's staged read diverted a reference lift to the wrong web the
same way. A lever may not be called dead while a FAKE construct sits on the
register it is trying to move — so measure every lever on every subset of
the FAKE constructs, in one call.

Usage (WSL, venv active, repo root):
  python3 tools/fake_ablate.py --func <func> --file <stem> --candidate <form.c> [--json] [--keep]

<form.c> is a COMPLETE function definition (the memory/grind/<func>/candidate.c
shape). Variants are written under tmp/grind/<func>/ablate/ and scored with
tools/sweep_variants.py (which restores src/<stem>.c byte-exact afterwards).
The grid: all 2^n subsets for n <= 4 FAKE units; for n > 4: none, every
singleton, every pair, and all-removed. A variant that fails to compile
(e.g. a removed holder still referenced) is reported as ERR, not skipped.

Pure-C policy note: removing a FAKE construct can only make a form MORE
honest; this tool never adds anything. A score-0 variant is still a PROPOSAL
and goes through the self-vet, layer-1, and Judge like any candidate.
"""
from __future__ import annotations

import argparse
import itertools
import json
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
FAKE_RE = re.compile(r"/\*\s*FAKE\b|//\s*FAKE\b")
DO_RE = re.compile(r"^\s*do\s*\{")
WHILE0_RE = re.compile(r"^\s*\}\s*while\s*\(\s*0\s*\)\s*;")


@dataclass
class Unit:
    kind: str          # "line" | "wrap"
    lines: list        # 0-based line indexes removed when the unit is ablated
    text: str          # first line, for reports


def _match_while0(lines: list[str], do_idx: int) -> int | None:
    """Index of the `} while (0);` closing the `do {` at do_idx (brace depth)."""
    depth = 0
    for i in range(do_idx, len(lines)):
        depth += lines[i].count("{") - lines[i].count("}")
        if i > do_idx and depth == 0 and WHILE0_RE.match(lines[i]):
            return i
        if i > do_idx and depth <= 0 and not WHILE0_RE.match(lines[i]):
            return None
    return None


def find_fake_units(text: str) -> list[Unit]:
    lines = text.split("\n")
    units: list[Unit] = []
    for i, ln in enumerate(lines):
        if not FAKE_RE.search(ln):
            continue
        if DO_RE.match(ln):
            j = _match_while0(lines, i)
            if j is not None:
                units.append(Unit("wrap", [i, j], ln.strip()))
                continue
        units.append(Unit("line", [i], ln.strip()))
    return units


def remove_units(text: str, units: list[Unit]) -> str:
    drop = {i for u in units for i in u.lines}
    lines = text.split("\n")
    return "\n".join(ln for i, ln in enumerate(lines) if i not in drop)


def ablation_masks(n: int) -> list[tuple]:
    if n <= 4:
        return list(itertools.product([False, True], repeat=n))
    masks = {tuple([False] * n), tuple([True] * n)}
    for k in (1, 2):
        for combo in itertools.combinations(range(n), k):
            masks.add(tuple(i in combo for i in range(n)))
    return sorted(masks, key=lambda m: (sum(m), m))


def mask_name(mask: tuple) -> str:
    return "keep-all" if not any(mask) else "drop-" + "".join("1" if b else "0" for b in mask)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--func", required=True)
    ap.add_argument("--file", required=True, help="src stem")
    ap.add_argument("--candidate", required=True, help="complete function definition to ablate")
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--keep", action="store_true", help="keep the variant files after scoring")
    a = ap.parse_args()

    text = Path(a.candidate).read_text(encoding="utf-8", errors="replace")
    units = find_fake_units(text)
    if not units:
        print(f"no FAKE-annotated constructs found in {a.candidate}; nothing to ablate")
        return 0
    outdir = ROOT / "tmp" / "grind" / a.func / "ablate"
    outdir.mkdir(parents=True, exist_ok=True)
    for old in outdir.glob("*.c"):
        old.unlink()
    plan = []
    for mask in ablation_masks(len(units)):
        removed = [u for u, b in zip(units, mask) if b]
        p = outdir / f"{mask_name(mask)}.c"
        p.write_text(remove_units(text, removed), encoding="utf-8", newline="\n")
        plan.append({"variant": str(p), "removed": [u.text for u in removed]})

    r = subprocess.run([sys.executable, "tools/sweep_variants.py", "--func", a.func,
                        "--file", a.file, "--variants", str(outdir), "--json"],
                       capture_output=True, text=True, cwd=ROOT)
    try:
        sweep = json.loads(r.stdout)
    except json.JSONDecodeError:
        print("sweep_variants produced no JSON:\n" + (r.stdout + r.stderr)[-1500:], file=sys.stderr)
        return 1
    scores = {rec["variant"]: rec for rec in sweep["results"]}
    rows = []
    for item in plan:
        rec = scores.get(item["variant"], {})
        rows.append({"variant": Path(item["variant"]).name, "score": rec.get("score"),
                     "build_insns": rec.get("build_insns"), "removed": item["removed"],
                     "error": rec.get("error")})
    rows.sort(key=lambda x: (x["score"] is None, x["score"] if x["score"] is not None else 0))
    if a.json:
        print(json.dumps({"func": a.func, "file": a.file,
                          "units": [{"kind": u.kind, "lines": u.lines, "text": u.text} for u in units],
                          "results": rows}, indent=2))
    else:
        print(f"FAKE units in {a.candidate}: {len(units)}")
        for k, u in enumerate(units):
            print(f"  [{k}] {u.kind:4s} L{u.lines[0] + 1}: {u.text[:100]}")
        print("\nscore  bi    variant        removed")
        for x in rows:
            sc = " ERR" if x["score"] is None else f"{x['score']:4d}"
            print(f"{sc}  {str(x['build_insns']):>4}  {x['variant']:<14} "
                  f"{'; '.join(t[:40] for t in x['removed']) or '(none)'}")
        best = next((x for x in rows if x["score"] is not None), None)
        base = next((x for x in rows if x["variant"] == "keep-all.c"), None)
        if best and base and base["score"] is not None and best["score"] < base["score"]:
            print(f"\nABLATION WIN: {best['variant']} scores {best['score']} vs keep-all {base['score']} "
                  "— a FAKE construct was masking this form. Bank the winning variant.")
    if not a.keep:
        for p in outdir.glob("*.c"):
            p.unlink()
    return 0


if __name__ == "__main__":
    sys.exit(main())
