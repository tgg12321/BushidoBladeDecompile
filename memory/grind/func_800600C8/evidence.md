# Evidence bank — func_800600C8

## s1 (recon, 2026-07-27) — floor 13 -> 6, CHEAT-FREE, insn count 121/121

- Baseline re-measured: canonical verdict C, sandbox --disable all = 13 (with
  HEAD's 6 regfix rules dropped; HEAD source still carried 2 source cheats).
- Both source cheats REMOVED (`if ((!hi) && (!hi)) {}` empty-if dead-read;
  `s.zero1C = 0 & 0xFFFFu` masked zero). Best form is 100% clean C.
- Floor history: 13 (cheated source) -> 16 (naive clean) -> 6 (candidate.c).
  Current src/text1b.c HAS the floor-6 form applied.
- The 6 regfix rules' entire coverage is now understood:
  - 2x `addu $4,$sp,24 <-> sw $2,48($sp)` swaps = width-store-after-a0 — FIXED
    by per-arm s.width stores (cross-jump merge restores target order).
  - `sh $2,72 <-> sh $2,74` swap = d0/d1 init order — FIXED by
    `s.d1 = hi; s.d0 = hi;` (4A-first) + pair-then-p0 flow protection.
- Compiler-mechanism facts (verified in tools/gcc-2.7.2 source + -da dumps in
  tmp/grind/func_800600C8/s1/):
  - flow.c dead-store elim of stack slots is a SINGLE-slot tracker
    (`last_mem_set`): store to any other slot REPLACES it; any mem read
    clears it (flow.c:2393); calls clear it (:1630); sp-based stores exempt
    (:1985); rtx_equal_p match required (:1740-41). d0-init died in flow
    (present in .cse2, gone in .flow), copy then died in combine.
  - sched1 tie-break chain: priority -> class-vs-last-scheduled -> INSN_LUID
    (sched.c:2399-2455). This cc1 has BB2_RANK_DEBUG env tracing built in
    (sched.c:2436).
  - expand folds `(&s.d0)[const]` to direct frame mem; cse canonicalizes a
    single-set pointer var's deref to the frame address before flow — address
    respelling CANNOT hide a stack slot from flow DSE (measured, probes 4/5).
- Remaining 6 diffs: single transposition of [copy+sh4A+sh48] vs
  [lui/addiu B6FC + sw 0x18] between `mult a3,t0` and `sll v0,a0,2`.
  Everything else byte-matches (prologue t1 copy, width diamonds, divmod
  interleave, loop, tail).
- Sibling corroboration: func_80060414 (adjacent, matched) uses per-arm
  direct struct-pointer stores and a named arg1 copy — same idioms.
- Artifacts: tmp/grind/func_800600C8/s1/ (probe objdumps 1-3, cc1 -da dump set
  text1b.i.{rtl,cse,cse2,flow,combine,sched,...} for the probe-2 source,
  dump.sh to regenerate).

## == imported from memory/wip notes.md (historical, pre-s1) ==
HEAD "matched" via 6 regfix subst rules + 2 source cheats; honest distance 13;
old analysis called this multi-session orchestrator grade with ~8 coupled
clusters — s1 showed it was 3 mechanisms (cross-jump width, flow DSE order,
and the residual sched tie).

- [s1] Floor 6 form is applied in src/text1b.c and is 100% cheat-free: empty-if dead-read of hi and 0&0xFFFFu masked-zero both removed; zero rules needed for the fixed clusters

- [s1] All 6 regfix subst rules' coverage is now mechanism-explained: 2x addu/sw call-site swaps = width store order (fixed), 1x sh 72<->74 = d0/d1 init order (fixed)

- [s1] Remaining 6 diffs are ONE transposition: ours [move v0,s1; sh 0x4A; sh 0x48] then [lui/addiu B6FC; sw 0x18]; target the reverse. All other 115 insns byte-match

- [s1] sched.c rank_for_schedule tie chain = priority -> class-vs-last-scheduled -> INSN_LUID (source order); flow forces pair-before-p0 in source, LUID therefore emits pair first — the residual is a priority/class question, not orderable by statement order alone

- [s1] This cc1 build has env-gated BB2_RANK_DEBUG tracing (sched.c:2436-2446) usable to read the actual tie terms next session

- [s1] flow.c last_mem_set facts banked: single slot, any-store evicts, any-mem-read clears (:2393), calls clear (:1630), sp-based stores exempt (:1985)

- [s1] Sibling func_80060414 (adjacent, matched) uses the same idioms: per-arm direct struct stores, named arg1 copy
