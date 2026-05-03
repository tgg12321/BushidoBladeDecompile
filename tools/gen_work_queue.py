#!/usr/bin/env python3
"""Generate WORK_QUEUE.md at the project root — the canonical, ordered
list of remaining functions for future agents to pull from.

Reads tmp/batch_attempt.csv (which must include both inline_asm and
asmfix entries — run batch_attempt --classify-only and classify_asmfix
first), filters to in-scope categories, and writes a Markdown queue
grouped by complexity tier.

Tier ordering (top of queue = pull these first):
  1. standard, small      (<=40 insns)
  2. standard, medium     (41-100)
  3. standard, large      (101-200)
  4. standard, huge       (201+)
  5. needs_lwl_fix        (any size, in size order)
  6. needs_rodata_split   (any size, in size order)
  7. gte_function         (any size, in size order)

Within each tier, functions are sorted by ascending instruction count,
with src file as a tiebreaker so siblings (which often share a recipe)
sit next to each other.

Out-of-scope categories (permanently_blocked, bios_or_syscall, multi_function,
aspsx_delay_swra, not_found) are NOT in the queue. They are gated out by
the classifier and listed separately at the bottom of the .md for
auditability.
"""
from __future__ import annotations

import csv
from collections import defaultdict
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent.parent
CSV_PATH = ROOT / "tmp" / "batch_attempt.csv"
OUT = ROOT / "WORK_QUEUE.md"

IN_SCOPE = {"standard", "gte_function", "needs_rodata_split", "needs_lwl_fix"}


def tier_for(rec: str, size: int, tags: str) -> tuple[int, str]:
    """Return (tier_index, tier_label). Lower tier_index = higher priority.

    `tags` is the comma-separated blocker_tags from the classifier.
    Functions tagged `aliasing_heavy` are pushed to a later tier even
    if their `recommendation` is `standard`, because their pointer-
    chasing patterns hide complexity that instruction count doesn't
    predict (see func_80060B70 post-mortem).
    """
    is_aliasing_heavy = "aliasing_heavy" in (tags or "")

    if rec == "standard":
        if is_aliasing_heavy:
            # Bump aliasing-heavy `standard` functions to a dedicated
            # later tier — same kind of work but expect more iterations.
            if size <= 100:
                return (4, "standard, aliasing_heavy / small (<=100)")
            if size <= 200:
                return (5, "standard, aliasing_heavy / medium (101-200)")
            return (5, "standard, aliasing_heavy / large (201+)")
        if size <= 40:
            return (1, "standard / small (<=40 insns)")
        if size <= 100:
            return (2, "standard / medium (41-100)")
        if size <= 200:
            return (3, "standard / large (101-200)")
        return (4, "standard / huge (201+)")
    if rec == "needs_lwl_fix":
        return (6, "needs_lwl_fix")
    if rec == "needs_rodata_split":
        return (7, "needs_rodata_split")
    if rec == "gte_function":
        return (8, "gte_function (cop2 inline asm allowed for ops)")
    return (99, f"other ({rec})")


def main() -> int:
    if not CSV_PATH.exists():
        print(
            f"ERROR: {CSV_PATH} missing -- run "
            f"`python3 tools/batch_attempt.py --classify-only` "
            f"and `python3 tools/classify_asmfix.py` first.",
            file=sys.stderr,
        )
        return 1

    rows = list(csv.DictReader(CSV_PATH.open()))
    in_scope = [r for r in rows if r["recommendation"] in IN_SCOPE]
    out_scope = [r for r in rows if r["recommendation"] not in IN_SCOPE]

    # Group by tier, sort within tier
    tiers: dict[tuple[int, str], list[dict]] = defaultdict(list)
    for r in in_scope:
        size = int(r.get("size_insns", "0") or 0)
        tier = tier_for(r["recommendation"], size, r.get("blocker_tags", ""))
        tiers[tier].append(
            {
                "func": r["func"],
                "size": size,
                "rec": r["recommendation"],
                "src": r.get("src", "").replace("src/", ""),
                "kind": r.get("kind", ""),
                "tags": r.get("blocker_tags", ""),
            }
        )

    # Sort within each tier: by size asc, then src (siblings together), then name
    for tier in tiers:
        tiers[tier].sort(key=lambda x: (x["size"], x["src"], x["func"]))

    # Build flat ordered queue
    ordered: list[dict] = []
    for tier in sorted(tiers.keys()):
        for entry in tiers[tier]:
            entry["tier_label"] = tier[1]
            ordered.append(entry)

    # Out-of-scope grouped by recommendation, alphabetized
    out_groups: dict[str, list[str]] = defaultdict(list)
    for r in out_scope:
        out_groups[r["recommendation"]].append(r["func"])
    for rec in out_groups:
        out_groups[rec].sort()

    # Write
    lines: list[str] = []
    lines.append("# BB2 Work Queue")
    lines.append("")
    lines.append(
        "**Canonical ordered list of remaining functions.** Pull from the top. "
        "Do not search for easier targets; do not skip ahead. Per "
        "`feedback_workflow_rules.md` THE HARD RULE, every function in this "
        "queue gets worked end-to-end to pure C match."
    )
    lines.append("")
    lines.append(
        f"Generated by `tools/gen_work_queue.py` from `tmp/batch_attempt.csv`. "
        f"To refresh: run `bash tools/dc.sh refresh-queue`."
    )
    lines.append("")
    lines.append(f"**Total in-scope: {len(ordered)} functions.**")
    lines.append("")
    lines.append("## How to use")
    lines.append("")
    lines.append("```")
    lines.append("# Get the next function to work")
    lines.append("bash tools/dc.sh next")
    lines.append("```")
    lines.append("")
    lines.append(
        "After matching: commit the result, then `dc.sh refresh-queue` "
        "to regenerate this file (the matched function falls off the queue)."
    )
    lines.append("")
    lines.append("## Tier summary")
    lines.append("")
    lines.append("| Tier | Count | Description |")
    lines.append("|------|-------|-------------|")
    tier_counts: dict[tuple[int, str], int] = defaultdict(int)
    for tier, entries in tiers.items():
        tier_counts[tier] = len(entries)
    for tier in sorted(tier_counts.keys()):
        idx, label = tier
        lines.append(f"| {idx} | {tier_counts[tier]:>3} | {label} |")
    lines.append("")
    lines.append("---")
    lines.append("")
    lines.append("## Queue (top = next)")
    lines.append("")
    lines.append("Format: `<#>  <func>  <size insns>  <rec>  <src>  [tags]`")
    lines.append("")

    # Group by tier with headers in the queue body too
    cur_tier_label: str | None = None
    pos = 0
    for entry in ordered:
        if entry["tier_label"] != cur_tier_label:
            if cur_tier_label is not None:
                lines.append("```")
                lines.append("")
            lines.append(f"### {entry['tier_label']}")
            lines.append("")
            lines.append("```")
            cur_tier_label = entry["tier_label"]
        pos += 1
        tags = f"  [{entry['tags']}]" if entry["tags"] else ""
        lines.append(
            f"{pos:>4}  {entry['func']:<32s}  "
            f"{entry['size']:>4d}  "
            f"{entry['rec']:<20s}  "
            f"{entry['src']:<24s}{tags}"
        )
    if cur_tier_label is not None:
        lines.append("```")
        lines.append("")

    lines.append("---")
    lines.append("")
    lines.append("## Out of scope (auto-gated by `dc.sh classify`)")
    lines.append("")
    lines.append(
        "These are NOT in the queue. They cannot be reached by pure C with the "
        "GCC 2.7.2 + ASPSX 2.34 toolchain (or are explicitly accepted-as-asm "
        "exceptions per the project goal). Listed for auditability only — "
        "do not attempt them."
    )
    lines.append("")
    lines.append(f"**Total out-of-scope: {len(out_scope)} functions.**")
    lines.append("")
    for rec in sorted(out_groups.keys()):
        funcs = out_groups[rec]
        lines.append(f"### {rec} ({len(funcs)})")
        lines.append("")
        lines.append("```")
        for f in funcs:
            lines.append(f"  {f}")
        lines.append("```")
        lines.append("")

    OUT.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
    print(f"Wrote {OUT.relative_to(ROOT)} ({len(ordered)} in-scope, "
          f"{len(out_scope)} out-of-scope)")

    # Print top 10 for convenience
    print()
    print("Top 10 (next to work):")
    for i, e in enumerate(ordered[:10], 1):
        print(f"  {i:>3}. {e['func']:<32s} {e['size']:>4d} insns  "
              f"{e['rec']:<20s}  {e['src']}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
