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

## [s6] Forensics: name the exact GCC pass+decision for each floor-6 residual (Region A' preheader tie, Region B LICM order).
- mechanism: cc1 -da full RTL dumps on the floor-6 src; extract sched1 ready-lists + global.c allocation order + loop.c hoist point; A/B greg do-while(0) vs plain sum=0.
- probe: tmp/grind/damage_DebugDisp/s6/{gen_dumps.sh,diff_ops.sh,ab_greg.sh}; dumps/dd.c.* ; FORENSIC_SUMMARY.md.
- result:
  * Region A' = sched.c sched1 `rank_for_schedule` INSN_LUID tiebreak (sched.c:54-57) over preheader BB1 (insns 27 j / 30 bp / 36 sum, all prio=1). sum=0 (insn 36) is the block-bottom ref_count=0 leaf -> emits LAST; target emits it FIRST.
  * do-while(0) A/B PROVEN as sole cause of the sum/j allocation-order swap: global.c `allocno_compare` priority = floor_log2(n_refs)*n_refs*size/live_length*10000; n_refs=4/size=1 equal for 77&79, so the wrapper's live_length shortening (def relocated to block bottom) is what lifts sum(77) above j(79). Alloc order `100 78 79 77` (plain) -> `100 78 77 79` (do-while0).
  * IRREDUCIBLE COUPLING: RA win and scheduling loss both flow from sum's def LUID; A' is unflippable on the do-while(0) chassis by def position (explains s5's 6-ordering exhaustion).
  * Region B = loop.c `move_movables` `emit_insn_before(loop_start)` (loop.c:1652) puts hoisted invariants at preheader END (higher LUID than ap/a2p moves) -> sched1 LUID order emits moves-first. Target needs consts at lower LUID = pre-loop reference = constant-holder cheat. Dead.
- verdict: CONFIRMED (both mechanisms named with measurements). Neither residual is closable by preheader statement order; both require a whole-function LUID/priority change (directed-permuter axis, s4/s5 found no clean sub-6 form).

## [s6] Region A' (preheader emit order j,bp,sum vs target sum,bp,j) is set by sched.c sched1's INSN_LUID tiebreak, fed by the do-while(0)'s def-relocation of sum=0 to the preheader block-bottom.
- mechanism: sched.c rank_for_schedule (sched.c:54-57): after equal INSN_PRIORITY (all preheader inits are prio=1) and equal dep-class, the final tiebreak is INSN_LUID(tmp)-INSN_LUID(tmp2) = original insn order. In BB1 (insns 27 j:a1 / 30 bp:v1 / 36 sum:a0), sum=0 (insn 36) is the sole ref_count=0 leaf at block bottom -> scheduled to the last slot; the remaining {27,30} emit in LUID order -> net j,bp,sum. Target = sum,bp,j.
- probe: cc1 -da full RTL dump (tmp/grind/damage_DebugDisp/s6/dumps/dd.c.sched, dd_sched.txt BB1) + ready-list trace.
- result: sched BB1 ready init={36}; T-2 {27,30}->picks 30; T-3->27; emit j,bp,sum. Matches build objdump (build_ops.txt lines 5-7).
- verdict: CONFIRMED

## [s6] The do-while(0) is the SOLE cause of sum(pseudo77) allocating a0 vs a1 (Region A), via global.c allocno live_length priority — and that same def-relocation is what forces sum=0 to emit last (Region A'). RA win and scheduling loss are one coupled cause.
- mechanism: global.c allocno_compare: priority = floor_log2(n_refs)*n_refs*size/live_length*10000. For 77(sum) and 79(j), n_refs=4 and size=1 are equal, so only live_length decides. do-while(0) moves sum's def to insn 36 (block bottom, after NOTE_INSN_LOOP_BEG 285) -> shortens pseudo-77 live_length -> priority > j(79) -> sum allocated first -> a0. The identical relocation makes insn 36 the highest-LUID preheader insn -> sched1 LUID tiebreak emits sum=0 last.
- probe: A/B greg on identical TU, do{sum=0}while(0) vs plain sum=0 (tmp/grind/damage_DebugDisp/s6/ab_greg.sh, dd.c.greg vs dd_plain.c.greg).
- result: plain alloc order 100 78 79 77 -> 77=a1,79=a0 (floor-8 swap); do-while0 alloc order 100 78 77 79 -> 77=a0,79=a1 (target). Only 77<->79 differ. Explains s5's 6-ordering exhaustion: sum's a0 REQUIRES its def at block bottom = the exact slot sched1 emits last, so A' is unflippable by preheader statement order on this chassis.
- verdict: CONFIRMED

## [s6] Region B (build ap,a2p,C0,C1,C2 vs target C0,C1,C2,ap,a2p) is set by loop.c LICM inserting the hoisted invariants at the preheader END (higher LUID than the ptr moves), then sched1's LUID tiebreak emitting moves-first.
- mechanism: loop.c move_movables hoists invariants via emit_insn_before(newpat, loop_start) (loop.c:1652) = immediately before NOTE_INSN_LOOP_BEG, i.e. AFTER the pre-existing ap/a2p preheader init. Hoisted consts (insns 281/287/288) thus always get higher LUID than the moves (213/216); sched1 (all prio=1) emits in LUID order -> moves-first. Target = constants-first, so target's invariants had lower LUID (positioned before the ptr init).
- probe: dd_greg.txt insns 213-288 chain order + dd_sched.txt BB13 ready-list.
- result: sched BB13 emits 213,216,281,287,288 = moves-first. Constants are referenced ONLY inside the k-loop, so the only source form placing them earlier is a pre-loop reference = the rejected constant-holder cheat family (magic_base / regionB-*-alias). Dead s1-s6.
- verdict: CONFIRMED

## [s7] Region A's sum $a1->$a0 flip is a live_length effect (s6 claim). FALSIFIED.
- mechanism (s6): do-while(0) shortens pseudo-77 live_length -> global.c priority > j(79).
- probe: .lreg `dump_flow_info` (flow.c:2890) prints reg_n_refs + reg_live_length for 77/78/79 on
  do-while0 (dd.c) and plain (dd_plain.c) chassis.
- result: live_length(77)=live_length(79)=9 on BOTH chassis — unchanged. The lever is reg_n_refs:
  plain sum=10, do-while0 sum=11 (=j). `reg_n_refs += loop_depth` (flow.c:2081+); the do-while(0)'s
  NOTE_INSN_LOOP_BEG raises sum=0's block depth 1->2, adding +1 weighted ref. Tie then broken by
  allocno number (sum 77 < j 79) via global.c allocno_compare `return *v1-*v2`.
- verdict: s6 live_length mechanism KILLED; replaced with CONFIRMED reg_n_refs loop-depth-weight tie.

## [s7] A sanctioned construct can add sum's +1 loop-depth-weighted ref WITHOUT forcing sum=0 to block-bottom (decoupling A' from the RA win).
- mechanism: the RA win needs sum weighted-n_refs>=11 with sum=0 at low LUID; if achievable without a
  loop-note bracket at sum's def, sched1 could emit sum=0 first (A' closed) while sum still wins a0.
- probe: enumerate depth-raising constructs. do-while(0)/for(i<1)/while(1){break} all emit
  NOTE_INSN_LOOP_BEG (couple placement); if(1){} emits no loop note (adds 0 weight); an extra explicit
  sum ref at depth>=1 = dead code. Also the inverse: drop j 11->10 (j=0 hoist to depth0).
- result: KILLED. Every depth-raising wrapper re-introduces the bracket coupling; if(1){} adds nothing;
  extra sum ref = cheat (no-new-park-categories). j=0 can't leave depth1 (must reset each outer
  iteration). Sibling func_80037F40 achieves the natural allocation only because its accumulate is
  shallower-nested (runs once pre-outer-loop) — a shape damage's semantics forbid.
- verdict: KILLED. A' remains coupled to the RA win through the loop-note bracket; no clean decoupler
  found. Whole-function permuter axis (s4/s5) also found none.

## [s7] Region A's sum $a1->$a0 flip is caused by the do-while(0) shortening pseudo-77's live_length (s6's named mechanism).
- mechanism: s6: do-while(0) relocates sum's def to preheader block-bottom, shortening reg_live_length(77) so global.c allocno priority rises above j(79).
- probe: cc1 -da .lreg dump (flow.c:2890 dump_flow_info prints reg_n_refs + reg_live_length verbatim) for pseudos 77/78/79 on do-while0 (dd.c) and plain (dd_plain.c) chassis.
- result: live_length(77)=live_length(79)=9 on BOTH chassis — never changes. The only delta is reg_n_refs(sum): plain 10 -> do-while0 11. s6's live_length story is false.
- verdict: KILLED

## [s7] The do-while(0)'s Region-A win is a loop-depth-weighted reg_n_refs effect producing an exact global.c priority tie broken by allocno number (sum pseudo 77 < j 79).
- mechanism: reg_n_refs[regno] += loop_depth (flow.c:2081/2329/2515/2725): the do-while(0)'s NOTE_INSN_LOOP_BEG raises sum=0's block loop_depth 1->2, +1 weighted ref (10->11=j). allocno_compare pri = floor_log2(n_refs)*n_refs/LL*1e4*size; floor_log2(10)=floor_log2(11)=3, LL9, size1 -> 33333<36666 (plain, j=a0) vs 36666==36666 (do-while0, TIE); tie -> `return *v1-*v2`; allocnos ascend by pseudo (global.c:384-397) so sum(77)<j(79) wins a0.
- probe: greg alloc order (plain `100 78 79 77` vs do-while0 `100 78 77 79`) + global.c allocno_compare/allocno-assignment source read + the .lreg n_refs/LL numbers.
- result: Confirmed exactly. The RA solve is a fragile exact priority tie decided by sum's lower pseudo number. Coupling recharacterized: the SAME NOTE_INSN_LOOP_BEG both grants the +1 ref (win) and forces sum=0 to block-bottom/highest-LUID (sched1 emits it last = A' residual).
- verdict: CONFIRMED

## [s7] A sanctioned construct can add sum's +1 loop-depth-weighted ref WITHOUT forcing sum=0 to block-bottom, decoupling Region A' from the RA win.
- mechanism: If sum reaches weighted-n_refs>=11 with sum=0 at low LUID (no loop-note bracket at its def), sched1 could emit sum=0 first (A' closed) while sum still wins a0; alternatively drop j 11->10 so plain-sum's 10 ties.
- probe: Enumerate depth-raising constructs (do-while(0)/for(i<1)/while(1){break} all emit NOTE_INSN_LOOP_BEG; if(1){} emits none; extra explicit sum ref = dead code) and the j-reduction axis (hoist j=0 to depth 0).
- result: KILLED. Every loop-note wrapper re-couples placement; if(1){} adds 0 weight; a synthetic extra sum ref is a cheat (no-new-park-categories). j=0 must reset each outer iteration (can't leave depth 1); j++/j<0x24 are minimal depth-2 refs. Sibling func_80037F40 lands the natural allocation only because its accumulate is shallower-nested (runs once pre-outer-loop) — a shape damage's semantics forbid.
- verdict: KILLED

## [s8] Rederiving the outer loop via m2c (negated-compare + goto, increments inside the if) yields a structurally different shape that flips Region A or A'.
- mechanism: a different outer-loop BB layout changes the sched1 preheader LUID ordering (Region A') and/or the whole-function conflict graph feeding Region A.
- probe: applied m2c's goto/negated-compare shape (PLAIN sum=0, then +do{sum=0}while(0)); sandbox --disable all + objdump vs target.
- result: PLAIN=8 (Region A unsolved, == plain break form); +do-while0=6 with objdump residual IDENTICAL to candidate (A' preheader j,bp,sum vs target sum,bp,j + Region B). Outer CFG does not move the preheader inits' LUIDs.
- verdict: KILLED. Rederive of the outer loop is score/residual-equivalent to candidate.

## [s8] Region A' can be decoupled from the RA win by raising sum's loop-depth-weighted refs to 11 via a NON-def sum reference (the post-loop compare) in a do-while(0), keeping sum=0 plain at low LUID.
- mechanism (s7 frontier item 1): if sum reaches weighted-refs 11 (tie j) without relocating sum=0's def, sched1 emits sum=0 first (A' matches target) while sum still wins $a0 via the allocno tiebreak.
- probe: (a) `do{ if(sum!=chk){incr;goto} }while(0)`; (b) `do{ matched=(sum==chk); }while(0); if(!matched){incr}`. sandbox --disable all each.
- result: BOTH score 12 (from 6). The loop-note bracket around the tail raises loop_depth for the matched outer IVs (i=a2/chkptr=t0/offset=a3) and/or materializes a boolean, cascading the outer allocation.
- verdict: KILLED. The +1 sum ref cannot be sourced from the tail region; it must come from sum's own def or the inner accumulate, both re-introducing the s6/s7 A'/A coupling. No manual decoupler exists.

## [s8] Kengo transplant supplies the original source structure for damage_DebugDisp.
- mechanism: Marionation engine structure is preserved BB2(PS1)->Kengo(PS2); the matched Kengo source reveals the original C shape.
- probe: kengo_matches.csv + kengo_ref.py dump of the Kengo equivalent.
- result: KILLED. Match is name-only (is_damage_calc, 0.00 confidence); Kengo's damage_DebugDisp @0x11fd88 is an unrelated font/matrix debug-display function (fnt_print/atan2/conv_matrix_rotation), not the checksum validator. No structural transplant source. decomp.me scrape env-blocked (curl_cffi/network absent).
- verdict: KILLED.

## [s8] Rederiving the outer loop via m2c (negated compare + goto, increments inside the if) yields a structurally different shape that flips Region A or A'.
- mechanism: A different outer-loop BB layout changes sched1's preheader LUID ordering (Region A') and/or the whole-function conflict graph feeding Region A's sum/j allocation.
- probe: Applied m2c's goto/negated-compare shape with PLAIN sum=0, then with do{sum=0}while(0); sandbox --disable all + objdump vs target.
- result: PLAIN=8 (Region A unsolved, identical to plain break form); +do-while0=6 with objdump residual IDENTICAL to candidate (A' preheader j,bp,sum vs target sum,bp,j + Region B). Outer CFG does not move the preheader inits' LUIDs.
- verdict: KILLED

## [s8] Region A' can be decoupled from the RA win by raising sum's loop-depth-weighted refs to 11 via a NON-def sum reference (the post-loop sum==chk compare) inside a do-while(0), keeping sum=0 plain at low LUID so it emits first (target A' order).
- mechanism: s7 frontier item 1: if sum reaches weighted-refs 11 (tie j) without relocating sum=0's def, sched1 emits sum=0 first while sum still wins $a0 via the allocno-number tiebreak (sum 77 < j 79).
- probe: Two spellings: (a) do{ if(sum!=chk){incr;goto} }while(0); (b) do{ matched=(sum==chk); }while(0); if(!matched){incr}. sandbox --disable all each.
- result: BOTH score 12 (from floor 6). The loop-note bracket around the tail raises loop_depth for the matched outer IVs (i=a2, chkptr=t0, offset=a3) and/or materializes a boolean, cascading the outer allocation.
- verdict: KILLED

## [s8] The Kengo (PS2 successor) match supplies the original source structure for damage_DebugDisp for transplant.
- mechanism: Marionation engine structure is preserved BB2(PS1)->Kengo(PS2); the matched Kengo source reveals the original C shape.
- probe: kengo_matches.csv lookup + kengo_ref.py disassembly dump of the Kengo equivalent.
- result: Match is name-only (is_damage_calc, 0.00 confidence). Kengo's damage_DebugDisp @0x11fd88 is an unrelated font/matrix debug-display function (fnt_locate/fnt_print/conv_matrix_rotation/atan2), NOT the checksum validator. No structural transplant source.
- verdict: KILLED

## [s9] A fresh m2c-reconstructed structure (its inner-body order and/or a value-preserving accumulate rederivation) closes Region A' or Region B.
- mechanism: rederive the original C shape; a different inner-body statement order or a peeled/first-element accumulate might give sum a non-bracketed depth-weighted reference (closing A' while keeping the RA win), or reveal a Region-B structure with the consts naturally before the ap/a2p moves.
- probe: ran tools/m2c/m2c.py on asm/funcs/damage_DebugDisp.s (artifact s9/m2c_out.c); measured (a) m2c inner order `j++; sum+=*bp; bp++` on the floor-6 do-while0 chassis, (b) accumulate-peel `sum=*bp; bp++; j=1; do{...}while(j<0x24)`. sandbox --disable all each.
- result: m2c reproduces the candidate structure (outer goto == s8; Region B = plain preloop moves + identical (u32)(ptr-0x80000000)<=0x1FFFFF check — no alt structure). (a) = 8 (reorder undoes the do-while0 sum=$a0 RA win; candidate order is load-bearing). (b) = 12 (extra pre-loop insns + altered trip count diverge). A byte-neutral +1 sum ref is unavailable by rederivation; it only comes from sum's own def bracket (do-while0 -> couples A').
- verdict: KILLED. Rederive (m2c / inner-order / peel / sibling func_80037F40) yields no clean sub-6 form; reconfirms s6/s7 irreducible A'/A coupling and s1-s8 Region-B constant-holder dead-end from a fresh angle.

## [s9] A fresh m2c-reconstructed inner-body order closes Region A' while keeping the do-while(0) sum=$a0 RA win.
- mechanism: m2c reconstructs the inner accumulate as `j++; sum+=*bp; bp++` (var_a1+=1; var_a0+=*var_v1; var_v1+=1); a different body order might shift sum's def/use LUID to let sched1 emit sum=0 first (target A' order) without losing the RA win.
- probe: Applied m2c's `j++; sum+=*bp; bp++` on the floor-6 do-while0 chassis; sandbox --disable all.
- result: score 8 (from 6): the reorder shifts sum's def/use LUID and UNDOES the do-while0 sum=$a0 RA win (Region A reopens). Candidate's `sum+=*bp; bp++; j++` order is load-bearing.
- verdict: KILLED

## [s9] Peeling the first accumulate iteration gives sum a real, non-bracketed depth-1 reference (raising its weighted refs without the do-while0 def-relocation that couples A').
- mechanism: sum=*bp (first byte) at low LUID is a value-identical rederivation of the 0x24-byte checksum; a genuine surviving sum reference could raise its allocno priority above j without a loop-note bracket.
- probe: `bp=base+offset; sum=*bp; bp++; j=1; do{sum+=*bp;bp++;j++}while(j<0x24)`; sandbox --disable all.
- result: score 12 (from 6): peeling emits an extra pre-loop lbu+addu and changes j's init/trip-count, diverging from target's clean 0x24-iteration do-while with plain sum=0. A byte-neutral +1 sum ref is unavailable by peeling.
- verdict: KILLED

## [s9] A fresh m2c decompile reveals an alternative Region-B source structure that places the range constants before the ap/a2p moves without a constant-holder cheat.
- mechanism: If the original C referenced 0x80000000/0x1FFFFF pre-loop for a genuine reason, LICM would not control their placement and they would precede the plain preloop moves (target order).
- probe: Ran tools/m2c/m2c.py --target mips-gcc-c on asm/funcs/damage_DebugDisp.s (artifact s9/m2c_out.c); inspected the k-loop reconstruction.
- result: m2c reconstructs Region B as plain preloop moves (var_a0_2=arg0; var_a2_3=var_a0_2) plus the identical `(u32)(ptr-0x80000000)<=0x1FFFFF` check — the constants are referenced only inside the loop. No alternative structure exists; the target's consts-before-moves imply an original pre-loop constant reference = constant-holder cheat for us.
- verdict: KILLED

## [s10] Region B closable in pure C via index-based k-loop + do-while(0) on k (REFUTES s1-s9 constant-holder-cheat conclusion).
- mechanism: explicit ap/a2p preheader moves force LICM range-consts to hoist AFTER them (move_movables emit_insn_before(loop_start)); index form (base+k*4/base+k*2) makes ap/a2p strength-reduced givs whose inits land AFTER the consts (strength_reduce runs after move_movables) -> consts-first == target. do-while(0) on k=0 gives a dominating const-0 biv init (folds the giv inits to plain moves) AND its loop-depth ref bump wins k the $a1 tiebreak over the a2p giv (== target k=$a1/a2p=$a2).
- probe: index-based k-loop + `do{k=0}while(0)`; sandbox --disable all + objdump.
- result: score 2 (from floor 6), build 79, Region B BYTE-EXACT (lui/lui/ori; move a0,t1; move a2,a0; lw 0x78(a0); lhu 0xD0(a2)). Waypoints: two-path k=0 = correct-regs-but-bloated (6); single merged k=0 = folded-but-regswap (9).
- verdict: CONFIRMED — Region B closed in pure C, floor 6->2.

## FRONTIER RESET (s10) — Region A' is the SOLE residual (score 2). Next ladder pass:
1. **Directed permuter on the score-2 index-B chassis.** The whole-function pseudo/LUID
   numbering CHANGED vs the s4/s5 campaigns (those ran on the old explicit-ap/a2p Region-B
   chassis). Re-run a fresh full-TU directed permuter seeded from the score-2 candidate to
   sweep for the Region A' fix (sum=0 emitted first while sum still wins $a0). mechanism:
   the 2 residual insns are the inner-preheader emit order (target sum,bp,j vs build j,bp,sum);
   sched1 rank_for_schedule INSN_LUID tiebreak emits the do-while(0)-bracketed block-bottom
   sum=0 LAST. next_probe: build ws from cpp(src) with the score-2 body, target.o from
   asm/funcs, honest pipeline; --stop-on-zero; vet any closing form (the a2p/const-holder
   dead-alias family is the known Region-B trap, now moot since B is closed).
2. **Natural sum-11th-ref (the A'/A decoupler).** Target reaches sum=$a0 with sum=0 emitted
   FIRST (low LUID, no bracket) => target's sum has weighted reg_n_refs>=11 (or ties j)
   WITHOUT a do-while(0). Inner loop is byte-identical build/target, so the extra ref is NOT
   in the loop body. mechanism: reg_n_refs += loop_depth (flow.c). s7-s9 killed peel/tail-
   bracket/inner-reorder as the source. next_probe: read flow.c reg-ref counting for what
   distinguishes a depth-2 sum ref target keeps but our chassis drops; check if the index-B
   numbering shifted the sum/j tie (measured: plain sum-first still 9, so tie unchanged — but
   a permuter-found structural variant may still exist).
3. **Drop j 11->10 on the index-B chassis (closes A AND A' with NO do-while(0)).** If j's
   weighted refs drop to 10, plain low-LUID sum=0 (10) ties j and wins $a0 on pseudo -> sum=$a0
   AND sum=0 emits first. mechanism: j total 11 = j=0(d1) + j++(d2) + j<0x24(d2), all byte-fixed
   by target (sltiu ...,0x24). Prior sessions found no byte-neutral j-ref drop; re-examine only
   if the permuter surfaces a counter reformulation keeping the sltiu 0x24 byte.

## [s10-synth] Floor 2 reverified; A' localized to 2 preheader reg=0 moves; plain-in-position=4 (corrects ledger's 9).
- mechanism: per-function objdump (tmp/grind/damage_DebugDisp/s10/{plain,dw0}_ops.txt vs asm/funcs).
  PLAIN sum=0 (unbracketed, in candidate position): preheader BYTE-PERFECT (insn4 addu a0,zero,zero /
  insn5 addu v1,t1,a3 / insn6 addu a1,zero,zero == target .L8003801C), only 4 diffs = inner a0<->a1
  swap (j=$a0/sum=$a1) at insns 8/9/10/15 -> score 4. DO-WHILE0 (floor 2): inner loop byte-identical
  to target (sum=$a0/j=$a1), residual = exactly 2 diffs — the emit-POSITION swap of insn4/insn6:
  build j=0 at pos4 & sum=0 at pos6 vs target sum=0 at pos4 & j=0 at pos6. The bracket forces sum=0
  to block-bottom (highest LUID) -> sched1 rank_for_schedule INSN_LUID tiebreak emits it at pos6.
- probe: apply candidate (=2) and plain-in-position (=4); objdump each vs target; src reverted to HEAD.
- result: floor 2 reverified; plain-in-position = 4 (NOT 9 — the s10 "plain=9" was sum-first-position);
  A' = 2-insn preheader reg=0 emit-order swap; do-while0 trade = -4 inner swap +2 preheader = 2.
- verdict: CONFIRMED. A' is purely the schedule slot of sum=0's def; both residual insns are `addu $reg,zero,zero`.

## FRONTIER RESET (s10 synthesis pass) — SUPERSEDES the s10 reset above. Region A' is the SOLE residual (score 2).
Sharpened picture: target = sum=$a0 with sum=0 emitted FIRST (pos4, unbracketed) => target's sum has
weighted reg_n_refs>=11 (tie j=11) from a source that does NOT relocate sum=0's def. On our chassis we
can get EITHER a byte-perfect preheader (plain, but sum=$a1 swap) OR sum=$a0 (do-while0, but sum=0 at
pos6). The whole game is: give sum its 11th weighted ref WITHOUT bracketing sum=0's def, OR drop j to 10.

1. **Directed permuter on the score-2 index-B chassis (PRIMARY).** The Region-B index rewrite renumbered
   the whole-function pseudos/LUIDs vs the STALE s4/s5 campaigns (old explicit-ap/a2p chassis). This is
   the single highest-value unexplored lead and is a permuter-modality job. mechanism: sum needs a
   non-coalescing depth>=2 ref target keeps but our chassis drops; the changed tail topology (index givs
   vs walking pointers) may have shifted the sum/j numbering enough that a structural mutation surfaces
   the natural 11th ref. next_probe: build ws from cpp(src) with the committed score-2 body, target.o
   from asm/funcs (drop `.set gp=64`), honest pipeline, --stop-on-zero, ~20-30k fresh-seed iters; vet
   every sub-2 find against dead-store/constant-holder (the Region-B alias trap is now MOOT, B is closed).
2. **Drop j 11->10 (closes A AND A' in ONE move -> score 0, no do-while0).** NEW crisp framing: plain's
   preheader is ALREADY byte-perfect, so if j drops to 10, plain sum(10) ties j(10), sum wins $a0 by
   lower pseudo -> sum=$a0 with sum=0 at pos4 = target EXACTLY. mechanism: j's depth-2 refs (j++, j<0x24)
   are byte-fixed (target sltiu ...,0x24 / addiu a1,a1,1); the only movable weight is j=0's depth-1 def,
   which must reset per outer iteration (can't hoist to depth 0). next_probe: on the index-B numbering,
   dump .lreg reg_n_refs for j; seek a counter reformulation keeping the three j bytes but shedding one
   flow-counted ref (permuter-surfaced or a loop-shape where one j use lands shallower). Highest payoff.
3. **Natural sum-11th-ref decoupler (frontier #1 refined).** Target's sum reaches 11 with sum=0 at low
   LUID. The inner body, sum=0 def, and sum==chk compare are ALL byte-identical to ours, so the extra ref
   is a loop_depth effect GCC keeps in flow-counting but drops in codegen. mechanism: reg_n_refs +=
   loop_depth (flow.c); the +1 must come from a genuine depth>=2 sum reference in a block free of the
   outer IVs (s7-s9 killed peel/reorder/tail-bracket as the source). next_probe: on the index-B chassis,
   test whether the changed tail loop_depth topology admits a sum reference one level deeper without a
   bracket; a SECOND live sum pseudo used post-loop that GCC will NOT coalesce (s2's acc-split coalesced —
   seek a non-coalescing variant). Vet against dead-store.

## [s10] The committed floor-2 index-B candidate (do-while(0) on sum=0 + index-based Region-B k-loop + do-while(0) on k) still measures honest distance 2 on this chassis, with Region B byte-exact and judge-PASSed.
- mechanism: Applied candidate.c to src/code6cac_c_mid.c; sandbox --disable all builds the honest object with regfix/asmfix disabled and cheat-asm stripped. Region A solved by do-while(0) sum=0 (reg_n_refs loop-depth bump ties j, allocno-number tiebreak gives sum $a0); Region B closed by index reformulation (strength_reduce giv-inits land after LICM range constants) — judge ruling docs/grind/decisions.md 2026-07-22 06:02 PASS.
- probe: cp candidate body into src; & tools/wteng.ps1 main sandbox damage_DebugDisp --disable all
- result: score 2, target_insns 79, build_insns 79, 8 rules dropped, 34 cheat-asm stripped. src reverted to clean HEAD after measurement.
- verdict: CONFIRMED

## [s10] Plain sum=0 (unbracketed, in candidate position) on the index-B chassis scores 4 (NOT 9 as the s10 note implied), and the sole floor-2 residual (Region A') is exactly the emit-position swap of the two preheader `addu $reg,zero,zero` moves (sum=0 vs j=0).
- mechanism: Per-function objdump of both builds vs asm/funcs/damage_DebugDisp.s. PLAIN: preheader byte-perfect (insn4 addu a0,zero,zero / insn5 addu v1,t1,a3 / insn6 addu a1,zero,zero == target .L8003801C); only 4 diffs = inner-loop a0<->a1 swap (j=$a0/sum=$a1) at insns 8/9/10/15 -> score 4. DO-WHILE0: inner loop byte-identical to target (sum=$a0/j=$a1), residual = 2 diffs: build emits j=0 at pos4 & sum=0 at pos6 vs target sum=0 at pos4 & j=0 at pos6. The do-while(0) bracket forces sum=0 to the mini-loop block-bottom (highest preheader LUID); sched1 rank_for_schedule INSN_LUID tiebreak emits the block-bottom leaf last. Trade: do-while0 = -4 inner swap +2 preheader order = score 2.
- probe: Edit do{sum=0}while(0) -> plain sum=0; sandbox --disable all (=4); extract_ops.py per-function objdump of plain vs do-while0 vs target.
- result: plain-in-position = 4; A' = 2-insn preheader reg=0 emit-order swap; the s10 'plain=9' referred to moving sum=0 to first position, a different perturbation. Both residual insns are addu $reg,zero,zero (byte 0x2120/0x2128 0000).
- verdict: CONFIRMED
