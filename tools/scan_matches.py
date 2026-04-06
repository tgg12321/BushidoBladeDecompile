#!/usr/bin/env python3
"""
scan_matches.py - Scan all permuter directories for integrable matches.

Reports:
  - INTEGRABLE: score 0, no blockers → ready for integrate_match.py
  - FALSE_POS:  score 0 but has M2C_ERROR/blockers → unreliable score
  - NEAR_MISS:  score 1-500, may be matchable with small fixes
  - Also shows breakdown of what blocks each function

Usage:
    python3 tools/scan_matches.py [--score-limit N] [--rescore]
"""
import argparse
import os
import re
import subprocess
import sys

PROJ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(PROJ)


def get_active_stubs():
    """Get set of function names that are still INCLUDE_ASM stubs."""
    stubs = {}
    for fname in sorted(os.listdir("src")):
        if not fname.endswith(".c"):
            continue
        path = os.path.join("src", fname)
        with open(path) as f:
            content = f.read()
        for m in re.finditer(r'INCLUDE_ASM\("asm/funcs",\s*(\w+)\)', content):
            stubs[m.group(1)] = path
    return stubs


def check_blockers(base_c_path):
    """Check base.c for integration blockers. Returns list of (tag, description)."""
    with open(base_c_path) as f:
        content = f.read()
    if "\n" not in content and "\\n" in content:
        content = content.replace("\\n", "\n")

    blockers = []

    if "M2C_ERROR" in content:
        count = content.count("M2C_ERROR")
        blockers.append(("M2C_ERROR", f"drops {count} GTE/cop instruction(s) — score UNRELIABLE"))

    if re.search(r'goto\s+\.L', content):
        count = len(re.findall(r'goto\s+\.L', content))
        blockers.append((".L_LABELS", f"{count} assembler-style goto label(s)"))

    if re.search(r'(?<![a-zA-Z_])sp(?:\s*[+\-=;)]|\s*\[)', content):
        blockers.append(("SP_VAR", "raw stack pointer manipulation"))

    if "M2C_FIELD" in content:
        count = content.count("M2C_FIELD")
        blockers.append(("M2C_FIELD", f"{count} field access macro(s) — needs m2c_macros.h"))

    if "M2C_UNK" in content:
        blockers.append(("M2C_UNK", "unresolved types — needs m2c_macros.h"))

    if "switch" in content:
        # Check if target asm has jump table
        func_name = os.path.basename(os.path.dirname(base_c_path))
        asm_path = os.path.join("asm", "funcs", f"{func_name}.s")
        if os.path.isfile(asm_path):
            asm = open(asm_path).read()
            if "jtbl_" in asm or "jlabel" in asm:
                blockers.append(("SWITCH_JTBL", "switch with jump table — needs rodata split"))

    return blockers


def get_score(func_name):
    """Score via try_match.sh. Returns (score, penalties_str) or (None, error)."""
    try:
        r = subprocess.run(
            ["bash", "tools/try_match.sh", func_name],
            capture_output=True, text=True, timeout=30)
        output = r.stdout + r.stderr
        m = re.search(r"Score:\s*(\d+)", output)
        if not m:
            return (None, "no score")
        score = int(m.group(1))

        # Parse penalty breakdown
        parts = []
        for line in output.splitlines():
            pm = re.match(
                r'(Stack Differences|Branch Differences|Register Differences|'
                r'Reorderings|Insertions|Deletions):\s+(\d+)\s+\((\d+)\)', line)
            if pm and int(pm.group(2)) > 0:
                name = pm.group(1).replace(" Differences", "")
                parts.append(f"{name}={pm.group(2)}x{pm.group(3)}")

        return (score, " ".join(parts) if parts else "perfect")
    except subprocess.TimeoutExpired:
        return (None, "timeout")
    except Exception as e:
        return (None, str(e))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--score-limit", type=int, default=500,
                        help="Max score to report as near-miss (default 500)")
    parser.add_argument("--rescore", action="store_true",
                        help="Re-score all functions (slow)")
    parser.add_argument("--all", action="store_true",
                        help="Show all scored functions, not just interesting ones")
    args = parser.parse_args()

    stubs = get_active_stubs()
    print(f"Active stubs: {len(stubs)}")

    integrable = []
    false_pos = []
    near_miss = []
    other = []

    total = sum(1 for f in stubs if os.path.isfile(f"permuter/{f}/base.c"))
    done = 0

    for func_name, src_file in sorted(stubs.items()):
        base_c = f"permuter/{func_name}/base.c"
        if not os.path.isfile(base_c):
            continue

        done += 1
        sys.stderr.write(f"\r[{done}/{total}] {func_name:40s}")
        sys.stderr.flush()

        blockers = check_blockers(base_c)
        score, penalties = get_score(func_name)

        if score is None:
            continue

        has_fatal = any(tag in ("M2C_ERROR",) for tag, _ in blockers)
        has_blockers = len(blockers) > 0

        entry = {
            "func": func_name,
            "src": os.path.basename(src_file),
            "score": score,
            "penalties": penalties,
            "blockers": blockers,
        }

        if score == 0 and not has_blockers:
            integrable.append(entry)
        elif score == 0 and has_fatal:
            false_pos.append(entry)
        elif score == 0:
            # Score 0 with non-fatal blockers (e.g., .L labels, M2C_UNK)
            near_miss.append(entry)
        elif score <= args.score_limit:
            near_miss.append(entry)
        else:
            other.append(entry)

    sys.stderr.write("\r" + " " * 60 + "\r")

    # Report
    print(f"\n{'='*70}")

    print(f"\nINTEGRABLE (score 0, no blockers): {len(integrable)}")
    for e in integrable:
        print(f"  {e['func']:40s} in {e['src']}")
    if not integrable:
        print("  (none)")

    print(f"\nFALSE POSITIVES (score 0 but M2C_ERROR drops instructions): {len(false_pos)}")
    for e in false_pos:
        tags = [t for t, _ in e['blockers']]
        print(f"  {e['func']:40s} [{', '.join(tags)}]")

    print(f"\nNEAR MISSES (score 0 with fixable blockers, or score 1-{args.score_limit}): {len(near_miss)}")
    for e in sorted(near_miss, key=lambda x: x['score']):
        tags = [t for t, _ in e['blockers']]
        tag_str = f" [{', '.join(tags)}]" if tags else ""
        print(f"  {e['func']:40s} score={e['score']:>5d}  {e['penalties']}{tag_str}")

    if args.all and other:
        print(f"\nOTHER (score {args.score_limit+1}+): {len(other)}")
        for e in sorted(other, key=lambda x: x['score'])[:30]:
            tags = [t for t, _ in e['blockers']]
            tag_str = f" [{', '.join(tags)}]" if tags else ""
            print(f"  {e['func']:40s} score={e['score']:>5d}  {e['penalties']}{tag_str}")

    # Summary
    print(f"\n{'='*70}")
    print(f"Summary:")
    print(f"  Integrable now:     {len(integrable)}")
    print(f"  False positives:    {len(false_pos)}")
    print(f"  Near misses:        {len(near_miss)}")
    print(f"  Other (scored):     {len(other)}")
    print(f"  No permuter dir:    {len(stubs) - total}")

    # Blocker frequency
    from collections import Counter
    blocker_freq = Counter()
    for e in false_pos + near_miss + other:
        for tag, _ in e['blockers']:
            blocker_freq[tag] += 1
    if blocker_freq:
        print(f"\nBlocker frequency (across all scored functions):")
        for tag, count in blocker_freq.most_common():
            print(f"  {tag:20s}: {count}")


if __name__ == "__main__":
    sys.exit(0 if main() is None else 1)
