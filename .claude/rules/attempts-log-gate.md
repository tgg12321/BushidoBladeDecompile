---
name: attempts-log-gate
paths: ["src/*.c", "asmfix.txt", "inline_asm_canonical.txt"]
description: "Escape-valve commits (INLINE_MOVE_ALIASING / asmfix bridge / inline_asm_canonical) are gated on .bb2_attempts/<func>.jsonl showing ≥4 distinct technique categories, ≥6 attempts, ≥30 cumulative minutes."
metadata:
  type: rules
---

# Why this exists

The documented escape valves (`INLINE_MOVE_ALIASING` in
[[inline-move-aliasing]], asmfix bridges, `inline_asm_canonical.txt`
entries) accept "≥2 failed pure-C attempts" as justification. That bar
is trivially met — an agent who tries 3 variations of "different
declaration order" and one short permuter run claims "exhausted."

The 2026-05-18 cheat-cleanup-side session demonstrated this exact
failure mode on `func_800483DC`: 3 attempts, all in one technique
category, single-seed permuter run, then jumped straight to
`INLINE_MOVE_ALIASING`. The user pushed back: "Are we confident this
is canonical, or did we just fail to crack the C?"

This rule encodes the missing structural enforcement so future agents
can't take the same shortcut.

# The gate

`tools/hooks/escape_valve_guard.sh` fires at `git commit` time and
detects staged additions of:

- New `INLINE_MOVE_ALIASING:` comment block in `src/*.c`
- New `<func>: replace_with_asmfile "..."` line in `asmfix.txt`
  (uncommented; commented-out `# RETIRE:` lines are *removing* a
  bridge and are not flagged)
- New `<func>` line in `inline_asm_canonical.txt`

For each affected function, the hook runs `tools/check_attempts.py
<func>`, which validates `.bb2_attempts/<func>.jsonl` against:

| Threshold | Value | Why |
|---|---|---|
| `MIN_CATEGORIES` | 4 distinct | Catches "3 variations of decl reorder" — forces breadth, not depth-of-one |
| `MIN_ATTEMPTS` | 6 total | Catches "I tried 4 things and gave up" |
| `MIN_MINUTES` | 30 cumulative | Catches "I spent 10 min and called it done" |

A commit failing the gate is BLOCKED with the specific missing
categories surfaced to the agent.

# Technique categories (each attempt logs ONE)

Defined in `tools/check_attempts.py::CATEGORIES`:

- **C_structure** — body structural change: decl reorder, intermediate
  vars, loop reshape, expression rearrangement
- **C_types** — signature type changes, pointer-vs-int, cast-style
  changes
- **C_hints** — `volatile` annotations, `register asm("$N")` pin
  exploration (note: pins are a *hint*, not the escape valve itself)
- **permuter_seed** — permuter run with specific seed (count separate
  entries per distinct seed value)
- **permuter_long** — permuter with `--max-time >=600s` and
  `--max-flat >=120s`
- **m2c_restart** — fresh-start from m2c's auto-decompile of `target.s`
- **manual_steer** — single-instruction inline asm for codegen control,
  NOT the full `INLINE_MOVE_ALIASING` template (e.g. lone `lui` to
  force constant materialization position)
- **sibling_match** — compare to / borrow from an already-matched
  sibling function with similar shape

# How to log an attempt

```
bash tools/dc.sh log-attempt <func> <category> <variant_name> <minutes> '<outcome>'
```

The outcome string must be ≥20 characters AND should reference a
concrete observation — diff count, permuter score, specific GCC
behavior, error message. Vague outcomes ("didn't work") are caught
by the LLM auditor later.

Logs live at `.bb2_attempts/<func>.jsonl` (one JSON object per line,
append-only). They're git-tracked so future agents inherit history
and don't redo the same attempts.

Check status anytime: `bash tools/dc.sh check-attempts <func>`.

# Override path (rare)

When the user explicitly authorizes an escape valve after observing
exhausted effort, add an OVERRIDE entry:

```
bash tools/dc.sh log-attempt <func> OVERRIDE override \
  "user approved 2026-XX-XX after N-hour effort: <details>"
```

OVERRIDE entries satisfy ALL gate criteria. Use sparingly — the LLM
audit will scrutinize OVERRIDE commits more carefully than
gate-passed ones.

# Related rules

- [[inline-move-aliasing]] — the escape valve this gate sits in front of
- [[hard-rule]] — the broader "no premature stopping" contract
- [[no-voluntary-stops]] — context fatigue is the root failure mode
  this gate counters
