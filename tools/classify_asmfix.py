#!/usr/bin/env python3
"""Classify the 72 `replace_with_asmfile` functions from asmfix.txt and
append the rows to tmp/batch_attempt.csv. Run after batch_attempt
--classify-only to extend the CSV with asmfix entries (which are not
inline_asm or INCLUDE_ASM and so are missed by batch_attempt).
"""
from __future__ import annotations

import csv
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TOOLS = ROOT / "tools"
ASM_FUNCS = ROOT / "asm" / "funcs"
ASMFIX = ROOT / "asmfix.txt"
CSV_PATH = ROOT / "tmp" / "batch_attempt.csv"


def find_src(func: str) -> str:
    src_dir = ROOT / "src"
    pat = re.compile(rf"\b{re.escape(func)}\b")
    for c in sorted(src_dir.glob("*.c")):
        try:
            text = c.read_text(encoding="utf-8", errors="ignore")
        except Exception:
            continue
        if pat.search(text):
            return str(c.relative_to(ROOT))
    return ""


def asm_size(func: str) -> int:
    p = ASM_FUNCS / f"{func}.s"
    if not p.exists():
        return 0
    return sum(
        1
        for line in p.read_text(encoding="utf-8").splitlines()
        if "/* " in line and " */" in line
    )


def classify(func: str) -> tuple[str, str]:
    """Returns (recommendation, blocker_tags). Falls back to 'standard' on
    any error."""
    try:
        out = subprocess.check_output(
            ["python3", str(TOOLS / "classify_func.py"), func, "--json"],
            stderr=subprocess.DEVNULL,
            cwd=str(ROOT),
            timeout=30,
        ).decode()
        import json

        data = json.loads(out)
        rec = data.get("recommendation", "standard")
        tags = ",".join(data.get("blocker_tags", []) or [])
        return rec, tags
    except Exception:
        return "standard", ""


def main() -> int:
    if not ASMFIX.exists():
        print(f"ERROR: {ASMFIX} missing", file=sys.stderr)
        return 1

    text = ASMFIX.read_text(encoding="utf-8")
    funcs = re.findall(r"^(\w+):\s+replace_with_asmfile", text, re.MULTILINE)
    if not funcs:
        print("No replace_with_asmfile entries found.", file=sys.stderr)
        return 0

    existing: set[str] = set()
    if CSV_PATH.exists():
        existing = {r["func"] for r in csv.DictReader(open(CSV_PATH))}

    new_rows = []
    for i, func in enumerate(funcs, 1):
        if func in existing:
            print(f"[{i}/{len(funcs)}] {func} -- already in CSV, skip")
            continue
        src = find_src(func)
        size = asm_size(func)
        rec, tags = classify(func)
        print(
            f"[{i}/{len(funcs)}] {func:30s} size={size:4d}  rec={rec:30s}  tags={tags}"
        )
        new_rows.append(
            {
                "func": func,
                "src": src,
                "kind": "asmfix",
                "result": "CLASSIFY",
                "score": "",
                "stage": "",
                "reason": "",
                "elapsed": "",
                "attempts": "",
                "recommendation": rec,
                "size_insns": str(size),
                "blocker_tags": tags,
            }
        )

    if not new_rows:
        print("Nothing to add.")
        return 0

    fieldnames = [
        "func",
        "src",
        "kind",
        "result",
        "score",
        "stage",
        "reason",
        "elapsed",
        "attempts",
        "recommendation",
        "size_insns",
        "blocker_tags",
    ]
    with CSV_PATH.open("a", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fieldnames)
        for r in new_rows:
            w.writerow(r)
    print(f"\nAppended {len(new_rows)} rows to {CSV_PATH.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
