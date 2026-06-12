#!/usr/bin/env python3
"""Score N candidate bodies for one function in a single invocation.

Promotes the ad-hoc per-session pattern (tmp/sys_vsync_variants*.py, sessions
5-7 of the sys_VSync grind) into a reusable tool: instead of an agent doing
edit -> `sandbox` -> read -> edit (one full agent turn + context reload per
variant, measured 12-20x per worker session), the agent writes each candidate
form to a file and sweeps them all in ONE tool call.

Usage (run under WSL, venv active — same env as `python3 -m engine.cli`):

  python3 tools/sweep_variants.py --func sys_VSync --file ings2 \
      --variants tmp/vsync_variants/            # dir of *.c candidates
  python3 tools/sweep_variants.py --func sys_VSync --file ings2 \
      --variants v1.c v2.c v3.c                 # explicit files
  ... [--apply-best] [--json]

Each candidate file holds the COMPLETE replacement function definition
(signature through closing brace; an optional leading block comment is
spliced along with it). The tool:

  1. Locates the function's current span in src/<file>.c (signature line +
     brace matching).
  2. For each candidate: splices it in, runs
     `python3 -m engine.cli sandbox <func> --disable all`, records the score.
  3. ALWAYS restores src/<file>.c byte-exact afterwards (finally-guarded),
     unless --apply-best is given AND at least one candidate scored strictly
     better than the baseline — then the best candidate is left applied.

The sandbox builds into tmp/ and never touches build/, so a sweep is safe to
run mid-session with a clean or dirty tree. Scores are the same
cheat-stripped honest distance the loop uses as its gradient.

REMINDER (cheat policy): a low score is a PROPOSAL, not a winner — vet every
closing form against the cheat catalog before adopting it
([[no-new-park-categories]] "Auto-search tools" section applies to this tool
exactly as it does to the permuter).
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def find_function_span(text: str, func: str) -> tuple[int, int]:
    """Return (start, end) char offsets of the function's definition in text.

    start = beginning of the signature line; end = char after the closing
    brace (+ trailing newline). Brace matching skips string/char literals and
    comments. Raises ValueError if not found or unbalanced.
    """
    # Signature line: <func> followed by ( on a line that doesn't end in ';'
    # and isn't a call (must start a definition: return type or name at col 0
    # area). We search for a line containing `<func>(` or `<func> (` whose
    # statement ends with `{` before a `;`.
    sig_re = re.compile(
        r"^[ \t]*[A-Za-z_][\w \t\*]*\b" + re.escape(func) + r"[ \t]*\(",
        re.MULTILINE,
    )
    for m in sig_re.finditer(text):
        # Find the opening brace of the body (skip the parameter list).
        i = m.end() - 1  # at '('
        depth_paren = 0
        n = len(text)
        while i < n:
            c = text[i]
            if c == "(":
                depth_paren += 1
            elif c == ")":
                depth_paren -= 1
                if depth_paren == 0:
                    i += 1
                    break
            i += 1
        # After params: skip whitespace/comments; expect '{' (definition) or
        # ';' (prototype -> keep searching).
        while i < n and text[i] in " \t\r\n":
            i += 1
        if i >= n or text[i] != "{":
            continue  # prototype or macro use; try next match
        # Brace-match the body.
        depth = 0
        j = i
        in_str = in_chr = in_line_comment = in_block_comment = False
        while j < n:
            c = text[j]
            nxt = text[j + 1] if j + 1 < n else ""
            if in_line_comment:
                if c == "\n":
                    in_line_comment = False
            elif in_block_comment:
                if c == "*" and nxt == "/":
                    in_block_comment = False
                    j += 1
            elif in_str:
                if c == "\\":
                    j += 1
                elif c == '"':
                    in_str = False
            elif in_chr:
                if c == "\\":
                    j += 1
                elif c == "'":
                    in_chr = False
            elif c == "/" and nxt == "/":
                in_line_comment = True
                j += 1
            elif c == "/" and nxt == "*":
                in_block_comment = True
                j += 1
            elif c == '"':
                in_str = True
            elif c == "'":
                in_chr = True
            elif c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    end = j + 1
                    if end < n and text[end] == "\n":
                        end += 1
                    return m.start(), end
            j += 1
        raise ValueError(f"unbalanced braces matching {func}")
    raise ValueError(f"definition of {func} not found")


def run_sandbox(func: str) -> dict:
    p = subprocess.run(
        [sys.executable, "-m", "engine.cli", "sandbox", func, "--disable", "all"],
        capture_output=True, text=True, cwd=ROOT,
    )
    try:
        return json.loads(p.stdout)
    except json.JSONDecodeError:
        return {"score": None, "error": (p.stdout + p.stderr)[-400:]}


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--func", required=True)
    ap.add_argument("--file", required=True, help="src stem, e.g. ings2")
    ap.add_argument("--variants", nargs="+", required=True,
                    help="candidate .c files, or one directory of them")
    ap.add_argument("--apply-best", action="store_true",
                    help="leave the best strictly-improving candidate applied "
                         "(default: restore the original source)")
    ap.add_argument("--json", action="store_true", help="machine output")
    a = ap.parse_args()

    src = ROOT / "src" / f"{a.file}.c"
    if not src.exists():
        print(f"no such file: {src}", file=sys.stderr)
        return 2

    paths: list[Path] = []
    for v in a.variants:
        p = Path(v)
        if p.is_dir():
            paths.extend(sorted(p.glob("*.c")))
        elif p.exists():
            paths.append(p)
        else:
            print(f"skipping missing variant: {p}", file=sys.stderr)
    if not paths:
        print("no candidate files found", file=sys.stderr)
        return 2

    original = src.read_bytes()
    text = original.decode("utf-8", errors="replace")
    try:
        start, end = find_function_span(text, a.func)
    except ValueError as e:
        print(f"error: {e}", file=sys.stderr)
        return 2

    results = []
    best = None
    try:
        baseline = run_sandbox(a.func)
        results.append({"variant": "<baseline>", "score": baseline.get("score"),
                        "build_insns": baseline.get("build_insns")})
        for p in paths:
            body = p.read_text(encoding="utf-8", errors="replace")
            if not body.endswith("\n"):
                body += "\n"
            new_text = text[:start] + body + text[end:]
            src.write_bytes(new_text.encode("utf-8"))
            r = run_sandbox(a.func)
            rec = {"variant": str(p), "score": r.get("score"),
                   "build_insns": r.get("build_insns")}
            if r.get("error"):
                rec["error"] = r["error"]
            results.append(rec)
            if (rec["score"] is not None
                    and (best is None or rec["score"] < best["score"])):
                best = rec
    finally:
        keep = (a.apply_best and best is not None
                and baseline.get("score") is not None
                and best["score"] < baseline["score"])
        if keep:
            body = Path(best["variant"]).read_text(encoding="utf-8",
                                                   errors="replace")
            if not body.endswith("\n"):
                body += "\n"
            src.write_bytes((text[:start] + body + text[end:]).encode("utf-8"))
        else:
            src.write_bytes(original)

    results.sort(key=lambda r: (r["score"] is None, r["score"]))
    if a.json:
        print(json.dumps({"func": a.func, "file": a.file, "results": results,
                          "applied": best["variant"] if keep else None}, indent=2))
    else:
        for r in results:
            score = "ERR " if r["score"] is None else f"{r['score']:4d}"
            print(f"  {score}  bi={r.get('build_insns')}  {r['variant']}"
                  + (f"  [{r['error']}]" if r.get("error") else ""))
        if keep:
            print(f"applied best variant to src/{a.file}.c: {best['variant']}")
        else:
            print("source restored (use --apply-best to keep an improving winner)")
        print("NOTE: a low score is a PROPOSAL — vet the form against the cheat "
              "catalog before adopting (no-new-park-categories §auto-search).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
