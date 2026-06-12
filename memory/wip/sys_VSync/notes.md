# sys_VSync (src/ings2.c) — WIP checkpoint

**TL;DR (after session 6, 2026-06-12):** `candidate.c` is UNCHANGED from
sessions 1-5 (Region A named-base lever applied, no inner do-while, floor
7 = build_insns 81 vs target 82). Session 6 (a) re-tested ALL session-1
measured negatives at the clean floor-7 baseline (operand reassociation,
merged if/else, inverted prologue, last-arg hoist, eager-flag, tail-
store reorder, single-call-block arg) — ALL ruled out, no improvement;
(b) swept 30+ additional C89-valid Region B BB2 variants in
`tmp/sweep_vsync2.py` + `tmp/sweep_vsync3.py` — ALL stuck at floor 7. The
lever surface within C89-valid pure-C for Region B BB2 is comprehensively
exhausted. NEW MECHANISTIC FINDING: B11
(`func_80082A14(g_sys_dma_region+1, s0_val != 0xFFFFFFFF)`) reached
build_insns=82 / floor 6 — confirming that an in-BB use of s0_val before
the second jal IS the sched1 lever (it extends lw s0's path-to-end
priority enough to make sched1 schedule the v0 chain FIRST, matching
target's layout). HOWEVER every pure-C expression that USES s0_val and
PRODUCES the constant 1 is either semantically wrong (B11) or
cheat-by-spelling (no semantic purpose, only effect is sched1 priority
shift). So the lever is mechanistically IDENTIFIED but pure-C
INACCESSIBLE without finding the original source's mechanism. B6
(decl-after-statement, C89-invalid) was a false positive — its score-5
was an artifact of GCC 2.7.2 silently compiling broken code. Candidate.c
floor unchanged at 7.

**TL;DR (after session 5, 2026-06-12):** `candidate.c` is UNCHANGED from
sessions 1-4 (Region A named-base lever applied, no inner do-while, floor
7 = build_insns 81 vs target 82). Session 5 probed the dual-named-
intermediate (vs + arg) family in Region B, found it reaches floor 6
(build_insns 82 = target) via M5 + 9 isomorphs — but the cheat-reviewer
FAILed the entire family as identity-bounce / LUID-coercion. The
forbidden-family ruling NARROWS the lever surface: any future pure-C
lever for Region B's residual gap must either come from outside
the dual-intermediate family OR name a real sub-computation (Region A's
`base = D_800A151C - 1` style — but Region B has no obvious analogous
sub-computation candidate). See rejected/m5-vs-arg-identity-bounce.c
for the cheat-reviewer's full reasoning. The session adds 1 ruled-out
family + 1 rejected/ file + an updated next-session worklist; the
candidate.c floor is unchanged at 7.

**Earlier (session 1-4):** Region A named-base lever survives layer-2
review (commit `4dd017d8` reverted as `01471d90`; orchestrator preserved
the lever in `candidate.c`). Session 2 discovered that the prior
`candidate_floor: 5` baseline depended on an inherited inner
`do { } while (0);` at HEAD `src/ings2.c` line 69 (introduced 2026-04-09
in commit d7c91eba, pre-dating both the do-while-zero-exception rule and
the cheat-catalog expansion). The session-2 cheat-reviewer FAILed that
body — the inner do-while is a sched1-fence cheat, explicitly out of
scope for the do-while-zero-exception (which only sanctions
LABEL_OUTSIDE_LOOP_P / reorg.c invert-jump peephole uses). Same family
as the outer do-while that FAILed commit 4dd017d8. The inner do-while
has been REMOVED from `candidate.c`; the honest baseline is floor 7
(build_insns 81 vs target 82). Region A lever's measurable benefit
(HEAD's TRUE pure-C floor 9 -> candidate's TRUE pure-C floor 7) is
preserved.

## Resume instructions

1. Apply `candidate.c` to `sys_VSync` in `src/ings2.c`. The CHANGES from HEAD
   are:
   - Region A: replace `frame = (D_800A151C + a0) - 1;` with
     `s32 base = D_800A151C - 1; frame = base + a0;` (named-base lever).
   - Inner block: REMOVE the `do { } while (0);` that follows
     `func_80082A14(frame, count);` (session-2 cheat-reviewer FAIL).
2. `& tools/eng.ps1 sandbox sys_VSync --disable all` — expect floor **7**
   (build_insns 81 vs target 82).
3. Work the `next_hypotheses` in `meta.json`. Session 2 ruled out:
   - `hoist-call-arg-local-flips-jal-delay` family — does NOT apply (worse).
   - Shared `volatile s32 *ptr` across the call — catastrophic (5->23).
   The remaining `next_hypotheses` are: RTL diagnosis via cc1 BB2_SCHED_DEBUG,
   m2c reconstruction, sibling .greg comparison, session-1 negatives
   re-measured at floor 7, decomp-permuter with clean single-function target.

## Pointers

- Reverted match commit (full context + worker's measured negatives):
  `git show 4dd017d8`
- Layer-2 verdict detail: `meta.json.reviewer` + `rejected/do-while-sched1-fence.c`
- Session-2 cheat-reviewer verdict on inner do-while:
  `meta.json.sessions[1].cheat_reviewer_verdict_on_prior_candidate`
- Related rules: [[do-while-zero-exception]] (scope: reorg.c only),
  [[hoist-call-arg-local-flips-jal-delay]], [[register-alloc-pure-c]],
  [[review-discipline-before-commit]] (cheat-reviewer architecture),
  [[no-new-park-categories]] (cheats-by-any-spelling policy)

## Session 2 measurements (2026-06-12)

| Configuration                                                          | floor | build_insns |
|------------------------------------------------------------------------|-------|-------------|
| HEAD (no Region A, with inner do-while) — engine baseline              | 7     | 82          |
| HEAD (no Region A, WITHOUT inner do-while) — TRUE pure-C HEAD floor    | 9     | 81          |
| Candidate (Region A lever, with inner do-while) — prior tainted baseline | 5  | 82          |
| **Candidate (Region A lever, WITHOUT inner do-while) — NEW baseline**  | **7** | **81**      |
| Hypothesis 1 — block-hoist `s32 dma_arg`, inner do-while removed       | 7     | 81          |
| Hypothesis 2 — shared `volatile s32 *ptr` pre-call + post-call         | 23    | 79          |
| Variant — scoped `volatile s32 *p` (single use), inner do-while removed | 7    | 81          |

The Region A lever DOES measurably lower the true pure-C floor (9 -> 7).
The +2 improvement at engine-measurement level (7 -> 5) the prior session
reported required the inner do-while to be in source — a sched1-fence
construct that current policy (do-while-zero-exception scope) classifies as
a cheat.

## Region B's root-cause hypothesis (refined by session-3 instrumented dump)

Target keeps `lw $s0, 0($v0)` (s0_val reload) BEFORE the second call's arg
setup, with a load-delay nop between the lui/lw chain. cc1 with our clean
source naturally interleaves the s0 load with the arg load. Session 2's
hypothesis was that sched1's `priority()` (longest-path-to-end) values
`lw $a0, %hi(D_800A2634)` higher than `lw $v0, %hi(D_800A1510)` because
the a0 chain reaches the jal (USE), while the v0 chain ends in s0 (a
callee-save not USEd until post-call).

**Session-3 update:** The BB2_SCHED_DEBUG dump REFUTES the
sched1-priority hypothesis at the layout level. sched1's recorded picks
for block 9 place insn 102 (`lw $v0, D_800A1510`, LUID 6) at block
position 5 and insn 97 (`lw $a0, g_sys_dma_region`, LUID 4) at position 6
— i.e. lw v0 BEFORE lw a0 in sched1's claimed layout. But cc1's actual
.s emission (tmp/ings2.s lines 121-130) has lw a0 BEFORE lw v0. The
discrepancy implies a POST-sched1 pass (likely the delay-slot fill in
reorg.c, or mips_reorg) is reshuffling.

This shifts the lever surface: clean C variants that only target
sched1's LUID tie-break (named-intermediate hoists, declaration-order
variants — 6+ variants ruled out across sessions 1-3) will NOT close
the gap because sched1 is ALREADY doing the right thing. The decision
point is reorg.c choosing `li $a1, 1` over `addu $a0, $a0, 1` for the
jal delay slot. The next session's first action should be either:
  1. Add BB2_REORG_DEBUG instrumentation to tools/gcc-2.7.2/reorg.c and
     dump the delay-slot fill candidate list at the jal in Region B.
  2. Run decomp-permuter with a clean single-function target.o and let
     directed-PERM macros explore the reorg-influencing mutations the
     worker can't enumerate by inspection.

## Session 3 measurements (2026-06-12)

| Configuration                                                          | floor | build_insns |
|------------------------------------------------------------------------|-------|-------------|
| Clean candidate (sessions[1] state)                                    | 7     | 81          |
| Variant — `{ s32 dma = g_sys_dma_region; s0_val = *D_800A1510; func_80082A14(dma + 1, 1); }` | 7 | 81 |
| Variant — `{ s32 dma_arg; s0_val = *D_800A1510; dma_arg = g_sys_dma_region + 1; func_80082A14(dma_arg, 1); }` | 7 | 81 |

candidate.c UNCHANGED in session 3 — the session-2 cheat-reviewer PASS
still applies. Updates land in meta.json only.

## scan_hand_coded confirms LOW tier (session 3)

`python3 tools/scan_hand_coded.py --single sys_VSync --json` → tier
LOW, score 0, "no strong hand-coded indicators". 82-insn function with
3 spills and 6 distinct registers — standard cc1 codegen profile. The
function is firmly in TIGHT_C / pure-C territory; no canonical-asm
escape applies.

## Session 4 measurements (2026-06-12) — primary unprobed lever executed

Built the clean single-function permuter directory at `permuter/sys_VSync/`
per the WIP's PRIMARY UNPROBED LEVER. Files:

  - `base.c` — trimmed preprocessed source containing ONLY sys_VSync +
    its decls/typedefs (2.4 KB; the full whole-file preprocess was
    909 lines / 17 KB but caused 95% noise as warned in session 1's WIP).
  - `target.s` — asm/funcs/sys_VSync.s + `prelude.inc` (`.set gp=64`
    stripped for r3000).
  - `target.o` / `base.o` — both have sys_VSync at offset 0.
  - `compile.sh` — same pipeline as `engine sandbox` (cc1 + prologue_fix +
    maspsx + as).

Verified baseline permuter score = **495** (vs the prior tainted 15310
on full-file base.o). The ~30× reduction confirms the offset-noise was
indeed dominating the prior measurement.

Ran random-mode permuter for ~5 min with `-j 6` (estimated several
hundred iterations across workers). Best closing forms found:

| Score | Form | Verdict |
|---|---|---|
| 40 | `do { func_80082A14(g_sys_dma_region + 1, 1); } while (0);` | FAIL — sched1-fence cheat, same family as session-2 FAIL |
| 50 | do-while + dead-store-via-s0_val-reuse | FAIL — composite cheat |
| 300 | do-while wrap around bigger statement range | FAIL — same family |
| 355 | do-while wrap around first-call block | FAIL — same family |
| 455 | semantic-breaking s0_val reuse | INCORRECT — not a closing form |

Best non-cheat score = 495 (= baseline, no improvement). Permuter's
random-mode mutation surface is exhausted of legitimate closing forms
for this candidate.

This is strong evidence that the structural lever IS in reorg.c
(delay-slot fill), as the session-3 dump localised — and random C-level
mutations don't reach reorg.c's decision surface. The mutations the
permuter found were all sched1-fence cheats (because that IS what
defeats the gap, but it's the forbidden answer).

## Next session — what to try

1. **Directed PERM_* macros** (the un-tried part of the permuter lever):
   - `PERM_LINESWAP(...)` around Region B's 3 statements (s0_val read,
     dma compute, call) — scripted ordering mutations.
   - `PERM_VAR(...)` for local types — affects RTL pseudo allocation.
   - `PERM_TYPECAST(...)` on the volatile-load expression.
   - `PERM_FORCE_FORWARD_DECL(...)` on the call.
   These target reorg.c's surface more directly than random-mode.

2. **Re-test session-1's measured negatives at floor 7** (un-tested at
   clean baseline per the WIP's next_hypothesis #5):
   - operand reassociation in Region A
   - last-arg hoist on the second call
   - merged if/else for the `a0 > 0` two-check pattern
   - inverted branch sense on the prologue checks
   - eager-flag pre-computation
   Each ruled-out variant should be measured against the clean floor-7
   baseline; their original session-1 measurements were against the
   tainted baseline.

3. **Add `BB2_REORG_DEBUG` instrumentation to tools/gcc-2.7.2/reorg.c**
   (diagnostic only per [[no-compiler-divergence]]): dump the
   delay-slot-fill candidate list + chosen insn at the second jal. The
   pick logic surfaces whether the lever is in dependence-cone analysis
   or simple-fill heuristic.

## Permuter directory layout (preserved for next session)

`permuter/sys_VSync/` — ready to re-run. Useful commands:

```
python3 tools/decomp-permuter/permuter.py permuter/sys_VSync \
    --stop-on-zero -j 6
```

To add directed PERM_* macros: edit `permuter/sys_VSync/base.c` around
sys_VSync's body. The full `base.c.full` (whole-file preprocess) is
preserved as a backup; the active `base.c` is the trimmed
single-function version.

**SESSION 5 (2026-06-12) update:** permuter directory was NOT preserved
between sessions (gitignored). Rebuild from `asm/funcs/sys_VSync.s`
+ `tools/decomp-permuter/prelude.inc` (strip `.set gp=64`); the
session-4 setup recipe is in `meta.json.sessions[2].discoveries[1]`.

## Session 5 measurements (2026-06-12) — dual-intermediate family RULED OUT

Tested 30+ Region-B variant shapes via `tmp/sys_vsync_variants*.py`.
The dual named-intermediate family with vs-declared-first ALL hit
floor 6 (build_insns 82 == target):

| Form key | Form | Floor | bi | Verdict |
|---|---|---|---|---|
| M5 | `{ s32 vs = read; s32 arg = compute+1; s0_val = vs; func(arg, 1); }` | **6** | **82** | **FAIL** (identity bounce) |
| R1 | M5 with +1 at call site | 6 | 82 | FAIL same family |
| R2 | M5 with `arg += 1;` then call | 6 | 82 | FAIL same family |
| R4 | uninit decls, vs assigned first, then arg, then s0_val=vs, then call (+1 at site) | 6 | 82 | FAIL same family |
| R7 | vs init + arg init + `s0_val = vs, func(arg, 1)` comma | 6 | 82 | FAIL same family |
| R8 | R7 with `(void)(s0_val = vs); func(arg, 1);` | 6 | 82 | FAIL same family |
| R9 | M5 with cast-away-volatile `(s32 *)D_800A1510` | 6 | 82 | FAIL — separate cheat (cast away volatile) |
| Q5 | M5 with `s0_val = vs + 0;` | 6 | 82 | FAIL same family |
| Q6 | M5 with explicit `volatile s32 *p = D_800A1510;` | 6 | 82 | FAIL same family |
| Q7 | uninit decls then assign vs first, arg second | 6 | 82 | FAIL same family |
| M3 / P3 | `s0_val = direct; { s32 arg = ...; func(arg, 1); }` | 7 | 81 | (baseline — single intermediate, no lever) |
| P1 | M5 with `s0_val = vs;` moved AFTER call | 7 | 81 | no lever (vs survives call, mechanism lost) |
| Q1 | second-arg `s32 one = 1;` named | 7 | 81 | no lever |
| Q2 | both args named (arg1, arg2), no vs | 7 | 81 | no lever |
| Q4 | `s32 r; r += 1;` separate increment, no vs | 7 | 81 | no lever |
| R3, R5, R6, R10 | various 1-intermediate / wrong-order forms | 7 | 81 | no lever |

## Cheat-reviewer adjudication on M5 (verbatim summary, full text in rejected/)

Decision: **FAIL** (default to FAIL when torn — and here the call wasn't
borderline).

Reviewer's reasoning:
  - The `vs` two-step `s32 vs = *D_800A1510; s0_val = vs;` is an IDENTITY
    BOUNCE. Semantically equivalent to `s0_val = *D_800A1510;`. No
    sub-computation to name → NOT the SOTN named-intermediate family.
    (Region A's `base = D_800A151C - 1` PASSes because `base` names a
    real frame-offset sub-computation; identity bounces have no such
    sub-computation.)
  - The companion `arg` named intermediate has no semantic gain over
    inline `g_sys_dma_region + 1` either.
  - The declaration-order sensitivity (only `vs` first AND `arg` second
    works; swap order → floor 7) is textbook LUID-ordering coercion —
    a GCC-internals-justified construct, not program logic.
  - The whole dual-intermediate-vs-first family is REJECTED across all
    syntactic spellings.

## What is now ruled out for Region B

Floor 6 is REACHABLE via the dual-intermediate family but NOT via any
cheat-policy-permitted form found this session. The remaining lever
surface for sys_VSync's Region B is correspondingly narrower:

1. **Single-intermediate forms** are exhausted (M3, R5, Q1, Q2, Q4 all
   floor 7).
2. **Dual-intermediate forms** are exhausted (and the lever family is
   forbidden).
3. **Statement reordering** within Region B is exhausted (P1-P9 family).
4. **Pointer-aliasing tricks** (volatile pointer alias) are exhausted.

## Session 6 measurements (2026-06-12)

Ran two sweeps over the clean candidate baseline (Region A lever, no
inner do-while, floor 7 / build_insns 81):

**Sweep 1 (`tmp/sweep_vsync.py`)** — re-tested session-1 measured
negatives at clean floor-7:

| Variant | Floor | Notes |
|---|---|---|
| E1_eager_mask_local (`s32 mask_val = s0_val & 0x400000;` before call) | 19 | catastrophic — extra `and` instruction |
| M1_merged_a0pos (merged if/else for both a0>0 checks) | 14 | restructures Region A; worse |
| I1_inverted_prologue (a0>=0 nest + a0!=1) | 20 | catastrophic |
| L1_one_local_early (`s32 one_val = 1;` at function top) | 11 | worse |
| OA1_reassoc_a0_minus_1 (`D_800A151C + (a0 - 1)`) | 7 | no change |
| XOR1_pre_xor_hoist (named `first_diff` in spin-wait) | 7 | no change |
| TS1_tail_reorder (1518 before 151C) | 13 | semantically dubious; worse |
| S1_local_ptr_decl_top (volatile s32 *ptr_510 at top) | 7 | no change |
| G1_a0_local_after_s0 (block-scoped a0_arg after s0 read) | 7 | no change |
| H1_inline_volatile_use (scoped p+s0_val read same block) | 7 | no change |
| H2_scope_arg_one_local (one local around call) | 7 | no change |

**Sweep 2 (`tmp/sweep_vsync2.py`)** — Region B BB2 focused:

| Variant | Floor | Notes |
|---|---|---|
| B1_paren_extra | 7 | no change |
| B2_blocked | 7 | no change |
| B3_cast `(s32)(*D_800A1510)` | 7 | no change |
| B4_comma `(s0_val=*D_800A1510, dma+1)` | 7 | no change |
| B5_increment_local `dma++` | 7 | no change |
| B6_dma_local_post_read | 5 | **FALSE** — C89-invalid (decl-after-stmt); broken compile (a0=0 at jal) |
| B7_inline_load (read AFTER call) | 23 | semantically wrong |
| B8_a1_init_in_block `s32 a1_init=1;` | 7 | no change |
| B9_double_read_used | 8 | **build_insns=82** but extra in-BB load mismatches target |
| B10_addiu_stmt (named dma_arg) | 7 | no change |
| B11_a1_uses_s0 `s0_val != 0xFFFFFFFF` | 6 | **build_insns=82** — MECHANISM CONFIRMED but semantically wrong + cheat-by-spelling |
| B12_dma_then_read (dma before read) | 7 | no change |
| B13_dma_arg_post_read | 7 | no change |
| B14_one_plus_dma `1 + g_sys_dma_region` | 7 | no change |
| B15_plus_via_local_named_dma_plus_one | 7 | no change |

**Sweep 3 (`tmp/sweep_vsync3.py`)** — C89-corrected Region B variants:

| Variant | Floor | Notes |
|---|---|---|
| C1-C18 (18 forms) | 7 | all stuck at 7, EXCEPT C11 (volatile-on-local-ptr-decl: 15 — cheat anyway) and C14 (ternary on a0: 29 — adds branch) |

## Session 6 mechanistic finding

**B11 confirms the missing lever IS an in-BB use of s0_val before the
second jal.** With `s0_val != 0xFFFFFFFF` as arg2, build_insns rises
from 81 to 82 (matching target) and floor drops from 7 to 6. The
mechanism: extending lw s0's sched1 priority chain via an in-BB USE
makes sched1 schedule the v0 chain FIRST (matching target's layout
`lui v0/lw v0/[nop]/lw s0/lui a0/lw a0/li a1/addiu a0/jal`).

**The lever is forbidden in pure C** because:
  1. **Every constant-1-from-s0_val expression is either semantically
     wrong or cheat-by-spelling.** Candidates surveyed: `s0_val | 1`
     (not 1 in general), `s0_val != K` (depends on K), `s0_val ^ s0_val
     + 1` (cheat — no semantic purpose), `(s0_val & 0) + 1` (cheat),
     `((&s0_val) - (&s0_val)) + 1` (cheat).
  2. Per [[no-new-park-categories]] cheat-by-any-spelling principle: an
     expression whose only effect is GCC-internals coercion (sched1
     priority shift) and whose value has no programmer-explainable
     role IS a cheat regardless of syntactic form.

**What this implies about the original source**: either (a) the original
had a DIFFERENT mechanism we haven't found — e.g., a sibling call,
struct field access, or larger arg expression that naturally uses
s0_val without violating semantics; OR (b) the lever surface that
mattered was at reorg.c's delay-slot fill (which session-3's
BB2_SCHED_DEBUG implicated), not sched1's priority computation. The
B11 finding doesn't distinguish (a) from (b).

## Next session — what to try (priority order)

1. **Re-examine matched siblings with the session-6 mechanistic
   finding.** Look specifically for sibs whose source has a USE of a
   pre-call volatile-loaded value IN the second call's arg-compute
   expression. session-5 reported "None obvious on first reading" but
   with the session-6 finding (in-BB use of s0 IS the lever),
   re-examination should target this SPECIFIC pattern, not generic
   "volatile + call" pattern. Candidates in src/ings2.c:
   `motion_make_table`, `func_80082C58`, `sys_SetVsyncMode`.

2. **Directed PERM_* macros** (the un-tried part of the permuter lever
   from session 4 next_hypothesis). PERM_LINESWAP / PERM_VAR /
   PERM_TYPECAST scripted into base.c. Required setup: rebuild
   `permuter/sys_VSync/` per session 4's recipe (now ~1 hour of
   setup; not preserved between sessions).

2. **Look outside Region B**: the 6-instruction gap at M5 is in
   Region B's delay-slot / register-allocation choices. The HEAD floor
   of 7 might also have contributions from OTHER regions. Check Region C
   (the post-call `s0_val & 0x400000` spin-wait) and the tail-stores
   region (post-call writes to D_800A151C / D_800A1518) — session-1
   measured-negatives never re-tested at the clean floor-7 baseline:
   operand reassociation in Region A, last-arg hoist on the second
   call, merged if/else for the `a0 > 0` two-check pattern, inverted
   branch sense on prologue checks, eager-flag pre-computation. These
   target REGIONS THE M5 FAMILY DID NOT TOUCH and may be closable
   without dual-intermediate coercion.

3. **BB2_REORG_DEBUG instrumentation to tools/gcc-2.7.2/reorg.c**:
   diagnostic only (per [[no-compiler-divergence]] — instrumentation
   is sanctioned alongside BB2_SCHED_DEBUG / BB2_PRIO_DEBUG /
   BB2_ALLOC_DEBUG, but the gate is strict: instrumentation dumps WHY
   reorg.c picks `li $a1, 1` over `addiu $a0, $a0, 1` for the jal
   delay slot; the FIX must come from clean C, not from patching
   reorg.c).
