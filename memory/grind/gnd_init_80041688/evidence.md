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
