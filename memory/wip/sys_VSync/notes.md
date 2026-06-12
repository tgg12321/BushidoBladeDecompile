# sys_VSync (src/ings2.c) — WIP checkpoint

**TL;DR:** Region A named-base lever survives layer-2 review (commit `4dd017d8`
reverted as `01471d90`; orchestrator preserved the lever in `candidate.c`).
Session 2 (2026-06-12) discovered that the prior `candidate_floor: 5` baseline
depended on an inherited inner `do { } while (0);` at HEAD `src/ings2.c` line
69 (introduced 2026-04-09 in commit d7c91eba, pre-dating both the
do-while-zero-exception rule and the cheat-catalog expansion). The session-2
cheat-reviewer FAILed that body — the inner do-while is a sched1-fence cheat,
explicitly out of scope for the do-while-zero-exception (which only sanctions
LABEL_OUTSIDE_LOOP_P / reorg.c invert-jump peephole uses). Same family as the
outer do-while that FAILed commit 4dd017d8. The inner do-while has been
REMOVED from `candidate.c`; the new honest baseline is floor 7 (build_insns
81 vs target 82). Region A lever's measurable benefit (HEAD's TRUE pure-C
floor 9 -> candidate's TRUE pure-C floor 7) is preserved.

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
