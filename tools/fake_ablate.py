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
  python3 tools/fake_ablate.py --func <func> --file <stem> --candidate <form.c>
      [--json] [--keep] [--max-variants N]

UNIT VOCABULARY — what counts as one ablatable FAKE unit:
  * inline marker — the marker sits after code on the same line
    (`x = 1; /* FAKE: ... */`, `x = 1; // FAKE`); the unit is that line.
  * comment block above the construct — the marker opens a comment-ONLY line
    (`/* FAKE: ... */`, possibly continued over several ` * ...` lines); the
    unit spans the whole comment block PLUS the construct it annotates (the
    next non-blank, non-comment line). 16 of 31 live FAKE units are this
    shape — ablating only the comment would leave the device in place.
  * wrap — the annotated construct is a multi-line `do { ... } while (0);`
    (marker on the `do {` opener, on the `} while (0);` closer, or in the
    comment block above it); the unit removes BOTH brace lines and keeps the
    wrapped body.
  * one-line wrap — `do { BODY } while (0);` on a single line; the unit
    REWRITES the line to `BODY` (never deletes it). Nested one-liners peel
    one level per unit.
Prose mentioning FAKE inside a comment body (a continuation line starting
with `*`) is NOT a unit.

Variants are written under tmp/grind/<func>/ablate/ and scored with
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
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SWEEP_TIMEOUT = 3600
# A marker that OPENS a comment (comment-only line) …
FAKE_OPEN_RE = re.compile(r"^\s*(?:/\*|//)\s*FAKE\b")
# … or one that follows code on the same line.
FAKE_INLINE_RE = re.compile(r"/\*\s*FAKE\b|//\s*FAKE\b")
COMMENT_CONT_RE = re.compile(r"^\s*(?:/\*|\*|//)")
DO_RE = re.compile(r"^\s*do\s*\{")
WHILE0_RE = re.compile(r"^\s*\}\s*while\s*\(\s*0\s*\)\s*;")
ONELINE_DO_RE = re.compile(r"^(\s*)do\s*\{(.*)\}\s*while\s*\(\s*0\s*\)\s*;(.*)$")
BLOCK_COMMENT_RE = re.compile(r"/\*.*?\*/", re.DOTALL)
LINE_COMMENT_RE = re.compile(r"//.*$")


@dataclass
class Unit:
    kind: str          # "line" | "wrap"
    lines: list        # 0-based line indexes removed when the unit is ablated
    text: str          # first line, for reports
    replace: dict = field(default_factory=dict)  # line index -> replacement text


def _strip_comments(line: str) -> str:
    """Comment-free view of a line, for brace counting (M9)."""
    out = BLOCK_COMMENT_RE.sub(" ", line)
    out = re.sub(r"/\*.*$", " ", out)      # unterminated block-comment opener
    return LINE_COMMENT_RE.sub(" ", out)


def _is_comment_only(line: str) -> bool:
    s = line.strip()
    if not COMMENT_CONT_RE.match(s):
        return False
    if "*/" in s:
        return s.endswith("*/") or not s.split("*/", 1)[1].strip()
    return True


def _comment_block_end(lines: list, i: int) -> int:
    """Last line index of the comment block opened on line i."""
    s = lines[i].strip()
    if s.startswith("//"):
        return i
    if "*/" in s:
        return i
    for j in range(i + 1, len(lines)):
        if "*/" in lines[j]:
            return j
    return i


def _match_while0(lines: list, do_idx: int):
    """Index of the `} while (0);` closing the `do {` at do_idx (brace depth)."""
    depth = 0
    for i in range(do_idx, len(lines)):
        code = _strip_comments(lines[i])
        depth += code.count("{") - code.count("}")
        if i > do_idx and depth == 0 and WHILE0_RE.match(lines[i]):
            return i
        if i > do_idx and depth <= 0 and not WHILE0_RE.match(lines[i]):
            return None
    return None


def _match_do(lines: list, close_idx: int):
    """Index of the `do {` matched by the `} while (0);` at close_idx (I3)."""
    depth = 0
    for i in range(close_idx, -1, -1):
        code = _strip_comments(lines[i])
        depth += code.count("}") - code.count("{")
        if i < close_idx and depth == 0 and DO_RE.match(lines[i]):
            return i
        if i < close_idx and depth <= 0 and not DO_RE.match(lines[i]):
            return None
    return None


def _peel_oneliner(line: str):
    """`do { BODY } while (0); /* FAKE */` -> `BODY` (one level), or None."""
    m = ONELINE_DO_RE.match(line)
    if not m:
        return None
    indent, body, _tail = m.group(1), m.group(2), m.group(3)
    return indent + body.strip()


def _next_construct(lines: list, start: int):
    """First non-blank, non-comment line at/after start."""
    for j in range(start, len(lines)):
        if not lines[j].strip():
            continue
        if _is_comment_only(lines[j]):
            continue
        return j
    return None


def _unit_for_construct(lines: list, j: int, prefix: list, text: str):
    """Build the unit covering the construct on line j (plus `prefix` lines)."""
    peeled = _peel_oneliner(lines[j])
    if peeled is not None:
        return Unit("wrap", list(prefix), text, {j: peeled})
    if DO_RE.match(lines[j]):
        k = _match_while0(lines, j)
        if k is not None:
            return Unit("wrap", prefix + [j, k], text)
    if WHILE0_RE.match(lines[j]):
        k = _match_do(lines, j)
        if k is not None:
            return Unit("wrap", sorted(prefix + [j, k]), text)
    return Unit("line", prefix + [j], text)


def find_fake_units(text: str) -> list:
    lines = text.split("\n")
    units = []
    consumed = set()
    for i, ln in enumerate(lines):
        if i in consumed:
            continue
        stripped = ln.strip()
        if FAKE_OPEN_RE.match(stripped) and _is_comment_only(ln):
            # C1: comment block above the annotated construct.
            end = _comment_block_end(lines, i)
            block = list(range(i, end + 1))
            j = _next_construct(lines, end + 1)
            if j is None:
                u = Unit("line", block, stripped)
            else:
                u = _unit_for_construct(lines, j, block, stripped)
        elif not COMMENT_CONT_RE.match(stripped) and FAKE_INLINE_RE.search(ln):
            # inline marker after code
            u = _unit_for_construct(lines, i, [], stripped)
        else:
            continue
        units.append(u)
        consumed.update(u.lines)
        consumed.update(u.replace)
    return units


def remove_units(text: str, units: list) -> str:
    drop = {i for u in units for i in u.lines}
    repl = {}
    for u in units:
        repl.update(u.replace)
    lines = text.split("\n")
    out = []
    for i, ln in enumerate(lines):
        if i in drop:
            continue
        out.append(repl.get(i, ln))
    return "\n".join(out)


def ablation_masks(n: int) -> list:
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
    ap.add_argument("--max-variants", type=int, default=24,
                    help="refuse a grid larger than this (default 24)")
    a = ap.parse_args()

    text = Path(a.candidate).read_text(encoding="utf-8", errors="replace")
    units = find_fake_units(text)
    if not units:
        print(f"no FAKE-annotated constructs found in {a.candidate}; nothing to ablate")
        return 0
    masks = ablation_masks(len(units))
    if len(masks) > a.max_variants:
        print(f"FAKE units in {a.candidate}: {len(units)}")
        for k, u in enumerate(units):
            print(f"  [{k}] {u.kind:4s} L{u.lines[0] + 1}: {u.text[:100]}")
        print(f"\ngrid is {len(masks)} variants > --max-variants {a.max_variants}; "
              f"re-run with --max-variants {len(masks)} to sweep it anyway", file=sys.stderr)
        return 2
    outdir = ROOT / "tmp" / "grind" / a.func / "ablate"
    outdir.mkdir(parents=True, exist_ok=True)
    for old in outdir.glob("*.c"):
        old.unlink()
    plan = []
    for mask in masks:
        removed = [u for u, b in zip(units, mask) if b]
        p = outdir / f"{mask_name(mask)}.c"
        with open(p, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(remove_units(text, removed))
        plan.append({"variant": p, "removed": [u.text for u in removed]})

    keep_files = set()
    try:
        try:
            r = subprocess.run([sys.executable, "tools/sweep_variants.py", "--func", a.func,
                                "--file", a.file, "--variants", str(outdir), "--json"],
                               capture_output=True, text=True, cwd=ROOT, timeout=SWEEP_TIMEOUT)
        except subprocess.TimeoutExpired as e:
            out = (e.stdout or "") + (e.stderr or "")
            if isinstance(out, bytes):
                out = out.decode("utf-8", errors="replace")
            print(f"sweep_variants timed out after {SWEEP_TIMEOUT}s; captured:\n"
                  + out[-1500:], file=sys.stderr)
            return 1
        if r.returncode != 0:
            print(f"sweep_variants failed (rc={r.returncode}):\n"
                  + (r.stdout + r.stderr)[-1500:], file=sys.stderr)
            return 1
        try:
            sweep = json.loads(r.stdout)
        except json.JSONDecodeError:
            print("sweep_variants produced no JSON:\n" + (r.stdout + r.stderr)[-1500:], file=sys.stderr)
            return 1
        scores = {Path(rec["variant"]).name: rec for rec in sweep.get("results", [])}
        rows = []
        for item in plan:
            rec = scores.get(item["variant"].name, {})
            rows.append({"variant": item["variant"].name, "score": rec.get("score"),
                         "build_insns": rec.get("build_insns"), "removed": item["removed"],
                         "error": rec.get("error")})
        rows.sort(key=lambda x: (x["score"] is None, x["score"] if x["score"] is not None else 0))
        best = next((x for x in rows if x["score"] is not None), None)
        base = next((x for x in rows if x["variant"] == "keep-all.c"), None)
        win = (best is not None and base is not None and base["score"] is not None
               and best["score"] < base["score"])
        if win:
            keep_files.add(best["variant"])
        if a.json:
            print(json.dumps({"func": a.func, "file": a.file,
                              "units": [{"kind": u.kind, "lines": u.lines, "text": u.text} for u in units],
                              "results": rows,
                              "win": (outdir / best["variant"]).as_posix() if win else None}, indent=2))
        else:
            print(f"FAKE units in {a.candidate}: {len(units)}")
            for k, u in enumerate(units):
                print(f"  [{k}] {u.kind:4s} L{u.lines[0] + 1}: {u.text[:100]}")
            print("\nscore  bi    variant        removed")
            for x in rows:
                sc = " ERR" if x["score"] is None else f"{x['score']:4d}"
                print(f"{sc}  {str(x['build_insns']):>4}  {x['variant']:<14} "
                      f"{'; '.join(t[:40] for t in x['removed']) or '(none)'}")
            if win:
                print(f"\nABLATION WIN: {best['variant']} scores {best['score']} vs keep-all {base['score']} "
                      "— a FAKE construct was masking this form. Bank the winning variant.")
                print(f"  kept: {(outdir / best['variant']).as_posix()}")
    finally:
        if not a.keep:
            for p in outdir.glob("*.c"):
                if p.name not in keep_files:
                    p.unlink()
    return 0


if __name__ == "__main__":
    sys.exit(main())
