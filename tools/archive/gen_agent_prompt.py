#!/usr/bin/env python3
"""Generate a ready-to-use agent prompt for a function matching task.

Reads the agent template, fills in context from pipeline report or auto-detection.
Output is a complete prompt string for the Agent tool.

Usage:
    python3 tools/gen_agent_prompt.py func_80043C7C --tier haiku
    python3 tools/gen_agent_prompt.py func_80043C7C --tier sonnet --score 150
    python3 tools/gen_agent_prompt.py func_80043C7C --from-report permuter/func_80043C7C/report.json
"""
import argparse
import json
import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TEMPLATE = ROOT / "tools" / "decomp_agent.md"
ASM_DIR = ROOT / "asm" / "funcs"
SRC_DIR = ROOT / "src"
PERMUTER_DIR = ROOT / "permuter"

TIER_CONFIG = {
    "haiku": {"max_attempts": 5, "techniques": "Tier 1 only"},
    "sonnet": {"max_attempts": 15, "techniques": "Tiers 1-2"},
    "opus": {"max_attempts": 20, "techniques": "Tiers 1-3"},
}


def find_src_file(func_name):
    for src in sorted(SRC_DIR.glob("*.c")):
        with open(src) as f:
            if f'INCLUDE_ASM("asm/funcs", {func_name})' in f.read():
                return str(src.relative_to(ROOT))
    return "unknown"


def count_insns(func_name):
    asm_file = ASM_DIR / f"{func_name}.s"
    if not asm_file.exists():
        return 0
    with open(asm_file) as f:
        # Each instruction line has a /* offset addr opcode */ prefix
        return sum(1 for line in f if "/*" in line)


def is_leaf(func_name):
    asm_file = ASM_DIR / f"{func_name}.s"
    if not asm_file.exists():
        return False
    with open(asm_file) as f:
        return "jal" not in f.read()


def main():
    parser = argparse.ArgumentParser(description="Generate agent prompt")
    parser.add_argument("func_name")
    parser.add_argument("--tier", choices=["haiku", "sonnet", "opus"], default="haiku")
    parser.add_argument("--score", type=int, default=None, help="Current score (auto-detected if omitted)")
    parser.add_argument("--src-file", default=None)
    parser.add_argument("--steps", default="pipeline not yet run")
    parser.add_argument("--from-report", help="Read context from JSON report file")
    args = parser.parse_args()

    func = args.func_name
    tier = args.tier
    cfg = TIER_CONFIG[tier]

    # Load from report if provided
    if args.from_report and os.path.exists(args.from_report):
        with open(args.from_report) as f:
            report = json.load(f)
        src_file = report.get("src_file", find_src_file(func))
        insn_count = report.get("insn_count", count_insns(func))
        leaf = report.get("leaf", is_leaf(func))
        score = report.get("score", "N/A")
        steps = " → ".join(report.get("steps", []))
    else:
        src_file = args.src_file or find_src_file(func)
        insn_count = count_insns(func)
        leaf = is_leaf(func)
        score = args.score if args.score is not None else "N/A"
        steps = args.steps

    # Read template
    with open(TEMPLATE) as f:
        prompt = f.read()

    # Fill placeholders
    prompt = prompt.replace("{{FUNC_NAME}}", func)
    prompt = prompt.replace("{{SRC_FILE}}", src_file)
    prompt = prompt.replace("{{INSN_COUNT}}", str(insn_count))
    prompt = prompt.replace("{{LEAF}}", str(leaf))
    prompt = prompt.replace("{{SCORE}}", str(score))
    prompt = prompt.replace("{{STEPS}}", steps)
    prompt = prompt.replace("{{MAX_ATTEMPTS}}", str(cfg["max_attempts"]))

    # Add tier-specific header
    tier_header = f"\n## Model Tier: {tier.upper()}\n"
    tier_header += f"Focus on: {cfg['techniques']}\n"
    if tier == "haiku":
        tier_header += "Do NOT attempt register asm or inline asm — stick to basic C transformations.\n"
        tier_header += "Hard stop at attempt 5. If score > 100, report STUCK — a stronger model will take over.\n"
    elif tier == "sonnet":
        tier_header += "You may use register asm and scheduling barriers.\n"
        tier_header += f"At attempt 8 of {cfg['max_attempts']}: pause and reassess. If score > 200, report STUCK now — don't use remaining attempts chasing diminishing returns.\n"
        tier_header += f"Hard stop at attempt {cfg['max_attempts']}. If score > 30, report STUCK — Opus will handle it.\n"
    elif tier == "opus":
        tier_header += "Full toolkit available including inline asm and advanced scheduling tricks.\n"
        tier_header += f"At attempt 10 of {cfg['max_attempts']}: pause and reassess. If score > 50 and you have no clear next move, report TABLED — don't burn remaining attempts without a hypothesis.\n"
        tier_header += f"Hard stop at attempt {cfg['max_attempts']}. Save your best base.c (it's already there), report your best score and the exact remaining diff. This function will be revisited — a clean handoff is more valuable than a desperate last attempt.\n"

    prompt = prompt + tier_header

    print(prompt)


if __name__ == "__main__":
    main()
