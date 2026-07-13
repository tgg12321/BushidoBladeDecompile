#!/usr/bin/env python3
"""memory-check — surface project memory + prior-work context for one
function, before any decomp work starts.

The decomp-next skill's §2.5 calls this BEFORE launching the §3 pipeline.
The goal is to catch two failure modes:

  1. Agent re-derives a pattern that's already documented somewhere
     (gotcha, recipe, prior incident with this function or its cluster).
  2. Agent spends N attempts pure-C-matching a function whose original
     source was hand-coded asm — wasted work in a direction the
     toolchain cannot reach.

Output is concise and structured so the agent can act on it:
  - CANONICAL_ASM yes/no (from inline_asm_canonical.txt)
  - Direct memory hits on the function name
  - Sibling memory hits (neighbors in same .c file)
  - Pointer to feedback_hand_coded_asm_recognition.md for the
    asm-canonical determination criteria.

The DECISION (C vs asm) stays in the skill — this tool just surfaces
data. Skill §2.5 has the rules.
"""

from __future__ import annotations
import argparse
import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))

INLINE_ASM_CANONICAL = ROOT / "inline_asm_canonical.txt"


def candidate_memory_dirs() -> list[Path]:
    """Memory lives outside the project tree (user-level Claude config,
    Windows-side). Try several common locations; allow override via env var.

    From WSL, the Windows home is at /mnt/c/Users/<user>/. From native
    Windows Python, it's at C:\\Users\\<user>\\. Path.home() returns the
    WSL home (e.g. /home/trenton) when invoked from WSL — wrong for our
    case. Look in both places."""
    if env := os.environ.get("BB2_MEMORY_DIR"):
        return [Path(env)]
    project_dir = "C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile"
    candidates = [
        # Windows-side path (works for native Windows Python AND from WSL)
        Path("/mnt/c/Users/Trenton/.claude/projects") / project_dir / "memory",
        Path("C:/Users/Trenton/.claude/projects") / project_dir / "memory",
        # WSL home (fallback if user kept memory inside WSL)
        Path.home() / ".claude" / "projects" / project_dir / "memory",
    ]
    return [p for p in candidates if p.exists()]


def check_canonical_asm(func: str) -> str | None:
    """Returns the canonical-asm justification line if `func` is listed,
    else None."""
    if not INLINE_ASM_CANONICAL.exists():
        return None
    for line in INLINE_ASM_CANONICAL.read_text(
        encoding="utf-8", errors="ignore"
    ).splitlines():
        s = line.strip()
        if not s or s.startswith("#"):
            continue
        name = s.split("#", 1)[0].strip()
        if name == func:
            return s
    return None


def get_neighbors(func: str) -> list[str]:
    """Use agent_brief's neighbor logic to find functions in the same
    .c file. Returns up to 6 names (3 before, 3 after)."""
    try:
        from agent_brief import classify, src_neighbors
    except ImportError:
        return []
    cls, _ = classify(func)
    if not cls:
        return []
    src = cls.get("src", {}).get("file")
    if not src:
        return []
    text = src_neighbors(func, src)
    names: list[str] = []
    for line in text.splitlines():
        for n in re.findall(r"\b([A-Za-z_][A-Za-z0-9_]*)\b", line):
            if n in ("prev", "next", "start", "of", "file", "end"):
                continue
            if n == func:
                continue
            if n not in names:
                names.append(n)
    return names


def grep_memory_files(needle: str, dirs: list[Path]) -> list[tuple[Path, int]]:
    """Return [(file, hit_count)] for memory files mentioning `needle`."""
    out: list[tuple[Path, int]] = []
    for d in dirs:
        for f in sorted(d.glob("*.md")):
            try:
                t = f.read_text(encoding="utf-8", errors="ignore")
            except OSError:
                continue
            count = t.count(needle)
            if count:
                out.append((f, count))
        # Also scan one level of subdirectories (e.g. archive/)
        for sub in sorted(d.iterdir()):
            if sub.is_dir():
                for f in sorted(sub.glob("*.md")):
                    try:
                        t = f.read_text(encoding="utf-8", errors="ignore")
                    except OSError:
                        continue
                    count = t.count(needle)
                    if count:
                        out.append((f, count))
    return out


def first_context_line(path: Path, needle: str, max_chars: int = 140) -> str:
    """Pull the first non-blank line containing `needle` for quick context."""
    try:
        for line in path.read_text(encoding="utf-8", errors="ignore").splitlines():
            if needle in line:
                s = line.strip()
                if not s or s == needle:
                    continue
                if len(s) > max_chars:
                    s = s[: max_chars - 1] + "…"
                return s
    except OSError:
        pass
    return ""


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("func")
    ap.add_argument(
        "--quiet",
        action="store_true",
        help="suppress the trailing decision-rules block (just data)",
    )
    args = ap.parse_args()

    func = args.func
    mem_dirs = candidate_memory_dirs()

    print(f"=== memory-check: {func} ===")
    print()

    # 1. inline_asm_canonical.txt
    canonical = check_canonical_asm(func)
    if canonical:
        print("CANONICAL_ASM: yes")
        print(f"  {canonical}")
        print()
        print("  This function is already authorized as canonically hand-coded asm.")
        print("  Skip the §3 programmatic pipeline (it's tuned for pure-C matching).")
        print("  Pursue inline-asm-via-glabel/endlabel (see feedback_hand_coded_asm_recognition.md).")
        print()
    else:
        print("CANONICAL_ASM: no  (not listed in inline_asm_canonical.txt)")
        print()

    # 2. Automated hand-coded-asm signal scan (data-driven §2.5.b)
    try:
        from scan_hand_coded import analyze_one, fmt_signals_human, ASM_DIR
        s = analyze_one(ASM_DIR, func)
        if s is not None:
            print(fmt_signals_human(s))
            print()
            if s.tier == "STRONG" and not canonical:
                print("  >>> STRONG tier + NOT on inline_asm_canonical.txt.")
                print("      This is the case the §2.5.b checklist exists for.")
                print("      Confirm signals by reading asm/funcs/<func>.s, then")
                print("      surface evidence to the user and request authorization.")
                print()
            elif s.tier == "TIGHT_C":
                print("  >>> TIGHT_C tier. This is a pure-C function (probably part")
                print("      of a C cluster like GTE 3x3 or calc_fc_frame family).")
                print("      Proceed to §3 with extra attention to sibling templates.")
                print()
    except Exception as e:
        print(f"(hand-coded-asm scan unavailable: {e})")
        print()

    if not mem_dirs:
        print("MEMORY_DIR: not found")
        print("  Set BB2_MEMORY_DIR env var if memory lives elsewhere.")
        print("  Proceeding without memory grep — agent should still apply §2.5.b.")
        return 0

    print(f"MEMORY_DIR: {mem_dirs[0]}")
    print()

    # 2. Direct hits on function name
    direct = grep_memory_files(func, mem_dirs)
    if direct:
        print(f"Direct hits ({len(direct)} file{'s' if len(direct)!=1 else ''}):")
        for f, count in direct:
            rel = f.name if f.parent == mem_dirs[0] else f.relative_to(mem_dirs[0])
            ctx = first_context_line(f, func)
            mark = " *" if count > 1 else ""
            print(f"  {rel}{mark}")
            if ctx:
                print(f"      {ctx}")
        print()
    else:
        print("Direct hits: none")
        print()

    # 3. Sibling hits (neighbors in same .c file)
    neighbors = get_neighbors(func)
    if neighbors:
        print(f"Neighbors in same file: {', '.join(neighbors)}")
        sibling_hits: dict[str, list[str]] = {}
        for n in neighbors:
            hits = grep_memory_files(n, mem_dirs)
            if hits:
                sibling_hits[n] = [
                    h[0].name if h[0].parent == mem_dirs[0]
                    else str(h[0].relative_to(mem_dirs[0]))
                    for h in hits
                ]
        if sibling_hits:
            print("Sibling memory hits:")
            for n, files in sibling_hits.items():
                print(f"  {n}:")
                for fn in files:
                    print(f"    {fn}")
        else:
            print("Sibling memory hits: none")
        print()
    else:
        print("Neighbors: (could not resolve)")
        print()

    if args.quiet:
        return 0

    # 4. Decision-rules block — keep this brief; skill §2.5 has full text.
    print("--- Canonical-form determination (skill §2.5.b) ---")
    print()
    if canonical:
        print("  Decision: ASM (already authorized). Use inline-asm path.")
    else:
        print("  Default: C-canonical. Run §3 programmatic pipeline.")
        print()
        print("  Promote to asm-canonical ONLY if 3+ strong signals are present")
        print("  (see memory/feedback_hand_coded_asm_recognition.md):")
        print("    1. Uniform 2-cycle multu/mflo pacing on EVERY pair")
        print("    2. Front-loaded loads (≥4 in first few insns of long kernel)")
        print("    3. Tight register packing across 60+ insns, no spills")
        print("    4. Idiom signatures GCC doesn't emit (INT_MIN guard with empty body, etc.)")
        print("    5. Cluster behavior (siblings share skeletal pattern w/ operand diffs)")
        print()
        print("  3+ signals → STOP, surface evidence to user, request authorization.")
        print("  Do not pre-authorize yourself.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
