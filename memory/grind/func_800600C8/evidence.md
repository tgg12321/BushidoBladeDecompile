# Evidence bank — func_800600C8

## s2 (structural, 2026-07-27) — floor 6 -> 0, CHEAT-FREE MATCH, 121/121

- MATCH: sandbox --disable all = 0 with pure statement ordering (form in
  candidate.c, applied in src/text1b.c). No constructs, no FAKE, no asm.
- The winning order: `hi=arg0; s.p0=&B6FC; s.d1=hi; s.d0=hi;
  hi=((s16)arg0)/10; s.d1=hi%10; s.d0=((s16)arg0)%10;` — three coupled
  levers: p0-store FIRST, init pair d1-then-d0, overwrite pair
  d1-ow-then-d0-ow (source swap of the two overwrite statements).
- MECHANISM CORRECTION to s1 model: GCC 2.7.2 sched.c is a REVERSE
  (bottom-up) list scheduler (sched.c:38-58; pick site ~3939: picks
  ready[0], builds chain backward). Pick order = reverse emit order.
  Priority = producer-chain height (priority() sched.c:1425: max over
  producers of pri(x)+cost-1); all five contested leaf insns are pri 1
  class 3, so ties fall to INSN_LUID with HIGHER LUID picked first ->
  emit order among tied leaves == RTL chain order (stable). The s1
  "forward, after mult issues" framing was wrong but its conclusions
  (LUID-stable, priority/class tie chain) were directionally right.
- schedule_select (sched.c:2643) rotates the largest-potential-hazard
  insn to front WITHIN the top same-priority group: stores beat li/move
  (explains insn 89 leapfrogging 144/i=0 in list order — NOT a LUID
  violation).
- flow.c last_mem_set DSE scan is BACKWARD (life analysis direction):
  d0-init dies iff, scanning backward from the block end, the tracker
  still holds fp+72 when d0-init is reached. Swapping the OVERWRITE
  statement order (d1-ow later in chain than d0-ow) makes the backward
  scan visit 109(72-ow) then 141(74-ow) -> tracker=fp+74 at the init
  pair -> both inits survive with init order [d1, d0]. This decoupled
  the flow constraint from the emit-order constraint — the deadlock s1
  saw ("flow forces pair-before-p0") dissolves.
- Measured ladder (all cheat-free, 121/121 unless noted):
  - candidate-s1 (d1,d0,p0): 6 (re-confirmed baseline)
  - p0,d0,d1 ("Form P"): 2 (only the two init sh transposed)
  - d0,p0,d1 (frontier-3): 5 — KILLED (banked in rejected/)
  - p0,d1,d0 + swapped overwrites: 0 — MATCH
- Instrumentation notes: instrumented cc1 = tools/gcc-2.7.2/cc1 (in-tree,
  has BB2_RANK_DEBUG + BB2_SCHED_DEBUG + BB2_PRIO_DEBUG); the production
  build/cc1 binary predates the instrumentation and has none. Whole-file
  traces are ambiguous (INSN_UIDs repeat per function) — use a standalone
  TU (verified byte-identical emission vs in-context, labels aside).
- Artifacts: tmp/grind/func_800600C8/s2/ (standalone.c + full -da dump
  set, scheddbg.txt block-3 full pick trace, rankdbg.txt, formP_objdump,
  cmp.sh/sa.sh/uids.py tooling).

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
