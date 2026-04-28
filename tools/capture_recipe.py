#!/usr/bin/env python3
"""Analyze a match commit and propose a recipe JSON for the library.

When an agent (or human) commits a function match, the changes typically
fall into a small number of recipe categories. This tool reads the diff,
classifies the patterns used, and either:
  - Reports "matches existing recipe <X>" (a confirmation; possibly bumps
    a usage counter on the recipe), OR
  - Drafts a candidate JSON for tools/recipes/ that captures the new
    pattern, so a future similar function can find it via
    `dc.sh recipes suggest`.

Usage:
    python3 tools/capture_recipe.py [<commit>]            # default: HEAD
    python3 tools/capture_recipe.py <commit> --write      # save the draft
    python3 tools/capture_recipe.py <commit> --json       # emit dict only

The classifier looks for these signal categories in the commit's diff:
  - C-edit shape: pointer-scaling cast, byte-array indexing, struct field
    access, intermediate-variable introduction, etc.
  - Regfix rule shape: subst-only, swap-only, drain_delay/fill_delay,
    reorder, full-instruction subst, multi-rule cycles
  - Number of rules + spread (single index / multiple indices / contiguous
    range)

When the combination matches a known recipe, the tool prints the recipe
name. Otherwise it drafts a new JSON with a generated name and the
relevant rule_skeleton and notes.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
RECIPES_DIR = ROOT / "tools" / "recipes"


def run(cmd: list[str], timeout: int = 30) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, capture_output=True, text=True,
                          cwd=str(ROOT), timeout=timeout)


def get_commit_info(commit: str) -> dict:
    msg = run(["git", "log", "-1", "--pretty=%B", commit]).stdout.strip()
    diff = run(["git", "show", "--no-color", commit]).stdout

    # Pull the function name from the commit subject ("Match func_X")
    m = re.search(r"Match\s+(\w+)", msg.split("\n")[0])
    func = m.group(1) if m else None

    return {"commit": commit, "subject": msg.split("\n")[0],
            "message": msg, "diff": diff, "func": func}


def collect_regfix_rules(diff: str, func: str | None) -> list[str]:
    """Find regfix.txt lines added in the diff for this function."""
    out = []
    in_regfix = False
    for line in diff.splitlines():
        if line.startswith("diff --git"):
            in_regfix = "regfix.txt" in line and "regfix_stage2" not in line
            continue
        if not in_regfix:
            continue
        if not line.startswith("+") or line.startswith("+++"):
            continue
        body = line[1:].strip()
        if not body or body.startswith("#"):
            continue
        if func and body.startswith(f"{func}:"):
            out.append(body)
        elif not func:
            out.append(body)
    return out


def collect_c_edits(diff: str) -> dict:
    """Inspect the C-side changes for known patterns. Returns flags +
    a sample of the most representative line."""
    added: list[str] = []
    removed: list[str] = []
    cur_file = None
    for line in diff.splitlines():
        if line.startswith("diff --git"):
            cur_file = None
            # `diff --git a/src/<x>.c b/src/<x>.c`
            if "/src/" in line and line.endswith(".c"):
                cur_file = line.split(" b/")[-1]
            continue
        if cur_file is None:
            continue
        if line.startswith("+") and not line.startswith("+++"):
            added.append(line[1:])
        elif line.startswith("-") and not line.startswith("---"):
            removed.append(line[1:])

    pat_byte_arith_added = any(
        re.search(r"\(u8\s*\*\)\s*&\w+\s*\+\s*\w+\s*\*", l) for l in added
    )
    pat_byte_arith_removed = any(
        re.search(r"\(\w+\s*\*\s*\d|0x\w+\)\s*\+\s*&\w", l) for l in removed
    )

    pat_byte_array_idx = any(
        re.search(r"\(&\w+\)\[\w+\]", l) for l in added
    )

    pat_extern_added = any(
        re.search(r"^\s*extern\s+\w+\s+\w+", l) for l in added
    )

    pat_volatile_added = any(
        re.search(r"\bvolatile\s+\w+\s*\*", l) for l in added
    )

    pat_intermediate_var = (
        len([l for l in added if re.match(r"^\s*s32\s+\w+\s*=\s*", l)]) >=
        len([l for l in removed if re.match(r"^\s*s32\s+\w+\s*=\s*", l)]) + 1
    )

    return {
        "added_lines": len(added),
        "removed_lines": len(removed),
        "byte_arith_fix": pat_byte_arith_added or pat_byte_arith_removed,
        "byte_array_index": pat_byte_array_idx,
        "extern_added": pat_extern_added,
        "volatile_pointer": pat_volatile_added,
        "intermediate_var_added": pat_intermediate_var,
    }


def parse_regfix_rule(rule: str) -> dict:
    """Pull the structured fields out of one regfix line."""
    m = re.match(r"\w+:\s*subst\s+\"([^\"]+)\"\s+\"([^\"]*)\"\s+@\s+(\d+)", rule)
    if m:
        return {"op": "subst", "pattern": m.group(1),
                "replacement": m.group(2), "idx": int(m.group(3))}
    m = re.match(r"\w+:\s*delete\s+@\s+(\d+)", rule)
    if m:
        return {"op": "delete", "idx": int(m.group(1))}
    m = re.match(r"\w+:\s*(insert(?:_after)?)\s+\"([^\"]+)\"\s+@\s+(\d+)", rule)
    if m:
        return {"op": m.group(1), "asm_text": m.group(2), "idx": int(m.group(3))}
    m = re.match(r"\w+:\s*reorder\s+([\d,]+)\s*@\s+(\d+)\s*-\s*(\d+)", rule)
    if m:
        return {"op": "reorder", "order": m.group(1),
                "start": int(m.group(2)), "end": int(m.group(3))}
    m = re.match(r"\w+:\s*(\$\w+)\s*<->\s*(\$\w+)(?:\s*@\s*(\d+)\s*-\s*(\d+))?", rule)
    if m:
        out = {"op": "swap", "reg_a": m.group(1), "reg_b": m.group(2)}
        if m.group(3):
            out["range"] = f"{m.group(3)}-{m.group(4)}"
        return out
    m = re.match(r"\w+:\s*fill_delay\s+@\s+(\d+)\s*<-\s*(\d+)", rule)
    if m:
        return {"op": "fill_delay", "jal_idx": int(m.group(1)),
                "src_idx": int(m.group(2))}
    m = re.match(r"\w+:\s*drain_delay\s+@\s+(\d+)", rule)
    if m:
        return {"op": "drain_delay", "jal_idx": int(m.group(1))}
    return {"op": "unknown", "raw": rule}


def classify_pattern(c_edits: dict, parsed_rules: list[dict]) -> tuple[str, str]:
    """Match the (c_edits, regfix_rules) shape to a known recipe name.
    Returns (recipe_name, reason). Returns ('novel', '...') if no match."""
    ops = Counter(r["op"] for r in parsed_rules)

    # Existing recipes:
    #   register_swap_simple        -- 1 swap rule (+/- range), no C edits
    #   lui_ori_sw_offset_fold      -- 1 subst + 1 delete on adjacent indices
    #   licm_unhoist                -- multiple delete + insert + insert_after
    #                                  + 1 swap (cascade-rename)
    #   signed_load_widening        -- C-side extern type widening, no regfix
    #   delay_slot_fill             -- 1 fill_delay rule

    if (c_edits.get("byte_arith_fix") or c_edits.get("byte_array_index")) \
            and ops["drain_delay"] >= 1 \
            and ops["subst"] == 0:
        return ("byte_indexing_with_drain_delay",
                "byte-arithmetic C edit + drain_delay regfix "
                "(matches func_80078B3C / func_80078B70 pattern)")

    if (c_edits.get("byte_arith_fix") or c_edits.get("byte_array_index")) \
            and ops["subst"] >= 2 \
            and ops["drain_delay"] == 0:
        return ("byte_indexing_with_la_fold_undo",
                "byte-arithmetic C edit + multiple substs to undo "
                "GCC's la-fold optimization (matches func_800550E8 pattern)")

    if ops["swap"] == 1 and ops["subst"] == 0 \
            and not c_edits.get("byte_arith_fix") \
            and not c_edits.get("byte_array_index"):
        return ("register_swap_simple",
                "single full-range register swap, no C-level edits")

    if ops["subst"] >= 2 and len(set(r.get("idx") for r in parsed_rules
                                     if r["op"] == "subst")) == 1:
        return ("3cycle_full_insn_subst",
                "multiple substs at the SAME index (likely 3-register cycle "
                "needing full-instruction subst -- matches func_8007C748)")

    if ops["drain_delay"] >= 1 and ops["subst"] == 0 \
            and not c_edits.get("byte_arith_fix"):
        return ("drain_delay_only",
                "single drain_delay rule (matches delay_slot_fill recipe inverse)")

    if ops["fill_delay"] >= 1:
        return ("delay_slot_fill",
                "fill_delay rule applied")

    if ops["subst"] == 1 and ops["delete"] == 1:
        substs = [r for r in parsed_rules if r["op"] == "subst"]
        deletes = [r for r in parsed_rules if r["op"] == "delete"]
        if abs(substs[0]["idx"] - deletes[0]["idx"]) == 1:
            return ("lui_ori_sw_offset_fold",
                    "single subst + adjacent delete pattern")

    if ops["subst"] >= 2 and ops["swap"] == 0:
        return ("multi_subst_register_renames",
                f"{ops['subst']} substs across "
                f"{len(set(r.get('idx') for r in parsed_rules))} indices "
                "(plain register renames)")

    return ("novel", f"unfamiliar combo: {dict(ops)}")


def draft_new_recipe(func: str, name: str, reason: str,
                     c_edits: dict, parsed_rules: list[dict]) -> dict:
    """Build a recipe JSON skeleton for a novel pattern."""
    structural = []
    if c_edits.get("byte_arith_fix"):
        structural.append("base.c does pointer-scaling: `(v * N) + &EXTERN`")
    if c_edits.get("byte_array_index"):
        structural.append("base.c uses `*(&EXTERN + (v * N))` array-deref form")
    if c_edits.get("extern_added"):
        structural.append("missing extern declaration was added during integration")
    if c_edits.get("volatile_pointer"):
        structural.append("a volatile pointer cast was needed")

    rule_skeleton = []
    for r in parsed_rules:
        rule_skeleton.append({k: v for k, v in r.items() if k != "raw"})

    return {
        "name": name,
        "title": f"AUTOGEN: pattern from {func} -- review and rename",
        "summary": f"Captured by capture_recipe.py from a successful match.\n"
                   f"Reason: {reason}",
        "from_commit": func,
        "fingerprint": {
            "structural": structural or [
                f"{c_edits['added_lines']}+/{c_edits['removed_lines']}- C lines, "
                f"{len(parsed_rules)} regfix rules"
            ],
            "blocker_class": name,
        },
        "rule_skeleton": rule_skeleton,
        "notes": [
            "AUTOGENERATED -- review the fingerprint and rule_skeleton "
            "before relying on this recipe.",
            "Promote to a hand-curated recipe by replacing the title + "
            "summary and tightening the structural fingerprint.",
        ],
    }


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("commit", nargs="?", default="HEAD",
                    help="Commit ref to analyze (default: HEAD)")
    ap.add_argument("--write", action="store_true",
                    help="If pattern is novel, save the draft JSON to "
                    "tools/recipes/<name>.json (with -draft suffix)")
    ap.add_argument("--json", action="store_true",
                    help="Emit just the analysis dict")
    args = ap.parse_args()

    info = get_commit_info(args.commit)
    if info["func"] is None:
        print(f"WARN: couldn't extract function name from commit "
              f"subject: {info['subject']}", file=sys.stderr)

    rules = collect_regfix_rules(info["diff"], info["func"])
    parsed = [parse_regfix_rule(r) for r in rules]
    c_edits = collect_c_edits(info["diff"])

    name, reason = classify_pattern(c_edits, parsed)

    summary = {
        "commit": info["commit"],
        "subject": info["subject"],
        "func": info["func"],
        "regfix_rules": rules,
        "parsed_rules": parsed,
        "c_edits": c_edits,
        "matched_recipe": name,
        "reason": reason,
    }

    if args.json:
        print(json.dumps(summary, indent=2))
        return 0

    print(f"=== capture_recipe: {info['commit']} ===")
    print(f"  subject : {info['subject']}")
    print(f"  func    : {info['func'] or '(unknown)'}")
    print(f"  C edits : {dict((k, v) for k, v in c_edits.items() if v and k not in ('added_lines','removed_lines'))}")
    print(f"            +{c_edits['added_lines']}/-{c_edits['removed_lines']} lines")
    print(f"  regfix  : {len(rules)} rules ({Counter(r['op'] for r in parsed)})")
    print()
    if name == "novel":
        print(f"NOVEL pattern. Reason: {reason}")
        draft = draft_new_recipe(info["func"] or "anonymous",
                                 f"{info['func'] or 'anon'}_pattern_draft",
                                 reason, c_edits, parsed)
        if args.write:
            target = RECIPES_DIR / f"{draft['name']}.json"
            target.write_text(json.dumps(draft, indent=2),
                              encoding="utf-8", newline="\n")
            print(f"  wrote {target.relative_to(ROOT)} (review before promoting)")
        else:
            print("\nDraft recipe (use --write to save to tools/recipes/):")
            print(json.dumps(draft, indent=2))
    else:
        print(f"Matches recipe: {name}")
        print(f"  reason: {reason}")
        existing = RECIPES_DIR / f"{name}.json"
        if existing.exists():
            print(f"  recipe file: {existing.relative_to(ROOT)}")
        else:
            print(f"  (no JSON file yet for '{name}' -- consider creating one)")

    return 0


if __name__ == "__main__":
    sys.exit(main())
