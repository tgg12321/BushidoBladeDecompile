---
name: attempts-log-gate
paths: ["src/*.c", "asmfix.txt", "inline_asm_canonical.txt"]
description: "Gates a canonical-asm authorization (inline_asm_canonical entry) on .bb2_attempts/<func>.jsonl showing >=4 technique categories, >=6 attempts, >=30 min of pure-C effort. NOTE (2026-05-21): INLINE_MOVE_ALIASING + new regfix/pins are now HARD-BLOCKED by audit_asm_cheats, not gated — this log is evidence for the canonical exception, not a key that unlocks tier-3."
metadata:
  type: rules
---

> **Scope narrowed 2026-05-21 ([[tier4-sota-standard]]).** `INLINE_MOVE_ALIASING`,
> new regfix rules, and new `register asm` pins are **no longer escape valves** —
> they are **hard-blocked** by `audit_asm_cheats.py --check-new` (cannot be
> committed at all, with or without an attempt log). This gate's remaining job is
> to require genuine breadth-of-pure-C-effort **before a canonical-asm
> authorization** (`inline_asm_canonical.txt` entry) — it is *evidence for the
> canonical exception*, not a key that unlocks tier-3 debt. A new
> `replace_with_asmfile` bridge also still requires explicit user authorization
> ([[bridge-is-not-decomp]]). The technique categories below remain useful as a
> record of what pure-C avenues were tried.

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
