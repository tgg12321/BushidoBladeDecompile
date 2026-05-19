#!/usr/bin/env python3
"""bb2_diag_diff.py -- show the asm-level diff between the permuter's
best candidate and target.o for a function.

When `bb2_retire.py` reports `min_score != 0` (plateau), call this to
see WHAT instructions remain wrong so you can reason about which C-level
mutation would help.

Usage:
  python3 tools/bb2_diag_diff.py <func>            # uses the best score dir
  python3 tools/bb2_diag_diff.py <func> --base     # uses base.c directly
  python3 tools/bb2_diag_diff.py <func> --score=60 # uses output-60-* dir
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path


def find_best_output_dir(permdir: Path) -> Path | None:
    """Return the output-N-K dir with the smallest N. Skip output-0-* (matches)."""
    candidates = []
    for d in permdir.glob("output-*"):
        if not d.is_dir():
            continue
        m = re.match(r"output-(\d+)-\d+", d.name)
        if m and int(m.group(1)) > 0:
            candidates.append((int(m.group(1)), d))
    if not candidates:
        return None
    candidates.sort()
    return candidates[0][1]


def compile_to_objdump(c_path: Path, compile_sh: Path) -> str:
    """Compile a base.c-style file and return its objdump -d output."""
    tmp_o = Path("/tmp/bb2_diag.o")
    r = subprocess.run(
        ["bash", str(compile_sh), str(c_path), "-o", str(tmp_o)],
        capture_output=True, text=True,
    )
    if r.returncode != 0 or not tmp_o.exists():
        raise SystemExit(f"compile failed:\n{r.stderr or r.stdout}")
    r2 = subprocess.run(
        ["mipsel-linux-gnu-objdump", "-d", str(tmp_o)],
        capture_output=True, text=True,
    )
    return r2.stdout


def objdump_target(target_o: Path) -> str:
    r = subprocess.run(
        ["mipsel-linux-gnu-objdump", "-d", str(target_o)],
        capture_output=True, text=True,
    )
    return r.stdout


_INSN_RE = re.compile(r"^\s*([0-9a-f]+):\s+([0-9a-f]+)\s+(.*?)\s*$")


def parse_objdump(text: str) -> list[tuple[str, str, str]]:
    """Return [(offset, bytes, mnemonic+operands)] for each instruction line."""
    out = []
    for line in text.split("\n"):
        m = _INSN_RE.match(line)
        if m:
            out.append((m.group(1), m.group(2), m.group(3)))
    return out


def analyze_diff(target: list, built: list, context: int = 2) -> str:
    """Walk through aligned instructions, report differences with context."""
    diffs = []
    # Align by offset
    max_len = max(len(target), len(built))
    by_off_target = {off: (b, m) for off, b, m in target}
    by_off_built = {off: (b, m) for off, b, m in built}
    all_offs = sorted(set(by_off_target) | set(by_off_built),
                      key=lambda x: int(x, 16))
    differing = []
    for off in all_offs:
        t = by_off_target.get(off)
        b = by_off_built.get(off)
        if t != b:
            differing.append((off, t, b))
    if not differing:
        return "(no differences)"

    out_lines = [f"DIFFS: {len(differing)} instructions differ\n"]
    # Group nearby diffs for readability
    prev_off = -1
    for off, t, b in differing:
        off_int = int(off, 16)
        if prev_off >= 0 and off_int - prev_off > 32:
            out_lines.append("")  # blank between groups
        prev_off = off_int
        out_lines.append(f"  off=0x{off:>4s}")
        out_lines.append(f"    TARGET: {t[1] if t else '(none)':<40s} [{t[0] if t else ''}]")
        out_lines.append(f"    BUILT:  {b[1] if b else '(none)':<40s} [{b[0] if b else ''}]")
        # Try to highlight what changed (register name diff is the
        # most common)
        if t and b:
            ts, bs = t[1].split(), b[1].split()
            if len(ts) >= 1 and len(bs) >= 1 and ts[0] == bs[0]:
                # Same mnemonic; find which operand differs
                hint = []
                for i, (tt, bb_) in enumerate(zip(ts[1:], bs[1:])):
                    if tt != bb_:
                        hint.append(f"operand {i}: {tt!r} → {bb_!r}")
                if hint:
                    out_lines.append(f"    HINT: {', '.join(hint)}")
    return "\n".join(out_lines)


def summarize_register_swaps(target: list, built: list) -> str:
    """Tally register-name differences to identify a uniform swap pattern.
    If most diffs are `$t2 → $t3` and `$t3 → $t2`, that's a 2-way swap;
    suggests the regfix is a register-swap rule.
    """
    swaps = {}
    for (otarg, btarg, mtarg), (obuilt, bbuilt, mbuilt) in zip(target, built):
        if otarg != obuilt:
            continue
        if mtarg == mbuilt:
            continue
        ttoks = re.findall(r"\$[a-z0-9]+", mtarg)
        btoks = re.findall(r"\$[a-z0-9]+", mbuilt)
        for tt, bb_ in zip(ttoks, btoks):
            if tt != bb_:
                key = (tt, bb_)
                swaps[key] = swaps.get(key, 0) + 1
    if not swaps:
        return ""
    out = ["", "REGISTER DIFFERENCE TALLY:"]
    for (tt, bb_), n in sorted(swaps.items(), key=lambda x: -x[1]):
        out.append(f"  target {tt} → built {bb_}: {n} occurrences")
    # Detect uniform swap
    pairs = list(swaps.keys())
    pair_set = {frozenset(p) for p in pairs}
    if len(pair_set) == 1 and len(pairs) == 2:
        a, b = pairs[0]
        out.append(f"  ↳ uniform 2-way swap: {a} ↔ {b}")
        out.append(f"     (this is what a `$N <-> $M` regfix rule does;"
                   f" the pure-C fix is to pin a variable to {a} or {b})")
    return "\n".join(out)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("func")
    p.add_argument("--base", action="store_true",
                   help="Use permuter/<func>/base.c (default: use best output-*)")
    p.add_argument("--score", type=int, default=None,
                   help="Use output-<score>-1 specifically")
    args = p.parse_args()

    permdir = Path(f"permuter/{args.func}")
    if not permdir.exists():
        sys.exit(f"No permuter dir for {args.func}; run bb2_retire.py first")

    target_o = permdir / "target.o"
    compile_sh = permdir / "compile.sh"
    if not target_o.exists() or not compile_sh.exists():
        sys.exit(f"target.o or compile.sh missing in {permdir}")

    if args.score is not None:
        src_c = permdir / f"output-{args.score}-1" / "source.c"
        if not src_c.exists():
            sys.exit(f"{src_c} not found")
        print(f"[diag] using output-{args.score}-1/source.c")
        built_dump = compile_to_objdump(src_c, compile_sh)
    elif args.base:
        src_c = permdir / "base.c"
        print(f"[diag] using base.c")
        built_dump = compile_to_objdump(src_c, compile_sh)
    else:
        best = find_best_output_dir(permdir)
        if best is None:
            print(f"[diag] no output-*/ dirs; falling back to base.c")
            built_dump = compile_to_objdump(permdir / "base.c", compile_sh)
        else:
            print(f"[diag] using {best.name}/source.c")
            built_dump = compile_to_objdump(best / "source.c", compile_sh)

    target_dump = objdump_target(target_o)
    target = parse_objdump(target_dump)
    built = parse_objdump(built_dump)
    print()
    print(analyze_diff(target, built))
    print(summarize_register_swaps(target, built))


if __name__ == "__main__":
    main()
