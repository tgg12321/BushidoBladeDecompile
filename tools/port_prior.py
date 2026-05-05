#!/usr/bin/env python3
"""port_prior.py -- splice prior-worker src body + regfix rules into current worktree.

Used by `dc.sh port-prior <func> <prior_branch>` for the deep-retry
quick-port path. Audit found this is 5-10x faster than re-running the
search (9-21min vs 90-150min).

What it does:
  1. Read prior branch's src/<file>.c, extract the function body
     for <func> (from first definition to matching closing brace).
  2. Splice it into the current worktree's src/<file>.c, replacing
     the existing INCLUDE_ASM / inline_asm stub for that function.
  3. Read prior branch's regfix.txt, extract lines starting with
     "<func>:" or "# <func>:" (function-specific rules + comment).
  4. Append those rules to the current regfix.txt if not already
     present (idempotent on rule text).
  5. Print a summary of what was applied.

The caller (dc.sh) then runs fix-label-drift, build, verify.
"""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def git_show(branch: str, path: str) -> str:
    """Read a file's content from a git branch."""
    try:
        out = subprocess.check_output(
            ["git", "show", f"{branch}:{path}"],
            cwd=ROOT, text=True, errors="replace"
        )
        return out
    except subprocess.CalledProcessError:
        return ""


def extract_function_body(src: str, func: str) -> str | None:
    """Find the function definition for <func> and return source up to its
    closing brace. Heuristic: look for `<word>+ <func>(` (return type then
    name then open paren), then count braces.
    """
    # Match: typically "s32 func_X(...)" or "void func_X(" or "static s32 func_X("
    # Allow leading whitespace; require word-boundary before func name.
    pattern = re.compile(
        rf"^(?P<sig>(?:[A-Za-z_][\w*\s]*\s+)?{re.escape(func)}\s*\([^)]*\)\s*\{{)",
        re.MULTILINE
    )
    m = pattern.search(src)
    if not m:
        return None
    start = m.start()
    # Walk braces from the opening { we already matched
    depth = 0
    i = m.end() - 1  # position of the opening {
    while i < len(src):
        c = src[i]
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return src[start:i + 1]
        i += 1
    return None


def extract_function_externs(src: str, func: str) -> list[str]:
    """Find extern declarations near the function (within 10 lines above
    the definition). Workers often add forward decls / typedefs.
    """
    pattern = re.compile(
        rf"^(?:[A-Za-z_][\w*\s]*\s+)?{re.escape(func)}\s*\([^)]*\)\s*\{{",
        re.MULTILINE
    )
    m = pattern.search(src)
    if not m:
        return []
    start = m.start()
    # Walk back up to 30 lines collecting `extern ` and `typedef ` lines
    upstream = src[:start]
    lines = upstream.split("\n")
    nearby = lines[-30:]
    return [l for l in nearby if re.match(r"^(extern\s|typedef\s)", l)]


def replace_function_in_file(file_path: Path, func: str, new_body: str) -> bool:
    """Replace the function (or its INCLUDE_ASM stub) with new_body."""
    text = file_path.read_text(encoding="utf-8", errors="replace")

    # Try INCLUDE_ASM stub first
    stub_pat = re.compile(
        rf'INCLUDE_ASM\s*\(\s*"asm/funcs"\s*,\s*{re.escape(func)}\s*\)\s*;'
    )
    if stub_pat.search(text):
        text = stub_pat.sub(new_body, text)
        file_path.write_text(text, encoding="utf-8", newline="\n")
        return True

    # Try replacing existing C body or inline_asm body
    pattern = re.compile(
        rf"^(?P<sig>(?:[A-Za-z_][\w*\s]*\s+)?{re.escape(func)}\s*\([^)]*\)\s*\{{)",
        re.MULTILINE
    )
    m = pattern.search(text)
    if not m:
        return False
    start = m.start()
    depth = 0
    i = m.end() - 1
    while i < len(text):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                end = i + 1
                text = text[:start] + new_body + text[end:]
                file_path.write_text(text, encoding="utf-8", newline="\n")
                return True
        i += 1
    return False


def extract_regfix_rules(text: str, func: str) -> list[str]:
    """Pull all lines that target <func> from a regfix.txt-style file."""
    rules = []
    for line in text.split("\n"):
        stripped = line.strip()
        if stripped.startswith(f"{func}:") or stripped.startswith(f"# {func}:"):
            rules.append(line)
    return rules


def main() -> int:
    if len(sys.argv) < 4:
        print(f"Usage: {sys.argv[0]} <func> <prior_branch> <src_file>", file=sys.stderr)
        return 1
    func = sys.argv[1]
    prior_branch = sys.argv[2]
    src_file = sys.argv[3]

    # 1. Get prior src file content
    prior_src = git_show(prior_branch, src_file)
    if not prior_src:
        print(f"ERROR: could not read {src_file} from {prior_branch}", file=sys.stderr)
        return 1

    # 2. Extract the function body
    body = extract_function_body(prior_src, func)
    if not body:
        print(f"ERROR: could not find {func} definition in prior {src_file}", file=sys.stderr)
        return 1
    print(f"[port_prior] extracted {len(body)} chars of {func} body from {prior_branch}")

    # 3. Replace in current worktree
    target_path = ROOT / src_file
    if not target_path.exists():
        print(f"ERROR: {target_path} not found", file=sys.stderr)
        return 1
    if replace_function_in_file(target_path, func, body):
        print(f"[port_prior] spliced {func} into {src_file}")
    else:
        print(f"WARN: could not splice {func} into {src_file} (no stub found)", file=sys.stderr)
        # Continue -- regfix rules might still be useful

    # 4. Append regfix rules if not already there
    prior_regfix = git_show(prior_branch, "regfix.txt")
    if prior_regfix:
        new_rules = extract_regfix_rules(prior_regfix, func)
        if new_rules:
            current = (ROOT / "regfix.txt").read_text(encoding="utf-8", errors="replace")
            to_add = [r for r in new_rules if r not in current]
            if to_add:
                with open(ROOT / "regfix.txt", "a", encoding="utf-8", newline="\n") as f:
                    f.write("\n# port-prior: rules from " + prior_branch + "\n")
                    for r in to_add:
                        f.write(r + "\n")
                print(f"[port_prior] appended {len(to_add)} regfix rules ({len(new_rules) - len(to_add)} already present)")
            else:
                print(f"[port_prior] all {len(new_rules)} rules already present in regfix.txt")

    print(f"[port_prior] done. Now run: bash tools/dc.sh fix-label-drift --apply && make && bash tools/dc.sh verify {func}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
