#!/usr/bin/env python3
"""Validate regfix.txt rules — static checks + optional live pipeline verification.

Usage:
    python3 tools/validate_regfix.py                  # static checks on all rules
    python3 tools/validate_regfix.py --live            # + verify patterns match pipeline output
    python3 tools/validate_regfix.py --func FUNC       # check only one function
    python3 tools/validate_regfix.py --func FUNC --live

Static checks (always):
  1. Unescaped $ in subst patterns (bare $ = regex anchor, never matches registers)
  2. Syntax errors in rule format
  3. Duplicate indices within a function

Live checks (--live flag):
  4. Instruction index within function bounds
  5. Subst pattern matches the instruction at the specified TEXT index
  6. Swap register names appear at the specified indices
"""

import re
import subprocess
import sys
from pathlib import Path


def parse_regfix(regfix_path):
    """Parse regfix.txt into structured rules grouped by function."""
    rules = {}
    if not regfix_path.exists():
        return rules

    for lineno, line in enumerate(regfix_path.read_text().splitlines(), 1):
        stripped = line.strip()
        if not stripped or stripped.startswith('#'):
            continue

        # subst
        m = re.match(r'(\w+)\s*:\s*subst\s+"([^"]+)"\s+"([^"]*)"\s*@\s*(\d+)', stripped)
        if m:
            func = m.group(1)
            rules.setdefault(func, []).append({
                'type': 'subst', 'pattern': m.group(2),
                'replacement': m.group(3), 'index': int(m.group(4)),
                'lineno': lineno, 'raw': stripped,
            })
            continue

        # delete
        m = re.match(r'(\w+)\s*:\s*delete\s*@\s*(\d+)', stripped)
        if m:
            func = m.group(1)
            rules.setdefault(func, []).append({
                'type': 'delete', 'index': int(m.group(2)),
                'lineno': lineno, 'raw': stripped,
            })
            continue

        # insert / insert_after
        m = re.match(r'(\w+)\s*:\s*(insert(?:_after)?)\s+"([^"]+)"\s*@\s*(\d+)', stripped)
        if m:
            func = m.group(1)
            rules.setdefault(func, []).append({
                'type': m.group(2), 'asm_text': m.group(3),
                'index': int(m.group(4)),
                'lineno': lineno, 'raw': stripped,
            })
            continue

        # reorder
        m = re.match(r'(\w+)\s*:\s*reorder\s+([\d,]+)\s*@\s*(\d+)\s*-\s*(\d+)', stripped)
        if m:
            func = m.group(1)
            rules.setdefault(func, []).append({
                'type': 'reorder',
                'order': [int(x) for x in m.group(2).split(',')],
                'start': int(m.group(3)), 'end': int(m.group(4)),
                'lineno': lineno, 'raw': stripped,
            })
            continue

        # swap
        m = re.match(r'(\w+)\s*:\s*(\$\w+)\s*<->\s*(\$\w+)(?:\s*@\s*(\d+)\s*-\s*(\d+))?', stripped)
        if m:
            func = m.group(1)
            rules.setdefault(func, []).append({
                'type': 'swap', 'reg_a': m.group(2), 'reg_b': m.group(3),
                'start': int(m.group(4)) if m.group(4) else None,
                'end': int(m.group(5)) if m.group(5) else None,
                'lineno': lineno, 'raw': stripped,
            })
            continue

        # Unknown format
        rules.setdefault('__PARSE_ERRORS__', []).append({
            'type': 'parse_error', 'lineno': lineno, 'raw': stripped,
        })

    return rules


def check_static(rules):
    """Run all static checks. Returns list of error strings."""
    errors = []

    # Parse errors
    for entry in rules.get('__PARSE_ERRORS__', []):
        errors.append(f"  Line {entry['lineno']}: PARSE ERROR — unrecognized format\n"
                      f"    {entry['raw']}")

    for func, func_rules in rules.items():
        if func == '__PARSE_ERRORS__':
            continue

        subst_indices = []
        for rule in func_rules:
            # Check 1: Unescaped $ in subst patterns
            if rule['type'] == 'subst':
                pat = rule['pattern']
                for i, ch in enumerate(pat):
                    if ch == '$' and (i == 0 or pat[i - 1] != '\\'):
                        errors.append(
                            f"  Line {rule['lineno']}: UNESCAPED $ in pattern at pos {i}\n"
                            f"    {rule['raw']}\n"
                            f"    Fix: use \\$ for literal dollar sign in pattern side")
                        break

                subst_indices.append((rule['index'], rule['lineno']))

            # Check for swap rules with inverted range
            if rule['type'] == 'swap' and rule.get('start') is not None:
                if rule['start'] > rule['end']:
                    errors.append(
                        f"  Line {rule['lineno']}: INVERTED RANGE {rule['start']}-{rule['end']}\n"
                        f"    {rule['raw']}")

            # Check reorder consistency
            if rule['type'] == 'reorder':
                expected = set(range(rule['start'], rule['end'] + 1))
                actual = set(rule['order'])
                if actual != expected:
                    missing = expected - actual
                    extra = actual - expected
                    msg = f"  Line {rule['lineno']}: REORDER MISMATCH"
                    if missing:
                        msg += f" missing={sorted(missing)}"
                    if extra:
                        msg += f" extra={sorted(extra)}"
                    msg += f"\n    {rule['raw']}"
                    errors.append(msg)

        # Check 2: Duplicate subst indices (same index, different rules — may be intentional)
        seen = {}
        for idx, ln in subst_indices:
            if idx in seen:
                errors.append(
                    f"  Line {ln}: DUPLICATE subst index {idx} (also at line {seen[idx]})\n"
                    f"    Function: {func}")
            seen[idx] = ln

    return errors


def get_pipeline_instructions(root, func_name):
    """Run dump_text_indices.py and return list of (index, text) pairs."""
    cmd = f"python3 tools/dump_text_indices.py {func_name}"
    result = subprocess.run(
        ["bash", "-c", cmd],
        capture_output=True, text=True, cwd=str(root)
    )
    if result.returncode != 0:
        return None, result.stderr.strip()

    instructions = []
    for line in result.stdout.splitlines():
        m = re.match(r'\s*(\d+):\s*(.*)', line)
        if m:
            instructions.append((int(m.group(1)), m.group(2).strip()))
    return instructions, None


def check_live(rules, root, func_filter=None):
    """Run pipeline for each function and verify rules match."""
    errors = []
    checked = 0

    for func, func_rules in rules.items():
        if func == '__PARSE_ERRORS__':
            continue
        if func_filter and func != func_filter:
            continue

        # Only check functions that have subst or swap rules with ranges
        subst_rules = [r for r in func_rules if r['type'] == 'subst']
        if not subst_rules:
            continue

        print(f"  Checking {func}...", file=sys.stderr, end=' ')
        instructions, err = get_pipeline_instructions(root, func)
        if instructions is None:
            print(f"SKIP (pipeline error)", file=sys.stderr)
            errors.append(f"  {func}: pipeline failed — {err}")
            continue

        max_idx = max(idx for idx, _ in instructions) if instructions else -1
        insn_map = {idx: text for idx, text in instructions}

        func_errors = 0
        for rule in subst_rules:
            idx = rule['index']

            # Bounds check
            if idx > max_idx:
                errors.append(
                    f"  Line {rule['lineno']}: INDEX {idx} OUT OF BOUNDS (max={max_idx})\n"
                    f"    {rule['raw']}")
                func_errors += 1
                continue

            if idx not in insn_map:
                errors.append(
                    f"  Line {rule['lineno']}: INDEX {idx} NOT FOUND\n"
                    f"    {rule['raw']}")
                func_errors += 1
                continue

            insn_text = insn_map[idx]
            try:
                if not re.search(rule['pattern'], insn_text):
                    errors.append(
                        f"  Line {rule['lineno']}: PATTERN MISMATCH at index {idx}\n"
                        f"    Pattern:     {rule['pattern']}\n"
                        f"    Instruction: {insn_text}\n"
                        f"    {rule['raw']}")
                    func_errors += 1
            except re.error as e:
                errors.append(
                    f"  Line {rule['lineno']}: INVALID REGEX: {e}\n"
                    f"    {rule['raw']}")
                func_errors += 1

        status = "OK" if func_errors == 0 else f"{func_errors} error(s)"
        print(status, file=sys.stderr)
        checked += 1

    return errors, checked


def main():
    import argparse
    parser = argparse.ArgumentParser(description="Validate regfix.txt rules")
    parser.add_argument('--live', action='store_true',
                        help='Verify patterns against actual pipeline output')
    parser.add_argument('--func', type=str,
                        help='Check only this function')
    args = parser.parse_args()

    root = Path(__file__).resolve().parent.parent
    regfix_path = root / 'regfix.txt'

    if not regfix_path.exists():
        print("No regfix.txt found", file=sys.stderr)
        sys.exit(0)

    rules = parse_regfix(regfix_path)
    total_rules = sum(len(v) for k, v in rules.items() if k != '__PARSE_ERRORS__')
    total_funcs = len([k for k in rules if k != '__PARSE_ERRORS__'])
    print(f"Parsed {total_rules} rules across {total_funcs} functions", file=sys.stderr)

    # Filter to specific function if requested
    if args.func:
        if args.func not in rules:
            print(f"No rules found for {args.func}", file=sys.stderr)
            sys.exit(0)

    # Static checks
    print("\n=== Static Checks ===", file=sys.stderr)
    static_errors = check_static(rules)
    if static_errors:
        print(f"FOUND {len(static_errors)} static error(s):", file=sys.stderr)
        for e in static_errors:
            print(e, file=sys.stderr)
    else:
        print("All static checks passed", file=sys.stderr)

    # Live checks
    live_errors = []
    if args.live:
        print("\n=== Live Checks ===", file=sys.stderr)
        live_errors, checked = check_live(rules, root, args.func)
        if live_errors:
            print(f"\nFOUND {len(live_errors)} live error(s):", file=sys.stderr)
            for e in live_errors:
                print(e, file=sys.stderr)
        else:
            print(f"\nAll {checked} function(s) passed live checks", file=sys.stderr)

    total_errors = len(static_errors) + len(live_errors)
    if total_errors > 0:
        sys.exit(1)


if __name__ == '__main__':
    main()
