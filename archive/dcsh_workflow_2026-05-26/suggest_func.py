#!/usr/bin/env python3
"""Per-function technique surfacer.

Reads classify_func + bridge/retire state + (when available) diff_summary,
and prints a short prioritized "do this, don't do that, see these files"
rollup. Designed to be cheap enough to run on every function and short
enough to actually be read.

This is NOT a replacement for agent_brief (which is the full kitchen-sink
dump) — it's the rollup the agent should look at first to pick a direction.

Pre-build mode (no build artifact for this function yet):
    Surfaces orientation based on classify + state. "What kind of function
    is this; what techniques tend to work; what NOT to reach for."

Post-build mode (build artifact present):
    Adds diff_summary verdict + recipes suggest output + diff-shape hints
    (cascade-misread warning, byte_diff/4 single-instruction signal).

Usage:
    python3 tools/suggest_func.py <func>
    python3 tools/suggest_func.py <func> --json
    python3 tools/suggest_func.py <func> --mode pre|post|auto
"""
from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TOOLS = ROOT / "tools"
ASMFIX = ROOT / "asmfix.txt"
BUILD_ELF = ROOT / "build" / "bb2.elf"
WORK_QUEUE = ROOT / "WORK_QUEUE.md"


# ---------------- State detection -------------------------------------------

def is_bridged(func: str) -> bool:
    if not ASMFIX.is_file():
        return False
    needle = f"{func}: replace_with_asmfile"
    for line in ASMFIX.read_text(encoding="utf-8", errors="replace").splitlines():
        s = line.lstrip()
        if s.startswith("#"):
            continue
        if s.startswith(needle):
            return True
    return False


def is_retiring(func: str) -> bool:
    if not ASMFIX.is_file():
        return False
    needle = f"# RETIRE: {func}: replace_with_asmfile"
    for line in ASMFIX.read_text(encoding="utf-8", errors="replace").splitlines():
        if line.lstrip().startswith(needle):
            return True
    return False


def queue_tags(func: str) -> list[str]:
    """Tags from WORK_QUEUE.md for this function (e.g., 'inline_asm_debt')."""
    if not WORK_QUEUE.is_file():
        return []
    text = WORK_QUEUE.read_text(encoding="utf-8", errors="replace")
    tags: list[str] = []
    for line in text.splitlines():
        if func not in line:
            continue
        for tag in ("inline_asm_debt", "aliasing_heavy", "needs_function_split",
                    "needs_lwl_fix", "needs_delay_slot_ra", "gte_function",
                    "asmfix_retirement"):
            if tag in line:
                tags.append(tag)
        break
    return sorted(set(tags))


def has_build_artifact() -> bool:
    return BUILD_ELF.is_file()


def function_state(func: str, classify: dict) -> str:
    # Bridge state wins over classification — a bridged function's classify
    # signals describe the target, not the live build path.
    if is_bridged(func):
        return "BRIDGED"
    if is_retiring(func):
        return "RETIRING"
    kind = classify.get("src", {}).get("kind")
    qtags = queue_tags(func)
    # inline_asm_debt is a queue verdict: "the inline __asm__ block here is
    # non-canonical and needs narrowing." It wins over INLINE_AUTHORIZED
    # because the work-item still exists.
    if kind == "inline_asm" and "inline_asm_debt" in qtags:
        return "INLINE_DEBT"
    if classify.get("recommendation", "").startswith("permanently_blocked:"):
        if kind == "inline_asm":
            return "INLINE_AUTHORIZED"
        return "PERMANENT_BLOCK"
    if kind == "c_function":
        return "C_BODY"
    if kind == "include_asm":
        return "INCLUDE_ASM_STUB"
    return "NORMAL"


# ---------------- Subprocess wrappers ---------------------------------------

def run_json(argv: list[str], timeout: int = 60) -> dict | None:
    try:
        r = subprocess.run(argv, capture_output=True, text=True,
                           cwd=str(ROOT), timeout=timeout)
    except (subprocess.TimeoutExpired, OSError):
        return None
    if not r.stdout.strip():
        return None
    try:
        return json.loads(r.stdout)
    except json.JSONDecodeError:
        return None


def classify_json(func: str) -> dict | None:
    return run_json([sys.executable, str(TOOLS / "classify_func.py"),
                     func, "--json"], 30)


def diff_summary_json(func: str) -> dict | None:
    return run_json([sys.executable, str(TOOLS / "diff_summary.py"),
                     func, "--json"], 60)


def recipes_suggest_text(func: str, limit: int = 3) -> list[str]:
    """Return the top `limit` recipe lines, raw, or [] on failure."""
    try:
        r = subprocess.run(
            [sys.executable, str(TOOLS / "recipes.py"), "suggest", func],
            capture_output=True, text=True, cwd=str(ROOT), timeout=60,
        )
    except (subprocess.TimeoutExpired, OSError):
        return []
    lines = [ln.rstrip() for ln in r.stdout.splitlines() if ln.strip()]
    out: list[str] = []
    for ln in lines:
        if not ln.startswith("  ") and not ln[:1].isdigit():
            continue
        out.append(ln.strip())
        if len(out) >= limit:
            break
    return out


# ---------------- Suggestion engine ----------------------------------------

# Each pick is: (title, [actions], [see refs])
Pick = tuple[str, list[str], list[str]]


def picks_for_state(state: str, func: str) -> list[Pick]:
    picks: list[Pick] = []
    if state == "BRIDGED":
        picks.append((
            "RETIREMENT WORKFLOW (function is bridged)",
            [f"`dc.sh retire {func}` — comment out the bridge in asmfix.txt",
             f"then `dc.sh build-active {func}` for bridge-aware verify",
             "use `dc.sh verify-c` (NOT plain `dc.sh verify` — bridge-blind)"],
            ["feedback_bridge_is_not_decomp.md",
             "feedback_retire_auxiliary_asmfix.md",
             "feedback_quick_reference.md G2"],
        ))
    elif state == "RETIRING":
        picks.append((
            "RETIRING — bridge already commented, iterate on the C body",
            [f"`dc.sh build-active {func}` rebuilds with auto-repair",
             f"`dc.sh diff-summary {func}` for categorized verdict",
             "`dc.sh preflight-cascade <func>` to see drift surface"],
            ["feedback_auto_drift_repair.md",
             "feedback_retirement_recipes.md"],
        ))
    elif state == "INLINE_DEBT":
        picks.append((
            "INLINE-ASM CHEAT — currently a non-canonical __asm__ block",
            ["read the existing inline asm in src/ (find via `dc.sh inline-locate`)",
             "if it's hand-coded asm style (GTE primitive, BIOS, etc.) → maybe authorize",
             "otherwise → decomp to pure C; the inline asm is the cheat being removed"],
            ["feedback_hand_coded_asm_recognition.md",
             "project_inline_asm_offenders.md"],
        ))
    elif state == "INLINE_AUTHORIZED":
        picks.append((
            "INLINE-ASM AUTHORIZED — already a canonical __asm__ wrapper",
            ["check inline_asm_canonical.txt for the auth entry",
             f"`dc.sh inline-verify {func}` to confirm the block matches target",
             "if MATCH: this function is done — commit"],
            ["feedback_hand_coded_asm_recognition.md"],
        ))
    elif state == "INCLUDE_ASM_STUB":
        picks.append((
            "INCLUDE_ASM STUB — never started; standard decomp",
            [f"`dc.sh agent-brief {func}` for full context",
             f"`dc.sh attempt {func}` runs the mechanical pipeline first"],
            [],
        ))
    elif state == "C_BODY":
        picks.append((
            "C BODY — already decomped; iterate until match",
            [f"`dc.sh build-active {func}` rebuild + auto-repair drift",
             f"`dc.sh diff-summary {func}` to route the diff"],
            ["feedback_quick_reference.md"],
        ))
    elif state == "PERMANENT_BLOCK":
        picks.append((
            "PERMANENTLY BLOCKED — can't emit in pure C with GCC 2.7.2",
            ["check `known_blocked.txt` for the reason",
             "options: (a) leave as INCLUDE_ASM, (b) authorize inline asm (needs user OK)"],
            ["feedback_hand_coded_asm_recognition.md"],
        ))
    return picks


def picks_for_blockers(classify: dict, func: str) -> list[Pick]:
    picks: list[Pick] = []
    tags = set(classify.get("blocker_tags", []))
    rec = classify.get("recommendation", "")

    if "gte_ops" in tags or rec == "gte_function":
        picks.append((
            "GTE wrapper — uses cop2 ops",
            [f"`dc.sh gte {func}` shows which gte_*() macros from include/gte.h apply",
             f"`dc.sh gte-migrate {func} --setup` scaffolds C + constrained __asm__",
             "if 3x3 matrix shape → apply the gte_3x3 recipe verbatim"],
            ["feedback_gte_3x3_recipe.md",
             "feedback_scratchpad_gte_recipe.md"],
        ))
    if "lwl_swl" in tags or rec == "needs_lwl_fix":
        picks.append((
            "LWL/SWL — unaligned load/store",
            ["toggle the function in FIX_LWL_FILES",
             "use the lwl/swl-aware build path"],
            ["feedback_matching_playbook.md"],
        ))
    if rec == "needs_function_split":
        picks.append((
            "Function-split — multiple jr $ra suggest splat merged two functions",
            ["this is structural-split queue work, not normal decomp",
             "`dc.sh next-structural` if you intend to split"],
            [],
        ))
    if rec == "needs_rodata_split":
        picks.append((
            "Rodata split — jlabel/jtbl switch",
            ["needs CU split for jtbl interposition"],
            ["feedback_matching_playbook.md (CU split section)"],
        ))
    if rec == "needs_delay_slot_ra":
        picks.append((
            "Delay-slot $ra — sw $ra in branch delay slot",
            ["use prologue_fix's delay-slot-ra support"],
            [],
        ))
    if "aliasing_heavy" in tags or "aliasing_heavy" in queue_tags(func):
        picks.append((
            "Aliasing-heavy — pointer-chasing; iteration count is undercounted",
            [f"`dc.sh diff-align {func}` EARLY (cascade undercounts the real diff)",
             "expect asymmetric reload patterns; type-widening hints often help"],
            ["feedback_matching_playbook.md (aliasing section)",
             "feedback_quick_reference.md"],
        ))
    if rec.startswith("psyq_stdlib_"):
        idiom = rec[len("psyq_stdlib_"):]
        picks.append((
            f"PsyQ stdlib idiom — matches {idiom}",
            [f"replace with the libc/psyq C idiom; don't decomp the asm"],
            [],
        ))
    if rec.startswith("bios_or_syscall:"):
        picks.append((
            "BIOS / syscall trampoline — allowed-as-asm exception",
            ["this is canonical asm-only; leave as INCLUDE_ASM or inline asm"],
            [],
        ))
    return picks


def picks_for_diff(diff: dict, func: str) -> list[Pick]:
    """Diff-shape-driven picks. Layered on top of diff_summary's own routing."""
    picks: list[Pick] = []
    if diff.get("verdict") == "MATCH":
        picks.append((
            "MATCH — no diff; commit and refresh queue",
            [f"`git add` your changes; commit (hook checks verify)",
             "`dc.sh refresh-queue` after"],
            [],
        ))
        return picks

    diff_count = diff.get("diff_count", 0)
    cats = diff.get("categories", {})

    # Cascade-misread warning. The diff_summary already labels cascades, but
    # the agent often reads `dc.sh diff` (index-aligned) first and is misled.
    if diff_count > 15:
        picks.append((
            "CASCADE LIKELY — read with care",
            [f"{diff_count} 'diffs' often = 1 structural + N cascade",
             f"run `dc.sh diff-align {func}` BEFORE `dc.sh diff` or regfix-suggest",
             "compute byte_diff/4 — that's your real missing-instruction count"],
            ["feedback_quick_reference.md Part 5"],
        ))

    # Pure register-rename → swap, NOT permuter
    reg_rename = cats.get("register-rename", {}).get("count", 0)
    structural = cats.get("structural", {}).get("count", 0)
    if reg_rename >= 3 and structural == 0:
        picks.append((
            "PURE REGISTER-RENAME — regfix swap, not permuter",
            [f"`dc.sh add-regfix {func} swap $X $Y @ <idx>` per swap",
             "the permuter will not help — there is nothing structural to find"],
            ["feedback_regfix_reference.md",
             "feedback_quick_reference.md Part 5"],
        ))

    # Opcode-only diffs → subst recipes
    opcode_only = cats.get("opcode-only", {}).get("count", 0)
    if opcode_only >= 2:
        picks.append((
            "OPCODE-ONLY DIFFS — try recipe library",
            [f"`dc.sh recipes suggest {func}` — often lhu/lbu or s8/u8 cast"],
            ["feedback_matching_playbook.md"],
        ))

    # Branch-offset dominant → label drift
    branch_off = cats.get("branch-offset", {}).get("count", 0)
    if branch_off >= 2:
        picks.append((
            "BRANCH-OFFSET DRIFT — likely .L<N> renaming",
            ["`bash tools/dc.sh fix-label-drift --apply`"],
            [],
        ))

    return picks


def donts(classify: dict, state: str, diff: dict | None) -> list[str]:
    out: list[str] = []
    rec = classify.get("recommendation", "")
    insns = classify.get("size", {}).get("insns", 0)

    if state in ("BRIDGED", "RETIRING"):
        out.append("don't use plain `dc.sh verify` — bridge-blind; use `verify-c`")
    if state == "PERMANENT_BLOCK":
        out.append("don't run the pure-C pipeline — it cannot match")
    if rec == "gte_function":
        out.append("don't reach for the permuter first — GTE structure is fixed; "
                   "use the GTE recipes")
    if insns and insns < 15:
        out.append("don't run long permuter runs first — small functions rarely "
                   "have structural alternatives worth exploring")
    if diff:
        cats = diff.get("categories", {})
        if (cats.get("register-rename", {}).get("count", 0) >= 3
                and cats.get("structural", {}).get("count", 0) == 0):
            out.append("don't permute a pure register-rename diff — regfix swap")
        if (cats.get("structural", {}).get("count", 0) == 1
                and diff.get("diff_count", 0) > 5):
            out.append("don't write 20 regfix rules for a 1-structural diff — "
                       "the rest are cascade; fix the structural first")
    return out


def next_step(state: str, func: str, diff: dict | None) -> str:
    if state == "PERMANENT_BLOCK":
        return f"read known_blocked.txt entry for {func}; decide with user"
    if state == "INLINE_AUTHORIZED":
        return f"bash tools/dc.sh inline-verify {func}"
    if state == "BRIDGED":
        return f"bash tools/dc.sh retire {func}"
    if state == "RETIRING":
        return f"bash tools/dc.sh build-active {func}"
    if state == "INLINE_DEBT":
        return f"bash tools/dc.sh inline-locate {func}"
    if diff is None:
        return f"bash tools/dc.sh agent-brief {func}"
    if diff.get("verdict") == "MATCH":
        return f"git commit; bash tools/dc.sh refresh-queue"
    return f"bash tools/dc.sh diff-align {func}"


# ---------------- Output ----------------------------------------------------

def render(func: str, classify: dict, state: str, diff: dict | None,
           recipe_lines: list[str]) -> str:
    out: list[str] = []
    out.append(f"=== suggest: {func} ===")

    rec = classify.get("recommendation", "?")
    sz = classify.get("size", {})
    insns = sz.get("insns", "?")
    qtags = queue_tags(func)
    btags = classify.get("blocker_tags") or []
    ptags = classify.get("permanent_blocker_tags") or []

    out.append(f"State:   {state}")
    out.append(f"Class:   {rec}   ({insns} insns)")
    if btags:
        out.append(f"Tags:    {', '.join(btags)}")
    if ptags:
        out.append(f"PERMANENT: {', '.join(ptags)}")
    if qtags:
        out.append(f"Queue:   {', '.join(qtags)}")
    kengo = classify.get("kengo")
    if kengo:
        out.append(f"Kengo:   {kengo}")
    if diff:
        out.append(f"Build:   {diff.get('verdict', '?')}  "
                   f"({diff.get('diff_count', 0)} diff(s))")
    else:
        out.append("Build:   (no artifact yet — pre-build mode)")

    all_picks = picks_for_state(state, func)
    # In settled states (already authorized / permanently blocked / matched),
    # blocker-driven picks are noise — the function is in a terminal shape
    # and the work is verification, not technique selection.
    settled = state in ("INLINE_AUTHORIZED", "PERMANENT_BLOCK")
    if not settled:
        all_picks += picks_for_blockers(classify, func)
    if diff:
        all_picks += picks_for_diff(diff, func)

    if all_picks:
        out.append("")
        out.append("Top picks:")
        for i, (title, actions, refs) in enumerate(all_picks, 1):
            out.append(f"  {i}. {title}")
            for a in actions:
                out.append(f"     • {a}")
            if refs:
                out.append(f"     see: {', '.join(refs)}")
    else:
        out.append("")
        out.append("Top picks: (no specific recommendation — standard workflow)")
        out.append(f"  • `dc.sh agent-brief {func}` for the full context dump")
        out.append("  • see feedback_quick_reference.md Part 9 decision trees")

    if diff:
        routing = diff.get("routing")
        if routing:
            out.append("")
            out.append("Diff routing (from diff_summary):")
            for ln in routing.splitlines():
                out.append(f"  {ln}")

    if recipe_lines:
        out.append("")
        out.append("Recipe candidates (recipes suggest):")
        for ln in recipe_lines:
            out.append(f"  {ln}")

    dont = donts(classify, state, diff)
    if dont:
        out.append("")
        out.append("Don't:")
        for d in dont:
            out.append(f"  • {d}")

    out.append("")
    out.append(f"Next:    {next_step(state, func, diff)}")
    return "\n".join(out)


# ---------------- Main ------------------------------------------------------

def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func")
    ap.add_argument("--json", action="store_true",
                    help="Emit structured JSON instead of text")
    ap.add_argument("--mode", choices=("pre", "post", "auto"), default="auto",
                    help="Force pre/post mode (default auto = post if build "
                         "artifact exists)")
    args = ap.parse_args()

    classify = classify_json(args.func)
    if classify is None or not classify.get("found", True):
        print(f"suggest: {args.func} not found (or classify failed)",
              file=sys.stderr)
        return 1

    state = function_state(args.func, classify)

    use_post = (args.mode == "post"
                or (args.mode == "auto" and has_build_artifact()
                    and state in ("RETIRING", "C_BODY", "INLINE_AUTHORIZED")))
    diff = diff_summary_json(args.func) if use_post else None

    recipe_lines: list[str] = []
    if diff and diff.get("verdict") != "MATCH":
        recipe_lines = recipes_suggest_text(args.func, 3)

    if args.json:
        out = {
            "func": args.func,
            "state": state,
            "classify": classify,
            "diff": diff,
            "recipes": recipe_lines,
            "picks": [
                {"title": t, "actions": a, "refs": r}
                for t, a, r in (
                    picks_for_state(state, args.func)
                    + picks_for_blockers(classify, args.func)
                    + (picks_for_diff(diff, args.func) if diff else [])
                )
            ],
            "donts": donts(classify, state, diff),
            "next_step": next_step(state, args.func, diff),
        }
        print(json.dumps(out, indent=2))
        return 0

    print(render(args.func, classify, state, diff, recipe_lines))
    return 0


if __name__ == "__main__":
    sys.exit(main())
