#!/usr/bin/env python3
"""Named regfix recipes -- list / suggest / apply.

A recipe is a small JSON file in tools/recipes/<name>.json that documents a
known matching pattern with: a fingerprint, the rule_skeleton needed, the
memory entry it was derived from, and notes/gotchas. This is the seed of
cross-function recipe propagation -- when a new function shows the same
pattern, you can pull up the recipe and apply it without rediscovering.

Subcommands:

    list                       Print every recipe and its one-line summary.
    show <recipe>              Pretty-print one recipe in detail.
    suggest <func>             Heuristically score every recipe against the
                               function's diff and rank the top N matches.
    apply <recipe> <func>      Print the apply plan -- the concrete add-regfix
                               commands and any source-file fix-ups. Does NOT
                               modify files; copy/paste the commands once you
                               fill in the indices.

Recipes are aware of `from_memory:` pointers. If you discover a new pattern,
write a `feedback_*.md` memory entry first, then drop a sibling json here.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
RECIPE_DIR = Path(__file__).resolve().parent / "recipes"


def load_recipes() -> list[dict]:
    out = []
    if not RECIPE_DIR.exists():
        return out
    for p in sorted(RECIPE_DIR.glob("*.json")):
        try:
            d = json.loads(p.read_text(encoding="utf-8"))
            d["__path"] = str(p.relative_to(ROOT))
            out.append(d)
        except json.JSONDecodeError as e:
            print(f"WARN: {p.name}: {e}", file=sys.stderr)
    return out


def cmd_list(args):
    recs = load_recipes()
    if not recs:
        print("(no recipes in tools/recipes/)")
        return 0
    for r in recs:
        print(f"  {r['name']:30s}  {r.get('title', '')}")
        if args.verbose:
            print(f"    {r['__path']}")
            print(f"    derived from: {r.get('from_memory', '?')}")
            print()
    return 0


def cmd_show(args):
    for r in load_recipes():
        if r["name"] == args.name or r["__path"].endswith(args.name + ".json"):
            print(f"# {r['name']} -- {r.get('title', '')}")
            print(f"#   path: {r['__path']}")
            print(f"#   from: {r.get('from_memory', '?')}")
            print()
            if r.get("summary"):
                print("summary:")
                for line in r["summary"].splitlines():
                    print(f"  {line}")
                print()
            if r.get("fingerprint"):
                print("fingerprint:")
                fp = r["fingerprint"]
                if "blocker_class" in fp:
                    print(f"  blocker_class: {fp['blocker_class']}")
                for s in fp.get("structural", []):
                    print(f"  - {s}")
                print()
            if r.get("rule_skeleton"):
                print("rule_skeleton:")
                for step in r["rule_skeleton"]:
                    op = step.get("op", "?")
                    print(f"  - {op}: {json.dumps({k: v for k, v in step.items() if k != 'op'})}")
                print()
            if r.get("notes"):
                print("notes:")
                for n in r["notes"]:
                    print(f"  * {n}")
            return 0
    print(f"ERROR: recipe '{args.name}' not found", file=sys.stderr)
    return 1


# --- Suggestion: cheap heuristic over generated regfix output --------------

def suggest_for_func(func: str) -> list[tuple[str, int, list[str]]]:
    """Run gen_regfix.py and asm_analysis.py to get a quick fingerprint for
    `func`, then score each recipe by how well its blocker class fits.

    Returns [(recipe_name, score, reasons), ...] sorted desc."""
    reasons_per_recipe: dict[str, list[str]] = {}
    score_per_recipe: dict[str, int] = {}

    # 1. Try gen_regfix to get a sense of the diff shape
    try:
        r = subprocess.run(
            [sys.executable, str(Path(__file__).parent / "gen_regfix.py"), func],
            capture_output=True, text=True, cwd=str(ROOT), timeout=60,
        )
        gr_text = r.stdout + "\n" + r.stderr
    except Exception:
        gr_text = ""

    # 2. asm_analysis to see if the function has loop / hoist signals
    try:
        r = subprocess.run(
            [sys.executable, str(Path(__file__).parent / "asm_analysis.py"),
             str(ROOT / "asm" / "funcs" / f"{func}.s")],
            capture_output=True, text=True, cwd=str(ROOT), timeout=30,
        )
        asm_text = r.stdout
    except Exception:
        asm_text = ""

    haystack = gr_text + "\n" + asm_text

    # 3. Score each recipe
    for rec in load_recipes():
        score = 0
        reasons: list[str] = []
        bclass = rec.get("fingerprint", {}).get("blocker_class", "")

        if bclass == "register_cycle_2":
            if re.search(r"swap\b|<->", gr_text):
                score += 3
                reasons.append("gen_regfix proposed a register swap")
            if re.search(r"\$\d+\s*<->\s*\$\d+", gr_text):
                score += 2
        elif bclass == "ori_sw_offset_fold":
            if re.search(r"\bori\s+\$\d+,\$\d+,\d", haystack) and re.search(r"\bsw\s+\$\d+,0\(\$\d+\)", haystack):
                score += 3
                reasons.append("ori-then-sw(0) sequence visible")
        elif bclass == "licm_hoist":
            if "loop" in asm_text.lower() and re.search(r"\blui\b.*\bori\b", haystack, re.DOTALL):
                score += 2
                reasons.append("loop+lui+ori signature visible")
            if "preheader" in asm_text.lower():
                score += 1
        elif bclass == "delay_slot_unfilled":
            if re.search(r"\bnop\b", gr_text) and re.search(r"\bjal\b|\bbranch\b", gr_text):
                score += 2
                reasons.append("nop+branch nearby in gen_regfix output")
        elif bclass == "signed_load_mismatch":
            if re.search(r"\blhu\b|\blbu\b", gr_text) and re.search(r"\blh\b|\blb\b", gr_text):
                score += 3
                reasons.append("lh/lhu (or lb/lbu) flip visible in diff")

        if score:
            score_per_recipe[rec["name"]] = score
            reasons_per_recipe[rec["name"]] = reasons

    ranked = sorted(score_per_recipe.items(), key=lambda kv: -kv[1])
    return [(name, sc, reasons_per_recipe[name]) for name, sc in ranked]


def cmd_suggest(args):
    suggestions = suggest_for_func(args.func)
    if not suggestions:
        print(f"(no recipe fingerprints matched {args.func})")
        return 0
    for name, score, reasons in suggestions[:args.top]:
        print(f"  {name:30s}  score={score}")
        for r in reasons:
            print(f"    - {r}")
    return 0


def cmd_apply(args):
    recs = load_recipes()
    rec = next((r for r in recs if r["name"] == args.recipe), None)
    if rec is None:
        print(f"ERROR: recipe '{args.recipe}' not found "
              f"(known: {', '.join(r['name'] for r in recs)})", file=sys.stderr)
        return 1

    print(f"# Apply plan for recipe '{rec['name']}' on function {args.func}")
    print(f"# Source: {rec['__path']}")
    if rec.get("from_memory"):
        print(f"# Memory ref: {rec['from_memory']}")
    print()

    # First, dump current TEXT indices so the user can pick the right N
    print("Step 1 -- get TEXT indices for the function:")
    print(f"  bash tools/dc.sh dump-text {args.func} | head -60")
    print()

    print("Step 2 -- fill in concrete indices/registers, then run:")
    for step in rec.get("rule_skeleton", []):
        op = step["op"]
        if op == "swap":
            cmd = (f"  bash tools/dc.sh add-regfix swap {args.func} "
                   f"'{step['reg_a']}' '{step['reg_b']}' "
                   f"--range {step.get('range', 'N-M')}")
            print(cmd + (f"   # {step['comment']}" if step.get("comment") else ""))
        elif op == "subst":
            cmd = (f"  bash tools/dc.sh add-regfix subst {args.func} "
                   f"'{step['pattern']}' '{step['replacement']}' "
                   f"--idx {step.get('idx', 'N')}")
            print(cmd + (f"   # {step['comment']}" if step.get("comment") else ""))
        elif op == "delete":
            cmd = (f"  bash tools/dc.sh add-regfix delete {args.func} "
                   f"--idx {step.get('idx', 'N')}")
            print(cmd + (f"   # {step['comment']}" if step.get("comment") else ""))
        elif op == "insert":
            cmd = (f"  bash tools/dc.sh add-regfix insert {args.func} "
                   f"'{step.get('asm_text', 'asm here')}' "
                   f"--idx {step.get('idx', 'N')}")
            print(cmd + (f"   # {step['comment']}" if step.get("comment") else ""))
        elif op == "insert_after":
            cmd = (f"  bash tools/dc.sh add-regfix insert_after {args.func} "
                   f"'{step.get('asm_text', 'asm here')}' "
                   f"--idx {step.get('idx', 'N')}")
            print(cmd + (f"   # {step['comment']}" if step.get("comment") else ""))
        elif op == "fill_delay":
            cmd = (f"  bash tools/dc.sh add-regfix fill_delay {args.func} "
                   f"--jal-idx {step.get('jal_idx', 'N')} "
                   f"--src-idx {step.get('src_idx', 'M')}")
            print(cmd + (f"   # {step['comment']}" if step.get("comment") else ""))
        elif op == "fix_at_source":
            print(f"  # source-side fix: {step.get('instruction', '')}")
        else:
            print(f"  # unknown op '{op}' -- see recipe notes")

    print()
    if rec.get("notes"):
        print("Notes:")
        for n in rec["notes"]:
            print(f"  * {n}")
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd")

    p = sub.add_parser("list")
    p.add_argument("-v", "--verbose", action="store_true")
    p.set_defaults(_fn=cmd_list)

    p = sub.add_parser("show")
    p.add_argument("name")
    p.set_defaults(_fn=cmd_show)

    p = sub.add_parser("suggest")
    p.add_argument("func")
    p.add_argument("--top", type=int, default=5)
    p.set_defaults(_fn=cmd_suggest)

    p = sub.add_parser("apply")
    p.add_argument("recipe")
    p.add_argument("func")
    p.set_defaults(_fn=cmd_apply)

    args = ap.parse_args()
    if args.cmd is None:
        ap.print_help()
        return 1
    return args._fn(args)


if __name__ == "__main__":
    sys.exit(main())
