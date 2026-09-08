# Evidence bank — func_8002E6B0

## s1 (2026-09-08, recon) — chassis: -mel -msoft-float, INCLUDE_ASM on main, no FAKE constructs anywhere

- [s1] BASELINE: banked body G (`retired-chassis-2026-08/body.c`, cross_point-before-cross_center, plain `return 0` exits) installed on the current chassis measures sandbox 64 / 94 insns (== target insn count). canonical = C, hand_coded_tier LOW (S4 only). No DATA MODEL signals: the function touches no globals (four pointer params only), so there is no declaration-fix hypothesis for this function.
- [s1] OBJECT MODEL: n/a — func_8002E6B0 references zero globals (pure leaf over `s32 *arg0..arg3`); nothing to flag, nothing to measure.
- [s1] TARGET EXIT STRUCTURE (read from asm/funcs/func_8002E6B0.s): the first `bltz` carries `addu $v0,$zero,$zero` in its delay slot, BOTH early-exit branches jump straight to the epilogue label .L8002E814, and the third block's `srl $v0` falls through into the epilogue with no `j` and no separate `move v0,zero` block. Body G instead emits `j .Lend` + a shared `.Lret0: move v0,zero` block (jump.c cross-jump of the two `return 0`s) and reorg fills the first bltz from the FALLTHROUGH thread (`subu a1,t5,t1`). reorg.c `mostly_true_jump` predicts a `bltz` vs 0 not-taken, so the fallthrough thread is tried first and wins whenever its first insn is eligible — the target's slot content therefore cannot have come from stealing the `.Lret0` block; `v0 = 0` must have been a real insn BEFORE the branch in the target's pre-reorg stream, i.e. a result variable initialised to 0 on the main path with the exits jumping to a shared end label.
- [s1] MEASURED `ret` FORMS (all with `goto end` exits + `end: return ret;`): G-order (cross_point first) + `ret = 0` before the first test = 72/96; G-order + `s32 ret = 0;` initialiser = 72/96 (textual placement of `ret = 0` is irrelevant: sched1 moves it — it has no deps and priority 1); HEAD-order (cross_center first) + ret-before-test = **60/97**; HEAD-order + initialiser = 60/97. The 60 form is banked as candidate.c (v3). It reproduces the target's block structure (delay-slot `move ?,zero`, both bltz → epilogue, no `j`), the target's multiply ORDER in all three blocks (center-x product, point-x product, point-z product, center-z product), and the target's `mfhi t7; sra v0; subu s?,t7,v0` centroid shape; the residual is a register-seat cascade: `ret` lands in $a2 (so `xor/nor/srl` go to a2 and a `move v0,a2` remains), an extra $s6 save appears, and the caller-saved temps are permuted (dz/dx in a1/a0 vs target's v0/v0, first product in s0 vs t9).
- [s1] ROOT CAUSE OF THE SEAT CASCADE (from tmp/grind/func_8002E6B0/dumps + the sched_solver model of v3, tmp/grind/func_8002E6B0/s1/sched1_picks.txt): pseudo 98 (`ret`) conflicts with hard $v0 in .greg because sched1 placed `ret = 0` (insn 98, pri 1) after the last mult but while the dx*(pz-z0) product (local pseudo 112, allocated $v0) was still live. In the target, $v0 hosts x-sum → dz → dx → ret SEQUENTIALLY because `center_x`'s `sra/subu` is scheduled BEFORE `dz` and dz/dx are emitted immediately before their first mult. In our sched1 (backward list scheduling) `dz` (insn 54) and `dx` (insn 61) sit in the ready list at priority 2 while everything around them carries LAUNCH_PRIORITY 0x7f000001, so they are picked last in the backward pass = emitted FIRST in program order, far from their mults. sched.c:2543 `adjust_priority` grants LAUNCH_PRIORITY only when `birthing_insn_p` (sched.c:2505) is true, and that requires `reg_n_sets[dest] == 1` (sched.c:2526). `dz`, `dx`, `cross_center`, `cross_point` and `ret` are all function-scope variables assigned in every block (3 sets each), so NONE of their defs can ever be boosted. This is the mechanism behind the brief's "one open item" (center_x computed immediately after the two mults in the target): it is not a scheduling-priority tie, it is the single-set predicate.

- [s1] SINGLE-SET dz/dx CONFIRMED BY MEASUREMENT: block-scoped `{ s32 dz = ...; s32 dx = ...; cross_center = ...; cross_point = ...; }` per block with cross_center/cross_point still FUNCTION-scope and `ret`/goto-end exits = **45/93** (v7, was 60). The same with cross_center/cross_point ALSO block-scoped = 60/96 (v5); fully inlined expressions (no dz/dx/cross names, CSE unifies) = 60/96 (v6). So dz/dx must be single-set (per-block) and cross_center/cross_point must stay multi-set function-scope pseudos: in the target both are consistently cc->$a0 / cp->$v1 in all three blocks, the signature of ONE global pseudo each allocated by global.c, not of per-block locals (a per-block cc in block 3 would take $v0, which is free there, via local-alloc's ascending scan).
- [s1] EXIT SPELLING ON THE v7 CHASSIS: replacing `ret`+`goto end` by plain `return 0;` exits and `return (cc ^ cp) >= 0;` = **40/93** (v12, banked as candidate.c; floor 64 -> 40 this session). With v12, block 1's ORDER matches the target line-for-line from the first load through the first bltz (tmp/grind/func_8002E6B0/s1/sbs.py side-by-side): the residual is (a) register seats: arg2[2] in $a0 vs target $t5, mfhi in $t5 vs $t7, point-x product in $t0 vs $a2, (pz-z0) in $t5 vs $t0, (cz-z0) in $a2 vs $a1, 3rd product $a1 vs $a0, 4th product $v1 vs $s5 (target saves $s5 = 6 callee-saves, ours 5), cross_center $v1 vs $a0, cross_point $v0 vs $v1; and (b) the exit structure: ours `j .Lend` + `.Lret0: move v0,zero` block + fallthrough-stolen delay slots, target `addu v0,zero,zero` in slot 1 and both bltz straight to the epilogue.
- [s1] THE && SPELLINGS ARE DEAD: `ret = t1 >= 0 && t2 >= 0 && t3 >= 0` (v8) = 64/94 and `return t1 >= 0 && ...` (v9) = 64/94. Both expand via expr.c TRUTH_ANDIF_EXPR (`emit_clr_insn` into a PSEUDO; v9's slot shows `move t6,zero`, so the hard $v0 was NOT used as the target) and end with a branch + `li 1` for the third term; the target's third term is a do_store_flag `xor/nor/srl` VALUE, and jump.c's store-flag conversion (jump.c:1023-1190) is disabled on this port (`BRANCH_COST >= 2 || HAVE_conditional_move` both false), so no && form can produce the target's tail. stmt.c:2725 `HAVE_return` return-as-jumps path is also inert at expand time (mips_can_use_return_insn returns 0 before reload).
- [s1] WHY $v0 HOLDS NOTHING ELSE IN THE TARGET'S BLOCK 2: target block 2 uses a0/a3/v1/a1/a2 only, no $v0, and block 3 uses $v0 freely as a temp until the final xor; cross_point/cross_center are $v1/$a0 in every block. In OUR compiles (v3/v7) the `ret` pseudo conflicts with hard $v0 (greg `96 conflicts: ... 2 3`) because local-alloc hands block-2 temps $v0 first (ascending scan; global pseudos are invisible to local-alloc), and global.c then cannot give `ret` $v0 (and cannot allocate it before cross_point: allocno priority floor_log2(refs)*refs*size/livelen is ~4242 for ret vs 20000 for cross_point). A `ret` pseudo therefore cannot reach $v0 by allocation order on this chassis; the only mechanism that keeps $v0 out of block 2's local allocation is hard $v0 being live there, and the C front end only writes DECL_RESULT (hard $v0, REG_FUNCTION_VALUE_P, function.c expand_function_start) through `return`. Every `return X` emits `v0 = X; jump Lend` (stmt.c expand_return -> expand_value_return), so an early hard `v0 = 0` on the main path has NO ordinary-C spelling: the delay-slot `addu v0,zero,zero` must instead come from reorg stealing it (next entry). This kills the entire "result variable initialised to 0" family (v1-v4, v7) as the explanation of the target's exits even though v7 measured 45.
- [s1] REORG PATH TO THE TARGET'S EXIT STRUCTURE (read from tools/gcc-2.7.2/reorg.c, NOT yet measured): for a `bltz` vs 0, `mostly_true_jump` predicts not-taken, so `fill_eager_delay_slots` tries the FALLTHROUGH thread first and steals its first insn unless that insn sets a register in `opposite_needed` = `mark_target_live_regs(branch target)`. `mark_target_live_regs` (reorg.c ~2500-2730) falls back to ALL REGISTERS LIVE when `find_basic_block(target)` returns -1, i.e. when the target label is not a `basic_block_head` known to the last flow analysis. If the shared `move v0,zero` block's label is such a label, the fallthrough insn (`subu a0,t5,t1`, sets a0) is REJECTED, the target thread is tried, `move v0,zero` is stolen into slot 1 (v0 is dead on the fallthrough) and the branch redirected past it; for the second bltz `redundant_insn` finds the slot-1 `move v0,zero` on every path, skips it (new_thread = epilogue), the branch is redirected to the epilogue label (a flow-known block: v0/s-regs live, a0 free) and `subu a0,t5,t3` is stolen from the fallthrough; the orphaned `move v0,zero` block is deleted and the `j .Lend` before it becomes jump-to-next and is deleted. That reproduces the target's lines 48-49, 68-69 and 84-90 EXACTLY. The instrumented cc1 (tools/gcc-2.7.2/cc1) already carries a what-if hook for precisely this path: `BB2_ALLLIVE_LABEL=<label uid>` forces the everything-live result for one target (reorg.c bb2_alllive_match; `BB2_DBR_DEBUG=1` prints `DBRDBG mtlr target=N block=B`). v12's dumps show the two `v0 = 0` exit blocks (insns 106, 170, both `jump 240`) survive jump1/flow and are cross-jumped only in jump2 (post-reload: label 111 and insn 106 gone in .jump2), yet our reorg still stole the fallthrough insn; so which label reorg resolved, and whether it was flow-known, is the FIRST thing to measure next session (DBRDBG on v12), before any further spelling.
- [s1] sched_solver goal mapper cannot be used yet on this function: with ~37 register renames + 20 moves between our stream and the target, `perturb.py --goal-from-target` (pinned target: tmp/grind/func_8002E6B0/s1/tgt_pinned.s, honest-asm format) reports "goal is not a topological order" for both blocks in both passes. Re-run it only after the seat cascade is closed. `inverse_compose.py classify` (object path) reports PRE-RA solely because of the consequences of the seat cascade (`move v0,aN` on our side, the $s5 save/restore on the target's); the multisets otherwise match.
- [s1] ARTIFACTS: tmp/grind/func_8002E6B0/s1/ holds every variant (v1-v12 .c + .o + disassembly .txt), measure.ps1 (batch sandbox harness: `pwsh tmp/grind/func_8002E6B0/s1/measure.ps1 -V "a.c,b.c"`), install.py, sbs.py (side-by-side vs target), sched1_picks.txt (v3 sched1 pick trace), classify.log, sched_loop*.log, tgt_pinned.s. Dumps for v12 are in tmp/grind/func_8002E6B0/dumps/.

- [s1] Baseline: banked body G measures 64/94 on the current -mel -msoft-float chassis; canonical = C, hand_coded_tier LOW (S4 only); function touches no globals.

- [s1] Target exit structure: `addu v0,zero,zero` in the first bltz's delay slot, both bltz jump straight to the epilogue, third block's `srl v0` falls through with no `j` and no separate `move v0,zero` block.

- [s1] v12 (candidate.c) = 40/93: cross_center-first statement order, block-scoped single-set dz/dx, function-scope cross_center/cross_point, plain `return 0` exits. Block 1 order matches target line-for-line; residual is a register-seat permutation (arg2[2] $a0 vs $t5, cc $v1 vs $a0, cp $v0 vs $v1, 4th product $v1 vs $s5 so target saves s0-s5, ours s0-s4) plus the exit structure.

- [s1] The C front end writes hard $v0 (DECL_RESULT) only through `return` (stmt.c expand_return -> expand_value_return emits `v0 = X; jump Lend`), so no ordinary C places `v0 = 0` before the first test; the target's delay-slot zero must be reorg stealing it from the shared return-0 block.

- [s1] reorg.c mark_target_live_regs assumes ALL registers live when find_basic_block(target) == -1 (label unknown to flow); that rejects the fallthrough insn (sets a0), steals `move v0,zero` from the target thread, and via redundant_insn redirects the second bltz to the epilogue, deleting the orphan block and the `j` - reproducing the target's exits exactly. The instrumented cc1 has BB2_ALLLIVE_LABEL / BB2_DBR_DEBUG hooks for this path; v12's two return-0 blocks (insns 106/170) survive jump1/flow and are cross-jumped only in jump2, yet our reorg still used precise liveness - which label it resolved is the next measurement.

- [s1] sched_solver goal mapper is unusable until the seat cascade closes (37 renames + 20 moves -> non-topological goal); inverse_compose classify says PRE-RA only because of the cascade's consequences (move v0,aN vs $s5 save/restore).

- [s1] Rejected forms banked: memory/grind/func_8002E6B0/rejected/s1_*.c (7 files, scores in names).

## s2 (2026-09-08, structural) — chassis: -mel -msoft-float, INCLUDE_ASM on main, candidate.c (v12) installed, no FAKE constructs

- [s2] CHASSIS RE-MEASURE: candidate.c (v12) still measures **40 / build_insns 93** on HEAD. The s1 floor is intact; nothing in the chassis moved between s1 and s2.
- [s2] **METHOD CORRECTION (load-bearing for every future side-by-side):** `objdump -d` COLLAPSES runs of identical zero words into a `...` elision line, so plain `-d` silently HIDES the `nop` (0x00000000) padding maspsx inserts after `mflo`/`mfhi` before a following `mult`. s1's side-by-side (and my first read of it) therefore showed our stream as 93 insns "missing four nops" vs the target's 98 — a disassembly artifact, not a real gap. With `objdump -dz` our v12 body is **97 insns vs the target's 98**, and every `mflo,nop,nop,mult` group is present on BOTH sides. Always use `-dz` (tmp/grind/func_8002E6B0/s2/dis2.py + sbs.py consume it). A maspsx/assembler-fidelity hypothesis built on the elided listing (`multu_pad_funcs.txt` etc.) would have been chasing nothing: maspsx emits the pads (verified directly, tmp/grind/func_8002E6B0/s2/maspsx_out.s) and the assembler keeps them (verified with -O0 and -O1, tmp/grind/func_8002E6B0/s2/probe*.full.txt).
- [s2] TRUE RESIDUAL (v12 vs target, `-dz` side-by-side tmp/grind/func_8002E6B0/s2/v12z.txt): instruction ORDER is identical from the prologue through the first `bltz` (target lines 4-48 == our lines 3-47, offset 1 because the target saves a SIXTH callee-save `s5`) and again for the whole of blocks 2 and 3. The ENTIRE residual is (a) a register permutation — target `lw t5,8(a2)` / ours `lw a0,8(a2)`; target `mfhi t7,a1` / ours `mfhi t5,a2`; target's block-1 4th product `mflo s5` / ours `mflo v1` (so the target needs `sw s5`/`lw s5` and we do not); target cross_center->$a0, cross_point->$v1 / ours cross_center->$v1, cross_point->$v0 — and (b) the exit structure (target: `move v0,zero` in the first bltz's delay slot, both bltz straight to the epilogue, no `j`; ours: `j .Lend` + a shared `move v0,zero` block, fallthrough-stolen slots).
- [s2] **EXIT-SHAPE SPELLINGS ARE NORMALIZED TO ONE OBJECT.** Four structurally distinct C exit shapes on the v12 chassis — plain `if (...) return 0;` twice (v12), nested `if (t1>=0) { blk2; if (t2>=0) { blk3; return ...; } } return 0;` (v20), `goto ret0;` twice + a trailing `ret0: return 0;` label (v21), and `if (...) return 0; else { blk }` arms (v25) — all produce **md5-IDENTICAL `code6cac_b.o`** (45e95a1329ccc6c9cceb08218344b024), score 40. jump.c's jump-threading/cross-jumping collapses them before RA, so the exit STRUCTURE cannot be steered by choosing among these shapes. Frontier-1's suggested "nested ifs / reordered return-0" probes are therefore spent.
- [s2] **REORG ALL-LIVE MECHANISM CONFIRMED BY DIRECT INSTRUMENTATION.** Built a single-function TU (tmp/grind/func_8002E6B0/s2/solo.c = `typedef int s32;` + candidate.c) and ran the instrumented cc1 (tools/gcc-2.7.2/cc1) with `BB2_DBR_DEBUG=1` (tmp/grind/func_8002E6B0/s2/dbr2.sh). Baseline prints exactly ONE `DBRDBG mtlr target=170 block=4`: for our compile flow KNOWS the first bltz's target label (block 4, precise liveness), which is why `fill_eager_delay_slots` takes the fallthrough insn. Re-running with `BB2_ALLLIVE_LABEL=170` (dbr3.sh) forces the everything-live result for that one target and the first bltz's delay slot becomes **`move $2,$0`** with the branch redirected past it — byte-for-byte the target's line 48-49 pair. So s1's reorg reading is CORRECT and measured, not hypothesised.
- [s2] …but forcing ONE label is not the whole target shape: with only label 170 forced, the SECOND bltz (a different target, `DBRDBG mtlr target=183 block=0`) still gets a `nop` slot and the trailing `j .Lend` + `move v0,zero` block survives, so the forced body is 99 insns (worse than our 97). The target needs BOTH bltz targets to be flow-unknown (then `redundant_insn` sees the slot-1 `move v0,zero` on every path, redirects bltz #2 to the epilogue and the orphan block + `j` are deleted). Artifacts: dbr/solo.s, dbr/solo_all.s, dbr/solo_err.txt, dbr/solo_all_err.txt, s2/solo_all.txt.
- [s2] The C question is now sharp and narrow: **our shared return-0 label is a label flow already knew (block=4); the target's must be one flow never saw.** jump2's cross-jump of the two `return 0` blocks REUSED an existing (flow-known) label in every exit spelling measured above — that is exactly why all four spellings produce the same object. What is needed is a body in which the block that survives cross-jumping is reached through a label CREATED after the last `find_basic_blocks`.
- [s2] STRUCTURAL LEVERS MEASURED (all on the v12 chassis, sandbox score / build_insns): xor operand order `(cross_point ^ cross_center)` = 40 (DIFFERENT object bytes, same score — the only structural edit that moved bytes without moving the score); `cross_point` declared before `cross_center` = 40 (byte-identical); `center_z` computed before `center_x` = 40 (byte-identical); arg3 coords hoisted to named function-scope locals = 40 (byte-identical); arg0+arg3 coords hoisted = 40 (byte-identical); named `sum_x`/`sum_z` before the /3 = 40; per-block named difference temps for BOTH the center and point deltas = 40; for the center deltas only = 40; for the POINT deltas only = **44 (worse)**; function-scope but single-set distinct `dz1..dz3`/`dx1..dx3` = 40; assigning `cross_point` before `cross_center` inside each block = **57 (worse)**. CSE/jump normalisation absorbs all of the neutral ones.
- [s2] ARTIFACTS: tmp/grind/func_8002E6B0/s2/ (v12z.txt + sbs, v20/v21/v25/v30-v35/v40-v42/v50-v52 .c+.o, dis2.py for `-dz` listings, maspsx_probe.sh/maspsx_out.s, as_probe*.sh/probe*.full.txt, dbr*.sh + dbr/ with solo/solo_all .s/.o/stderr, measure.ps1).

- [s2] candidate.c (v12) still measures 40 / 93 on HEAD; the s1 floor is intact and the chassis did not move between s1 and s2.

- [s2] METHOD CORRECTION: `objdump -d` collapses runs of identical zero words into a `...` elision line, hiding the maspsx-inserted `nop,nop` between an mflo/mfhi and a following mult. s1's side-by-side therefore looked like our body was four nops short of the target. With `objdump -dz` our v12 body is 97 insns vs the target's 98, and every mflo,nop,nop,mult group is present on BOTH sides. Always use -dz (tmp/grind/func_8002E6B0/s2/dis2.py).

- [s2] Directly disproved the maspsx/assembler-fidelity theory the elided listing suggested: maspsx DOES emit the pads (tmp/grind/func_8002E6B0/s2/maspsx_out.s, produced by running maspsx.py with the exact buildconfig flags) and the assembler keeps them at both -O0 and -O1 (probe0.full.txt / probe.full.txt). No multu_pad_funcs.txt or other gate-list hypothesis exists for this function.

- [s2] TRUE RESIDUAL: instruction ORDER matches the target exactly from the prologue through the first bltz (target lines 4-48 == ours 3-47, offset 1 because the target saves a SIXTH callee-save s5) and again through blocks 2 and 3. What is left is (a) a register permutation - target `lw t5,8(a2)` vs our `lw a0,8(a2)`; target `mfhi t7` / `mfhi a1` vs our `mfhi t5` / `mfhi a2`; target's block-1 4th product `mflo s5` vs our `mflo v1` (hence the target's extra sw s5 / lw s5); cross_center $a0 vs our $v1; cross_point $v1 vs our $v0 - and (b) the exit structure.

- [s2] The C question is now sharp: our shared return-0 label is one flow already knew (block=4); the target's must be a label flow never saw. jump2's cross-jump reused an existing flow-known label in EVERY exit spelling measured, which is exactly why all four spellings produce one object.

- [s2] The instrumented-cc1 loop (tmp/grind/func_8002E6B0/s2/dbr2.sh, single-function TU, seconds per body, no sandbox round trip) is a cheap oracle for that question: grep `DBRDBG mtlr target=<uid> block=` and look for block=-1 before spending a sandbox measurement.

## s3 (2026-09-08, structural) — chassis: -mel -msoft-float, INCLUDE_ASM on main, no FAKE constructs

- [s3] CHASSIS RE-MEASURE: v12 (`candidate.c`) = **40 / 93 insns** on HEAD this session. Floor confirmed unchanged.
- [s3] **THE SCORE IS MISLEADING ON THIS FUNCTION.** The ret-var body banked by s1 as
  `rejected/s1_blockscoped_dzdx_ret_var_goto_45.c` (score 45, re-measured 45/93 this session) is the ONLY
  banked body whose EXIT STRUCTURE matches the target line-for-line: its `bltz #1` carries the `ret = 0`
  insn in the delay slot (target: `addu $v0,$zero,$zero`), BOTH `bltz` branch straight to the epilogue,
  block 3 falls through into the epilogue with no `j`, and there is NO orphan `move v0,zero` block. v12
  (score 40) has the WRONG exits: `j .L295` + an orphan `.L294: move v0,zero`. The 5-point score
  difference is register-seat penalty, not structure. Body kept at
  `memory/grind/func_8002E6B0/chassis_retvar_exit_correct_45.c`.
- [s3] The ONLY residual on that ret-var chassis is the seat: our `ret` pseudo is allocated `$a1`, so a
  trailing `move v0,a1` survives (1 extra insn) and the whole downstream cascade shifts. Target's `ret` is
  `$v0`.
- [s3] MEASURED WHY: from the `.greg` dump of the real TU (`tmp/grind/func_8002E6B0/s3/real.i.greg`,
  `;; Function func_8002E6B0`): the ret pseudo's conflict set CONTAINS hard reg 2 (`$v0`), so `find_reg`
  can never give it `$v0`. `$v0` is occupied by LOCAL-allocated block temps — the .greg RTL shows
  `(reg:SI 2 v0)` as the `(a[0]+b[0]+c[0])` accumulator, its `sra`, and as `(reg/v:SI 2 v0)` for the
  block-scoped `dz`/`dx` in every block. MIPS defines no `REG_ALLOC_ORDER`, so both `local_alloc` and
  `global.c find_reg` scan hard regs in plain ascending order and `$v0` (reg 2) is always taken first;
  `local_alloc` runs BEFORE `global_alloc` and cannot see the function-scope `ret` pseudo.
- [s3] TARGET CONTRAST (asm/funcs/func_8002E6B0.s): the target's block 2 (0x8002E774-0x8002E7BC) uses
  `$a0/$v1/$a3/$a1/$a2` and NEVER `$v0`, while its block 1 uses `$v0` freely up to `mult $v0,$a1`
  (0x8002E758) and its block 3 reuses `$v0`. So in the ORIGINAL compile `$v0` was live-and-occupied across
  block 2 AT LOCAL-ALLOC TIME — i.e. the `v0 = 0` was already a HARD `$v0` set sitting before block 1's
  branch when local-alloc ran. That same occupancy is what raises block-1 pressure enough to force the
  target's SIXTH callee-save (`$s5` for block 1's 4th product, `mflo $s5`); our bodies need only five.
- [s3] Both the target and every body we build carry a PHANTOM `$s4` (saved + restored, never referenced),
  so `$s4` is NOT a differentiator; the differentiator is the target's extra REAL callee-save `$s5`.
- [s3] cc1 leaves BOTH `bltz` delay slots UNFILLED in this function (verified on the real TU and on a solo
  TU: `bltz $2,.L294` with no `.set noreorder`); maspsx/as fills them from the fall-through insn. That is
  why `relax_delay_slots`' redundant-target redirect never fires here — reorg.c:3950 gates that whole
  block on `GET_CODE (PATTERN (insn)) == SEQUENCE` ("Now look only at cases where we have filled a delay
  slot"), and an unfilled branch is not a SEQUENCE.

- [s3] Chassis re-measured this session: v12 (candidate.c) = 40 / 93 insns. Floor unchanged at 40.

- [s3] THE SCORE IS MISLEADING ON THIS FUNCTION: the ret-var body (s1's rejected/s1_blockscoped_dzdx_ret_var_goto_45.c, re-measured 45/93) is the only banked body whose EXIT STRUCTURE matches the target line-for-line - the `ret = 0` insn sits in bltz#1's delay slot (target: addu $v0,$zero,$zero), BOTH bltz branch straight to the epilogue, block 3 falls through into the epilogue with no `j`, and there is no orphan `move v0,zero` block. v12 (score 40) has the wrong exits (j .L295 plus an orphan .L294). Kept as memory/grind/func_8002E6B0/chassis_retvar_exit_correct_45.c.

- [s3] The only residual on that chassis is one register seat: our ret pseudo is allocated $a1, so a trailing `move v0,a1` survives (one extra insn) and the downstream cascade shifts; the target's ret is $v0.

- [s3] Measured cause (real-TU .greg dump, ';; Function func_8002E6B0'): the ret pseudo's conflict set contains hard reg 2, so find_reg can never hand it $v0. The .greg RTL shows (reg:SI 2 v0) as the (arg0[0]+arg1[0]+arg2[0]) accumulator and its sra, and (reg/v:SI 2 v0) as the block-scoped dz/dx in every block - all local-allocated.

- [s3] TARGET CONTRAST: the target's block 2 (0x8002E774-0x8002E7BC) uses $a0/$v1/$a3/$a1/$a2 and never $v0, while block 1 uses $v0 up to `mult $v0,$a1` (0x8002E758) and block 3 reuses it. So in the original compile $v0 was occupied ACROSS block 2 at local-alloc time - i.e. the v0=0 was already a HARD $v0 set sitting before block 1's branch. That same occupancy is what forces the target's sixth callee-save ($s5 holding block 1's fourth product, `mflo $s5`); our bodies need only five.

- [s3] Both the target and every body we build carry a PHANTOM $s4 (saved and restored, never referenced), so $s4 is not a differentiator - the target's extra REAL callee-save $s5 is.

- [s3] cc1 leaves BOTH bltz delay slots UNFILLED in this function (no .set noreorder around either branch; verified on the real TU and on a solo TU); maspsx/as fills them from the fall-through insn afterwards.

## s4 (2026-09-08, permuter) — chassis: -mel -msoft-float, INCLUDE_ASM on main, no FAKE constructs in the baselines

- [s4] CHASSIS RE-MEASURE (both banked bodies, on HEAD this session): v12 (`candidate.c` as inherited) = **40 / 93**; the s3 exit-exact ret-var body (`chassis_retvar_exit_correct_45.c`) = **45 / 93**. Both intact, chassis unmoved. KILL RE-AUDIT of the closest instance kill (s3's "no ret-var spelling moves ret off $a1") was performed on the second of those two bodies; `fake_ablate.py` is **not applicable** to either — neither body contains a single FAKE construct (they are plain locals, plain arithmetic, plain returns), so there is no carrier occupying the contested pseudo and the s3 kill stands as measured.
- [s4] **FLOOR 40 -> 34.** The winning body is `memory/grind/func_8002E6B0/candidate.c` (= s4 workspace `q345.c`, found by decomp-permuter as `tmp/grind/func_8002E6B0/s4/perm_b/output-345-1`). Its ONLY delta from v12 is one statement at the top of block 2:
      cross_point = dx;
      cross_center = (dz * (center_x - arg0[0])) - (cross_point * (center_z - arg0[2]));
  i.e. the pre-existing function-scope local `cross_point` is borrowed to carry block 2's `dx` for the cross_center product and is overwritten with its own real value two statements later. sandbox = **34 / 95 insns**.
- [s4] **THE BORROW IS LOAD-BEARING AND POSITION-SPECIFIC** (ablation sweep, all sandbox --disable all): a FRESH named intermediate in the identical position (`s32 dxa = dx;` … `- (dxa * …)`) = **40** (CSE folds it straight back), the same borrow in block 1 only = **40**, in all three blocks = **61**, and borrowing `cross_center` to stage `dz` instead = **53**. Only the block-2 borrow of `cross_point` reaches 34. Two spellings TIE at 34 and are therefore free variants: xor operands swapped throughout (`s4_q345_xor_swap_ties_34.c`) and mixed exit forms, first exit `goto end` + trailing `end: return 0;` (`s4_q345_mixed_exit_ties_34.c`).
- [s4] **WHAT THE 34 BODY BUYS, READ FROM THE -dz SIDE-BY-SIDE** (`tmp/grind/func_8002E6B0/s4/q345.ins.txt` vs `asm/funcs/func_8002E6B0.s`): the target's **SIXTH callee-save is now present** — our prologue emits `sw s5,20(sp)` and block 1's fourth product lands in `mflo s5`, both exactly as the target does. Lines 1-17, 20, 22-23, 26-34, 37, 39 and 45 are identical. s3's frontier-3 ("the 6th callee-save is a consequence, not a target") is therefore CONFIRMED: it fell out of raising block-1/2 pressure, with no spelling aimed at it.
- [s4] **THE RESIDUAL AT 34 IS NOW TWO SEPARABLE HALVES, AND EACH IS INDIVIDUALLY REACHABLE.** (a) The exit structure: the 34 body still emits the wrong tail — target line 49 is `addu v0,zero,zero` in the first bltz's delay slot with both bltz going straight to the epilogue, ours still has `j L` + an orphan `move v0,zero`. (b) A handful of block-1 seats (`lw t5,8(a2)` vs our `lw a0,8(a2)`, `mfhi t7/a1` vs our `mfhi t5/a2`) which then permute blocks 2 and 3. Meanwhile the ret-var + borrow body (`s4_retvar_plus_borrow_exit_exact_loses_s5_45.c`, sandbox 45) has the EXACT target exit structure (`bltz` + zero-move in the slot, both to the epilogue, no `j`) but **loses the s5 save** (5 callee-saves) and puts ret in `$a1`. So the two halves are currently anti-correlated: the borrow buys the 6th callee-save, the ret-var buys the exit shape, and no body measured this session has both.
- [s4] Adding the borrow to block 1 as well ON the ret-var chassis (an attempt to re-raise pressure and recover s5 while keeping the exit shape) = **45**, unchanged, and still 5 callee-saves (`s4_retvar_borrow_blocks1and2_45.c`).
- [s4] ORDINARY-C SPELLINGS MEASURED AND DEAD this session (all on the v12/ret-var chassis, no FAKE constructs): `return !((cc ^ cp) < 0)` = 40; `return (cc ^ cp) > -1` (GT rather than GE) = 40; every vertex coordinate hoisted into a named local = 44; the ret-var defaulted to 1 and zeroed on the two failing paths = 41; swapping cross_point/cross_center assignment order in block 2 ONLY (s2 had only measured all-three-blocks, which was 57) = 46.
- [s4] CAMPAIGN TELEMETRY (three campaigns, all harvested and stopped in-session, none orphaned): `perm_a` (ret-var exit-exact chassis, base permuter score 630) 26,666 iterations / ~36 min, best 265 but every find below 355 is SEMANTICALLY BROKEN (it assigns `ret = dz` in block 2, so the early exit would return dz instead of 0 — the permuter does not check semantics); best sandbox-valid find 39. `perm_b` (v12 chassis, base 620) 26,669 iterations / ~36 min — this is the campaign that produced the 34 body. `perm_c` (RESEEDED on the 34 body itself, base 345) 24,869 iterations / ~23 min, **zero finds below its base**; its one same-score find is only a respelling of the same borrow (`dx = center_z - arg0[2];` then `cross_point * dx`). The 34 basin is therefore permuter-saturated on this chassis — a fourth campaign on it is not a valid probe.
- [s4] WORKSPACE RECIPE for future permuter sessions on this file: `tmp/grind/func_8002E6B0/s4/mkws.sh <dir>` builds a validated workspace (full cpp + cc1 with the CURRENT `-mel -msoft-float` flags + prologue_fix + maspsx + multu_pad, then extracts only this function's region). It is the func_800283D0 s4 recipe with `-msoft-float` added — the older `tools/mar_perm_workspace.sh` and the func_800283D0 copy both PREDATE the 2026-09-07 soft-float adoption and would compile a different chassis.
- [s4] ARTIFACTS: `tmp/grind/func_8002E6B0/s4/` — mkws.sh, measure.ps1, install.py, runwait.sh, perm_a/ perm_b/ perm_c/ (base.c, compile.sh, target.o, campaign.log, campaign_meta.json, output-*/), q345.c + q345.o + q345.dz.txt + q345.ins.txt (the -dz side-by-side input), w6.dz.txt/w6.ins.txt, and every measured variant .c/.o (v91-v96, p345/p350/p355, q345/q385/q445, r1/r4/r5/r6, w1/w2/w5/w6, x1/x2).

- [s4] CHASSIS RE-MEASURE: v12 (the inherited candidate.c) = 40 / 93 insns and the s3 exit-exact ret-var body = 45 / 93 on HEAD this session; the chassis has not moved since s3.

- [s4] NEW FLOOR 34 / 95 insns. The body is memory/grind/func_8002E6B0/candidate.c; its only delta from v12 is `cross_point = dx;` at the top of block 2 followed by `cross_center = (dz * (center_x - arg0[0])) - (cross_point * (center_z - arg0[2]));` - the pre-existing function-scope local cross_point borrowed to carry dx, then overwritten with its own real value two statements later.

- [s4] The borrow is a FAKE construct in the variable-reuse family (.claude/rules/defeat-licm-hoist-var-reuse.md, gated by .claude/rules/staged-value-reused-variable.md - an EXISTING local borrowed, not an invented one). It is unannotated in candidate.c because this session submits nothing; any candidate-ready must add the /* FAKE: what + mechanism + lever-exhaustion */ line and quote that rule's scope sentence verbatim. The best FAKE-FREE form is still v12 at 40, banked as rejected/s4_v12_plain_no_varreuse_40.c.

- [s4] The borrow is load-bearing and position-specific: a fresh named intermediate in the identical position measures 40 (CSE folds it back), block-1-only 40, all-three-blocks 61, borrowing cross_center for dz 53. Two spellings tie at 34 (xor operand swap; mixed exit forms).

- [s4] The 34 body finally reproduces the target's SIXTH callee-save - our prologue emits sw s5,20(sp) and block 1's fourth product lands in mflo s5, exactly as the target does. -dz side-by-side lines 1-17, 20, 22-23, 26-34, 37, 39 and 45 are identical.

- [s4] The residual at 34 splits into two halves that are each individually reached but not yet held together: (a) the exit structure (target has addu v0,zero,zero in the first bltz's delay slot and both bltz to the epilogue; ours still has j + an orphan move v0,zero), and (b) a few block-1 seats (target lw t5,8(a2) / mfhi t7 / mfhi a1 vs our lw a0,8(a2) / mfhi t5 / mfhi a2) that then permute blocks 2 and 3.

- [s4] The ret-var + borrow body (rejected/s4_retvar_plus_borrow_exit_exact_loses_s5_45.c, sandbox 45) has the EXACT target exit structure but only five callee-saves and ret in $a1; adding a second borrow in block 1 (45) does not recover s5. The ret pseudo consumes the pressure the borrow creates.

- [s4] Ordinary-C spellings measured dead this session: return !((cc^cp) < 0) = 40, return (cc^cp) > -1 = 40, all eight vertex coordinates hoisted into named locals = 44, ret-var defaulting to 1 = 41, block-2-only cross_point/cross_center assignment swap = 46.

- [s4] CAMPAIGN TELEMETRY, all three harvested and stopped in-session (none orphaned; permuter_campaign.py status reports alive:false for all three): perm_a (ret-var chassis, base 630) 26,666 iterations / ~36 min - best permuter score 265 but every find below 355 is SEMANTICALLY BROKEN (it assigns ret = dz in block 2, so the early exit would return dz instead of 0; the permuter does not check semantics), best sandbox-valid find 39. perm_b (v12 chassis, base 620) 26,669 iterations / ~36 min - this is the campaign that produced the 34 body. perm_c (reseeded on the 34 body, base 345) 24,869 iterations / ~23 min - ZERO finds below base.

- [s4] REUSABLE RECIPE: tmp/grind/func_8002E6B0/s4/mkws.sh builds a validated permuter workspace for this file (full cpp + cc1 with the CURRENT -mel -msoft-float flags + prologue_fix + maspsx + multu_pad, then per-function extraction). It is the func_800283D0 s4 recipe with -msoft-float ADDED - that copy and tools/mar_perm_workspace.sh both predate the 2026-09-07 soft-float adoption and would compile a different chassis.

## s5 (2026-09-08, permuter) - chassis: -mel -msoft-float, INCLUDE_ASM on main

- [s5] CHASSIS RE-MEASURE at session start: the inherited `candidate.c` (s4's cross_point borrow) reproduces at **34 / 95 insns** on HEAD. Chassis unmoved since s4.
- [s5] **FLOOR 34 -> 26 / 96 insns.** New `candidate.c` = s5 workspace `e7_ret_stage_first_product_min.c`. It is the s4 ret-var/goto-end exit chassis (`s32 ret = 0;`, both early exits `goto end;`, tail `ret = (cc ^ cp) >= 0; end: return ret;`) with ONE borrow in block 2: block 2's `dz` staged through the return carrier itself, `cross_center = ((ret = dz) * (center_x - arg0[0])) - (dx * (center_z - arg0[2]));`, followed by `ret = 0;` at the end of block 2 to restore the carrier's real value before the second early exit.
- [s5] **THE s4 ANTI-CORRELATION IS BROKEN.** s4 measured the exit shape (ret-var chassis, 45, five callee-saves) and the sixth callee-save (cross_point borrow, 34, wrong exits) as mutually exclusive. Staging dz through the ret pseudo ITSELF supplies the pressure inside the ret-var chassis, so the 26 body holds BOTH: the -dz side-by-side vs `perm_d/target.o` shows `move v0,zero` in the FIRST bltz's delay slot, BOTH bltz branching straight to the epilogue, no `j`, no orphan zero block, AND `sw s5,20(sp)` + `mflo s5`.
- [s5] **THE 26 RESIDUAL IS TWO INSNS PLUS A CONSISTENT REGISTER PERMUTATION** (98 vs 96 insns in the objdump normalisation): (i) our ret pseudo lands in `$v1`, not `$v0`, costing a trailing `move v0,v1`; (ii) the `ret = 0;` restore costs a `move v1,zero` sitting in the SECOND bltz's delay slot - the target needs no restore because its `$v0` simply is never clobbered across block 2. Every other difference is a rename inside {v0,v1,a0,a1,a2} (`mfhi a2` vs `mfhi a1`, `mflo a0` vs `mflo a2`, `mflo a1` vs `mflo a0`, and their consumers). Artifact: `tmp/grind/func_8002E6B0/s5/cmp2.sh <stem>` prints this side-by-side for any measured `.o`.
- [s5] **PROVENANCE: the permuter proposed a SEMANTICALLY BROKEN form and the repair was the win.** `perm_d` (seeded on `rejected/s4_retvar_plus_borrow_exit_exact_loses_s5_45.c`, base 630, the untried structurally-distinct chassis named by the s4 frontier) produced `output-265-1` = the dz staging with NO restore, so the second early exit would return `dz` instead of 0. That raw body measures **25 / 95 insns** and is banked as `rejected/s5_permuter_find_SEMANTICALLY_BROKEN_returns_dz_25.c` - it is NOT a legal form. Adding the `ret = 0;` restore makes it correct at 26. This is the second time on this function that the permuter's best find was semantically invalid (s4's perm_a did the same); ALWAYS read the find's semantics before scoring it.
- [s5] THE RESTORE'S POSITION IS LOAD-BEARING. `ret = 0;` at the end of block 2 = 26; the same restore written just before the second `if` = 26 (identical object); moved INSIDE the second exit's branch = 45; dropped entirely with the second exit spelled `return 0;` inline = 45; dropped with a separate `zero:` label = 45. Removing the restore removes the pressure and the sixth callee-save with it.
- [s5] THE TOP INITIALISATION `s32 ret = 0;` IS ALSO LOAD-BEARING. Declaring `s32 ret;` uninitialised and assigning `ret = 0;` at the first exit instead = **30 / 95 insns**, and that body has NO `sw s5` and the wrong first exit (`bgez` + `j` + `move v1,zero`). Banked as `rejected/s5_stagedz_uninit_ret_loses_s5_30.c`.
- [s5] TIES AT 26 (free variants for a later session to permute from): the same body plus s4's `cross_point = dx;` borrow and the ret staging in BOTH block-2 products (`d2`); the ret staging in both products with no cross_point borrow (`e6`); `ret` declared before `cross_center`/`cross_point` (`e1`); and block 2's `center_z - arg0[2]` ADDITIONALLY staged through `dx`, either recomputing the dx expression at its later use (`i1`) or restoring `dx` (`i2`, banked as `rejected/s5_dx_staged_restored_ties_26.c`) - the `dx` staging came from `perm_e/output-350-1`, also semantically broken as found (it clobbered `dx` before its later use).
- [s5] MEASURED DEAD THIS SESSION (all sandbox --disable all, on the stated chassis): reusing an existing local as the RETURN carrier instead of declaring `ret` - `cross_center` = 34 (ties, same object as the 34 candidate), `cross_point` = 37, `center_x` = 50, `center_z` = 50, `cross_center` with mixed exits = 35; uninitialised `ret` assigned at both exits on the 34 chassis = 34 (jump.c folds it back to the plain-return object), at one exit only = 35, and the same without the borrow = 41; on the 45 ret-var+borrow chassis, adding the borrow to block 3 = 45, block 3 only = 45, and a second block-2 borrow staging `dz` through `cross_center` = 45 (all 93 insns, all still five callee-saves - the ret-var chassis is INSENSITIVE to borrows of the OTHER locals); a live READ of `ret` in block 2 (`... + ret`) instead of the staging = 58; the final path spelled as an inline `return (cc ^ cp) >= 0;` beside `end: return ret;` = 54.
- [s5] CAMPAIGN TELEMETRY (both harvested and stopped in-session; `status` reports alive:false for both): `perm_d` (label retvar_plus_borrow_45, base 630) ~5 min / ~4.5k iterations before it was stopped in favour of the repaired chassis - it had already produced the 265 find that became the floor drop. `perm_e` (label d2_stagedz_26, seeded on the repaired 26 body, base 365) 11,782 iterations / 444 s, 9 novel finds, best 265 - and every find below base is the SAME semantically-broken family (a dead `ret = 0;` moved above the staging, or `dx` clobbered before its later use). Repairing the two distinct broken shapes by hand gives 26 again, i.e. the 26 basin has not yielded a legal improvement.
- [s5] ARTIFACTS: `tmp/grind/func_8002E6B0/s5/` - mkws.sh / install.py / measure.ps1 / runwait.sh (copied from s4), cmp.sh + cmp2.sh (the target side-by-side helpers; cmp2.sh compares against the assembled `perm_d/target.o`, which is the correct normalisation - cmp.sh's text form has formatting noise), perm_d/ and perm_e/ (base.c, compile.sh, target.o, campaign.log, campaign_meta.json, output-*/), and every measured variant .c/.o (a1-a5, b1-b4, c1-c3, d0-d5, e1-e7, f1-f2, g1-g2, h1-h2, i1-i2).

- [s5] Chassis re-measure at session start: the inherited candidate.c (s4's cross_point borrow) reproduces at 34 / 95 insns on HEAD; chassis unmoved since s4.

- [s5] NEW FLOOR 26 / 96 insns. candidate.c is now the s4 ret-var/goto-end exit chassis (s32 ret = 0; both early exits `goto end;`, tail `ret = (cc ^ cp) >= 0; end: return ret;`) with ONE borrow: block 2's dz staged through the return carrier itself, `cross_center = ((ret = dz) * (center_x - arg0[0])) - (dx * (center_z - arg0[2]));`, followed by `ret = 0;` at the end of block 2 so the second early exit still returns 0.

- [s5] The 26 body reproduces the target's exit structure exactly (move v0,zero in the FIRST bltz's delay slot, both bltz straight to the epilogue, no j, no orphan zero block) AND the target's sixth callee-save (sw s5,20(sp) / mflo s5). s4 had these two halves in separate bodies and measured them as anti-correlated.

- [s5] The 26 residual is two insns plus a consistent register permutation (98 vs 96 insns in the objdump normalisation): our ret pseudo lands in $v1 rather than $v0, costing a trailing `move v0,v1`, and the `ret = 0;` restore costs a `move v1,zero` in the second bltz's delay slot (the target needs no restore because its $v0 is never clobbered across block 2). Everything else is a rename inside {v0,v1,a0,a1,a2}.

- [s5] PROVENANCE WARNING, now twice-observed on this function: the permuter's best find was SEMANTICALLY BROKEN both times. perm_d's output-265-1 (score 25 in the sandbox) omits the restore, so the second early exit would return dz instead of 0; it is banked as rejected/s5_permuter_find_SEMANTICALLY_BROKEN_returns_dz_25.c and is not a legal form. The entire s5 floor drop came from REPAIRING that find rather than discarding it.

- [s5] The restore's position is load-bearing: at the end of block 2 = 26, hoisted just before the second if = 26 (identical object), moved inside the second exit's branch = 45, dropped with the second exit spelled `return 0;` inline = 45, dropped with a separate zero: label = 45.

- [s5] The top initialisation is load-bearing: `s32 ret;` uninitialised with `ret = 0;` at the first exit = 30 / 95 insns, with no sw s5 and the wrong first exit.

- [s5] Ties at 26 (untried permuter seeds): d2 (this body plus s4's cross_point = dx borrow and the ret staging in both block-2 products), e6 (staging in both products, no cross_point borrow), e1 (ret declared first), i1/i2 (block 2's center_z - arg0[2] additionally staged through dx, recomputed or restored).

- [s5] Also measured dead: carrier-instead-of-ret variants (cross_point 37, center_x 50, center_z 50, cross_center 34 = identical object, mixed-exit 35); uninitialised ret assigned at both exits on the 34 chassis = 34, at one exit only = 35, without the borrow = 41; borrows of other locals on the ret-var chassis (block 3 45, blocks 2+3 45, cross_center staging dz 45, all 93 insns and five callee-saves); a live read of ret in block 2 instead of the staging = 58; the final path spelled as an inline return beside end: return ret = 54.

- [s5] Campaign telemetry, both harvested and stopped in-session (permuter_campaign.py status reports zero alive): perm_d (retvar_plus_borrow_45, base 630) ~4.5k iterations before being stopped in favour of the repaired chassis; perm_e (d2_stagedz_26, base 365) 11,782 iterations / 444 s / 9 novel finds, best 265, all semantically broken.

- [s5] The 26 body's construct is a variable-reuse borrow of an EXISTING local (the real return carrier) staging a real, immediately-consumed value and then written back with its own real value - a FAKE construct under .claude/rules/defeat-licm-hoist-var-reuse.md gated by .claude/rules/staged-value-reused-variable.md. It is unannotated in candidate.c because this session submits nothing; the best FAKE-free form on this chassis is still v12 at 40.

- [s5] New reusable tool: tmp/grind/func_8002E6B0/s5/cmp2.sh <stem> prints the normalised objdump side-by-side of any measured .o against the assembled target (perm_d/target.o). The older text-based cmp.sh has formatting noise and should not be used for counting.

## s6 (2026-09-08, synthesis) - chassis: -mel -msoft-float, INCLUDE_ASM on main

- [s6] CHASSIS RE-MEASURE: the inherited 26 body reproduces at **26 / 96 insns** on HEAD. Chassis unmoved since s5. Floor stays 26; no drop this session.
- [s6] **INHERITANCE DEFECT FIXED.** The s5 `candidate.c` header comment contained a literal close-comment sequence inside its prose (it described the FAKE annotation template), which terminated the block comment early; installing candidate.c verbatim into `src/code6cac_b.c` therefore FAILED TO COMPILE and the sandbox reported `func_8002E6B0 not found in tmp/sandbox/func_8002E6B0/code6cac_b.o` rather than a score. Any future session that sees that error message should check candidate.c's header first. s6's candidate.c is rewritten with no nested comment delimiters and re-measured at 26.
- [s6] **THE $v1-vs-$v0 SEAT IS A CONFLICT PROBLEM, NOT A PRIORITY PROBLEM - and this supersedes s1's allocno-priority story.** Ground truth from the `.greg` conflict dump of the 26 body itself (`tmp/grind/func_8002E6B0/s6/greg_seg.txt`, produced by `pwsh tools/grinder/dump.ps1 func_8002E6B0` with the 26 body installed):
  * the ret variable is pseudo **reg 96**, and it is **allocno ORDER 0** (`;; 31 regs to allocate: 96 95 94 ...`, ALLOCDBG ord=0 pri=17368 nrefs=11 livelen=19). It is allocated FIRST. Priority is not the obstacle, so no lever that raises ret's refs or live span can help.
  * reg 96's conflict line is `;; 96 conflicts: ... 2 29 64 66` - it carries a **hard-register conflict with $v0 (reg 2)**, and it has **no `;; 96 preferences:` line at all**.
  * by contrast the two xor operands reg 94 (6 uses / 8 insns) and reg 95 (6 uses / 7 insns) carry NO conflict with reg 2 and DO carry `preferences: 2 3 4`; reg 95 is allocno ord 1 and duly takes $v0, reg 94 takes $a0.
  * the missing preference is a CONSEQUENCE of the conflict: `prune_preferences` masks every preference set by `hard_reg_conflicts[allocno]` (global.c:895-909, the three `AND_COMPL_HARD_REG_SET` lines at 907-909), so the copy preference that insn 236 `(set (reg/i:SI 2 v0) (reg/v:SI 96))` records via `set_preference` (global.c:1484 -> 1717-1730) is deleted before find_reg ever runs.
  * the dump is a valid pre-allocation input, not post-hoc fallout: `dump_conflicts` is called at global.c:580, after `prune_preferences` (global.c:576) and BEFORE the allocation loop at global.c:583.
- [s6] **WHERE THE $v0 CONFLICT COMES FROM IS STILL OPEN, AND IT IS THE ONE QUESTION WORTH INSTRUMENTING.** In the `.lreg` RTL for this body (`tmp/grind/func_8002E6B0/s6/lreg_seg.txt`) hard $v0 appears in exactly two insns - `(insn 236 (set (reg/i:SI 2 v0) (reg/v:SI 96)))` carrying `REG_DEAD (reg 96)`, and `(insn 237 (use (reg/i:SI 2 v0)))`. No insn clobbers $v0 (the only clobbers in the function are hi/lo/accum from the mults). `global_conflicts` processes REG_DEAD notes at global.c:767-770 BEFORE `note_stores (PATTERN (insn), mark_reg_store)` at global.c:777, so insn 236 alone should NOT create a 96/$v0 conflict. Yet most long-lived pseudos in this function (76, 77, 78, 80, 85, 86, 87, 89, 91, 104, 105, 107, 109, 111, 112, 113, 115, 116, 126) ALSO conflict with reg 2 even though they are long dead by insn 236 - and they conflict with reg 29 ($sp, a FIXED register that `mark_reg_store` explicitly skips at global.c:1502). That pattern points at the block-entry path (`hard_regs_live` seeded from `basic_block_live_at_start` at global.c:695-698, then `record_conflicts` at global.c:725), i.e. flow believes hard $v0 is LIVE AT THE START of the blocks these pseudos span. Confirming that, and finding what makes flow think so, is the next instrumented probe - not another spelling search.
- [s6] **THE SOLVER ROUTE (s5 frontier item 1) IS DEAD ON THIS FUNCTION AS THE MODEL STANDS.** `tools/ra_solver/extract.py func_8002E6B0 code6cac_b` builds the model fine (31 allocnos, 53 dispositions), but `simulate.py` reproduces only **11/31** dispositions and misses at ORD 0: pseudo 96 sim=$a1 vs dump=$v1, pseudo 95 sim=$v1 vs dump=$v0, pseudo 94 sim=$a0 vs dump=$v1. `inverse.py global --goal` on the ret seat therefore searches from a WRONG baseline (it prints `96: '$a1', 95: '$v0'`) and its NEGATIVE RESULT ("no perturbation of any modelled input up to depth 2 reaches the target") is NOT a valid FORECLOSED verdict for this seat - it is a statement about a baseline we do not have. Do not cite it as a foreclosure. Log: `tmp/grind/func_8002E6B0/s6/inverse_global.log`.
- [s6] **FAKE-ABLATION RE-AUDIT of the closest kills** (`tools/fake_ablate.py --func func_8002E6B0 --file code6cac_b --candidate tmp/grind/func_8002E6B0/s6/base26_annotated.c`, an annotated copy of the 26 body - candidate.c itself is unannotated because no session has submitted): keep-all 26/96; drop the restore only 25/95 (this is the KNOWN semantically-broken form that returns dz from the second exit - `rejected/s5_permuter_find_SEMANTICALLY_BROKEN_returns_dz_25.c`, NOT a legal win, and the tool cannot know that); drop the staging line 55/86 in both remaining variants, because the tool's "unit" for a comment-block marker deletes the WHOLE `cross_center = ...` statement rather than just the borrow, which changes the program. **fake_ablate's ablation units are not meaningful on this body** - its only reportable finding is the already-banked broken 25. Full sweep: `tmp/grind/func_8002E6B0/ablate/`.
- [s6] **NEW STAGING POSITIONS MEASURED** (frontier item 2 - "a block-2 value other than dz staged through ret"), all sandbox --disable all on the 26 chassis with the single ret-staging borrow present:
  * `p1` staging moved to block 2's SECOND product (`cross_point = ((ret = dz) * (arg3[0] - arg0[0])) - ...`) = **26 / 96, byte-identical object** to the base (md5 921b8948...).
  * `p2` staging carrying a DIFFERENT value - `cross_center = (dz * (ret = center_x - arg0[0])) - ...`, i.e. the target's $t7 sub-expression rather than dz = **26 / 96, byte-identical object**.
  * `q3` both of those at once (two staged values, one restore) = **26 / 96, byte-identical object**.
  * `p3` staging `dx` instead of `dz` = **42 / 94**. Banked `rejected/s6_stage_dx_not_dz_42.c`.
  * `p4` staging in BLOCK 3 only, where no restore is needed at all because `ret` is overwritten by the final `ret = (cc ^ cp) >= 0;` = **48 / 95**. This is the cleanest possible "staging without a restore insn" and it is 22 worse. Banked `rejected/s6_stage_block3_only_no_restore_48.c`.
  * `p5` staging in block 2 (with restore) AND block 3 = **34 / 94**. Banked `rejected/s6_stage_b2_and_b3_34.c`.
  So the restore insn is not avoidable by moving the staged value or the staging site: every staging position that needs no restore also loses the pressure, exactly as s5 measured for the restore's POSITION.
- [s6] **TWO NEW DISTINCT-OBJECT TIES AT 26** (s5's ties d2/e6/e1/i1/i2 plus s6's p1/p2/q3 are all the SAME object, so they are not new permuter basins; these two are different objects):
  * `q1` = the FINAL statement's xor operands swapped only (`ret = (cross_point ^ cross_center) >= 0;`) = 26 / 96, md5 078e7ad4. Banked `rejected/s6_final_xor_swap_distinct_object_ties_26.c`.
  * `q2` = the FIRST if's xor operands swapped only = 26 / 96, md5 1b1fa1c0. Banked `rejected/s6_first_if_xor_swap_distinct_object_ties_26.c`.
  Both show the IDENTICAL residual against the target (ret in $v1, the `move v1,zero` restore in the second bltz slot, the trailing `move v0,v1`), so the seat is invariant to xor operand order.
- [s6] ARTIFACTS: `tmp/grind/func_8002E6B0/s6/` - base26.c (the compilable extraction of candidate.c), base26_annotated.c, p1/p2/p3/p4/p5 and q1/q2/q3 .c and .o, measure.ps1, install.py, cmp2.sh (fixed to point at the s5 `perm_d/target.o`), greg_seg.txt (the pre-allocation conflict/preference dump for the 26 body), lreg_seg.txt (the pre-global RTL), inverse_global.log; plus `tmp/grind/func_8002E6B0/dumps/` (all cc1 pass dumps for this body) and `tmp/grind/func_8002E6B0/ablate/`.

- [s6] CHASSIS RE-MEASURE: the inherited 26 body reproduces at 26 / 96 insns on HEAD; the chassis has not moved since s5 and the floor is unchanged this session.

- [s6] INHERITANCE DEFECT FIXED: the s5 candidate.c header comment contained a literal close-comment sequence inside its prose (it quoted the FAKE annotation template), which terminated the block comment early, so installing candidate.c verbatim into src/code6cac_b.c FAILED TO COMPILE and the sandbox reported 'func_8002E6B0 not found in tmp/sandbox/func_8002E6B0/code6cac_b.o' instead of a score. s6's candidate.c is rewritten with no nested comment delimiters and re-measured at 26. A future session that sees that sandbox error should check candidate.c's header first.

- [s6] THE $v1-vs-$v0 SEAT IS A CONFLICT PROBLEM, NOT A PRIORITY PROBLEM. From the pre-allocation .greg dump of the 26 body (tmp/grind/func_8002E6B0/s6/greg_seg.txt): ret is pseudo reg 96 and it is allocno ORDER 0 (pri 17368, nrefs 11, livelen 19), i.e. allocated FIRST; its conflict line is ';; 96 conflicts: ... 2 29 64 66' (a hard conflict with $v0) and it has NO preferences line; reg 94 and reg 95 (the xor operands, 8 and 7 insns of live length) have no conflict with reg 2 and both list 'preferences: 2 3 4'; reg 95 takes $v0 at ord 1. This supersedes s1's allocno-priority explanation of the ret seat.

- [s6] The missing $v0 preference on reg 96 is a consequence of the conflict, not an independent fact: prune_preferences masks hard_reg_preferences / hard_reg_copy_preferences / hard_reg_full_preferences by hard_reg_conflicts at global.c:907-909, deleting the copy preference that set_preference (global.c:1484) records for the epilogue copy (set (reg/i:SI 2 v0) (reg/v:SI 96)). dump_conflicts is called at global.c:580 - after prune_preferences (global.c:576) and before the allocation loop (global.c:583) - so the dumped graph is a genuine allocation INPUT.

- [s6] WHERE THE $v0 CONFLICT COMES FROM IS THE OPEN QUESTION. In the pre-global RTL (tmp/grind/func_8002E6B0/s6/lreg_seg.txt) hard $v0 appears in exactly two insns, insn 236 (set (reg/i:SI 2 v0) (reg/v:SI 96)) carrying REG_DEAD on reg 96, and insn 237 (use (reg/i:SI 2 v0)); nothing clobbers $v0 (the only clobbers in the function are hi/lo/accum from the mults). global_conflicts processes REG_DEAD notes at global.c:767-770 BEFORE note_stores(mark_reg_store) at global.c:777, so insn 236 by itself should not create the conflict. But nineteen other long-lived pseudos (76 77 78 80 85 86 87 89 91 104 105 107 109 111 112 113 115 116 126) also conflict with reg 2 although they are long dead by insn 236, and they conflict with reg 29 ($sp) which is FIXED and explicitly skipped by mark_reg_store at global.c:1502. That points at the block-entry path: hard_regs_live seeded from basic_block_live_at_start at global.c:695-698 followed by record_conflicts at global.c:725.

- [s6] THE SOLVER SUITE IS CURRENTLY UNUSABLE ON THIS FUNCTION: simulate.py reproduces 11/31 dispositions and misses at ord 0/1/2 (96 sim=$a1 dump=$v1, 95 sim=$v1 dump=$v0, 94 sim=$a0 dump=$v1), so inverse.py global searches from a wrong baseline and its printed NEGATIVE RESULT is not a foreclosure of the ret seat.

- [s6] FAKE-ABLATION RE-AUDIT (tools/fake_ablate.py on an annotated copy of the 26 body): keep-all 26/96; drop-the-restore-only 25/95, which is the already-banked SEMANTICALLY BROKEN form that returns dz from the second exit (rejected/s5_permuter_find_SEMANTICALLY_BROKEN_returns_dz_25.c) and is not a legal win; both variants that drop the staging line measure 55/86 because the tool's ablation unit deletes the whole cross_center assignment rather than just the borrow. fake_ablate's units are not meaningful on this body.

- [s6] NEW STAGING POSITIONS: p1 (block 2's second product) = 26/96 and p2 (staging center_x - arg0[0] instead of dz) = 26/96 and q3 (both) = 26/96, all three BYTE-IDENTICAL to the base object; p3 (staging dx) = 42/94; p4 (block 3 only, no restore needed) = 48/95; p5 (blocks 2 and 3) = 34/94.

- [s6] TWO NEW DISTINCT-OBJECT TIES AT 26 - q1 (final statement's xor operands swapped, md5 078e7ad4) and q2 (first if's xor operands swapped, md5 1b1fa1c0). Every s5 tie (d2/e6/e1/i1/i2) and every s6 respelling (p1/p2/q3) compiles to the SAME object as the base, so q1 and q2 are the only genuinely new permuter neighbourhoods at the floor.

## s7 (2026-09-08, solver) - chassis: -mel -msoft-float, INCLUDE_ASM on main

- [s7] CHASSIS RE-MEASURE: the inherited 26 body reproduces at 26 / 96 insns on HEAD (target 94). Floor unchanged; no drop this session. src/code6cac_b.c was reverted to INCLUDE_ASM at the end of the session.

- [s7] OBJECT-LEVEL CLASSIFY (inverse_compose.py classify with --target-object tmp/grind/func_8002E6B0/s5/perm_d/target.o --ours-object tmp/sandbox/func_8002E6B0/code6cac_b.o): the ENTIRE insn-multiset residual is "ours only: move #,#" twice - exactly the two known moves (the trailing `move v0,v1` seat copy and the `move v1,zero` restore). The tool prints FIRST DIVERGENCE: PRE-RA, but that verdict is a false attribution for the seat copy: an RA-generated register-to-register copy IS an RA product, and the multiset heuristic cannot see that. Treat a PRE-RA verdict from this classifier as unreliable whenever the only shape difference is `move`. (The stale memory/grind/func_8002E6B0/classify.log is from an older 93-vs-94-insn chassis and must not be quoted.)

- [s7] **GROUND TRUTH FOR THE RET SEAT, STRAIGHT FROM THE INSTRUMENTED cc1** (BB2_FINDREG_DEBUG on tools/gcc-2.7.2/cc1; full capture in tmp/grind/func_8002E6B0/s7/findregdbg.txt). For the 26 body, pseudo 96 (ret), allocno order 0:

      conflicts:        2 29
      someone_prefers:  4
      used_so_far:      0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 24 25 26 27 28 29 31
      pass0_used:       0 1 2 4 16 17 18 19 20 21 22 23 26 27 28 29 30 31
      pass1_used:       0 1 2 26 27 28 29 31
      own_copy_prefs:   (empty)      own_full_prefs: (empty)

  The pass-0 free set is therefore {3, 5, 6, 7, 8..15, 24, 25} and find_reg's ascending scan takes 3 = $v1. **Removing hard reg 2 from reg 96's conflict set is both NECESSARY and SUFFICIENT**: reg 2 is in used_so_far, is NOT in someone_prefers (which is only {4}), and is not fixed - so with the conflict gone it becomes the first candidate in BOTH pass 0 and pass 1, reg 96 seats in $v0, and the trailing `move v0,v1` collapses. No other allocation input needs to change. This is sharper than s6, which established only that the conflict caused the $v0 copy preference to be pruned.

- [s7] **s6's FRONTIER HYPOTHESIS 1 IS FALSIFIED: the 96/$v0 conflict does NOT come from the basic-block-entry path.** Two independent strands of direct evidence:
  * The `.lreg` flow dump prints the live-at-start set of every basic block of this body (tmp/grind/func_8002E6B0/s7/lreg_blocks.txt): bb0 = {4 5 6 7 29 30}; bb1 = {29 30 76 77 78 80 85 86 87 89 104 107 109 111 113 115}; bb2 = {29 30 76 78 80 85 87 89 109 113}; bb3 = {29 30 96}. **Hard reg 2 appears in NO block's live-at-start set**, so the `hard_regs_live` seed at global.c:695-698 followed by record_conflicts at global.c:725 cannot be the source. (That path IS the source of the ubiquitous reg-29 conflicts. Reg 30 is live at every block start yet appears in no conflict line because eliminable registers are stripped from every conflict and preference set at global.c:542-548, just before expand_preferences.)
  * The epilogue copy `(insn 236 (set (reg/i:SI 2 v0) (reg/v:SI 96)))` is not the source either: `mark_reg_clobber` returns immediately unless its setter is a CLOBBER (global.c:1528-1529), so the pre-REG_DEAD `note_stores` pass at global.c:764 is a no-op for a SET; and the REG_DEAD note for reg 96 is processed at global.c:769-770 BEFORE `note_stores (PATTERN (insn), mark_reg_store)` at global.c:777.

- [s7] **THE ACTUAL SOURCE OF THE 96/$v0 CONFLICT: local_alloc's blanket scratch register.** `dump_flow_info` lists local_alloc's assignments in the `.lreg` dump as `;; Register <pseudo> in <hardreg>.`. For this body (tmp/grind/func_8002E6B0/s7/local_alloc_map.txt) local_alloc placed **TWELVE pseudos in $v0 (79, 81, 84, 97, 100, 117, 121, 138, 146, 149, 153, 157) and only FOUR in $v1 (88, 90, 93, 142)**; the remainder went to $a0 (two), $a1, $a2, $a3, $t0 (one each). A locally-allocated pseudo is folded into its hard register by `reg_renumber` inside `mark_reg_store` (global.c:1490-1491) and then sets that bit in `hard_regs_live` (global.c:1509); every time reg 96 is (re)born, `record_one_conflict` performs `IOR_HARD_REG_SET (hard_reg_conflicts[ialloc], hard_regs_live)` at global.c:1392. Because $v0 is local_alloc's blanket scratch here and `ret` has FOUR births in this body (`ret = 0`, the `ret = dz` staging, the `ret = 0` restore, and the final `ret = (cc ^ cp) >= 0`), at least one birth necessarily lands inside a live $v0 quantity and the conflict is recorded. Reg 96 carries no conflict with reg 3, consistent with $v1 holding only four short local quantities.
  In our object the two branch conditions are computed into $v0 (`xor v0,a0,v0` / `bltz v0`) and ret lives in $v1; **in the target the roles are exactly mirrored** - both conditions are computed into $v1 (`xor v1,a0,v1` / `bltz v1`) and ret lives in $v0 (`move v0,zero` in the first bltz's delay slot, `srl v0,v0,0x1f` at the tail, and NO restore at all: the target's second bltz delay slot carries real block-3 work, `subu a0,t5,t3`). The heads of the two objects are otherwise the same instructions in the same order. **So the ret seat is a LOCAL-ALLOC question, not a global-alloc priority or preference question**: the C lever has to change which hard register local_alloc hands to the short-lived branch-condition and product temporaries in blocks 1-3, or move ret's births out of the live windows of the $v0 quantities.

- [s7] **ra_solver FORWARD-MODEL DEFECT FOUND AND TYPED (repo-wide, not specific to this function).** `tools/ra_solver/simulate.py:159-170` builds `regs_someone_prefers` from the model's `conflicts` map, which `extract.py` reads out of the `.greg` `;; N conflicts:` listing. But `dump_conflicts` prints the SYMMETRIC closure - `if (CONFLICTP (i, j) || CONFLICTP (j, i))` at global.c:1807 - whereas the real `prune_preferences` tests only the ONE-WAY bit, `if (CONFLICTP (allocno, allocno_order[j]))` at global.c:920. The matrix is genuinely asymmetric: `record_one_conflict` ORs the currently-live set only into the ROW of the allocno being born (global.c:1391-1404), so row I holds the allocnos that were already live when I was born. Consequence here: the model computes `someone_prefers[96] = {2,3,4}` (it believes 96 conflicts with 95 and 94, whose full_prefs are {2,3,4}), while the real value from FINDREGDBG is `{4}`. That single error excludes reg 3 from pass 0 and makes simulate.py pick $a1 for the ord-0 allocno instead of $v1; the wrong ord-0 seat then cascades through every later allocno, which accounts for the whole 11/31 reproduction rate s6 recorded. Cross-checked on the other two top allocnos: under the one-way rule someone_prefers[95] = full_prefs[94]{2,3,4} minus full_prefs[95]{2,3,4} = {} and someone_prefers[94] = full_prefs[89]{4} minus full_prefs[94]{2,3,4} = {}, both matching the FINDREGDBG capture exactly. A real fix needs a DIRECTIONAL conflict source, which the `.greg` listing cannot supply - extract.py would need either a birth-order proxy or a new cc1 hook dumping the raw matrix. tools/ is outside a grind session's allowed surface, so this is recorded for the operator rather than patched here.

- [s7] MEASUREMENTS ON THE 26 CHASSIS (all `sandbox --disable all`; the single ret-staging borrow is present unless stated):
  * `r1` both branch conditions carried in the EXISTING local `cross_center` (`cross_center ^= cross_point; if (cross_center < 0)`) = **32 / 96**, distinct object md5 1a75cc02. Banked `rejected/s7_cond_carrier_cross_center_32.c`.
  * `r2` the same with `cross_point` as the carrier = **26 / 96**, DISTINCT object md5 3bae0255 - a genuinely new tie basin (base26 is 921b8948). The seat does not move: `bltz v0`, `move v1,zero`, and the trailing `move v0,v1` are all still present. Banked `rejected/s7_cond_carrier_cross_point_distinct_object_ties_26.c`.
  * `r3` the `ret = 0` restore moved UP, between block 2's `cross_center` and `cross_point` statements = **26 / 96, byte-identical to the base** (md5 921b8948). Restore position WITHIN block 2 is codegen-neutral.
  * `r4` the staging borrow moved to BLOCK 1 (restore at the end of block 1, block 2 left plain) = **48 / 95**. Banked `rejected/s7_stage_block1_48.c`. Block 1 was the last untested staging block; all are now measured - b1 = 48, b2 = 26, b3 = 48, b2+b3 = 34.
  * `t0` this chassis with the borrow and restore simply removed = **45 / 93**. This is the honest FAKE-free floor of THIS chassis; the older FAKE-free 40 belongs to the s4 v12 chassis and is not comparable. Banked `rejected/s7_plain_noborrow_on_26_chassis_45.c`.
  * `t1` split-init accumulation (`cross_center = dz * (...); cross_center -= dx * (...);` - the ordinary-C form sanctioned by [[split-init-accumulation-sanctioned]]) applied to every cross product on the no-borrow chassis = **65 / 92**. Banked `rejected/s7_splitinit_accum_plain_65.c`.
  * `t2` the same split-init on the borrow chassis (blocks 1 and 3) = **68 / 93**; `t3` extending it to block 2's borrowed product = **68 / 93**. Banked `rejected/s7_splitinit_accum_borrow_68.c`. Split-init breaks the `mult`/`mflo` pairing the target relies on and is decisively worse, which closes the "find an ordinary-C pressure lever to replace the borrow" line on this chassis.

- [s7] ARTIFACTS: tmp/grind/func_8002E6B0/s7/ - findreg.sh + findregdbg.txt (the FINDREGDBG capture for pseudos 96/95/94/89), lreg_blocks.txt (per-block live-at-start), local_alloc_map.txt (the full `;; Register N in H.` list), body.c / base26.c, r1-r4 and t0-t3 .c and .o, install.py, measure.ps1.

- [s7] CHASSIS: the inherited 26 body reproduces at 26 / 96 insns on HEAD (target 94). Floor unchanged this session. src/code6cac_b.c was reverted to INCLUDE_ASM at the end of the session; the working tree carries only memory/grind ledger changes.

- [s7] The object-level classifier (inverse_compose.py classify --target-object tmp/grind/func_8002E6B0/s5/perm_d/target.o --ours-object tmp/sandbox/func_8002E6B0/code6cac_b.o) reports the entire insn-multiset residual as 'ours only: move #,#' twice - the trailing move v0,v1 seat copy and the move v1,zero restore. Its printed 'FIRST DIVERGENCE: PRE-RA' is a false attribution for the seat copy, because an RA-generated register copy IS an RA product and the multiset heuristic cannot see that. Treat a PRE-RA verdict from this classifier as unreliable whenever the only shape difference is 'move'. memory/grind/func_8002E6B0/classify.log is stale (an older 93-vs-94-insn chassis) and must not be quoted.

- [s7] FINDREGDBG ground truth for pseudo 96 (ret, allocno order 0) on the 26 body: conflicts {2,29}; someone_prefers {4}; used_so_far {0..15,24,25,26,27,28,29,31}; pass0_used {0,1,2,4,16..23,26..31}; pass1_used {0,1,2,26,27,28,29,31}; own_copy_prefs and own_full_prefs both empty. Reg 2 is excluded in both find_reg passes SOLELY by the conflict, so removing that one edge is necessary and sufficient for the $v0 seat.

- [s7] The .lreg per-block live-at-start sets for this body contain no hard reg 2 anywhere (bb0 {4 5 6 7 29 30}, bb1 {29 30 76 77 78 80 85 86 87 89 104 107 109 111 113 115}, bb2 {29 30 76 78 80 85 87 89 109 113}, bb3 {29 30 96}), so the block-entry record_conflicts path (global.c:695-698, 725) cannot be the source of the 96/$v0 conflict. It is the source of the ubiquitous reg-29 conflicts; reg 30 is live at every block start but appears in no conflict line because eliminable registers are stripped from conflicts and preferences at global.c:542-548.

- [s7] The epilogue copy (insn 236, set (reg/i:SI 2 v0) (reg/v:SI 96)) is not the source of the conflict either: mark_reg_clobber returns immediately unless its setter is a CLOBBER (global.c:1528-1529), so the pre-REG_DEAD note_stores pass at global.c:764 is a no-op for a SET, and the REG_DEAD note for reg 96 is processed at global.c:769-770 before mark_reg_store at global.c:777.

- [s7] local_alloc is the source. The .lreg dump's ';; Register N in H.' listing shows local_alloc placed TWELVE pseudos in $v0 (79 81 84 97 100 117 121 138 146 149 153 157) and only FOUR in $v1 (88 90 93 142). A locally-allocated pseudo is folded into its hard register by reg_renumber in mark_reg_store (global.c:1490-1491) and sets that bit in hard_regs_live (global.c:1509); record_one_conflict then IORs hard_regs_live into hard_reg_conflicts on every birth of reg 96 (global.c:1392), and this body gives ret four births.

- [s7] The target mirrors our register roles exactly: it computes both branch conditions into $v1 (xor v1,a0,v1 / bltz v1) and keeps ret in $v0 (move v0,zero in the first bltz's delay slot, srl v0,v0,0x1f at the tail) with NO restore - its second bltz delay slot carries real block-3 work (subu a0,t5,t3). The heads of the two objects are the same instructions in the same order.

- [s7] REPO-WIDE TOOLING DEFECT: tools/ra_solver/simulate.py:159-170 builds regs_someone_prefers from the .greg ';; N conflicts:' listing, which dump_conflicts prints as the SYMMETRIC closure (CONFLICTP (i,j) || CONFLICTP (j,i), global.c:1807), whereas the real prune_preferences tests only the one-way bit (CONFLICTP (allocno, allocno_order[j]), global.c:920) and record_one_conflict only ORs the live set into the ROW of the allocno being born (global.c:1391-1404). The model therefore over-accumulates someone_prefers, gets {2,3,4} instead of {4} for the ord-0 allocno, picks $a1 instead of $v1, and cascades - which fully accounts for simulate.py's 11/31 on this function. The fix needs a directional conflict source that the .greg dump cannot supply.

- [s7] MEASURED THIS SESSION (all sandbox --disable all on the 26 chassis unless stated): r1 cond-carrier cross_center = 32/96 (distinct object 1a75cc02); r2 cond-carrier cross_point = 26/96 (distinct object 3bae0255, new tie basin); r3 restore moved up inside block 2 = 26/96 byte-identical to base (921b8948); r4 staging moved to block 1 = 48/95; t0 borrow+restore deleted = 45/93; t1 split-init everywhere without borrow = 65/92; t2 split-init with borrow = 68/93; t3 t2 plus block-2 split = 68/93.

- [s7] There are now FOUR known distinct objects tied at the 26 floor: base26 (921b8948, the only basin a permuter has searched - s5's perm_e, 11,782 iterations), q1 (078e7ad4), q2 (1b1fa1c0) and s7's r2 (3bae0255).

- [s7] BB2_FINDREG_DEBUG on tools/gcc-2.7.2/cc1 gives the exact per-pseudo allocation inputs for one cc1 run per pseudo and is strictly more trustworthy than the ra_solver model on this function; tmp/grind/func_8002E6B0/s7/findreg.sh is a reusable two-line driver for it.

## s8 (2026-09-08, structural) - floor 26 -> 26

Chassis re-confirmed: candidate.c measures 26 / 96 insns on HEAD this session.

Measured this session (all `sandbox func_8002E6B0 --disable all`, -mel -msoft-float, INCLUDE_ASM on main):

| form | score / insns | object md5 |
|---|---|---|
| d0 base (= candidate.c) | 26 / 96 | 921b8948 |
| d1 ret declared first | 26 / 96 | 921b8948 |
| d2 cross vars declared first | 26 / 96 | 921b8948 |
| d3 cross_point before cross_center | 26 / 96 | 921b8948 |
| d4 center_z before center_x | 26 / 96 | 921b8948 |
| d5 ret between centers and cross vars | 26 / 96 | 921b8948 |
| d6 centers split decl/assign | 26 / 96 | 921b8948 |
| h1 plain + hoist arg3 coords | 49 / 93 | - |
| h2 borrow + hoist arg3 coords | 32 / 96 | - |
| h3 plain + hoist arg0 coords | 45 / 93 | fac5aca8 |
| h4 borrow + hoist arg0 coords | 26 / 96 | 921b8948 |
| h5 plain + hoist both | 49 / 93 | - |
| h6 borrow + hoist both | 32 / 96 | - |
| j0 plain (no borrow, no restore) | 45 / 93 | - |
| j_plain_ox / _qx / _all | 50 / 49 / 50, 93 | - |
| j_borrow_ox / _qx / _all | 31 / 30 / 31, 96 | - |
| i1 ret=0 moved before first if | 26 / 96 | 921b8948 |
| i2 ret=0 inside first exit arm | 30 / 95 | a8ace481 |
| i5 tail split through ret | 26 / 96 | 921b8948 |
| i6 i5 + tail xor operands swapped | 26 / 96 | 078e7ad4 (= s6 q1) |
| i7 i1 + i5 | 26 / 96 | 921b8948 |
| i8 restore spelled `ret -= dz;` | 26 / 96 | 921b8948 |
| k1 first exit inline return 0 | 30 / 95 | - |
| k3 k1 with ret uninitialised | 30 / 95 | - |
| k4 k3 + tail split | 30 / 95 | - |
| k5 both exits inline return 0 | 40 / 93 | - |

Facts established:

1. The base 26 basin (md5 921b8948) now absorbs SEVEN further structural spellings byte-identically
   (d1-d6, h4, i1, i5, i7, i8). Declaration order, `ret = 0` statement placement, the arithmetic
   spelling of the restore, and splitting the tail expression through ret are all codegen-neutral.
   Hand search of that basin's neighbourhood is exhausted.

2. Two independent forms that shorten reg 96's live range at the FRONT (i2, k1/k3/k4) each drop one
   insn but lose the sixth callee-save and turn the first `bltz` into `bgez` + `j`. Front-shortening
   and the sixth callee-save are anti-correlated on this chassis.

3. Conflict-source analysis of the `.lreg` RTL in CHAIN order (tmp/grind/func_8002E6B0/s8/an3.py,
   output conflict_sources.txt): reg 96 has six sets (chain idx 40, 49, 55, 75, 76, 77 = insns
   50, 129, 165, 230, 231, 232) and is live over chain idx 40..79 but dead across block 3. Only
   THREE of local_alloc's twelve $v0 pseudos overlap that range: 117 (block-1 branch condition,
   idx 43-44), 121 (a block-2 product, idx 51-54) and 138 (block-2 branch condition, idx 58-59).
   The block-3 $v0 quantities (146/149/153/157, idx 64-72) are irrelevant to the seat. The target
   computes both branch conditions into $v1, which is exactly what this model requires.

4. The target's `move v0,zero` occupies the FIRST `bltz`'s delay slot and executes unconditionally,
   serving both early exits; $v0 is then reused as block-3 scratch before the tail recomputes it.
   The ret-variable chassis is structurally correct.

5. The FAKE-free plain chassis (j0/t0 = 45 / 93) seats ret in $a1, not $v1 - a worse seat than the
   borrow chassis. The plain form is two structural steps from the target (missing `sw`/`lw s5` and
   carrying a trailing `move v0,a1`), not one.

Artifacts: tmp/grind/func_8002E6B0/s8/ (all variant .c and .o files, an3.py, conflict_sources.txt,
tgt.n + per-variant .n normalised disassemblies, measure.ps1, install.py, gen.py, gen2.py, dis2.sh).

- [s8] Chassis re-confirmed at dispatch: memory/grind/func_8002E6B0/candidate.c measures 26 / 96 insns on HEAD this session, matching the ledger floor, so every s5-s7 spelling conclusion remains chassis-valid.

- [s8] The base 26 basin (object md5 921b8948a0460d02b617959aacbd2403) now absorbs eleven further structural spellings byte-identically (d1-d6, h4, i1, i5, i7, i8) on top of the ties s5-s7 recorded; hand search of that basin's neighbourhood is exhausted.

- [s8] Every ordinary-C register-pressure lever measured on this chassis is neutral or worse: split-init accumulation 65-68 (s7), staging sites b1 = 48 / b3 = 48 / b2+b3 = 34 (s4-s6), coordinate and shared-sub-expression hoisting 45 -> 45/49/50 plain and 26 -> 26/30/31/32 borrow (s8). The FAKE staging borrow remains the only construct that buys the target's sixth callee-save.

- [s8] Two independent front-shortening forms (i2, and k1/k3/k4) each drop one insn to 95 but lose sw s5,20(sp) and convert the first bltz into bgez + j, so shortening reg 96's live range at the front and holding the sixth callee-save are anti-correlated on this chassis.

- [s8] Conflict-source analysis (tmp/grind/func_8002E6B0/s8/conflict_sources.txt) names the three quantities that create the seat: pseudos 117, 121 and 138, in two windows only (chain idx 43-44 and 51-59). This supersedes s7's twelve-pseudo statement.

- [s8] The target's move v0,zero sits in the FIRST bltz's delay slot and executes unconditionally, serving both early exits; $v0 is then reused as block-3 scratch before the tail recomputes it. The ret-variable chassis is structurally correct and reg 96 is legitimately dead across block 3 in our build too.

- [s8] The FAKE-free plain chassis (j0/t0 = 45 / 93) seats ret in $a1 (move a1,zero / srl a1,a1,0x1f / move v0,a1), a worse seat than the borrow chassis's $v1, so the plain form is two structural steps from the target, not one.

## s9 (2026-09-08, enumerate) — exhaustive per-block spelling sweep: 1,228 spellings, zero below 26

CHASSIS. Re-measured on HEAD this session: candidate.c = **26** (96 build insns;
target 94). The driver's dispatch brief reported "measurement unavailable"; the
ledger's 26 is CORRECT on the current -mel -msoft-float chassis.

KILL RE-AUDIT (mandated; `tools/sweep_variants.py` over
tmp/grind/func_8002E6B0/s9/reaudit/, result in s9/sweep_reaudit.json). Every
banked score reproduces byte-for-byte on the current chassis:

| form | s9 score | build insns | banked score |
|---|---|---|---|
| candidate.c (borrow present) | 26 | 96 | 26 |
| s6_first_if_xor_swap_distinct_object_ties_26 | 26 | 96 | 26 |
| s7_cond_carrier_cross_point_distinct_object_ties_26 | 26 | 96 | 26 |
| s8_first_exit_inline_return0_loses_s5 | 30 | 95 | 30 |
| s8_hoist_shared_subexpr_borrow | 31 | 96 | 31 |
| s7_plain_noborrow_on_26_chassis (FAKE-ABLATED control) | 45 | 93 | 45 |

The FAKE-ablation question the re-audit exists to answer is therefore settled
for this function: the ONLY FAKE construct in candidate.c is the block-2
variable-reuse borrow (`(ret = dz)` + the `ret = 0` restore), its ablated
control is the plain no-borrow form, and that control measures 45 — 19 worse,
not better. No lever this session was measured with a FAKE carrier masking it,
because the carrier is worth +19 and removing it does not open a seat: the
no-borrow form loses the sixth callee-save (93 insns, one FEWER than target).

THE ENUMERATION. The residual is two moves (trailing `move v0,v1`; `move
v1,zero` in the second bltz delay slot), so the three straight-line
cross-product blocks are the only regions whose spelling can move it. Each was
marked ENUM-BEGIN/ENUM-END and enumerated with `tools/spelling_enum.py`, then
scored with `tools/sweep_variants.py` (all JSON in tmp/grind/func_8002E6B0/s9/,
histograms collected in s9/histograms.txt):

| sweep | region | axis | n | score histogram |
|---|---|---|---|---|
| A  | block 1 (arg1-arg0) | named-vs-inlined x decl order x assign order over dz/dx/ax/az | 130 | 26:35, 31:30, 52:65 |
| B  | block 3 (arg2-arg1) | same | 130 | 26:19, 28:16, 29:14, 34:65, 53:16 |
| C  | block 2 (arg2-arg0, borrow) | same | 130 | 26:65, 43:65 |
| Ap | block 1 | same over dz/dx/px/pz (point deltas named) | 130 | 26:16, 30:49, 52:19, 53:46 |
| Bp | block 3 | same | 130 | 26:65, 34:19, 36:14, 38:16, 53:16 |
| Cp | block 2 | same | 130 | 26:65, 43:65 |
| As | block 1 | commutative operand swaps on all four products x inline/order over dz/dx | 160 | 26:10, 27:26, 28:22, 29:6, 31:8, 32:8, 52:52, 53:28 |
| Bs | block 3 | same | 160 | 26:5, 27:13, 28:11, 29:5, 30:4, 31:2, 34:5, 35:13, 36:11, 37:3, 39:2, 40:7, 41:10, 42:8, 43:5, 44:3, 45:4, 46:8, 47:8, 48:4, 49:2, 50:2, 51:1, 54:24 |
| Cs | block 2 | same | 128 | 26:64, 43:64 |

**ENUMERATION: 1,228 spellings swept, best 26, 264 of them AT the floor. Not one
spelling scored below 26.** Semantics are preserved by construction (the tool
only renames/inlines/reorders within def-before-use and swaps commutative
operands); the emitted variants were spot-checked to confirm the block-2 borrow
survives inlining as `(ret = (arg2[2] - arg0[2]))`.

TWO STRUCTURAL READINGS OF THE HISTOGRAMS (both new this session):

1. **Block 2 is spelling-RIGID and binary.** enumC / enumCp / enumCs each split
   exactly 50/50 between 26 and 43 and produce NO intermediate score. The
   discriminator is a single bit: whether the borrow product is written
   `(ret = dz) * <ax>` or `<ax> * (ret = dz)`, i.e. whether the borrowed pseudo
   is the FIRST operand of the multiply. Every other block-2 axis (which
   sub-expressions are named, in what order they are declared, in what order
   the two cross products are assigned) is byte-neutral. So no block-2 spelling
   change can reach the seat — the block has exactly one degree of freedom and
   it is already at its optimum.
2. **Blocks 1 and 3 are spelling-SENSITIVE but bounded below by 26.** They span
   26..53 across 20+ distinct scores, so the sweep genuinely explores different
   objects (it is not one basin re-emitted), and it still never crosses 26. The
   26 count differs sharply by axis (block 1: 35/130 named-vs-inlined but only
   10/160 once swaps are on; block 3: 19/130 and 5/160), which means most
   spellings of those blocks are actively harmful and the current candidate sits
   on a narrow ridge.

WHAT THIS ELIMINATES. The `enumerate` modality's own stopping rule applies: a
zero-hit sweep over a region is the strongest evidence that the residual does
not live in that region's spelling space. All three straight-line blocks are now
swept exhaustively on the three axes the tool models (naming, declaration and
statement order, commutative operand order). The residual is therefore NOT a
sub-expression-naming, statement-order or operand-order choice inside any single
block. What the sweep does NOT cover, and what the next session inherits:
  * the DECLARATION SCOPE axis (function-scope vs the current per-block braces) —
    the enumerator cannot move a declaration across a block boundary;
  * any spelling that spans the `if` boundary (the tool must keep anchors last,
    so a region cannot legally contain the early-exit branch);
  * the OBJECT MODEL (arg0/arg1/arg2/arg3 as four `s32 *` vs a struct/vector
    type), which changes the addressing, not the spelling.

- [s9] HEAD honest floor for func_8002E6B0 re-measured this session: 26 (96 build insns, target 94). The dispatch brief reported 'measurement unavailable'; the ledger's 26 is correct on the current -mel -msoft-float chassis.

- [s9] 1,228 distinct spellings of the three cross-product blocks were enumerated and scored this session. Best 26; 264 tie the floor; zero below it.

- [s9] Block 2 (the borrow block) is spelling-rigid: enumC, enumCp and enumCs each split exactly 50/50 between scores 26 and 43 with no intermediate value, so the block has exactly one byte-relevant degree of freedom - whether the borrowed pseudo is the first operand of its multiply. It is already set to the good value.

- [s9] Blocks 1 and 3 are spelling-sensitive but bounded below by 26: their sweeps span 26..54 across 20+ distinct scores. Most spellings are actively harmful (block 1 ties the floor in 35/130 without swaps but only 10/160 with swaps; block 3 in 19/130 and 5/160), so the candidate sits on a narrow ridge.

- [s9] FAKE-ablation control re-measured on the current chassis: the plain no-borrow form scores 45 and emits 93 insns, one FEWER than the target's 94. The borrow is worth +19 and is not masking a lever.

- [s9] The spelling enumerator structurally cannot reach three axes, which is what the next session inherits: declaration SCOPE (function-scope vs the current per-block braces, since the tool cannot move a declaration across a block boundary), any form spanning an early-exit `if` (the tool must keep anchors last), and the object model (four s32* parameters vs a struct/vector type).

- [s9] tools/sweep_variants.py restores src/ byte-exact; git status confirms src/code6cac_b.c was untouched at end of session.

## s10 (2026-09-08, forensics) — floor 26 -> 26; the ret seat typed down to a single local-alloc scan order

CHASSIS. Re-measured on HEAD this session with `tools/sweep_variants.py`:
candidate.c = **26 / 96 insns** (target 94). The dispatch brief again said
"measurement unavailable"; the ledger's 26 is correct on the current
-mel -msoft-float chassis.

KILL RE-AUDIT (mandated). The instance kill whose form sits closest to the
target is s8's k1 (`s8_first_exit_inline_return0_loses_s5_30.c`, 30 / 95 — one
insn FEWER than our 96, the closest any banked form gets). Re-measured this
session: **30 / 95**, unchanged. The FAKE-ablation control
(`s7_plain_noborrow_on_26_chassis_45.c`) re-measured **45 / 93**, unchanged.
Both kills stand on the current chassis; no lever was masked by the borrow.

### 1. The residual is a pure register PERMUTATION, not a code difference

Side-by-side normalised disassembly (tmp/grind/func_8002E6B0/s10/tgt.n vs
zz_reaudit_candidate.n) of the 26 body against the target:

  * The callee-save usage is **identical**: both save s0-s5, both use
    s0/s1/s2/s3/s5 at the same instructions, and both save s4 without ever
    using it (a phantom slot in the target too).
  * Every differing line except two is a register RENAME under one fixed
    permutation: $v0 <-> $v1 everywhere, plus a three-cycle a0 -> a1 -> a2 in
    the block-1 delta region (`mfhi a1` / `mflo a2` / `mflo a0` in the target vs
    `mfhi a2` / `mflo a0` / `mflo a1` in ours).
  * The only two extra instructions are the ones already banked: the restore's
    `move v1,zero` in the second bltz's delay slot, and the trailing
    `move v0,v1`.

So the whole 26-point residual is downstream of ONE allocation decision: which
of $v0 / $v1 the `ret` pseudo (reg 96) gets.

### 2. PASS ATTRIBUTION: local-alloc.c `find_free_reg`, the ascending scan

`tools/gcc-2.7.2/config/mips/mips.h` does **not** define `REG_ALLOC_ORDER`, so
`find_free_reg` (local-alloc.c, the `for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)`
loop with `regno = i`) scans hard registers in **ascending numeric order**. $v0
is register 2 — the first allocatable register on this target. Therefore every
local quantity that reaches the ascending scan without a suggestion takes $v0,
and only a quantity whose `used` set already contains bit 2 can land in $v1.

That is the complete explanation for the `.lreg` map this function produces
(dumped again this session for the candidate, tmp/grind/func_8002E6B0/s10/
dumps_zz_reaudit_candidate/code6cac_b.lreg): twelve pseudos "in 2."
(79 81 84 97 100 117 121 138 146 149 153 157) and only four "in 3."
(88 90 93 142). $v1 is reached only when $v0 is already excluded.

The chain to the seat, unchanged from s7/s8 but now with its first link named:
find_free_reg's ascending scan puts 117 / 121 / 138 in $v0 ->
global.c:1490-1491,1509 folds those renumbered locals into `hard_regs_live` ->
record_one_conflict (global.c:1392) ORs that into reg 96's `hard_reg_conflicts`
at each of reg 96's births -> global.c find_reg excludes reg 2 in BOTH passes
(s7's FINDREGDBG capture) -> reg 96 gets $v1.

`used` in find_free_reg can only acquire bit 2 two ways:
  (a) `IOR_HARD_REG_SET (used, regs_live_at[ins])` over the quantity's range —
      i.e. **hard reg 2 is live across that range**; or
  (b) a copy/arith suggestion diverts the quantity first
      (`qty_phys_copy_sugg` / `qty_phys_sugg`, the `just_try_suggested` pass),
      which on a leaf function with no calls only arises from copies to/from
      the incoming argument registers $a0-$a3.

### 3. PASS-INPUT ENUMERATION: what C shape puts hard reg 2 in `regs_live_at`?

GCC 2.7.2 gives a scalar return value the hard register directly: a source-level
`return 0;` expands to `(set (reg/i:SI 2 v0) (const_int 0))` with **no pseudo at
all**, whereas a `s32 ret` local produces `(reg/v:SI 96)` plus one epilogue copy
`(set (reg/i:SI 2 v0) (reg/v:SI 96))`. Both shapes were dumped this session and
grepped out of the `.lreg` (s10/dumps_zz_reaudit_candidate/, s10/dumps_zz_k5_
both_inline_40/).

MEASURED: that does **not** move the seat. In the both-exits-inline body
(`s8_both_exits_inline_40.c`, 40 / 93) the two `(set (reg/i:SI 2) 0)` insns sit
in the two exit ARMS — the taken edges — so hard reg 2 is never live on the
fall-through path through blocks 1 and 2, and the `.lreg` map is the same:
117 / 121 / 138 still "in 2.", conditions still `bltz v0`. The k1 mixed body
(30 / 95) behaves identically. So the inline-return family cannot supply
route (a).

What the TARGET does, read off its disassembly: hard $v0 is written
unconditionally in the middle of block 1 (`move v0,zero`, later stolen into the
first bltz's delay slot by reorg) and is then **untouched across the whole of
block 2** — target block 2 uses only a0 / v1 / a1 / a2 / t-regs. Its block-1
condition is in $v1 too, born after the last $v0 use of block 1. That is exactly
the signature of route (a): reg 2 live across the branch, blocks 1-and-2
quantities pushed to $v1, and the return value seated in $v0.

### 4. Measured this session (all `sandbox func_8002E6B0 --disable all`)

Declaration-SCOPE grid (the s9 frontier's #1 next probe, 13 bodies,
s10/sweep_scope.json):

| form | score / insns |
|---|---|
| per-block braces (= candidate), borrow | 26 / 96 |
| three DISTINCT function-scope dz/dx pairs, borrow | **26 / 96 (byte-identical)** |
| blocks 1+3 distinct fn-scope, block 2 braced, borrow | 26 / 96 |
| blocks 1+3 braced, block 2 fn-scope, borrow | 26 / 96 |
| per-block braces, plain | 45 / 93 |
| three distinct fn-scope pairs, plain | 45 / 93 |
| ONE REUSED fn-scope dz/dx pair, borrow | 55 / 94 |
| blocks 1+3 share one fn-scope pair, block 2 braced | 56 / 94 |
| blocks 2+3 share one fn-scope pair | 56 / 94 |
| one reused fn-scope pair, plain | 60 / 97 |

Alternative staging CARRIER family (10 bodies, s10/sweep_m.json):

| form | score / insns |
|---|---|
| block-2 staging through `cross_point` (goto/goto chassis) | 45 / 93 |
| same, staging in blocks 1+2 / blocks 2+3 | 45 / 93 both |
| block-2 staging through `cross_center` (cross_point stmt first) | 46 / 95 |
| that + the same in block 3 | 48 / 95 |
| both exits inline `return 0` + cross_point carrier | 53 / 93 |
| first exit inline + cross_point carrier | 56 / 99 |
| `ret` borrow (candidate) PLUS a cross_point borrow in block 1 | 26 / 96 |
| mixed exits + the `ret` borrow with restore | 30 / 95 |

Delta-HOISTING family (5 bodies, s10/sweep_n.json) — moving each block's
`dz`/`dx` computation above the first `if` (the one motion the s9 enumerator
structurally cannot make): block 3 only 58 / 95; block 2 only 68 / 94;
blocks 1+2 68 / 94; blocks 2+3 70 / 97; all three 70 / 97. Every hoist is far
worse.

### 5. Facts banked

- The declaration-SCOPE axis is byte-neutral as long as the three dz/dx pairs
  keep DISTINCT names: function-scope declarations compile to the identical
  object as the per-block braces (26 / 96). Only NAME reuse across blocks
  changes anything, and it costs 29-34 points (55-60) while landing on 94
  insns.
- An alternative staging carrier does not exist on this chassis. Staging block
  2's `dz` through `cross_point` or `cross_center` scores 45-48 — identical to
  the no-borrow control — because the carrier is overwritten by its own real
  value on the very next statement, so the staging store is dead and is removed
  before RA. The `ret` carrier works only because `ret` is live out through
  both early exits, which is what keeps the store alive.
- Hoisting any block's delta computation above the first `if` is 58-70.
- The candidate's callee-save allocation is byte-for-byte the target's,
  including the never-used s4 slot; the entire residual is the $v0/$v1 seat and
  the a0/a1/a2 three-cycle that follows it, plus the two known extra moves.

### 6. s10 POSTSCRIPT — one of the three $v0 contributors CAN be removed, and it is byte-neutral

A function-scope `s32 cond;` written in block 1 and again in block 2
(`cond = cross_center ^ cross_point; if (cond < 0) goto end;`) makes the block-1
condition a MULTI-BLOCK reference, so it is no longer a local-alloc quantity at
all. Measured (tmp/grind/func_8002E6B0/s10/sweep_o.json): o1 = 26 / 96 and the
object is **byte-identical** to the base basin (md5 921b8948). But the `.lreg`
map DID change (s10/dumps_zz_o1/code6cac_b.lreg): pseudo 117, the block-1
condition, has disappeared from the `;; Register N in H.` listing entirely, so
local_alloc now places ELEVEN pseudos in $v0 instead of twelve, and the
block-2 condition (renumbered 138) moved from "in 2." to "in 4." ($a0).

Reg 96 still lands in $v1 because pseudo 121 — a block-2 PRODUCT (an `mflo`
result consumed by the following subtraction) — is still "in 2." and is still
born inside reg 96's live range. So the seat needs ALL THREE contributors out
of $v0, and this session has shown that two of the three (both branch
conditions) can be evicted with ordinary C at zero byte cost. Variants that
route the tail through the same `cond` variable (o2, o4) cost 29 points
(55 / 96), so the eviction must stay confined to the two `if` conditions.

Two further controls: o3 (two DISTINCT single-block condition variables,
`cond1` / `cond2`) is also 26 / 96 — a single-block named condition stays a
local quantity, as expected.

- [s10] [s10] HEAD honest floor re-measured this session: 26 (96 build insns, target 94). The dispatch brief again reported 'measurement unavailable'; the ledger's 26 is correct on the current -mel -msoft-float chassis.

- [s10] [s10] The candidate's callee-save allocation is byte-for-byte the target's: both save s0-s5, both use s0/s1/s2/s3/s5 at the same instructions, and both save s4 without ever using it (a phantom slot in the target too).

- [s10] [s10] Every differing instruction between the 26 body and the target, except the two known extra moves (the restore's move v1,zero in the second bltz delay slot and the trailing move v0,v1), is a register RENAME under one fixed permutation: $v0 <-> $v1 throughout plus an a0 -> a1 -> a2 three-cycle in the block-1 delta region. There is no structural work left; the floor is one allocation decision wide.

- [s10] [s10] PASS ATTRIBUTION: tools/gcc-2.7.2/config/mips/mips.h defines no REG_ALLOC_ORDER, so local-alloc.c find_free_reg's scan loop runs ascending with regno = i and $v0 (reg 2) is the first register tried for every unsuggested quantity. That alone explains the twelve-in-$v0 / four-in-$v1 local map; $v1 is reached only when $v0 is already excluded from `used`.

- [s10] [s10] `used` in find_free_reg can only acquire bit 2 from IOR_HARD_REG_SET (used, regs_live_at[ins]) (hard reg 2 live over the range) or from the just_try_suggested pass (qty_phys_copy_sugg / qty_phys_sugg), which in this call-free leaf function can only come from copies to/from the incoming argument registers $a0-$a3.

- [s10] [s10] GCC 2.7.2 gives a scalar return value the hard register: a source-level `return 0;` expands to (set (reg/i:SI 2 v0) (const_int 0)) with NO pseudo, while `s32 ret` produces (reg/v:SI 96) plus one epilogue copy (set (reg/i:SI 2 v0) (reg/v:SI 96)). Both were read out of the .lreg dumps this session.

- [s10] [s10] The inline-return family does NOT make hard reg 2 live across blocks 1-2: in the both-exits-inline body the two reg-2 sets sit in the exit ARMS (taken edges), so the fall-through path is unaffected, 117/121/138 are still 'in 2.', and the seat is unchanged (40/93; mixed-exit variant 30/95).

- [s10] [s10] The target writes $v0 unconditionally in the middle of block 1 (later stolen into the first bltz's delay slot) and never touches it across the whole of block 2 - its block 2 uses only a0/v1/a1/a2/t-regs - which is the signature of the return register being live on the fall-through path at local-alloc time.

- [s10] [s10] Declaration SCOPE is byte-neutral with distinct names: three function-scope dz/dx pairs compile to the identical 26/96 object as the per-block braces. Cross-block NAME reuse costs 29-34 points (55/94, 56/94, 60/97) and is the only shape on this chassis that reaches the target's 94-insn count.

- [s10] [s10] Alternative staging carriers do not exist on this chassis: cross_point and cross_center score 45-48, exactly the no-borrow control, because the carrier is overwritten on the next statement so the staging store is dead before RA. `ret` works only because its staged value is live out through both early exits.

- [s10] [s10] Hoisting any block's dz/dx computation above the first `if` scores 58-70.

- [s10] [s10] A function-scope `cond` written in blocks 1 and 2 evicts pseudo 117 from local_alloc's map entirely (eleven pseudos in $v0 instead of twelve, and the block-2 condition moves from $v0 to $a0) while leaving the object byte-identical (26/96, md5 921b8948). The remaining blocker is pseudo 121, a block-2 mflo product still allocated to $v0 and still born inside reg 96's live range.
