# Hypothesis ledger — damage_DebugDisp

Floor 8 (candidate: for(;;) fence on CopyBlock loop). Verdict C, 79 insns.
The 8 diffs are TWO independent, well-localized problems. Raw objdump shows
9 positional diffs (4 + 5); sandbox masks one → score 8.

## Region A — inner accumulator loop: sum↔j register swap (build 0x124/0x128/0x12c/0x140)
BUILD  : j=$a0, sum=$a1  →  addiu a0,a0,1 ; addu a1,a1,v0 ; sltiu v0,a0,36 ; beq a1,v0
TARGET : sum=$a0, j=$a1  →  addiu a1,a1,1 ; addu a0,a0,v0 ; sltiu v0,a1,0x24 ; beq a0,v0
Scheduling is IDENTICAL (j++ already fills the lbu load-delay slot in both). Pure
register-allocation tie. greg dispositions: sum=pseudo 77→a1, j=pseudo 79→a0.
sum has the LOWER pseudo yet j wins the lower hard reg a0 ⇒ NOT a pseudo-order tie;
it's a local-alloc **priority** tiebreak (j, the counter, outranks sum for $a0).
sum is live across the loop-exit edge (post-loop `beq`), j is dead at exit — yet j
still wins. j is a real integer counter (target uses `sltiu ...,0x24`), so it CANNOT
be eliminated via a pointer-end compare (that would change the loop structure).

## Region B — 2nd-loop preheader: LICM-const block vs move block (build 0x1e8–0x1f8)
BUILD  order: move a0,t1 ; move a2,a0 ; lui t0,0x8000 ; lui a3,0x1f ; ori a3,0xffff
TARGET order: lui t0,0x8000 ; lui a3,0x1f ; ori a3,0xffff ; move a0,t1 ; move a2,a0
The two constants (0x80000000, 0x1FFFFF) are LICM-hoisted out of the k-loop into the
preheader; the two moves (ap=base, a2p=ap) are the pre-loop init. sched1 (basic-block
list scheduler) ties break to the lower-LUID insns; LICM inserts hoisted invariants at
the END of the preheader (higher LUID than the moves) → moves scheduled first. Target
had the constants at lower LUID / higher priority. Constant-holder named locals to bias
LUID were tried and REJECTED as cheat-by-spelling (magic_base/magic_max, score 4).

## Frontier (mechanism-grounded; for the drill session)
1. **Region A — flip the sum/j $a0 tie via priority, no dead code.** Probe: read
   local-alloc qty priority for pseudo 77(sum)/79(j) (.lreg); then try structural
   levers that lift sum's ref-priority or drop j's WITHOUT dead stores — e.g. hoist
   the check value `s32 chk = *(s32*)((u8*)chkptr+0x6C);` before the inner loop
   (changes sum's post-loop consumer to a local compare, may reprioritize sum), or
   narrow/relocate the sum/j init & compare. Verify each with `sandbox --disable all`.
2. **Region B — get LICM constants to schedule before the ap/a2p moves structurally.**
   Probe: change the range-check spelling / pointer-init placement so the moves' LUID
   rises above the hoisted constants OR the constants aren't hoisted into the same
   preheader block (e.g. compute a2p from ap inside the loop, or restructure the
   `(u32)(ptr-0x80000000)<=0x1FFFFF` test). NOT a named constant holder (cheat).
3. **Directed permuter seeded from floor-8.** PERM_* over statement order in the two
   regions + the range-check form; sweep the tie/schedule cross-product. Vet any
   closing form against the cheat catalog before proposing (dead-store/constant-holder
   forms are the known trap here).

## Ruled out (inherited — do NOT re-run)
- swap inner init `sum=0` first (V11): score 9 (rotated bp→a0, j→v1)
- `for(j=0;j<0x24;j++)` (V7): 9 ; `j` as s32 (V8): 9 (slti vs sltiu)
- swap inner body order (V10,V12): 5–10 ; sum at fn scope (V9): 8 (no change)
- combined decl+init (V2,V3): 8–13
- magic_base/magic_max const named locals (LUID bias): score 4 but CHEAT (rejected)

## [s1] The candidate for(;;) fence on the CopyBlock loop holds the floor at 8 (HEAD do-while = 9).
- mechanism: loop-exit-work-inside-loop-sched-fence materialises the trailing-word load-delay nop and pins k=0 after the sw; retires the two trailing-region regfix rules.
- probe: apply candidate.c to src; `sandbox damage_DebugDisp --disable all`
- result: score 8, verdict C, 79/79 insns, 8 rules dropped
- verdict: CONFIRMED

## [s1] Region A is a pure a0<->a1 register swap between accumulator sum and counter j, driven by a local-alloc priority tiebreak (not pseudo order, not scheduling).
- mechanism: Objdump vs target: identical scheduling (j++ fills lbu load-delay in both), only $a0/$a1 naming differs across addiu/addu/sltiu/beq. greg dispositions: sum=pseudo 77->a1, j=pseudo 79->a0 — sum has the lower pseudo yet j wins the lower hard reg $a0, so j (the counter) outranks sum for $a0 on local-alloc priority; sum is live past the loop, j dead at exit, yet j still wins.
- probe: objdump -drz sandbox .o (build_disasm.txt) diffed vs asm/funcs/damage_DebugDisp.s; cc1 -da greg dump register dispositions
- result: 4 positional diffs at 0x124/0x128/0x12c/0x140; confirmed pure allocation tie, no cheat needed
- verdict: CONFIRMED

## [s2] Region A naive structural levers are DEAD; the tie is whole-function context-dependent (permuter-only).
- Frontier item 1 (manual Region A flip) downgraded: chk-hoist=17, init-reorder=13 (bp cascades to $a0),
  sum-split coalesces to baseline=8. Register hand-out order is v1->a0->a1; target needs qty_order
  78(bp) 77(sum) 79(j) = priority bp>sum>j, but sum(range6,13333) genuinely ranks below j(range4,20000)
  and refs=4 are fixed by the target loop shape. A standalone TU does NOT reproduce the real allocation,
  so no manual source lever isolates the sum/j pick — only the real full-file build (permuter, frontier
  item 3) can sweep the LUID cross-product. Lengthening j's range needs a real post-loop consumer of the
  byte count (none exists; offset+=j is a fold-or-diverge trap). See evidence.md s2 for numbers.
- verdict: manual structural approaches KILLED; permuter (item 3) is the live path.

## [s3] Lengthening j's live range past the inner loop drops j's priority below sum's and flips the Region A a0/a1 swap.
- mechanism: global.c allocno priority = numerator/live_length. Baseline j 20000 > sum 13333 (j range 4 < sum range 6). `offset += j` (value-identical to +=0x24 since j==0x24 at exit) makes j live into the tail addu, raising j's live_length so its priority falls below sum's; sum then allocates first and takes $a0.
- probe: `offset += j;` in the outer tail; sandbox --disable all + qty_order dump.
- result: score 7 (from 8); qty_order 78 79 77 -> 78 77 79; dispositions sum=$a0/j=$a1/bp=$v1 == TARGET. FIRST measured flip of Region A. BUT emits `addu $a3,$a3,$a1` vs target literal `addiu $a3,$a3,0x24` (cannot byte-match) AND is RA-motivated cheat-by-spelling. Rejected; kept as evidence.
- verdict: CONFIRMED (mechanism real; Region A swap = pure j-vs-sum range/priority effect) / form REJECTED.

## [s3] A zero-cost outer-loop context perturbation can flip Region A without the offset instruction cost.
- mechanism: shift whole-function LUID/conflict context to lift sum's priority while leaving the outer emitted code (t1/a2/t0/a3 all matched) intact.
- probe: tail reorder `offset+=0x24;chkptr++;i++;` and index-based chk `((s32*)base)[i+0x1B]` (drop chkptr IV); sandbox each.
- result: tail-reorder=10, index-chk=14. Both disturb the matched outer registers and cascade worse. No zero-cost manual flip exists.
- verdict: KILLED. Target's sum has the same isolated inner range as build's (lw chk;nop;beq in both), so the target's sum>j priority is a whole-function conflict-graph effect only the directed permuter can sweep.

## [s1] Region B is an independent scheduling diff: LICM-hoisted range-check constants vs pre-loop pointer moves are emitted in opposite order to target.
- mechanism: The two invariants (0x80000000, 0x1FFFFF) are LICM-hoisted into the k-loop preheader; sched1 ties break to lower-LUID insns and LICM appends hoisted insns after the ap/a2p moves, so build emits moves-first while target emits constants-first. Naming constant-holder locals to bias LUID was already rejected as cheat-by-spelling.
- probe: objdump build 0x1e8-0x1f8 vs target 0x800380F0-
- result: 5 positional diffs; region independent of Region A
- verdict: CONFIRMED

## [s2] Hoisting the post-loop check value (s32 chk = *(s32*)((u8*)chkptr+0x6C)) before the inner loop reprioritizes sum and flips the sum/j $a0 tie.
- mechanism: chk becomes a preheader consumer of sum's value; intended to shorten sum's effective post-loop live range / lift its ref-priority.
- probe: Applied the hoist; sandbox --disable all.
- result: score 17 (from floor 8), build_insns 79->78. The hoisted load is a loop-carried pseudo live across the whole inner loop, adding pressure that cascades the entire allocation.
- verdict: KILLED

## [s2] Reordering the three inner-loop inits (j, bp, sum) surgically swaps sum/j so sum wins $a0.
- mechanism: qty creation/priority order follows init order; putting sum's def before j's should give sum the earlier register.
- probe: Tried bp;sum;j (VA-2) and sum;bp;j (VA-3 == inherited V11); sandbox + greg qty_order dump each.
- result: Both score 13. Register hand-out order is v1->a0->a1; moving bp later drops bp below j in priority so bp (or j) grabs the v1 slot and bp cascades into $a0. qty_order flips to 79(j) 78(bp) 77(sum). Source order cannot swap sum/j without disturbing bp.
- verdict: KILLED

## [s2] Block-local sum split (accumulate into short-lived acc, then sum=acc after the loop) shortens the accumulator live range to match j, letting the qty tiebreak favor sum.
- mechanism: acc dies at loop exit like j; equal ranges => tiebreak by qty number toward sum(77) < j(79).
- probe: s32 acc=0; loop accumulates acc; sum=acc; if(sum==chk)break; sandbox --disable all.
- result: score 8, build_insns 79 (unchanged). GCC copy-propagates/coalesces acc into sum (single qty); RTL byte-identical to baseline. The separate short-lived pseudo never exists. Hypothesis #2 dead.
- verdict: KILLED

## [s2] Region A is a pure sum/j register swap; every other register already matches target.
- mechanism: Full-file greg dispositions: base=t1, i=a2, chkptr=t0, offset=a3, bp=v1 all match target; only sum($a1 vs target $a0) and j($a0 vs target $a1) differ.
- probe: cpp|cc1 -da greg disposition dump of the sandbox-generated src, diffed vs asm/funcs/damage_DebugDisp.s.
- result: Confirmed: the 4 Region-A diffs are exactly the sum<->j a0/a1 swap. Local-alloc priority: j range4 refs4 => 20000 > sum range6 refs4 => 13333 (sum lives past the loop). refs are fixed by the target loop shape; only the birth/death LUID span (context-dependent, not isolable in a standalone TU) is free.
- verdict: CONFIRMED

## [s2] Region B ap/a2p pre-loop init order controls whether the LICM range constants schedule before the pointer moves.
- mechanism: sched1 ties break to lower LUID; LICM appends hoisted constants after the moves.
- probe: Swapped a2p=base; ap=(s32*)base; init order; sandbox --disable all.
- result: score 8 (unchanged) - GCC normalises the two base-copies. The constants (0x80000000, 0x1FFFFF) are hard literals with no semantic link to base, so no non-cheat lever makes them materialise earlier; a named constant-holder is the already-rejected magic_base cheat.
- verdict: KILLED

## [s3] Lengthening j's live range past the inner loop drops j's global-allocno priority below sum's and flips the Region A a0/a1 register swap to match target.
- mechanism: global.c priority = numerator/live_length. Baseline j 20000 (range 4) > sum 13333 (range 6), so j wins $a0. `offset += j` is value-identical to `offset += 0x24` (j==0x24 at loop exit) but makes j live into the tail addu, raising j's live_length so its priority falls below sum's; sum then allocates first and takes $a0.
- probe: Replaced `offset += 0x24;` with `offset += j;`; ran sandbox --disable all and dumped qty_order (tmp/grind/damage_DebugDisp/s2/dump.sh).
- result: score 7 (from 8). qty_order among inner pseudos 78 79 77 -> 78 77 79; dispositions sum=$a0(77 in 4), j=$a1(79 in 5), bp=$v1(78 in 3) == TARGET. First measured flip of Region A. But emits `addu $a3,$a3,$a1` vs target literal `addiu $a3,$a3,0x24` (cannot byte-match) and is RA-motivated cheat-by-spelling (no-new-park-categories test #4). Rejected as a form; kept as evidence.
- verdict: CONFIRMED

## [s3] A zero-cost outer-loop context perturbation can flip Region A without the offset instruction cost.
- mechanism: Shift whole-function LUID/conflict context to lift sum's priority while leaving the already-matched outer emitted code (base=t1, i=a2, chkptr=t0, offset=a3) intact.
- probe: Tail reorder `offset+=0x24; chkptr++; i++;` and index-based chk `((s32*)base)[i+0x1B]` (drops the chkptr walking-pointer IV); sandbox each.
- result: tail-reorder=10, index-chk=14. Both disturb the matched outer registers and cascade worse. No zero-cost manual flip found.
- verdict: KILLED

## [s3] Region B (LICM range-check constants vs pre-loop pointer moves) has a non-cheat structural lever that emits the constants before the ap/a2p moves.
- mechanism: The two constants (0x80000000, 0x1FFFFF) are LICM-hoisted from inside the k-loop; getting them to lower LUID than the moves would flip the sched1 tie.
- probe: Re-examined source-order control (s2 already measured init-swap=8-nochange). The constants are only referenced inside the loop, so LICM places them; source order cannot move them earlier.
- result: The only form that places the constants before the moves is the already-rejected magic_base constant-holder (cheat, score 4). No non-cheat manual lever exists; Region B is a context/permuter tie.
- verdict: KILLED

## [s4] do-while(0) wrapper on the inner `sum=0` init flips Region A's sum/j $a0<->$a1 swap and lowers the floor 8->6.
- mechanism: `do { sum = 0; } while (0);` delays sum's def LUID relative to j, raising sum's global-allocno priority above j's so sum allocates $a0 first (== target), WITHOUT lengthening j's live range (the offset+=j trap). do-while(0)-for-any-codegen-effect is sanctioned (FINAL 2026-07-06 do-while-zero-exception ruling).
- probe: directed permuter on the real full-TU basin (tmp/grind/damage_DebugDisp/s4/ws) found output-130-1; applied `do{sum=0}while(0)` to src; sandbox --disable all.
- result: sandbox score 6 (from floor 8). Region A inner-loop swap RESOLVED (4 diffs -> a residual 2-insn preheader reorder). New candidate.c / new floor.
- verdict: CONFIRMED — clean, sanctioned floor-lowering lever.

## [s4] Region B (LICM moves-vs-consts order) is permuter-closable from the floor-6 chassis.
- mechanism: reseed the permuter from the floor-6 (do-while0) base and let it flip the ap/a2p-moves vs LICM-const scheduling order.
- probe: fresh campaign ws6 (base_score 130); harvested all sub-130 finds and checked each with objdump vs target.
- result: KILLED. Every sub-130 find is invalid — output-75-1 (`chkptr=a2p`, s32*) reads wrong offset 0x340 (semantically broken); output-70-1 (`base=a2p`, u8*) is correct but emits 89 insns (+10 load-delay nops). Region B's tie only "moves" via wrong-type offset aliasing or nop bloat; no clean spelling reaches it in this basin.
- verdict: KILLED — no clean permuter lever for Region B (consistent with s1-s3 manual kills).

## [s4] A do-while(0) wrapper on the inner accumulator init `do { sum = 0; } while (0);` flips Region A's sum/j $a0<->$a1 register swap and lowers the floor 8->6.
- mechanism: The do-while(0) delays sum's def LUID relative to j, raising sum's global-allocno priority above j's so sum allocates $a0 first (== target), WITHOUT lengthening j's live range (unlike the rejected offset+=j which value-folded but cheated). do-while(0)-for-any-codegen-effect is sanctioned per the FINAL 2026-07-06 do-while-zero-exception ruling.
- probe: Built a permuter workspace on the REAL full-TU basin (base.c = cpp of src/code6cac_c_mid.c, honest pipeline, target.o from asm/funcs); campaign found output-130-1; applied `do{sum=0}while(0)` to src and ran sandbox --disable all.
- result: sandbox --disable all = 6 (from floor 8); Region A inner-loop swap RESOLVED (4 diffs -> a residual 2-insn preheader reorder). New candidate.c.
- verdict: CONFIRMED

## [s4] The full preprocessed-TU permuter basin reproduces the real allocation, unlike the s2 hand-built standalone.
- mechanism: The real cpp output carries the true extern types/context that drive damage_DebugDisp's register allocation; the s2 standalone guessed types and gave sum=$a2 instead of $a1.
- probe: Compiled base.c via the honest workspace pipeline and diffed damage_DebugDisp objdump vs target.o.
- result: 79 insns, exact sandbox diff reproduced (Region A a0/a1 swap + Region B move/const order). Basin validated as correct.
- verdict: CONFIRMED

## [s4] Region B (4-insn LICM moves-vs-consts order) is permuter-closable from the floor-6 do-while(0) chassis.
- mechanism: Reseed the permuter from the floor-6 base and let it flip the ap/a2p-moves vs LICM-const scheduling order.
- probe: Fresh campaign ws6 (base_score 130, 11k iters); harvested every sub-130 find and checked each with objdump vs target.
- result: KILLED. output-75-1 (`chkptr=a2p`, s32*) reads wrong byte offset 0x340 (semantically broken: `lhu 832(a2)` vs target `208`); output-70-1 (`base=a2p`, u8*, correct) emits 89 insns (+10 load-delay nops). Region B's tie only 'moves' via wrong-type offset aliasing or nop bloat; no clean spelling reaches it in this basin.
- verdict: KILLED

## [s5] The residual Region A' 2-insn preheader scheduling tie is closable by permuting the inner-loop preheader statement order (PERM_LINESWAP over j=0 / bp=base+offset / do{sum=0}while(0)).
- mechanism: Frontier item 1: a different init spelling/placement stacked on the do-while(0) restores target's bp-addu-before-sum-move order while holding the solved Region A allocation (score<=6).
- probe: Exhaustively sandboxed (--disable all) all 3!=6 orderings of the three preheader statements on the floor-6 chassis.
- result: (j,bp,dw)=6 [candidate, UNIQUELY optimal]; (j,dw,bp)=10; (bp,j,dw)=11; (bp,dw,j)=13; (dw,j,bp)=10; (dw,bp,j)=13. Every neighbor of the candidate regresses.
- verdict: KILLED

## [s5] A fresh-seed directed permuter campaign on the floor-6 (do-while(0)) chassis finds a clean pure-C lever that lowers the floor below 6 (closing Region A' or Region B).
- mechanism: Reseed a structurally-fresh full-TU basin from the floor-6 base (base_score 130) and sweep the whole-function LUID/scheduling cross-product that manual source reorder cannot reach surgically.
- probe: Launched floor6-s5-freshseed campaign (base.c = do-while(0) form, honest pipeline, target.o from asm/funcs), 20,895 iterations, 8 jobs; harvested every novel find and inspected/compiled each vs target.
- result: All sub-130 finds are the known-invalid Region B dead-variable-alias family: output-75-1 (chkptr=a2p, s32*+0xD0=+0x340 bytes, BROKEN), output-90-1 (src=a2p dead alias, cheat), output-70-1 (base=a2p dead reassignment, cheat, 79i), output-100-1 (u8*new_var=src redundant dead alias, cheat, score 100). No find touches the Region A' preheader tie; none reach floor<6; none clean. Exactly reproduces s4's floor-6 result with a fresh seed.
- verdict: KILLED
