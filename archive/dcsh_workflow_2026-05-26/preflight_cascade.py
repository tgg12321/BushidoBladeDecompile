#!/usr/bin/env python3
"""Pre-integration cascade-impact report. Read-only.

Use BEFORE integrating a new pure-C body into a busy compilation unit
(text1b.c, code6cac.c, main.c). Reports how many sibling asmfix/regfix
rules use literal `.LN` labels — those are the rules that can silently
break when your integration shifts GCC's file-wide label counter.

The report is informational. Auto-repair (in build-active / via
auto_drift_repair.py) handles drift after-the-fact. preflight-cascade
sets expectations: "this file has 42 fragile rules; expect auto-repair
to fire after integration."

Usage:
    python3 tools/preflight_cascade.py <func>

Output sections:
    - target function + source file
    - sibling function count in this CU
    - asmfix.txt rule breakdown (literal-LN vs address-LN vs other)
    - regfix.txt rule breakdown (literal-LN vs regex-LN vs other)
    - estimated drift surface
    - one-line guidance for what to expect during integration
"""
from __future__ import annotations

import argparse
import re
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "src"


# Function definition recognizer — both pure-C function bodies and
# `glabel <func>` inside an inline-asm wrapper count as "this .c defines it".
_FUNC_C_DEF_RE = re.compile(
    r"^[A-Za-z_][\w *]*?\s\**(\w+)\s*\([^)]*\)\s*\{", re.MULTILINE
)
_GLABEL_RE = re.compile(r"glabel\s+(\w+)")


def find_src_for_func(func: str) -> Path | None:
    """Return the src/*.c file that defines <func>, or None."""
    for p in sorted(SRC.glob("*.c")):
        try:
            text = p.read_text(encoding="utf-8", errors="ignore")
        except Exception:
            continue
        for m in _FUNC_C_DEF_RE.finditer(text):
            if m.group(1) == func:
                return p
        for m in _GLABEL_RE.finditer(text):
            if m.group(1) == func:
                return p
    return None


def funcs_in_file(src_path: Path) -> set[str]:
    """All function names defined in src_path (C bodies + glabel forms)."""
    try:
        text = src_path.read_text(encoding="utf-8", errors="ignore")
    except Exception:
        return set()
    funcs = {m.group(1) for m in _FUNC_C_DEF_RE.finditer(text)}
    funcs |= {m.group(1) for m in _GLABEL_RE.finditer(text)}
    return funcs


# Address-based label: `.L` + 8 hex digits (project convention for
# asmfix-slice rename targets). These are stable across GCC re-numbering.
_RE_ADDR_LABEL = re.compile(r"\.L[0-9A-Fa-f]{8}\b")
# Drift-immune regex form (rare but the goal): `\.L\d+` in pattern.
_RE_REGEX_LABEL = re.compile(r"\\\.L\\d\+")
# GCC auto-numbered label (drift-fragile): `.L<1-5 digit number>` not part of an address.
_RE_AUTO_LABEL = re.compile(r"\.L\d{1,5}\b")


def classify_rule(body: str) -> str:
    """Return drift-risk classification for a rule body:
      'address_label'   — uses only address-derived labels (stable)
      'literal_auto'    — uses GCC auto-numbered `.LN` literally (FRAGILE)
      'regex_auto'      — uses `\\.L\\d+` regex (drift-immune)
      'no_label'        — no .L references at all
    """
    # Strip address labels so we don't conflate `.L80056FB4` with `.L320`.
    stripped = _RE_ADDR_LABEL.sub("", body)
    has_address = bool(_RE_ADDR_LABEL.search(body))
    has_regex = bool(_RE_REGEX_LABEL.search(body))
    has_literal_auto = bool(_RE_AUTO_LABEL.search(stripped))

    if has_literal_auto:
        return "literal_auto"
    if has_regex:
        return "regex_auto"
    if has_address:
        return "address_label"
    return "no_label"


def scan_rules(filepath: Path, in_funcs: set[str]) -> list[tuple[str, str, str]]:
    """Return [(func, body, classification), ...] for rules whose function
    name is in `in_funcs`."""
    if not filepath.exists():
        return []
    out = []
    for line in filepath.read_text(encoding="utf-8").splitlines():
        s = line.strip()
        if not s or s.startswith("#"):
            continue
        m = re.match(r"^(\w+):\s+(.*)$", s)
        if not m:
            continue
        func, body = m.group(1), m.group(2)
        if func in in_funcs:
            out.append((func, body, classify_rule(body)))
    return out


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__.split("\n")[0],
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    ap.add_argument("func", help="Function name to preflight")
    ap.add_argument("--verbose", action="store_true",
                    help="List each fragile rule individually")
    args = ap.parse_args()

    src_path = find_src_for_func(args.func)
    if src_path is None:
        print(f"ERROR: could not find {args.func} in any src/*.c file",
              file=sys.stderr)
        return 1

    siblings = funcs_in_file(src_path)
    siblings.discard(args.func)  # exclude target itself

    asmfix_rules = scan_rules(ROOT / "asmfix.txt", siblings)
    regfix_rules = scan_rules(ROOT / "regfix.txt", siblings)

    asmfix_by_class = defaultdict(list)
    for f, body, cls in asmfix_rules:
        asmfix_by_class[cls].append((f, body))
    regfix_by_class = defaultdict(list)
    for f, body, cls in regfix_rules:
        regfix_by_class[cls].append((f, body))

    fragile_asmfix = asmfix_by_class.get("literal_auto", [])
    fragile_regfix = regfix_by_class.get("literal_auto", [])
    n_fragile = len(fragile_asmfix) + len(fragile_regfix)

    fragile_asmfix_funcs = {f for f, _ in fragile_asmfix}
    fragile_regfix_funcs = {f for f, _ in fragile_regfix}
    fragile_funcs = fragile_asmfix_funcs | fragile_regfix_funcs

    rel = src_path.relative_to(ROOT)

    print(f"preflight-cascade: {args.func}")
    print(f"  Source file: {rel}")
    print(f"  Sibling functions in this CU: {len(siblings)}")
    print()
    print(f"  asmfix.txt rules for siblings: {len(asmfix_rules)}")
    print(f"    {len(fragile_asmfix):>4}  FRAGILE   (literal `.LN` source — drifts on integration)")
    print(f"    {len(asmfix_by_class.get('address_label', [])):>4}  stable    (address-derived `.L<8hex>` only)")
    print(f"    {len(asmfix_by_class.get('regex_auto', [])):>4}  immune    (regex `\\.L\\d+`)")
    print(f"    {len(asmfix_by_class.get('no_label', [])):>4}  no-label")
    print()
    print(f"  regfix.txt rules for siblings: {len(regfix_rules)}")
    print(f"    {len(fragile_regfix):>4}  FRAGILE   (literal `.LN` in pattern or replacement)")
    print(f"    {len(regfix_by_class.get('address_label', [])):>4}  stable    (address-derived only)")
    print(f"    {len(regfix_by_class.get('regex_auto', [])):>4}  immune    (regex `\\.L\\d+`)")
    print(f"    {len(regfix_by_class.get('no_label', [])):>4}  no-label")
    print()
    print(f"  Estimated drift surface: {n_fragile} rule(s) across "
          f"{len(fragile_funcs)} sibling function(s)")

    if args.verbose and fragile_funcs:
        print()
        print(f"  Fragile siblings (rules sorted by function):")
        for func in sorted(fragile_funcs):
            n_asm = sum(1 for f, _ in fragile_asmfix if f == func)
            n_reg = sum(1 for f, _ in fragile_regfix if f == func)
            parts = []
            if n_asm:
                parts.append(f"{n_asm} asmfix")
            if n_reg:
                parts.append(f"{n_reg} regfix")
            print(f"    {func:<40}  {', '.join(parts)}")

    print()
    if n_fragile == 0:
        print(f"  No drift-fragile sibling rules. Integration is safe.")
    else:
        print(f"  Integration is likely to shift label numbering in {len(siblings)} siblings.")
        print(f"  Auto-repair runs automatically during `dc.sh build-active <func>`.")
        print(f"  Expect to see [auto-repair] lines on first build; the repair edits")
        print(f"  asmfix.txt / regfix.txt and re-runs make. Include those modifications")
        print(f"  in your commit.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
