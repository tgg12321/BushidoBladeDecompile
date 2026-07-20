# Evidence bank — gnd_init_80041688

- [s1] sandbox --disable all: score=2, target_insns=82, build_insns=82, rules_dropped=3, cheat_asm_stripped=23 (whole-file; nothing in gnd_init_80041688 itself).

- [s1] regfix.txt:477-479 are the 3 sole rules: pure subst on maspsx idx 68/69/70 swapping `$reg,offset($16)` operand of the three FALSE-color lbu insns. No cheat-asm, no pins, no barriers in the C source.

- [s1] asm/funcs/gnd_init_80041688.s .L80041798 target FALSE-color block: `lbu $v1,0x1A; lbu $a0,0x18; lbu $v0,0x19; sll a0<<16; sll v0<<8; or a0,a0,v0; or a0,v1,a0`. Register assignment b=v1,r=a0,g=v0.

- [s1] Cheat-free build FALSE-color block: same registers assigned (b=v1,r=a0,g=v0), same shifts, same OR-chain (`or a0,v1,a0` as final — b on LEFT of outer OR matches source `b | ((r<<16)|(g<<8))`). Only diff: lbu emission order is [r,g,b] instead of target's [b,r,g].

- [s1] Mechanism: sched1 backward-scheduling picks by INSN_PRIORITY = longest chain to end-of-block. r,g each have chain length 4 (lbu → sll → or_mid → or_final); b has chain length 2 (lbu → or_final). So r,g launch first. Target's original C must have made b's priority equal-or-higher (extra chain link on b) OR sched.c `true_dependence` saw an alias edge on the r,g MEMs it doesn't see for mine — a MEM_IN_STRUCT_P `/s` flag pair split (proven-spelling-class-reconstruction family).

- [s1] TRUE-color branch matches target: `func_8004881C(b,g,r)` arg-setup lbu's are ordered by argN (a0=b,a1=g,a2=r) — GCC's arg-passing naturally pins the emission order. No diff on that branch.

- [s1] or-tree-shape-shift (parenthesization reshuffle of the OR chain to give b a longer chain via `((b|r)|g)` etc.) is FORBIDDEN per the technique index — not a lever.

- [s2] s2 baseline (unchanged from s1): sandbox --disable all -> score=2, target_insns=82, build_insns=82, rules_dropped=3.

- [s2] Target FALSE branch (.L80041798): lbu v1,0x1A -> lbu a0,0x18 -> lbu v0,0x19 -> sll a0,a0,16 -> sll v0,v0,8 -> or a0,a0,v0 -> or a0,v1,a0. Registers b=v1, r=a0, g=v0.

- [s2] Build FALSE branch: lbu a0,0x18 -> lbu v0,0x19 -> lbu v1,0x1A -> sll a0,a0,16 -> sll v0,v0,8 -> or a0,a0,v0 -> or a0,v1,a0. IDENTICAL registers, IDENTICAL OR-tree shape (`or a0,v1,a0` final has b on LEFT). ONLY diff: the three lbu's emit in [r,g,b] instead of [b,r,g]. Diff = 3 lbu-swap-position insns => score=2 (masked weight).

- [s2] Target has TWO SEPARATE lbu blocks (TRUE arm @ 80041770-78 loading a0=0x1A, a1=0x19, a2=0x18 for `func_8004881C(b,g,r)` call; FALSE arm @ .L80041798). The TRUE-arm order is arg-passing-driven and matches naturally.

- [s2] sched1 chain-length priority: r,g each have 4-insn chain to final or; b has 2-insn chain. Priority-ordering places r,g ahead; b emits last regardless of declared/statement order.

- [s2] Four s2 structural axes measured dead at floor=2: MEM_IN_STRUCT_P (struct cast), MEM_ALIAS_SET (walking pointer), CFG restructure (ternary hoist), pseudo/decl order (block-local split). None move sched1's priority-driven emission.

- [s2] or-tree-shape-shift (repartner `(b|r)|g` to give b a longer chain) is FORBIDDEN per codegen-technique-index; not a lever.

- [s3] s3 baseline (unchanged from s2): sandbox --disable all -> score=2, target_insns=82, build_insns=82, 3 rules_dropped.

- [s3] s3 rg (block-local named intermediate): score=2. objdump FALSE @0x15cc-0x15e4: lbu a0,0x18 -> lbu v0,0x19 -> lbu v1,0x1A -> sll a0<<16 -> sll v0<<8 -> or a0,a0,v0 -> or a0,v1,a0. Byte-identical to baseline. `rg` folded pre-sched1.

- [s3] s3 cp (block-local): score=2. Base register in the three lbu's is $s0 (= player), not a distinct pseudo. Emission still [r,g,b]. cp -> player via GCC copy-prop before sched1 sees a distinct MEM.

- [s3] s3 cp (function-scope): score=15. Whole-function RA regression; cp's extended liveness costs more than any FALSE-branch gain. Function-scope liveness of a redundant pointer alias is inert for this reschedule AND destructive on total score.

- [s3] Cumulative structural axes measured dead across s1-s3: statement-reorder-b-first, struct-cast /s flip, walking-pointer bp arithmetic, ternary/hoist consolidation, block-local fb/fr/fg split, block-local cp copy, function-scope cp, block-local named-intermediate rg. Nine forms; all sched1 chain-length dominates.

- [s3] Structural conclusion: within block-local structural rearrangements of the FALSE arm, no axis moves sched1's INSN_PRIORITY-driven emission because r/g's chain-length-4 vs b's chain-length-2 is a HARD ordering — LUID/DECL/COPY-based tie-break only applies at equal priority. Every attempted axis either (a) folded before sched1 (rg, cp-block-local), (b) collapsed via combine (walking-pointer, struct-cast), or (c) net-regressed via broader RA cost (cp-function-scope, hoisted-loads).

- [s3] or-tree-shape-shift (repartner `(b|r)|g` to give b longer chain) FORBIDDEN per codegen-technique-index.

- [s4] s4 baseline sandbox --disable all: score=2, target_insns=82, build_insns=82, rules_dropped=3 (unchanged from s3).

- [s4] Permuter workspace built (tmp/grind/gnd_init_80041688/s4/perm/): base.c 1987 lines (text1a.c preprocessed with same CPP_FLAGS/DEFS as Makefile); compile.sh replicates cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin | prologue_fix | maspsx (aspsx-version=2.34, sdata/expand-lb/multu/label-nop gates from repo config) | multu_pad; extract_fn.py isolates the .ent gnd_init_80041688 .. .end region; target.o assembled from prelude_r3k.inc (prelude.inc minus .set gp=64) + asm/funcs/gnd_init_80041688.s.

- [s4] Base-vs-target objdump diff (baseline s4): single lbu swap — target has `lbu v1,26(s0)` at position 69 (b-first); build has it at position 71 (b-last). Same registers (b=v1, r=a0, g=v0), same OR-tree shape, only emission-order differs. Consistent with s1-s3 finding: sched1 chain-length priority (r/g=4 vs b=2) is the hard ordering.

- [s4] Permuter base_score = 40 (default weights: reg=10, insn=100; here 40 corresponds to the 2-line lbu swap under the workspace's weight profile). Ran 32k+ iters at -j 4 over ~24 min.

- [s4] Novel finds by score: 10×1, 20×2, 30×4, 40×2 (9 total, all fresh). Time-to-first-find below base = 88.7s (score 20); time-to-basin = 242.3s (score 10). No improvement in the following ~1200s window (per fresh-seed discipline).

- [s4] Score-10 mutation (best novel): stages loop1's `(*(s16*)(p+2)) >= 0` boolean through the existing function-scope local `b`. Semantically identical (b is 0 or 1 each iter). Cheat: b's stored value is dead every iteration (overwritten next iter; final value overwritten by `b = *(u8*)(player+0x1A)` in the FALSE arm). No LIVE-code spelling of this axis found by the permuter.

- [s4] All 9 chassis-1 finds are cheat family: 5 variable-reuse-with-dead-store, 3 or-tree-shape-shifts (FORBIDDEN per codegen-technique-index), 1 pointer alias + dead-code, 1 alias rename. Detailed catalog in memory/grind/gnd_init_80041688/rejected/loop1-boolean-stage-b-reuse.c commentary.

- [s4] Cross-check against ledger: score-10 axis (extending b's live range across loop1) DIRECTLY corroborates s3 frontier hypothesis #2 (register-class drift at FALSE-arm entry). The axis IS load-bearing; the byte-neutral spelling of it via dead-store is the only shape random permutation discovered.

- [s5] s5 baseline sandbox --disable all: score=2, target_insns=82, build_insns=82, rules_dropped=3, cheat_asm_stripped=23 (unchanged from s1-s4).

- [s5] s5 hypothesis-1 sandbox measurements (LIVE-b-hoist above loops): variant 1a score=9/insns=84, variant 1b score=11/insns=83, variant 1c score=9/insns=84. All REGRESSED from baseline; extending b's live range across the func_800486FC call forces a callee-save spill.

- [s5] s5 hypothesis-3 sandbox measurements (color-byte pre-load above if): r-hoist score=12/insns=83, g-hoist score=13/insns=82, b-hoist score=11/insns=83. All REGRESSED; same call-crossing spill mechanism.

- [s5] s5 permuter chassis-1 fresh-seed run: base_score=40, elapsed=934.7s, 19204 iters, 5 novel finds (1×10, 2×20, 1×30, 1×40). Best legit find = 40 (no score-drop). All sub-40 = cheat family (b=arg1 staged, loop1-boolean-stage-b, float NULL cast).

- [s5] score-10 form output-10-1 is BYTE-IDENTICAL by structure to s4's score-10: `b = (*((s16*)(p+2))) >= 0; if (b) { ... }` in loop1. Same dead-store cheat, already in memory/grind/gnd_init_80041688/rejected/loop1-boolean-stage-b-reuse.c. Two independent seeds converged to it.

- [s5] Chassis-1 permuter basin definitively exhausted: two seeds × ~15-24 min each, ~50k iters combined, zero legitimate sub-40 findings across all 14 novel outputs (9 in s4 + 5 in s5).

- [s5] The reuse-`b` axis moves sched1 (40->10) but ONLY when b is dead across the loops (per-iteration boolean staging that gets overwritten each iter and finally by the FALSE arm color load). Any LIVE spelling of the axis that keeps b usable at 0x15cc must extend b's live range across the func_800486FC call, which the s5 direct probes proved is net-regressive.

- [s6] [s6] sandbox --disable all: score=2, target_insns=82, build_insns=82, rules_dropped=3, cheat_asm_stripped=23 (unchanged from s1-s5).

- [s6] [s6] Standalone TUs baseline_standalone.c and cheat10_standalone.c (both containing gnd_init_80041688 with same extern decls) compiled with tmp/gccdbg/cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -da (BB2_ALLOC_DEBUG=1 BB2_SCHED_DEBUG=1 BB2_PRIO_DEBUG=1).

- [s6] [s6] .s diff between the two forms: single lbu-position swap. baseline: `lbu $3,26($16)` at line 129 (LAST of FALSE-arm lbu triple); cheat: `lbu $3,26($16)` at line 127 (FIRST). All three lbu operands identical, all registers identical, all other insns identical. Confirms the s1-s5 finding that the sole delta is emission order.

- [s6] [s6] RA is IDENTICAL between baseline and cheat: `;; Register dispositions:` shows `73→5 74→16 75→4 76→3 77→3 78→3 79→4 80→2` in both; `;; Hard regs used: 2 3 4 5 6 16 31` in both.

- [s6] [s6] Pseudo 78 identified as source-level `b` bound to $v1(3) in FALSE arm. From .sched RTL: `(insn 191 ... (set (reg/v:SI 78) (zero_extend:SI (mem/s:QI (plus:SI (reg/v:SI 74) (const_int 26))))))` in baseline; equivalent (insn 194) in cheat. Also from .loop/.flow: pseudo 78 has additional defs at insns 65-66 in the cheat form (`b = (*(s16*)(p+2)) >= 0`) — no such loop1 defs in baseline.

- [s6] [s6] `;; 78 conflicts:` widens from `78 2 4 29` (baseline, trivial) to `74 78 79 80 2 4 29` (cheat, spans player+$a0+$v0 pseudos). Correlated widening on 74/79/80 conflict lists. This confirms pseudo 78's live range crosses loop1 into the FALSE arm in the cheat form.

- [s6] [s6] ALLOCDBG per-pseudo priority-formula inputs: baseline pseudo 78 nrefs=4 livelen=3 pri=26666; cheat pseudo 78 nrefs=8 livelen=7 pri=34285 (both allocate to hardreg=3). Allocation delta measurable in the priority formula but nrefs/livelen change is not enough to force a different hardreg because the RA order still gives pseudo 78 first pick.

- [s6] [s6] BB18 (FALSE arm) sched1 SCHEDDBG trace, baseline: `PICK clock=4 picked=191 (pri=2130706433 luid=4)` — ready list `[191(p=2130706433,l=4) 194(p=2130706433,l=6) 193(p=2130706433,l=5)]`. All three lbu-family insns share the hazard-boosted priority 0x7F000001, tiebreak by luid ascending puts insn 191 (b-lbu) at clock=4 (mid-block).

- [s6] [s6] BB18 sched1 SCHEDDBG trace, cheat: `PICK clock=8 picked=194 (pri=1 luid=4)` — insn 194 (b-lbu) has plain pri=1, unboosted. Ready lists at clock=4/5/6/7 all show `194(p=1,l=4)` sitting at the tail while other pri=0x7F000001 insns launch first. Insn 194 finally emits at clock=8 (last-scheduled = first-in-code slot).

- [s6] [s6] Reverse-order interpretation: sched1's clock=1 slot picks the block-terminating jump (`picked=200 pri=2147483528` baseline; `picked=203` cheat). Higher clock = earlier in code emission. So baseline's b-lbu at clock=4 emits AFTER r/g (clock=7/8), matching baseline .s [r,g,b]; cheat's b-lbu at clock=8 emits BEFORE r/g (clock=6/7), matching cheat .s [b,r,g].

- [s7] [s7] sandbox --disable all after applying raw-halfword form to src (`b = *(s16*)(p+2); if (b >= 0) { ... }` in loop1): score REGRESSED 2 -> 11, target_insns=82, build_insns=81, rules_dropped=3, cheat_asm_stripped=23. Restored src to baseline after measurement; sandbox re-verified at score=2.

- [s7] [s7] Standalone instrumented-cc1 compile of raw-halfword form (tmp/grind/gnd_init_80041688/s7/s7raw_standalone.{c,s,greg,flow,sched,...}) confirms the FALSE-arm lbu order DOES flip to target's [b,r,g]: baseline .s FALSE 127-129 = [lbu $4,24 / lbu $2,25 / lbu $3,26]; s7raw .s FALSE 125-127 = [lbu $3,26 / lbu $4,24 / lbu $2,25]. Same target-order flip cheat10 produces. Mechanism (pseudo-78 fusion -> sched1 hazard-tag flip) reproduces via raw-halfword staging.

- [s7] [s7] .greg pseudo dispositions reveal loop1 RA cascade unique to raw-halfword (not present in cheat10): baseline pseudo 75(i)->4($a0), 76(p)->3($v1), 78(b)->3($v1); cheat10 IDENTICAL to baseline; s7raw pseudo 75(i)->6($a2), 76(p)->4($a0), 78(b)->3($v1). Raw-halfword form displaces p from $v1 to $a0 and i from $a0 to $a2 because b's intra-iter live range (lh -> bltz) conflicts with p's live range across the loop1 body (p used as base of lh AND for the p[1] store downstream). cheat10 avoids the cascade because `b = (X >= 0)` is a setcc/slt with near-zero intra-iter live range (def-use back-to-back), so greg keeps 76 and 78 both in $v1 via non-overlapping intra-iter allocation.

- [s7] [s7] Loop1 label placement diverges: baseline/cheat10 = `addu $3,$3,104` BEFORE `.L21` (first-iter pre-inc); .L21: `lh $2,2($3)` ... bnez ... `addu $3,$3,104` in DELAY SLOT (next-iter post-inc). s7raw = `.L5:` ABOVE `addu $4,$4,104`, then `lh $3,2($4)` — reorg.c chose a different loop rotation because the extra staged pseudo (78) interferes with the delay-slot fill of the loop back-branch. Adds another 2-3 emit diffs beyond the FALSE-arm flip.

- [s7] [s7] Net s7 forensic finding: raw-halfword staging IS a byte-non-neutral spelling of the cheat10 mechanism. The FALSE-arm flip is real (mechanism CONFIRMED to be reproducible via loop1-scope defs of source-level `b`) but comes with compensating loop1 RA/reorg regressions that raise the total sandbox score from 2 to 11. There is no in-loop1 spelling of the axis that is BOTH byte-neutral AND non-cheat: zero-intra-iter-live-range spellings are dead stores (cheat10 shape); non-zero-live-range spellings displace p from $v1 (s7 shape).

- [s7] s7 sandbox --disable all with raw-halfword form applied: score=11, target_insns=82, build_insns=81, rules_dropped=3, cheat_asm_stripped=23. Restored src, sandbox re-verified score=2.

- [s7] s7 standalone instrumented-cc1 compile: s7raw_standalone.s FALSE-arm color-lbu emission (lines 125-127) = [lbu $3,26 / lbu $4,24 / lbu $2,25] — byte-identical to cheat10_standalone.s FALSE 127-129, both matching target [b=$v1,r=$a0,g=$v0] order.

- [s7] s7 .greg pseudo dispositions: baseline 75→4($a0), 76→3($v1), 78→3($v1); cheat10 IDENTICAL to baseline for these; s7raw 75→6($a2), 76→4($a0), 78→3($v1). Raw-halfword displaces loop1 registers where cheat10 does not.

- [s7] s7 loop1 rotation divergence: baseline/cheat10 have `addu $3,$3,104` BEFORE `.L21` (pre-inc first iter) and again in delay slot of the loop back-branch (post-inc next iter); s7raw has `.L5:` label ABOVE `addu $4,$4,104`, then `lh $3,2($4)` — reorg.c picked a different rotation because the extra staged pseudo (78) interferes with delay-slot fill on the back-branch.

- [s7] s7 mechanistic explanation for the cheat10 vs s7raw divergence: cheat10's `b = (X >= 0)` compiles to setcc/slt with near-zero intra-iter live range (def-use back-to-back), so greg keeps pseudos 76(p) and 78(b) both in $v1 via non-overlapping intra-iter allocation. s7raw's `b = X; if (b >= 0)` gives b a 2-insn live range (lh through bltz) that overlaps p's live range across the loop1 body, forcing a greg conflict and hardreg split.

- [s7] s7 policy vetting: raw-halfword staging is a dead store to `b` for RA/scheduling coercion per [[no-new-park-categories]] cheats-by-any-spelling — same intent as loop1-boolean-stage-b-reuse.c, different spelling. The value stored to `b` in loop1 is immediately dead after `if (b >= 0)` (b is unread until the FALSE-arm reassignment overwrites it). Rejected on policy grounds even before the measurement showed it wasn't byte-neutral.

- [s8] s8 baseline sandbox --disable all: score=2, target_insns=82, build_insns=82, rules_dropped=3, cheat_asm_stripped=23 (unchanged from s1-s7).

- [s8] Fresh m2c decompile of asm/funcs/gnd_init_80041688.s (tmp/grind/gnd_init_80041688/s8/m2c_fresh.c): produces STRUCTURALLY DIFFERENT shape — no r/g/b locals in FALSE arm (loads inlined into OR expression); TRUE arm's func_8004881C call takes inlined lbu args; both arms assign to a SINGLE `var_a0_2` intermediate consumed by ONE gnd_load_tex(var_a0_2) OUTSIDE the if/else. Matches target's actual asm: one shared `jal gnd_load_tex` at .L800417B4 (TRUE arm reaches via `j .L800417B4`; FALSE arm falls through).

- [s8] m2c-shape variant (shared v + shared gnd_load_tex(v) outside arms): sandbox --disable all → score=12, build_insns=80 (regressed 2→12). jump2/find_cross_jump merged the trailing `or v0,v0,v1` (intermediate) and `or a0,a0,v0` (final, in delay slot) into a SHARED tail — collapsing 2 insns vs target. Target keeps arm-distinct final or's blocked from merge: TRUE `or a0,a0,v0` (v RIGHT), FALSE `or a0,v1,a0` (b LEFT in $v1).

- [s8] Inlined-loads FALSE-arm-only variant (drop r/g/b locals, keep separate gnd_load_tex per arm): sandbox score=2 (byte-identical to baseline). Combine folds inlined loads back to same RTL pre-sched1; no LUID/DAG effect.

- [s8] Inlined-loads BOTH-arms variant (also inline func_8004881C args): sandbox score=2 (unchanged). Load-inlining is inert for this function.

- [s8] m2c-shape disasm shared-tail region (tmp/grind/gnd_init_80041688/s8/m2c_shape.dis): `or v0,v0,v1; jal gnd_load_tex; or a0,a0,v0(delay)` — the shared tail spans FALSE's intermediate or + shared jal + shared final or. Target's shared region is just `jal gnd_load_tex; nop`. The m2c-shape collapses too much.

- [s8] Mechanistic finding: target achieves arm-distinct final-or shapes (blocking cross-jump merge past the jal) because b lands preserved in $v1 through the FALSE arm's ORs (`or a0,v1,a0` = b LEFT). This requires target's [b,r,g] lbu order — same sched1 chain-length wall as s1-s7. The rederive-via-m2c-shape path CONVERGES back to the same wall from a different angle rather than routing around it.

- [s8] Rederive-modality conclusion: fresh m2c does not reveal a new axis. The remaining unmeasured surfaces are (a) COMBINING axes (m2c-shape + loop1-raw-halfword; may have different RA than s7's arm-local raw-halfword form), (b) shared-v with |= statement chain per arm (may leave arm-distinct enough RTL to block jump2 merge past final or). Both are speculative combinatorial extensions of already-KILLED single axes.

- [s8] s8 baseline sandbox --disable all: score=2, target_insns=82, build_insns=82, rules_dropped=3, cheat_asm_stripped=23 (unchanged from s1-s7).

- [s8] s8 fresh m2c decompile (tmp/grind/gnd_init_80041688/s8/m2c_fresh.c): m2c reconstructs the FALSE arm without any r/g/b locals (loads inlined into OR expression), TRUE arm's func_8004881C call with inlined lbu args, AND crucially uses a SINGLE `var_a0_2` intermediate assigned in each arm consumed by ONE gnd_load_tex(var_a0_2) outside the if/else. This matches target's actual asm layout: one shared `jal gnd_load_tex` at .L800417B4 that both arms reach.

- [s8] s8 m2c-shape variant applied to src (shared v + shared gnd_load_tex(v)): sandbox score=12, build_insns=80. Regressed by 2 insns because jump2/find_cross_jump merged the final or's into shared position — build has `or v0,v0,v1; jal; or a0,a0,v0(delay)` shared while target keeps arm-distinct final or's (TRUE: `or a0,a0,v0` in `j` delay; FALSE: `or a0,v1,a0`).

- [s8] s8 inlined-loads FALSE-only variant: sandbox score=2 (unchanged). FALSE arm disassembly identical to baseline (still [r,g,b] order with intermediate r/g/b pseudos synthesized by combine).

- [s8] s8 inlined-loads BOTH-arms variant: sandbox score=2 (unchanged). Combine folds inlined loads back to same RTL; no effect on sched1.

- [s8] s8 mechanistic finding: target achieves arm-distinct final-or shapes (blocking cross-jump merge) because b lands in $v1 preserved through the FALSE arm's ORs (`or a0,v1,a0` = b LEFT operand). This requires target's [b,r,g] lbu order — the same sched1 chain-length priority wall s1-s7 hit. The m2c-shape rederive path converges back to the same wall from a different angle rather than routing around it.

- [s8] s8 conclusion: fresh-decompile axis measured dead. The remaining pseudo-78-fusion / cross-jump defeat surface for this function is narrow enough that s7's case-exhaustion proof (dead-store OR RA-cascade dichotomy) plus s8's shared-call-shape measurement close the standard rederive frontiers. What remains unmeasured: (a) using loop2's q-walker to establish live values reaching the FALSE arm that alter conflict-graph shape without direct b-fusion (speculative — likely dead-code-adjacent), (b) header-type-correction for g_player_ptrs' target type (currently s32* — would only matter if the u8 loads at +0x18/+0x19/+0x1A become non-byte, breaking match); (c) a whole owner escalation on the sched1 hazard-tag mechanism which s6 pinpointed.

- [s9] [s9] sandbox --disable all baseline: score=2, target_insns=82, build_insns=82, rules_dropped=3, cheat_asm_stripped=23 (unchanged from s1-s8).

- [s9] [s9] Frontier probe #2 variant (b): FALSE arm as `v = *((u8*)player+0x1A); v |= (s32)*((u8*)player+0x18)<<16; v |= (s32)*((u8*)player+0x19)<<8; gnd_load_tex(v);` with per-arm gnd_load_tex — sandbox --disable all: score=11, build_insns=81.

- [s9] [s9] Frontier probe #2 variant (c): reversed to `v = (s32)*((u8*)player+0x18)<<16; v |= (s32)*((u8*)player+0x19)<<8; v |= *((u8*)player+0x1A);` — sandbox --disable all: score=11, build_insns=81 (byte-identical to variant b — combine folds accumulation order).

- [s9] [s9] Frontier probe #2 variant (a): shared gnd_load_tex(v) outside arms + split-init FALSE — sandbox --disable all: score=11, build_insns=81.

- [s9] [s9] Variant (b) disasm FALSE arm (tmp/grind/gnd_init_80041688/s9/split_init_per_arm.dis): 15cc lbu v0,0x18; 15d0 lbu v1,0x1A (b lands in $v1!); 15d4 lbu a0,0x19; 15d8 sll v0,v0,0x10; 15dc or v1,v1,v0; 15e0 sll a0,a0,0x8; 15e4 jal gnd_load_tex; 15e8 or a0,v1,a0 (delay). Final delay-slot or IS `or a0,v1,a0` — matches target FALSE shape!

- [s9] [s9] Variant (b) TRUE arm still merges via `j 15e4; or a0,a0,v0 (delay)` — both arms terminate at shared jal at 15e4 with arm-distinct delay-slot or's. Cross-jump merged only the jal itself, not the final or (as split-init hoped) — but the FALSE arm lost one intermediate `or` that got hoisted into the jal delay slot.

- [s9] [s9] FALSE-arm lbu emission order in split-init form: [r=v0@0x18, b=v1@0x1A, g=a0@0x19] = [r,b,g]. Target is [b,r,g]. Split-init does NOT flip lbu order, though it DOES land b in $v1 for the final or. This confirms the sched1 chain-length priority wall (s1-s8) is orthogonal to the OR-tree structural axis: even when b's destination register matches target, the LOAD emission order stays chain-length-driven.

- [s9] [s9] Baseline src re-verified after all variants: sandbox --disable all -> score=2 (state restored).

- [s9] [s9] Sanctioned-form space now exhaustively swept: fresh-decompile (s8 m2c-shared-v) KILLED; sibling-shape-with-split-init-FALSE (s9) KILLED. Every remaining live-code respelling of the FALSE-arm OR-tree either (a) folds pre-sched1 via combine (s2-s3 named-intermediate/walking-pointer/struct-cast/split), (b) merges via jump2 when using shared-v shape (s8, s9), or (c) leaves the sched1 chain-length wall intact (s1-s5).

- [s10] s10 baseline sandbox --disable all: score=2, target_insns=82, build_insns=82, rules_dropped=3, cheat_asm_stripped=23 (unchanged s1-s9).

- [s10] s10 intra-BB init probe: `b = 0;` inserted at top of FALSE arm ahead of the r/g/b color-lbu triple → sandbox score=2, build_insns=82 (byte-identical to baseline). GCC DCE eliminates the `b = 0;` store before flow.c/life_analysis coalesces defs; pseudo 78's live range is NOT widened; sched1 insn_priority hazard-tag on b-lbu stays boosted. Axis INERT.

- [s10] s10 case-exhaustion synthesis of pseudo-78-fusion surface: (a) loop1-scope def — s7 dichotomy KILLED (dead-store cheat OR RA cascade); (b) pre-branch hoist — s5 KILLED (callee-save spill across func_800486FC); (c) intra-BB pre-color init — s10 KILLED (DCE'd inert). No fourth site exists for widening pseudo 78's live range to reach the FALSE-arm b-lbu. The sched1 hazard-tag lever has zero remaining sanctioned surface.

- [s10] Rejected form banked: memory/grind/gnd_init_80041688/rejected/intra-bb-b-zero-init.c.


- [s10] s10 baseline sandbox --disable all: score=2, target_insns=82, build_insns=82, rules_dropped=3, cheat_asm_stripped=23 (unchanged s1-s9).

- [s10] s10 intra-BB init probe (`b=0;` FALSE-arm-local, ahead of color triple): score=2, byte-identical to baseline. DCE eats the store before flow.c fusion runs; pseudo 78 not widened; hazard-tag on b-lbu stays boosted.

- [s10] s10 case-exhaustion synthesis: three CFG sites for widening pseudo 78's live range are ALL measurement-dead — loop1 (s7 dichotomy), pre-branch (s5 spill), intra-BB (s10 DCE). No fourth site exists.

- [s10] Src restored after s10 probe; sandbox re-verified at score=2.

- [s10] Rejected form banked: memory/grind/gnd_init_80041688/rejected/intra-bb-b-zero-init.c.

- [s10] Cumulative rejected bank now 17 forms; cumulative KILLED hypothesis families: statement-reorder, struct-cast, walking-pointer, ternary-hoist, block-local-split, named-intermediate, cp-block-local, cp-function-scope, LIVE-b-hoist-above-loops, color-byte-pre-load, permuter chassis-1 basin (dual-seed), raw-halfword-in-loop1, m2c-shared-v-shared-call, inlined-loads-FALSE-only, inlined-loads-both-arms, split-init-FALSE (a/b/c), intra-BB-b-init.

- [s11] s11 baseline sandbox --disable all: score=2, target_insns=82, build_insns=82, rules_dropped=3, cheat_asm_stripped=23 (unchanged from s1-s10).

- [s11] s11 variant A (goto+label between color-triple and gnd_load_tex, FALSE arm): sandbox score=2, build_insns=82 — byte-identical to baseline.

- [s11] s11 variant B (labeled entry before color triple, FALSE arm): sandbox score=2, build_insns=82 — byte-identical to baseline.

- [s11] s11 variant C (distinct trailing labels in BOTH arms via `goto x; x:;`): sandbox score=2, build_insns=82 — byte-identical to baseline.

- [s11] GCC 2.7.2 jump.c strips redundant labels (no cross-referencing jump) and elides `goto NEXT_STATEMENT;` forms before jump2/reorg/find_cross_jump see them. All three label-placement variants produce identical RTL to baseline at the jump2 entry point.

- [s11] s11 src restored after all probes; sandbox re-verified at score=2 (rules_dropped=3, cheat_asm_stripped=23, build_insns=82).

- [s11] SYNTHESIS: the cross-jump structural axis surface for this function is now complete — shared-tail-merged forms (s8 m2c-shared-v, s9 split-init variants a/b/c) regress via genuine insn-collapse (2→11/12); non-merged label forms (s11) are stripped inert by jump.c. There is no intermediate label form that both survives jump.c AND steers jump2 differently than the baseline.

- [s11] Combined with s10's pseudo-78-fusion case-exhaustion (loop1 defs KILLED s7; pre-branch hoist KILLED s5; intra-BB init KILLED s10), every sanctioned lever axis reachable from within the function body is now measurement-dead. The only remaining un-scoped angle is header/type-level (Kengo sibling-scrape → header-type-correction for g_player_ptrs' target type), which is orthogonal to the structural modality.

- [s11] Cumulative rejected bank now 18 forms (s11 adds label-placement-goto-arm-tails.c).

- [s12] [s12] baseline sandbox --disable all: score=2, target_insns=82, build_insns=82, rules_dropped=3, cheat_asm_stripped=23 (unchanged from s1-s11).

- [s12] [s12] loop1 = do-while: score=7, build_insns=83 (+1 insn vs baseline).

- [s12] [s12] loop1 = while(top-tested): score=7, build_insns=83 (+1 insn; identical regression signature to do-while).

- [s12] [s12] loop2 = while(head-tested): score=11, build_insns=84 (+2 insns; worse than loop1-shape variants).

- [s12] [s12] Both natural loop1 spellings produce identical regression (score=7/insns=83) — GCC 2.7.2 lowers do-while and while to functionally equivalent RTL that costs +1 insn vs manual goto/label.

- [s12] [s12] loop2's exit-in-middle shape is strongly optimized by manual goto/label; head-tested while loses 2 insns via the compensating loop-exit branch structure.

- [s12] [s12] Baseline src restored after all three probes; sandbox re-verified score=2 (target_insns=82, build_insns=82).

- [s12] [s12] Rejected forms banked: memory/grind/gnd_init_80041688/rejected/{loop1-do-while.c, loop1-while-top-tested.c, loop2-as-while.c}.

- [s12] [s12] SYNTHESIS: loop-shape axis measured dead by DIRECT MEASUREMENT across all three natural-loop spellings — the sole un-probed sub-axis after s11's label-placement synthesis. Confirms s11's proof-of-completion for the in-function structural surface empirically: no natural-loop restructuring improves (or leaves neutral) the sandbox score. Combined with s10's pseudo-78-fusion case-exhaustion (loop1-defs KILLED s7, pre-branch KILLED s5, intra-BB KILLED s10) and s11's cross-jump axis completion (m2c-shared-v KILLED s8, split-init KILLED s9, label-placement inert s11), every in-function structural axis is now measurement-dead.

- [s12] [s12] Cumulative rejected bank now 21 forms (s12 adds three loop-shape variants).

- [s12] [s12] The ledger's frontier action (OWNER-ESCALATION filing at docs/grind/decisions.md) is unchanged and now empirically justified: the last un-measured structural sub-axis (loop shape) has been directly probed and killed. No sanctioned in-function lever axis remains.

- [s13] [s13] Sandbox baseline re-verified: sandbox --disable all -> score=2, target_insns=82, build_insns=82, rules_dropped=3, cheat_asm_stripped=23 (unchanged from s1-s12).

- [s13] [s13] Chassis-2 permuter (tmp/grind/gnd_init_80041688/s13/perm/, label=s13-chassis2-splitinit): base_score=625, ~3112 iters, 6 novel finds ranging from score=390 to score=610. Best (output-390-1) = variable-reuse of previously-unused `r` local for staged shift expression. All finds cheat-family or structurally worse than baseline.

- [s13] [s13] Chassis-3 permuter (tmp/grind/gnd_init_80041688/s13/chassis3/, label=chassis3-shared-v-shared-call): base_score=745, ~1200 iters, ~30 novel finds ranging from score=185 to score=745. Best (output-185-1) = OR-tree split `v=(r<<16)|(g<<8); v=b|v;` — no byte-improvement. All finds are OR-tree partitions, decl-order shuffles, or variable-reuse.

- [s13] [s13] Comparative basin quality: chassis-1 best=10 (sandbox 2 masked+ dead-store cheat, s4/s5), chassis-2 best=390, chassis-3 best=185. Chassis-2 is 39x worse than chassis-1; chassis-3 is 18x worse. Neither approaches chassis-1 basin let alone sub-baseline.

- [s13] [s13] All score=2-baseline C form variants documented in prior ledger (s2 struct-cast, s3 named-intermediate-rg, s3 block-local cp, s8 inlined-loads FALSE-only, s8 inlined-loads both-arms) produce byte-identical sandbox output to chassis-1 baseline, meaning their pre-sched1 RTL collapses to chassis-1's via combine/CSE. Permuter runs on any of them would search chassis-1's already-exhausted neighborhood.

- [s13] [s13] Cumulative permuter exhaustion: chassis-1 (base=40) dual-seed ~50k iters s4+s5; chassis-2 (base=625) ~3k iters s13; chassis-3 (base=745) ~1.2k iters s13. Total across all chassis and seeds: ~54k iters, ~50 distinct novel finds, zero legitimate sub-baseline forms.

- [s13] [s13] Baseline sandbox re-verified after all measurements: score=2 (state clean; no src edits held).

- [s13] [s13] Two rejected forms banked this session: memory/grind/gnd_init_80041688/rejected/chassis2-splitinit-permuter-basin.c (chassis-2 shape + basin analysis), chassis3-shared-v-shared-call-permuter-basin.c (chassis-3 shape + basin analysis).

- [s14] baseline sandbox --disable all -> score=2, target_insns=82, build_insns=82, rules_dropped=3, cheat_asm_stripped=23 (unchanged from s1-s13)

- [s14] variant U32 (extern u32 g_player_ptrs[]) sandbox --disable all -> score=2, build_insns=82 — byte-identical to baseline

- [s14] variant VOIDPTR (extern void *g_player_ptrs[]) sandbox --disable all -> score=2, build_insns=82 — byte-identical to baseline

- [s14] baseline sandbox --disable all re-verified after both probes: score=2 (src state clean)

- [s14] Kengo/*.txt grep for g_player_ptrs and D_800A9A10: 0 hits — no sibling evidence

- [s14] tmp/decomp_me_corpus/ grep for 800A9A10: 5 hits, ALL unrelated-game false positives (aliased addresses)

- [s14] src/text1a.c use-site census: 12 hits (2 extern decls + 10 use sites); ZERO have signed-specific semantics — every use is NULL check (`== 0`), pointer cast (`(s16*)`, `(s32*)`), or zero write

- [s14] four-prong test on U32 flip: (a) FAIL zero signed sites, (b) FAIL no compensating cast is functionally necessary, (c) PASS one-extern-per-file (two identical edits, form-clean), (d) FAIL no casts eliminated — three of four prongs fail

- [s14] four-prong test on VOIDPTR flip: OUT OF SCOPE per rule line 183-185 (scalar<->pointer flip excluded from rule regardless of measurement)

- [s14] mechanistic disconnect: s6-CONFIRMED lever is sched.c insn_priority hazard-tag on pseudo-78 in BB18; player is loaded once at entry and thereafter aliased via u8*; g_player_ptrs' element type has zero downstream RTL effect on BB18 — the byte-identical measurement result confirms this predicted disconnection

- [s14] existing decisions.md entry 2026-07-20 13:32 (docs/grind/decisions.md:1019) previously ruled FAIL on canonical-asm authorization, directing this axis be exhausted first; that direction is now discharged
