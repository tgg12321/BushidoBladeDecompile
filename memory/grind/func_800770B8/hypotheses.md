# Hypothesis ledger — func_800770B8

## Session 1 (recon, 2026-09-01) — floor 175 → 26 → 20 → 15 → 14

CONFIRMED:
- H1 "function returns 1, not void" — target's pre-epilogue `addiu $v0,$zero,1`
  is a live return value; `return 1;` + caller prototype fix is load-bearing
  (part of the 26-floor clean chassis; the chassis-era tail rules @2173-2187
  largely evaporated).
- H2 "the srav+andi vs sllv+and diff is the tree-level single-bit fold" —
  `s32 mask = 1 << idx;` named intermediate keeps the shift; 26 → 20. The
  hoisted `$t1 = 1` invariant appears exactly as target. (Ordinary once-written
  once-read local holding a real consumed value; semantically truthful name.)
- H3 "target does NOT hoist &D_800A35D0 because the symbol materializes into a
  multi-set reused pseudo" — two-statement spelling into the reused `ptr`
  (`ptr = (u8*)&D_800A35D0; ptr = (t0*4) + ptr;`) defeats scan_loop admission;
  20 → 15. Single-statement forms hoist (fresh single-set temp).
- H4 "row-43 addu operand order follows int-vs-pointer domain of the address
  tree" — `(u8*)((t0*2) + (s32)base)` flips to target's (offset,base) order;
  15 → 14. Pointer-domain swaps are canonicalized away (byte-neutral).

KILLED (measured, forms banked in rejected/):
- K1 q-split second handle for the 0x30/0x34 stores → 28, insn collapse (cse
  forwards the handle, deletes the loop-preheader reload).
- K2 tail cached-pointer statement groups → 29 (cse merges the repeated
  D_800A36A0 reads; direct per-statement global derefs already match the tail).
- K3 common `row` pointer for p_6a/p_7e → 38 (restructures CSE of (t0*10));
  association-order spelling of the same expression → byte-neutral.

## Live frontier (for s2)

1. **reorg fill choice on the inner-loop-2 beqz** (residual class 4, ~2 diffs +
   phase). Mechanism: reorg.c picks our `ori` (fall-through steal, dest dead on
   taken path) where target steals `addiu a2+1` from the branch-target thread and
   retargets past it. Probe: read tmp/grind/func_800770B8/dumps/text1b.dbr for
   the beqz's candidate list/eligibility; then find the C shape that makes the
   ori ineligible or the thread-steal preferred (e.g. if-arm first statement
   writing a register live at the join; `for` vs `do-while` respelling; ONLY
   as a measured last resort the sanctioned duplicated-statement-into-arms
   family with its full prerequisites). This is the rule-era insert_label
   device residual — main's ledger has the mechanism background.
2. **0x30/0x34 store handle + a2=0 placement** (residual class 2, ~4 diffs).
   Target keeps the func_8006E49C result in $v0 for two stores while $s1 holds
   the copy. Probe: .lreg/.greg for the call-result pseudo's death; C levers
   that split the last two stores' addresses from p_old WITHOUT a cse-visible
   second handle (e.g. reorder the four post-call statements; store 0x30/0x34
   BEFORE D_800A36A0/+4; sub-expression shapes). K1 bounds the naive spelling.
3. **prologue schedule / ClearOTagR arg order + p_6a base coalesce** (classes
   1+3, ~8 diffs). Class 3 probe: .lreg death counts for the base vs offset
   pseudos at the row-62 addu ([[local-alloc-death-count-class-wall]] gate
   first — if the multi-death pseudo is punted, spelling levers are closed).
   Class 1 probe: .sched dump of block 0 — see whether a1-first arg evaluation
   (target) is reachable by arg-expression shape (e.g. hoisting D_800A374C into
   a named local) before touching anything else; prologue save order may be
   pure cascade from the body's register demand, so re-measure AFTER frontier
   items 1-2 land.

## [s1] Function returns 1 (not void); chassis tail rules were void-artifacts
- mechanism: live $v0=1 before epilogue; caller ignores result
- probe: s32 signature + return 1 + caller prototype fix, sandbox
- result: 26-floor clean chassis, tail byte-clean
- verdict: CONFIRMED

## [s1] srav+andi vs target sllv+and is the tree-level single-bit fold of (x & (1<<k))!=0
- mechanism: fold-const bit-test transform; separate named local hides the shift from the fold
- probe: s32 mask = 1 << idx; if (arg2 & mask)
- result: 26 -> 20; hoisted $t1=1 appears exactly as target
- verdict: CONFIRMED

## [s1] Target leaves lui/addiu D_800A35D0 in-loop because the symbol materializes into a multi-set reused pseudo
- mechanism: loop.c scan_loop movable admission requires n_times_set==1; expand_assignment targets ptr's pseudo directly only in the two-statement spelling
- probe: ptr = (u8*)&D_800A35D0; ptr = (t0*4) + ptr;
- result: 20 -> 15; rows 49-51 byte-clean; single-statement form still hoists
- verdict: CONFIRMED

## [s1] row-43 addu operand order follows int-vs-pointer domain of the address expression
- mechanism: pointer-domain PLUS is canonicalized (base,off); int-domain preserves written order
- probe: ptr = (u8*)((t0*2) + (s32)base)
- result: 15 -> 14
- verdict: CONFIRMED

## [s1] Second C handle q for the call result reproduces target's $v0-handle stores at 0x30/0x34
- mechanism: two live pseudos for one value
- probe: u8 *q = func_8006E49C(...); stores via q
- result: 28, build collapses 176->171 (cse forwards q, deletes preheader reload)
- verdict: KILLED

## [s1] Tail needs cached-pointer statement groups to share gp loads
- mechanism: one load per group
- probe: p = D_800A36A0; grouped stores
- result: 29; cse merges the repeated reads below target's count; direct derefs already match
- verdict: KILLED

## [s1] Common row pointer coalesces the p_6a base addu into the offset register
- mechanism: RA coalesce with offset pseudo
- probe: u8 *row = (u8*)((t0*10)+(s32)D_800A36A0)
- result: 38 isolated; association-order swap byte-neutral
- verdict: KILLED

## Session 2 (structural, 2026-09-01) - floor 14 -> 12 -> 10

CONFIRMED:
- H5 "the target's tail reads D_800A36A0 once for the {+0, +0x65} store pair" -
  a local `u8 *q = D_800A36A0;` scoped to just those two stores removes our
  redundant reload (lw + load-delay nop); 14 -> 12. s1's K2 stays dead: the rest
  of the tail must keep its per-statement global derefs.
- H6 "a2's initialiser is a statement INSIDE the outer loop body, not a preheader
  init plus a tail reset" - moving it to the first statement of the do-body sinks
  `addu $a2,$zero,$zero` from sched1 slot 30 to slot 37 (target's slot) and drops
  the duplicate; 12 -> 10; rows 30-34 and 37 byte-clean.
- H7 "the addiu-$s1 (`arg0 + 0x58`) prologue slot follows C statement order" -
  moving the statement after ClearOTagR moves the insn from slot 8 to slot 11
  (target: 12). CONFIRMED as a mechanism but NET WORSE (12 -> 13) because the
  freed slot goes to `lui %hi(D_800A374C)` rather than target's `sw $ra`.
  Form banked in rejected/s2-p_old-computed-after-clearotagr.c.
- H8 "class D is reorg's fall-through delay-slot steal winning where the target's
  build lost it" - PROVEN, not inferred. DBRDBG trace for jump_insn 314 shows
  likely=0 / own=1 / tif=0 / setsopp=0 / WINNER trial=327 (the `ori`), and the
  what-if BB2_ALLLIVE_LABEL=355 (force mark_target_live_regs conservative at the
  join) reproduces the target's `beq` + copied `addu $2,$6,1` delay slot and
  closes the 174->175 insn-count gap. Full reading in evidence.md.

KILLED (measured, forms banked in rejected/):
- K4 second C handle for the func_8006E49C result, spelled the OTHER way from K1
  (reuse `r` as an int handle, `p_old = (s32 *)r`, stores via `r`) -> 26, build
  collapses 174 -> 169 (cse forwards the handle, deletes the D_800A35D0 preheader
  reload). With K1 this closes the "name the call result twice" axis in both
  directions. rejected/s2-call-result-second-handle.c
- K5 int-domain address form for the p_6a/p_7e bases (the s1-H4 trick applied to
  rows 62-64) -> 34. The +0x6A/+0x7E constants fold into the int expression and
  destroy the shared (t0*10) CSE. Confirms rows 62-64 are an allocator decision,
  not an address-shape decision. rejected/s2-p6a-int-domain-address.c
- K6 (byte-neutral, not a regression) writing the 0x30/0x34 stores as
  `*(s32 *)(D_800A36A0 + 0x30) = 0;` - cse resolves the global load to p_old's
  pseudo, so it cannot reach the raw call-result pseudo. No form banked (it is
  one-line and neutral); do not re-measure.

## Live frontier (for s3)

1. **Class D via own_fallthrough (the ONE untried half of a proven mechanism).**
   Mechanism: reorg.c own_thread_p returns 0 for the fall-through thread if ANY
   CODE_LABEL sits between the beqz and the if-body's first active insn; with
   own_fallthrough==0 fill_eager_delay_slots skips the fall-through steal entirely
   and fills from the branch-target thread by COPY+redirect - the target's shape,
   worth the last insn of the count gap. Probe: C shapes that materialise a label
   at the then-arm head - `if (A || B) { ... }` short-circuit forms of the existing
   single test (e.g. splitting the bit test), and `goto`-into-the-arm shapes.
   Sanity-check each candidate by re-running s2/run_idump.sh with BB2_DBR_DEBUG=1
   and reading the `own=` field of `DBRDBG thr insn=<uid>` - that field tells you
   whether the shape worked BEFORE you look at the score. Do NOT spend probes on
   making $v0 live at the join (option (b)); it needs find_basic_block()==-1, which
   no C respelling controls.
2. **Class A(ii): ClearOTagR a1-first argument evaluation** (~3 rows, the largest
   remaining cluster). Target emits `addiu $a1,$zero,0x1008` BEFORE the
   `lui/lw %hi/%lo(D_800A374C)` pair; ours emits the 2-insn a0 chain first because
   sched1 ranks it higher (longer path to the jal). Probe: read s2/idumps/text1b.sched
   for block 0's priorities and ready-list order, then try arg-expression shapes that
   shorten the a0 chain or lengthen the a1 one (a named local for the OT pointer read
   before the call, a differently-typed OT arg). H7 says statement order does move
   block-0 insns, so this is a live spelling axis, not a wall - but re-measure the
   prologue-save rows together with it (H7's regression came from the save order).
3. **Class B/C are allocator decisions, not spellings.** B: our call-result temp is
   `Register 80 used 2 times` (dies at the copy) - both second-handle spellings are
   dead, so the next honest step is tools/ra_solver / inverse_compose.py classify on
   the p_old-vs-temp seat, not more C. C: same, .lreg "dies in N places" for the
   row-62 base/offset pseudos ([[local-alloc-death-count-class-wall]] gate first).
   Neither should consume a whole session before frontier 1 and 2 are spent.

## [s2] Target's tail groups ONLY the {+0, +0x65} stores behind one D_800A36A0 read
- mechanism: a store through a pointer may alias the global, so GCC reloads it between statements; one local for that pair removes the lw + load-delay nop
- probe: { u8 *q = D_800A36A0; *(s32 *)q = arg1; *(s8 *)(q + 0x65) = 0; }
- result: 14 -> 12 (176 -> 174 insns); tail becomes byte-clean modulo the global count gap
- verdict: CONFIRMED

## [s2] a2's initialiser is a statement inside the outer loop body, not a preheader init + tail reset
- mechanism: sched1 block-0 placement; the preheader form ranks a2=0 at slot 30, the in-body form leaves it at slot 37 after the 0x30/0x34 store cluster
- probe: single `a2 = 0;` as the first statement of the do-body; delete the preheader init and the tail reset
- result: 12 -> 10; rows 30-34 and 37 byte-clean
- verdict: CONFIRMED

## [s2] The prologue addiu $s1,$s0,0x58 slot follows C statement order
- mechanism: sched1 LUID tie-break in block 0
- probe: move `p_old = (s32 *)(arg0 + 0x58);` to after ClearOTagR
- result: insn moves slot 8 -> 11 (target 12) but score 12 -> 13; the freed slot goes to lui %hi(D_800A374C), not target's sw $ra
- verdict: CONFIRMED (mechanism) / form REJECTED (net worse)

## [s2] Class D is reorg's fall-through delay-slot steal, which the target build lost
- mechanism: reorg.c fill_eager_delay_slots, likely=0 => fall-through tried first; own_fallthrough=1 and setsopp=0 let our `ori` win the slot. Blocking it requires own_fallthrough==0 (a CODE_LABEL at the then-arm head) or mark_target_live_regs answering all-live at the join.
- probe: instrumented cc1 BB2_DBR_DEBUG=1 trace of jump_insn 314 + what-if BB2_ALLLIVE_LABEL=355
- result: what-if reproduces the target's beq + copied `addu $2,$6,1` delay slot and closes the 174->175 count gap
- verdict: CONFIRMED

## [s2] A second C-level handle for the func_8006E49C result reaches the $v0-based 0x30/0x34 stores
- mechanism: two live pseudos for one value (spelled the opposite way from K1: `r` as the int handle, p_old as the derived pointer)
- probe: r = (s32)func_8006E49C(...); p_old = (s32 *)r; stores via r
- result: 26, build collapses 174 -> 169 (cse forwards the handle, deletes the D_800A35D0 preheader reload) - same failure as K1 on a different chassis
- verdict: KILLED

## [s2] The s1 int-domain address trick (H4) generalises to the p_6a/p_7e bases at rows 62-64
- mechanism: int-domain PLUS preserves written operand order, so the addu dest should follow the offset pseudo
- probe: (s16 *)((t0 * 10) + (s32)D_800A36A0 + 0x6A)
- result: 34 - the byte constants fold into the int expression and destroy the shared (t0*10) CSE
- verdict: KILLED

## [s2] The target's tail reads D_800A36A0 once for the {+0 (arg1), +0x65} store pair only; our per-statement global derefs reload it between them.
- mechanism: A store through a pointer may alias the global, so GCC re-emits the gp-relative load; the reload costs lw + a load-delay nop that the target does not have. Target's tail grouping is exactly {+0,+0x65} | {+0x67} | {lbu +0x67, sb +0x66}.
- probe: { u8 *q = D_800A36A0; *(s32 *)q = arg1; *(s8 *)(q + 0x65) = 0; } scoped to just that pair; sandbox --disable all.
- result: 14 -> 12 (176 -> 174 insns); the whole tail from row 148 to the epilogue becomes byte-clean modulo the one-insn global count gap. s1's K2 (grouping the WHOLE tail) stays dead - only this pair groups.
- verdict: CONFIRMED

## [s2] a2's initialiser is a statement inside the outer loop body, not a preheader init plus a duplicate reset at the outer-loop tail.
- mechanism: sched1 block-0 placement: the preheader form ranks `addu $a2,$zero,$zero` into slot 30 (ahead of the hoisted loop invariants and the 0x30/0x34 store cluster); the in-body form leaves it at slot 37, exactly where the target has it.
- probe: Single `a2 = 0;` as the first statement of the outer do-body; delete both the preheader init and the tail reset. sandbox --disable all.
- result: 12 -> 10; rows 30-34 and row 37 all byte-clean; the pre-loop cluster residual (s1 class 2a) is fully closed.
- verdict: CONFIRMED

## [s2] The prologue `addiu $s1,$s0,0x58` slot follows C statement order (LUID tie-break in sched1's block 0).
- mechanism: sched.c rank_for_schedule tie-break on equal priority; the insn has no in-block dependents so its slot tracks its RTL position.
- probe: Move `p_old = (s32 *)(arg0 + 0x58);` from before ClearOTagR to after it; sandbox + positional diff.
- result: The insn does move (slot 8 -> 11; target has it at 12), so the mechanism is real - but the score goes 12 -> 13 because the vacated slot 7/8 pair goes to `lui %hi(D_800A374C)` rather than target's `sw $ra`/`sw $s1`. Form banked in rejected/s2-p_old-computed-after-clearotagr.c.
- verdict: CONFIRMED

## [s2] The last residual insn-count gap (ours 174, target 175) is reorg's fall-through delay-slot steal winning in our build where the target build lost it.
- mechanism: reorg.c fill_eager_delay_slots. mostly_true_jump returns 0 for our beqz (EQ test, equal rare_destination on both sides), so the FALL-THROUGH thread is tried first; own_fallthrough=1 and the trial's dest $v0 is not in opposite_needed (the join's first insn sets $v0 without reading it), so our `ori` legally takes the slot. The target instead filled from the branch-target thread by copying `addiu $v0,$a2,1` and redirecting past it.
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_DBR_DEBUG=1 -> trace for jump_insn 314; then the what-if knob BB2_ALLLIVE_LABEL=355 (force mark_target_live_regs to its conservative everything-live answer at the join thread, i.e. simulate find_basic_block()==-1) and diff the emitted .s.
- result: Trace: `DBRDBG thr insn=314 thread=327 opp=355 own=1 likely=0 tif=0 oppregs=200c0fe0_00000000 oppmem=1` / `trial=327 ... setsopp=0 trap=0` / `WINNER insn=314 trial=327`. The what-if reproduces the target EXACTLY in that region (beq + delay `addu $2,$6,1` + then `ori`) and takes the build 174 -> 175 insns. Only two things can block the steal: own_fallthrough==0 (a CODE_LABEL at the then-arm head - C-reachable) or an all-live mark_target_live_regs answer (not C-reachable).
- verdict: CONFIRMED

## [s2] A second C-level handle for the func_8006E49C result reproduces the target's $v0-based 0x30/0x34 stores.
- mechanism: Two live pseudos for one value; spelled the OPPOSITE way from s1's K1 (reuse `r` as the int handle, derive p_old = (s32 *)r, do the last two stores through r).
- probe: r = (s32)func_8006E49C(r, D_800A35D8); p_old = (s32 *)r; stores at +0x30/+0x34 through r; sandbox --disable all.
- result: 26, and the build collapses 174 -> 169 insns - cse forwards the handle and deletes the D_800A35D0 loop-preheader reload. Identical failure mode to K1 on a different chassis, so the 'name the call result twice' axis is now dead in BOTH directions. Separately measured byte-NEUTRAL: writing those stores as `*(s32 *)(D_800A36A0 + 0x30) = 0;` (cse resolves the global load to p_old's pseudo, never to the call-result temp).
- verdict: KILLED

## [s2] The s1 H4 int-domain address trick (which fixed the row-43 addu operand order) generalises to the p_6a/p_7e base addresses at rows 62-64.
- mechanism: int-domain PLUS preserves written operand order where pointer-domain PLUS is canonicalized, so the addu dest should follow the offset pseudo as it does in the target.
- probe: s16 *p_6a = (s16 *)((t0 * 10) + (s32)D_800A36A0 + 0x6A); same for p_7e; sandbox --disable all.
- result: 34 (from a 10 chassis). The +0x6A/+0x7E constants fold into the int expression, destroying the shared (t0*10) CSE and restructuring rows 55-70. With s1's K3 this closes the address-shape axis on rows 62-64 from both directions - it is a local-alloc dest-coalesce decision.
- verdict: KILLED
