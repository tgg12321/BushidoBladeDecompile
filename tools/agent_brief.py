#!/usr/bin/env python3
"""Print a complete agent context dump for a single function.

When an agent starts a session on a specific function, it needs:
  - Classification verdict (size, blockers, recommendation)
  - The target assembly
  - The current m2c-generated C
  - The live pipeline diff (gen_regfix output)
  - Recipe suggestions from the library
  - Kengo PS2-sequel reference if available
  - Prior tabled notes from archive/tabled_attempts/

Running each of these as separate `dc.sh` commands costs 5+ tool calls and
some token overhead per call. This combines them into one structured
report ready for the agent to read once and start work.

Usage:
    python3 tools/agent_brief.py <func>
    python3 tools/agent_brief.py <func> --no-asm     # skip the raw asm
    python3 tools/agent_brief.py <func> --json       # machine-readable

Designed to be the FIRST thing an agent runs on a function.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TOOLS = ROOT / "tools"
ASM_FUNCS = ROOT / "asm" / "funcs"
SRC_DIR = ROOT / "src"
PERMUTER = ROOT / "permuter"
TABLED = ROOT / "archive" / "tabled_attempts"


def run(cmd: list[str], timeout: int = 60) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, capture_output=True, text=True,
                          cwd=str(ROOT), timeout=timeout)


def section(title: str) -> str:
    return f"\n{'=' * 60}\n{title}\n{'=' * 60}\n"


def classify(func: str) -> tuple[dict, str]:
    """Returns (parsed dict, raw text)."""
    r = run([sys.executable, str(TOOLS / "classify_func.py"), func, "--json"], 30)
    try:
        d = json.loads(r.stdout) if r.stdout.strip() else {}
    except json.JSONDecodeError:
        d = {}
    raw = run([sys.executable, str(TOOLS / "classify_func.py"), func], 30).stdout
    return d, raw


def asm_body(func: str, max_lines: int | None = None) -> str:
    p = ASM_FUNCS / f"{func}.s"
    if not p.exists():
        return f"(no asm/funcs/{func}.s)"
    text = p.read_text(encoding="utf-8")
    if max_lines and len(text.splitlines()) > max_lines:
        lines = text.splitlines()
        head = "\n".join(lines[:max_lines])
        return f"{head}\n... ({len(lines) - max_lines} more lines truncated; full file at asm/funcs/{func}.s)"
    return text


def base_c(func: str) -> tuple[str, bool]:
    """Returns (text, is_post_processed). Strips the typedef preamble for
    readability — the agent already knows what u8/s32/GameObj/etc are."""
    p = PERMUTER / func / "base.c"
    if not p.exists():
        return f"(no permuter/{func}/base.c -- run `dc.sh inline-setup` or `dc.sh attempt`)", False
    text = p.read_text(encoding="utf-8")

    # Heuristic: drop everything up to the first non-typedef function decl.
    lines = text.splitlines()
    # Find the first line that looks like a function decl: `<type> <name>(args)`
    func_start = None
    for i, line in enumerate(lines):
        if re.match(rf"^[\w\s\*]+\b{re.escape(func)}\s*\(", line):
            func_start = i
            break
    if func_start is None:
        return text, False

    # Walk back to keep any extern decls right above the function
    keep_from = func_start
    while keep_from > 0:
        prev = lines[keep_from - 1].strip()
        if prev.startswith("extern ") or prev.startswith("/* "):
            keep_from -= 1
        else:
            break

    return "\n".join(lines[keep_from:]), True


def gen_regfix(func: str) -> str:
    r = run([sys.executable, str(TOOLS / "gen_regfix.py"), func], 60)
    return r.stdout if r.returncode == 0 else f"(gen_regfix failed: {r.stderr.strip()[:200]})"


def recipe_suggestions(func: str) -> str:
    r = run([sys.executable, str(TOOLS / "recipes.py"), "suggest", func], 30)
    return r.stdout.strip() or "(no recipe matches)"


def kengo_ref(func: str) -> str:
    kr = TOOLS / "kengo_ref.py"
    if not kr.exists():
        return ""
    r = run([sys.executable, str(kr), func, "--bb2"], 15)
    text = (r.stdout or r.stderr).strip()
    if not text or "no kengo match" in text.lower() or "not found" in text.lower():
        return ""
    return text


def tabled_notes(func: str) -> str:
    if not TABLED.exists():
        return ""
    matches = list(TABLED.glob(f"{func}_*.md")) + list(TABLED.glob(f"{func}_*.c"))
    if not matches:
        return ""
    parts = []
    for p in sorted(matches):
        parts.append(f"--- {p.relative_to(ROOT)} ---")
        try:
            parts.append(p.read_text(encoding="utf-8", errors="replace"))
        except Exception as e:
            parts.append(f"(error reading: {e})")
    return "\n".join(parts)


def existing_regfix(func: str) -> str:
    p = ROOT / "regfix.txt"
    if not p.exists():
        return ""
    out = []
    for line in p.read_text(encoding="utf-8").splitlines():
        if line.startswith(f"{func}:") or line.startswith(f"# {func}:"):
            out.append(line)
    return "\n".join(out)


def src_neighbors(func: str, src_relpath: str | None) -> str:
    """Show the function names immediately before and after this one in src/.
    Helps an agent figure out the surrounding subsystem for naming."""
    if not src_relpath:
        return ""
    p = ROOT / src_relpath
    if not p.exists():
        return ""
    text = p.read_text(encoding="utf-8", errors="ignore")
    func_decl_re = re.compile(
        r"^(?:static\s+)?(?:[\w_]+\s*\*?\s+){1,3}(\w+)\s*\([^;]*\)\s*\{",
        re.MULTILINE,
    )
    glabel_re = re.compile(r"glabel\s+(\w+)")

    funcs_in_order: list[str] = []
    for m in re.finditer(
        rf"(?:{func_decl_re.pattern})|(?:glabel\s+(\w+))",
        text, re.MULTILINE,
    ):
        n = m.group(1) or m.group(2)
        if n:
            funcs_in_order.append(n)

    if func not in funcs_in_order:
        return ""
    i = funcs_in_order.index(func)
    before = funcs_in_order[max(0, i - 3):i]
    after = funcs_in_order[i + 1:i + 4]
    return f"prev: {', '.join(before) or '(start of file)'}\nnext: {', '.join(after) or '(end of file)'}"


def build_brief(func: str, include_asm: bool = True,
                asm_max_lines: int = 200) -> dict:
    cls_dict, cls_raw = classify(func)
    src_kind = cls_dict.get("src", {}).get("kind") if cls_dict else None
    src_relpath = cls_dict.get("src", {}).get("file") if cls_dict else None

    return {
        "func": func,
        "classify": cls_raw.strip(),
        "classify_dict": cls_dict,
        "asm_body": asm_body(func, asm_max_lines if include_asm else 0)
                    if include_asm else "(skipped via --no-asm)",
        "base_c": base_c(func)[0],
        "gen_regfix": gen_regfix(func).strip(),
        "recipe_suggestions": recipe_suggestions(func),
        "kengo": kengo_ref(func),
        "tabled_notes": tabled_notes(func),
        "existing_regfix_rules": existing_regfix(func),
        "src_neighbors": src_neighbors(func, src_relpath),
    }


def render_text(b: dict) -> str:
    out = []
    out.append(f"AGENT BRIEF: {b['func']}")

    out.append(section("Classification"))
    out.append(b["classify"] or "(classify failed)")

    if b["src_neighbors"]:
        out.append(section("Source neighbors (subsystem context)"))
        out.append(b["src_neighbors"])

    if b["kengo"]:
        out.append(section("Kengo PS2 sequel reference"))
        out.append(b["kengo"])

    if b["tabled_notes"]:
        out.append(section("Prior tabled-attempt notes"))
        out.append(b["tabled_notes"])

    if b["existing_regfix_rules"]:
        out.append(section("Existing regfix rules for this function"))
        out.append(b["existing_regfix_rules"])

    out.append(section("Target assembly"))
    out.append(b["asm_body"])

    out.append(section("m2c output (current permuter/<func>/base.c)"))
    out.append(b["base_c"])

    out.append(section("Live pipeline diff (gen_regfix)"))
    out.append(b["gen_regfix"])

    if b["recipe_suggestions"] and "no recipe matches" not in b["recipe_suggestions"]:
        out.append(section("Recipe library suggestions"))
        out.append(b["recipe_suggestions"])

    out.append(section("Standard workflow"))
    out.append("\n".join([
        "  dc.sh attempt <func>            # full mechanical pipeline (classify -> setup -> smart -> permute -> regfix)",
        "  dc.sh near-miss <func> --apply  # auto-converter (byte_arith / drain_delay / plain reg substs)",
        "  dc.sh debug permuter/<func>     # full permuter --debug diff",
        "  dc.sh add-regfix <func> <op> <args>  # validated regfix rule editor",
        "  dc.sh inline-replace <func> <c>      # integrate matched C body into src/",
        "  dc.sh verify <func>             # binary-level verify against original",
    ]))
    return "\n".join(out)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func")
    ap.add_argument("--no-asm", action="store_true",
                    help="Skip the raw target assembly section")
    ap.add_argument("--asm-max-lines", type=int, default=200,
                    help="Truncate asm if longer than N lines (default: 200)")
    ap.add_argument("--json", action="store_true",
                    help="Emit a machine-readable JSON dict instead of text")
    args = ap.parse_args()

    b = build_brief(args.func, include_asm=not args.no_asm,
                    asm_max_lines=args.asm_max_lines)
    if args.json:
        print(json.dumps(b, indent=2))
    else:
        print(render_text(b))
    return 0


if __name__ == "__main__":
    sys.exit(main())
