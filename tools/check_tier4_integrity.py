#!/usr/bin/env python3
"""Standing Tier-4 integrity guard.

Every function marked `done` in the engine queue MUST be genuinely Tier-4 — zero
regfix/asmfix rules AND no tier-3 inline asm in source — UNLESS it is authorized
canonical-asm (listed in inline_asm_canonical.txt). Nothing else is a legitimate
finished state.

This is the audit backstop to the two PREVENTION mechanisms (engine/queue.py):
  - mark_done() refuses to record a function as done if it carries rules or
    non-canonical tier-3 inline asm;
  - generate() re-validates sticky 'done' entries and re-opens any that fail.
This tool DETECTS any cheated 'done' that slipped a weaker/older gate or a manual
edit. Exit code 1 on any violation; run it manually, in CI, or from a hook.

(A fully-pure-C done function is DROPPED from the queue by regen — 0 rules, 0
tier-3, stripped distance 0 — so it is clean by construction and simply not
listed. Only done entries that remain in the queue are checked here.)
"""
import json
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(REPO))
from engine import cheats, inlineasm  # noqa: E402

QUEUE = REPO / "engine" / "queue.json"


def main() -> int:
    if not QUEUE.exists():
        print("no engine/queue.json")
        return 0
    q = json.loads(QUEUE.read_text())
    canon = cheats.canonical_asm_funcs()
    done = [it for it in q.get("items", []) if it.get("status") == "done"]
    violations = []
    for it in done:
        f, stem = it["func"], it["file"]
        rules = (len(cheats.func_rule_lines(f, cheats.REGFIX))
                 + len(cheats.func_rule_lines(f, cheats.REGFIX2))
                 + len(cheats.func_rule_lines(f, cheats.ASMFIX)))
        t3 = inlineasm.file_func_tier3_count(stem, f)
        is_canon = f in canon
        tag = "canonical-asm" if is_canon else "pure-C"
        flag = ""
        if rules > 0:
            violations.append(f"{f} ({stem}.c): DONE but carries {rules} regfix/asmfix rule(s)")
            flag = "  ** RULES"
        elif t3 > 0 and not is_canon:
            violations.append(f"{f} ({stem}.c): DONE with {t3} tier-3 inline-asm block(s), NOT canonical-asm")
            flag = "  ** TIER-3 CHEAT"
        print(f"  done: {f:30} ({stem}.c)  [{tag}] rules={rules} tier3={t3}{flag}")
    if violations:
        print(f"\nTIER-4 INTEGRITY VIOLATIONS ({len(violations)}):")
        for v in violations:
            print(f"  ** {v}")
        print("\nFix: retire to pure C (strip the tier-3 / rules), or authorize as "
              "canonical-asm in inline_asm_canonical.txt with evidence.")
        return 1
    print(f"\nOK: all {len(done)} queue-done functions are Tier-4 pure C or authorized canonical-asm.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
