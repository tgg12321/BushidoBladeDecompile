# Model-capability experiment — Sonnet worker lanes (started 2026-09-15)

**Owner directive 2026-09-15:** move every worker lane to Sonnet 5, keep the two GATE
lanes on Opus 5, and observe. The question being answered: *how much of the grind
actually needs frontier reasoning, and how much is mechanical work that a cheaper
model can do?*

This is the first lane change in project history driven by a **quality question**
rather than by allowance exhaustion. All six prior flip-flops (2026-08-12, 09-06,
09-07, 09-10, 09-15) were 429-driven; none produced a quality finding, so there is
no prior art and no controlled comparison on the record.

## Configuration under test

| Lane | Before (Fable era) | Under test |
|---|---|---|
| execution (structural, rederive, synthesis, forensics, solver, escalation, permuter, enumerate, annotation-fix) | Fable 5.1 | **Sonnet 5** |
| recon | Fable 5.1 | **Sonnet 5** |
| object-model | Fable 5.1 | **Sonnet 5** |
| Judge (final call) | Opus 5 | Opus 5 (unchanged) |
| layer-1 cheat-reviewer | Opus 5 | Opus 5 (unchanged) |
| `$FallbackModel` (429 safety) | Opus 5 | Opus 5 (unchanged) |

**Contamination source to control for:** a Sonnet lane that hits a usage-limit 429
falls back to Opus 5 for the rest of that window. Those sessions are journaled as
`model-fallback session claude-sonnet-5[1m]->claude-opus-5[1m] until HH:MM` and must
be excluded from the comparison.

## Control — Fable era, 2026-09-02 .. 2026-09-15

Measured from `docs/grind/journal.md` at the moment of the switch.

| Metric | Value |
|---|---|
| Sessions | 515 |
| Completions | 82 |
| **Completions per session** | **0.159** |
| Layer-1 FAILs | 41 |
| **Layer-1 FAILs per completion** | **0.50** |
| Judge FAILs | 5 |
| One-session (recon) closes | 40 = 49% of completions |

All-time context at the switch: 270 completions / 1,622 sessions (mean 6.0). The
distribution is bimodal — 181 completions (67%) closed in 1-2 sessions for 17% of
total effort, while 30 functions at >=10 sessions consumed 1,094 sessions (67%).

## What to watch, in priority order

1. **Completions per session** — the headline. A drop toward ~0.08 means Sonnet is
   roughly half as productive per session and the tier is not paying for itself.
2. **Layer-1 FAILs per completion** — the cheat-drift rate. Baseline is already 0.50
   from a *frontier* model, so cheat drift is intrinsic to the execution lane, not a
   capability artifact. A large rise means more retries; it does NOT mean cheats are
   reaching `main` (the gates are unchanged and default-FAIL).
3. **One-session recon closes** — the sharpest single signal. Baseline 40/82. Recon
   asks the model to read 150-180 insns of MIPS and write a correct C body first try;
   if that collapses, the "recon is frontier work" claim is confirmed directly.
4. **False exhaustion claims** — the expensive, hard-to-see failure. Watch for
   `escalation`/`synthesis` sessions banking class-kills or foreclosures on thin
   predicates. The synthesis brief's CONTRADICTION RULE is the existing backstop.
5. **Floor-drop per session on the long tail** — whether Sonnet can still move
   functions that need GCC-internals modelling (global.c allocno priority, sched.c
   birthing_insn_p, combine.c added_sets_2, loop.c biv/giv).

## Safety argument for running this at all

No cheat can reach `main` on the strength of this change: the layer-1 cheat-reviewer
and the Judge are both unchanged Opus 5, both default-FAIL, and bytes are proven
before the Judge ever rules. The realistic downside is wasted sessions (retries,
weak re-derivations), not a corrupted tree. The one historical cheat wave in this
project — the fleet-era regressions folded back in by
`tools/grinder/reopen_regressions.py` under the 2026-07-06 ruling — came from a
workflow that had *no layer-1 gate at all*; it was a process gap, not a model gap.

## Result

_To be filled in. Re-measure with the same journal greps over the Sonnet window and
compare against the control table above._
