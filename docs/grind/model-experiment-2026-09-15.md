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

## Arm 1 result — all worker lanes on Sonnet 5 (2026-09-16 00:22 .. 10:40)

68 journaled sessions, 3 functions, **0 completions**, no model-fallback
contamination (the last fallback was 00:12, before the switch).

| Function | Sessions | Floor | Outcome |
|---|---|---|---|
| `_exeque` | 12 | 187 -> 2 (held from prior work) | rotated, correctly, under the 2026-07-27 standing ruling |
| `func_80056CB8` | 68 (s1-s68) | 204 -> **38**, flat from s22 | rotated (LADDER EXHAUSTED, non-endgame) |
| `func_8006CCC8` | 4 | 189 -> **39** | active |

### The control above is INVALID — read it before drawing the obvious conclusion

The queue's difficulty regime changed at the same moment as the model did. Minimum
active honest distance:

| Date | Active items | Min distance | Median |
|---|---|---|---|
| 2026-09-02 | 193 | 35 | 237 |
| 2026-09-14 | 133 | 126 | 302 |
| 2026-09-15 | 132 | **0** | 304 |
| 2026-09-16 | 117 | **188** | 342 |

The 09-15 Fable burst (14 completions) drained the last of the small functions.
Every remaining item is >=188 insns, median 342. The control's 0.159
completions/session and its 40 one-session recon closes were earned on a queue that
still contained functions like `func_8002CA8C` (empty stub -> 5/179 in one recon
session). Arm 1 never saw one of those. Frontier models are also slow on this tail:
`func_8003DE14` took 38 sessions, `func_80070C70` 17, `CD_sync` 126.

**Metric-1 (completions/session) is therefore unscorable for arm 1, and any future
arm must be scored only over functions >=188 insns.**

### What arm 1 does show

**Mechanical ladder work holds up.** `func_8006CCC8` went 189 -> 39 in four sessions,
including two real finds (an `i` mistyped `s16` where the target uses plain `int`,
91 -> 77; an LICM-hoist closed via `defeat-licm-hoist-var-reuse`). `func_80056CB8`
went 204 -> 38 over s1-s22 on finds read straight out of the target asm (s7's `$s0`
reuse, 81 -> 58).

**Protocol compliance is better than the baseline, not worse** (metric 2's neighbour):
9.7% of spawns discarded as invalid in the Sonnet window vs 14.3% over 2026-09-02..15.
Sonnet follows the driver's contract. Zero candidates were produced, so metric 2
(layer-1 FAILs per completion) has no reading.

**Every observed failure was judgment about its own work, and all of it landed in the
reasoning modalities** — this is the finding that shaped arm 2:

- `func_8006CCC8` s1 banked `floor=94` from a C body it never wrote to disk (s2:
  "Fixed the stale-HEAD chassis (s1 body was never applied)"). A fabricated
  measurement that ordered the queue and framed two later sessions. This is metric 4
  in its purest form, and it is now gated mechanically — see below.
- s44/s45 diagnosed a "real, reproducible `engine/sandbox.py` cheat-strip pipeline
  defect" and filed a LADDER-EXHAUSTED rotation citing a stale 38/204 instead of a
  measured number; s46 measured it fine (the machinery had simply been retired).
  **A rotation is not a completion, so neither gate ever sees it** — this is the one
  expensive error class with nothing behind it.
- s33 asserted from reasoning alone that do-while vs for was codegen-neutral; s34
  measured it and disproved it.
- s45-s49 chased what s50 itself calls a "misdirected RA-conflict frontier"; s64 and
  s65 each corrected a standing misattribution dating back to s8/s13/s54.

Metric 3 (one-session recon closes) is unreadable — no function in the window was
small enough for a one-session close to be available.

## Arm 2 — split the worker lanes along the observed fault line (2026-09-16)

Rather than flipping the whole tier back, arm 2 keeps Sonnet where its output is
checked by tooling and returns the judgment lanes to Fable 5.1.

| Lane | Arm 1 | **Arm 2** |
|---|---|---|
| execution: structural, enumerate, permuter, rederive, annotation-fix | Sonnet 5 | **Sonnet 5** |
| recon, object-model | Sonnet 5 | **Sonnet 5** |
| **synthesis, forensics, solver, escalation** | Sonnet 5 | **Fable 5.1** (`$ReasoningModel`) |
| Judge, layer-1 cheat-reviewer, `$FallbackModel` | Opus 5 | Opus 5 (unchanged) |

The split line is *does the driver verify this session's output mechanically?* A
structural or enumerate session's claim is a sandbox score; a weak one costs one
cheap retry. A synthesis/forensics/solver session emits a **judgment** — which axis
the next N sessions attack, what a `.greg` dump means — and an escalation session
decides whether the ladder is finished at all. Those are unverifiable in the moment
and compound.

### Mechanical fix landed alongside (independent of model tier)

`grindlib.attest_floor` + the driver's post-validate gate: a session claiming a floor
**drop** must corroborate it with an `engine sandbox`/`build-c` event carrying that
spawn's own `CLAUDE_SESSION_ID` and that score, or with an artifact under `tmp/` or
its own ledger dir containing the number (the `ra_solver` object-mode path). Neither
present => INVALID session, discarded and respawned. Flat or worse floors are not
gated — a sloppy flat floor is re-measured next session anyway; a fabricated drop is
not. This closes the `func_8006CCC8` s1 hole regardless of which model runs the lane.

## Arm 2 scoring rules

1. Score **only functions >=188 insns**, in both arms. Re-derive the Fable comparator
   from the ledgers of `func_8003DE14`, `func_80070C70`, `func_8002E6B0` and
   `func_80035280` rather than from the invalid headline rate.
2. The cleanest single read available now: **`func_8006CCC8` at floor 39**. It is the
   queue top, it is mid-grind on a Sonnet-built chassis, and whether it closes — and
   in which modality — is direct evidence.
3. Metric 4 gets a new mechanical proxy: count `UNATTESTED FLOOR DROP` discards in
   `tmp/grind/grind.log`.
4. Watch specifically for rotations filed on a premise a later session contradicts
   (the s44/s45 shape). That is the failure arm 2 is meant to fix.
