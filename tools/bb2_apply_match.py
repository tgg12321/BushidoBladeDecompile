#!/usr/bin/env python3
"""bb2_apply_match.py -- apply a permuter score=0 match back to src/.

Given a backlog function that the permuter matched, this:
  1. Reads permuter/<func>/output-0-*/source.c (the matching candidate)
  2. Locates the function definition in src/<file>.c
  3. Replaces the function body with the permuter's version
  4. (Optional) comments out the regfix rule(s) for the function

Use: bb2_apply_match.py <func> <src/file.c> [--retire-regfix]

The user should manually inspect the diff and re-run `make` to verify
SHA1 still matches before committing.
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path


def read_lf(path: Path) -> str:
    return path.read_bytes().decode("utf-8", errors="ignore")


def write_lf(path: Path, content: str) -> None:
    path.write_bytes(content.encode("utf-8"))


def extract_func_from_source_c(text: str, func_name: str) -> str | None:
    """Extract the function body (including signature) from a permuter
    output's source.c file by depth-tracking the braces.
    """
    lines = text.split("\n")
    pat = re.compile(rf"\b{re.escape(func_name)}\s*\(")
    start = None
    for i, line in enumerate(lines):
        if pat.search(line) and "{" not in line:
            if line.rstrip().endswith(";"):
                continue
            start = i
            break
        elif pat.search(line) and "{" in line and "(" in line:
            if line.rstrip().endswith(";"):
                continue
            start = i
            break
    if start is None:
        return None

    # Find first { from start
    brace = None
    for j in range(start, min(start + 10, len(lines))):
        if "{" in lines[j]:
            brace = j
            break
    if brace is None:
        return None

    depth = 0
    in_str = False
    in_chr = False
    end = None
    for j in range(brace, len(lines)):
        line = lines[j]
        for ch in line:
            if in_str:
                if ch == '"':
                    in_str = False
                continue
            if in_chr:
                if ch == "'":
                    in_chr = False
                continue
            if ch == '"':
                in_str = True
            elif ch == "'":
                in_chr = True
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    end = j
                    break
        if end is not None:
            break
    if end is None:
        return None
    return "\n".join(lines[start:end + 1])


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("func")
    p.add_argument("src_file")
    p.add_argument("--retire-regfix", action="store_true",
                   help="Comment out the function's regfix rules in regfix.txt")
    p.add_argument("--permuter-dir", default=None,
                   help="Override permuter dir (default: permuter/<func>)")
    args = p.parse_args()

    pdir = Path(args.permuter_dir) if args.permuter_dir else Path(f"permuter/{args.func}")
    src_path = Path(args.src_file)
    if not src_path.exists():
        sys.exit(f"src file not found: {src_path}")

    # Find output-0-N
    matches = sorted(pdir.glob("output-0-*"))
    if not matches:
        sys.exit(f"No score=0 output in {pdir} -- nothing to apply")
    match_dir = matches[0]
    print(f"Using match: {match_dir}")

    match_source = read_lf(match_dir / "source.c")
    matched_body = extract_func_from_source_c(match_source, args.func)
    if matched_body is None:
        sys.exit(f"Could not extract {args.func} from {match_dir}/source.c")

    src_text = read_lf(src_path)
    current_body = extract_func_from_source_c(src_text, args.func)
    if current_body is None:
        sys.exit(f"Could not find {args.func} in {src_path}")

    if current_body == matched_body:
        print("src body already matches; nothing to apply.")
    else:
        print(f"  Replacing {len(current_body.split(chr(10)))}-line body "
              f"with {len(matched_body.split(chr(10)))}-line matched version")
        src_text = src_text.replace(current_body, matched_body, 1)
        write_lf(src_path, src_text)
        print(f"  Updated {src_path}")

    if args.retire_regfix:
        regfix_path = Path("regfix.txt")
        rf = read_lf(regfix_path)
        retired = 0
        new_lines = []
        for line in rf.split("\n"):
            stripped = line.lstrip()
            if stripped.startswith(f"{args.func}:") and not stripped.startswith("#"):
                new_lines.append(f"# RETIRED (targeted-permuter): {line}")
                retired += 1
            else:
                new_lines.append(line)
        if retired > 0:
            write_lf(regfix_path, "\n".join(new_lines))
            print(f"  Retired {retired} regfix rule(s) for {args.func}")
        else:
            print(f"  No active regfix rules found for {args.func}")

    print()
    print("Next steps:")
    print(f"  1. Inspect diff: git diff {src_path}")
    if args.retire_regfix:
        print(f"  2. Inspect regfix: git diff regfix.txt")
    print(f"  3. rm -f build/src/$(basename {src_path} .c).o && make")
    print(f"  4. If 'bb2 matches!': git add ... && git commit")


if __name__ == "__main__":
    main()
