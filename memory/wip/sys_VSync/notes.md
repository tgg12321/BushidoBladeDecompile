# sys_VSync (src/ings2.c) — WIP checkpoint

**TL;DR:** Worker batch 2026-06-12 reached a full byte-match (commit `4dd017d8`)
but the layer-2 retro-audit **FAILed** it: the Region B closer was an unannotated
`do {} while (0)` used as a **sched1 fence** — out of scope for
[[do-while-zero-exception]] (reorg.c-only sanction), i.e. a scheduling barrier by
another spelling. The commit was reverted to the bridged state (2 regfix reorder
rules, oracle green). The **Region A lever survives review** (PASS) and is
preserved here: floor 7 → 5.

## Resume instructions

1. Apply `candidate.c` to `sys_VSync` in `src/ings2.c` (it differs from HEAD only
   in the Region A named-base split; the rest of the body is identical).
2. `& tools/eng.ps1 sandbox sys_VSync --disable all` — expect floor **5**
   (HEAD floor is 7).
3. The remaining 5 is Region B: the second `func_80082A14(g_sys_dma_region + 1, 1)`
   call's pre-call scheduling (volatile s0 reload must stay adjacent to its
   pointer load). HEAD bridges it with `sys_VSync: reorder 32,38,36,33,34,37,35
   @ 32-38` in regfix.txt.
4. Work the `next_hypotheses` in `meta.json`. Do NOT re-derive the rejected
   do-while fence (`rejected/do-while-sched1-fence.c`).

## Pointers

- Reverted match commit (full context + worker's measured negatives):
  `git show 4dd017d8`
- Layer-2 verdict detail: `meta.json.reviewer` + `rejected/do-while-sched1-fence.c`
- Related rules: [[do-while-zero-exception]], [[hoist-call-arg-local-flips-jal-delay]],
  [[register-alloc-pure-c]], [[loop-exit-work-inside-loop-sched-fence]]
