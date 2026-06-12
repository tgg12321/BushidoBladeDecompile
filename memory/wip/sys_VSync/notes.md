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

## Region B's root-cause hypothesis (un-verified, basis for next session's RTL diagnosis)

Target keeps `lw $s0, 0($v0)` (s0_val reload) BEFORE the second call's arg
setup, with a load-delay nop between the lui/lw chain. cc1 with our clean
source naturally interleaves the s0 load with the arg load. Hypothesis:
sched1's `priority()` (longest-path-to-end) values `lw $a0, %hi(D_800A2634)`
higher than `lw $v0, %hi(D_800A1510)` because the a0 chain reaches the jal
(USE), while the v0 chain ends in s0 (a callee-save not USEd until post-call).
To match target without a sched1 fence, the v0 chain needs higher priority —
which requires either a clean C structural change that incidentally elevates
its `priority()` weight, or evidence that the original cc1 used a different
priority calculation (unlikely; we share the toolchain).
