# Evidence bank — func_80022F34

- WIP rejected_form: {'form': 'inline the call args (drop idx1/val1 intermediates)', 'score': 11, 'reason': 'not a cheat; no change to the callee-saved promotion'}

- WIP rejected_form: {'form': 'switch directly on D_800A38DC (drop val temp)', 'score': 11, 'reason': 'not a cheat; no change'}

- WIP rejected_form: {'form': 'do/while loop instead of goto', 'score': 26, 'reason': 'not a cheat; worse'}

- == imported from memory/wip notes.md ==
# func_80022F34 — WIP (callee-saved over-promotion / frame plateau)

## TL;DR (2026-06-14)
HEAD carries 11 regfix rules: 10 frame-offset substs (prologue idx 0-7 +
epilogue idx 59-63, shifting the frame down) + 1 `insert "nop" @ 36`. Unpinned
masked floor = **11** and NO pure-C form found this session beats it. Two
distinct issues:

1. **maspsx .L-label load-delay nop** (the `insert nop @ 36`): store-value
   consumer variant `lhu v0,0(s2)` -> `.L` switch-merge label -> `sh v0,8(a0)`.
   This is a LEGITIMATE pure-C retirement path via
   [[maspsx-label-nop-gate]] (add func to `maspsx_label_nop_funcs.txt`, retire
   drops the rule). NOT the blocker.

2. **Callee-saved over-promotion (the real blocker):** compiling pure C, GCC
   2.7.2 allocates SEVEN callee-saved regs (s0,s1,s2,s3,s4,s5,ra) and a -48
   frame; the TARGET uses only FOUR (s0,s1,s2,ra) and a -32 frame. GCC promotes
   3 extra long-lived values (the &D_801027BC base, val1, an index temp) to
   s3/s4/s5 where target keeps them caller-saved. The 10 frame-offset substs +
   the register renames are correcting this. No pure-C restructuring tried this
   session changes the promotion.

## Ruled out this session (all >= floor 11, none a cheat)
- inline the call args (drop named idx1/val1 intermediates) — floor 11
- switch directly on D_800A38DC (drop `val` temp) — floor 11
- do/while loop instead of goto — floor 26 (worse)

## Why blocked, not matched
COMPLETED-C needs 0 rules + 0 pins + SHA1 == oracle. Frame-padding to coerce
the size is FORBIDDEN ([[dead-vars-local-array]]) and anyway the divergence is
GCC over-promoting to callee-saved (frame too BIG, not too small) — no
"subtract a local" lever exists. The maspsx nop is retirable but the
callee-saved over-promotion is a register-allocation plateau.

## Next hypotheses
- Directed permuter from baseline targeting the s3/s4/s5 demotion (register
  allocation is exactly the permuter's domain; caveat: high-cascade-adjacent
  file code6cac.c).
- ALLOCDBG / -da RTL dump to identify WHICH values GCC promotes to s3/s4/s5 and
  whether a lifetime-shortening restructure can keep them caller-saved.
- If the promotion is immovable: canonical-asm authorization (the rules encode
  a register-allocation the compiler won't produce).


## s1 (recon, 2026-07-23) — THE PRIOR "s3/s4/s5 OVER-PROMOTION" DIAGNOSIS IS FALSE

- [s1] BASELINE re-measured at HEAD: sandbox --disable all = 11, build_insns 69,
  target_insns 70, rules_dropped 11, canonical verdict C. Floor confirmed 11.

- [s1] **THE WIP/LEDGER CLAIM (GCC allocates 7 callee-saved s0-s5,ra, -48 frame,
  over-promotes 3 values to s3/s4/s5) IS WRONG for HEAD's source.** The honest
  cheat-stripped build uses ONLY s0,s1,s2,ra with a -0x28 (40) frame. The BODY
  is BYTE-IDENTICAL to target. The entire 11-distance decomposes as:
  10 diffs = frame is 8 BYTES TOO BIG (build vars=8 vs target vars=0; affects the
  sp adjust x2 + all 8 save/restore offsets), and 1 diff = the maspsx .L-label
  load-delay nop (lhu v0,0(s2); .L80022FD0: nop; sh v0,8(a0) — store-value
  consumer, retirable via maspsx_label_nop_funcs.txt).

- [s1] **THE +8 IS A PHANTOM FRAME SLOT (reload stale-ref bug — see
  func_80037540 s3 mechanism).** cc1 -da lreg dump: "Register 100 ... block 10;
  ST_REGS or none; pointer." reg 100 = the &D_801027BC[idx1*5] ADDRESS temp
  (insn 102: reg100 = symbol_base + idx1*20; insn 104: val1 = *reg100). combine
  FOLDS these into per-access `lw a1,%lo(D_801027BC)(at)` addressing (matching
  target) but leaves reg 100 stranded via a stray `(use (reg:SI 100))` emitted at
  code_label 85 (the SWITCH MERGE label). Stale reg_n_refs -> ST_REGS ->
  alter_reg gives it a frame slot nothing references -> vars=8.

- [s1] **THIS IS THE INVERSE of the sibling wall (func_80049A2C / func_80037540):
  they must ADD a phantom +8 slot (hard, owner-gated); WE must REMOVE one
  (legitimate clean-C, not a coercion).**

- [s1] Instrument: standalone reproducer tmp/grind/func_80022F34/s1/probe_base.c
  + reuse of tmp/grind/func_80037540/s2/build.sh prints cc1's `.frame` comment
  (# vars=V) — the direct gradient on the phantom (the sandbox score cannot
  separate wrong-frame from wrong-codegen).

- [s1] KILLED (all vars=8, phantom persists): idx1/idx2 declared s32; val s16;
  (s32)-cast the *5 multiply; separate pointer var for the a0 reload; val1
  computed AFTER idx2 (late a1 order); if-else chain instead of switch;
  base pointer declared INSIDE the loop (vF) or per-use (vG). None touch the
  strand. Reordering/inlining consistent with the banked "inline args -> 11".

- [s1] **vD (function-scope loop-invariant `s32 *b = &D_801027BC;` shared by both
  accesses) KILLS the phantom: vars=0 — BUT promotes b to a 5th callee-saved reg
  (regs=5/0), which the target lacks.** Banked
  rejected/hoisted-base-ptr-kills-phantom-but-promotes-5th-reg.c. This proves the
  phantom is 100% dissolvable in pure C; the tension is phantom-vs-extra-reg.

- [s1] **vH (m2c-faithful, FULLY INLINED: both call args inline `(&D_801027BC)[..]`,
  a0-reload as double-indirection `*(u8**)a0` in arg3, NO named temps, goto loop)
  -> vars=0, regs=4/0, frame 0x20 — EXACTLY the target frame, phantom GONE, no
  extra reg.** Applied to src + full-TU sandbox --disable all = 11 with the
  PROLOGUE/EPILOGUE now BYTE-MATCHING target (confirmed by objdump). Saved as
  memory/grind/func_80022F34/candidate.c. m2c reconstruction (mipsel-gcc)
  corroborates this shape (inlines both D_801027BC accesses into the call, a0
  reload as temp_a0->unk0->unk4A double-indirection).

- [s1] **vH TRADES the phantom for a base-CSE.** With the frame now correct, the
  remaining 11 are ENTIRELY in the D_801027BC access region: GCC CSEs
  &D_801027BC into a shared base (la $a1; addu ...,$a1; lw ...,0($n)) while
  target re-materializes %hi/%lo(D_801027BC) PER ACCESS; plus the a0 self-reload
  lands in $v0 (lw v0,0(a0); lh a0,74(v0)) vs target's in-place $a0. Standalone
  vH ALSO CSEs (la $5) — so this is not full-TU register pressure, it's the
  inlined form's CSE visibility. CORE TENSION: per-access %lo fold <=> phantom
  strand (HEAD/probe_base); CSE'd base <=> no phantom but wrong body (vH). Target
  has BOTH per-access %lo AND no phantom — a form not yet found.

- [s1] do/while loop (m2c's loop form) is STRICTLY WORSE: vI -> regs=6/0
  (promotes more). Keep the goto loop. (Matches banked "do/while -> 26".)

- [s1] src REVERTED to HEAD this session: HEAD's 10 frame-offset regfix substs
  were correcting the OLD phantom frame; with vH the frame is already correct, so
  leaving vH + the stale substs would CORRUPT the oracle build. The DONE path for
  vH must retire those 10 substs + allowlist the nop (queue done/retire — outside
  a grind session's surface). candidate.c preserves vH for the next session.

- [s1] Artifacts: tmp/grind/func_80022F34/s1/{probe_base.c, vD_baseptr.c, vH_m2c.c,
  build_now.txt, target_now.txt, dumps/base.i.lreg, dumps/base.i.combine,
  dumps/base.i.greg}. rejected/named-temps-per-access-lo-but-phantom-frame.c,
  rejected/hoisted-base-ptr-kills-phantom-but-promotes-5th-reg.c.

- [s1] Baseline sandbox --disable all = 11 (build 69 / target 70 insns), verdict C, 11 rules_dropped.

- [s1] Honest cheat-stripped build uses s0,s1,s2,ra + -0x28 frame; body byte-identical to target. Floor = frame 8-bytes-too-big (10 diffs) + 1 maspsx .L-label load-delay nop.

- [s1] The +8 is a reload stale-ref phantom slot; phantom pseudo = reg 100 (the &D_801027BC[idx1*5] address), stranded by a stray (use) combine emits at code_label 85 (switch merge).

- [s1] This is the INVERSE of siblings func_80049A2C/func_80037540: they must ADD a phantom slot (owner-gated); this function must REMOVE one (legitimate clean-C).

- [s1] KILLED (all vars=8): idx s32; val s16; cast multiply; separate reload ptr; late val1; if-else instead of switch; base ptr inside loop / per-use.

- [s1] vD (function-scope shared base ptr) kills the phantom (vars=0) but promotes it to a 5th callee-saved reg the target lacks.

- [s1] vH (m2c-faithful fully-inlined, goto loop) = vars=0 / regs=4/0 / frame 0x20 = exact target frame; saved as candidate.c. m2c reconstruction corroborates the inlined shape.

- [s1] vH residual 11 = base-CSE (la $a1 shared base vs target's per-access %hi/%lo) + a0 self-reload in $v0 vs target $a0. Standalone vH also CSEs, so it's the inlined form's CSE visibility, not full-TU pressure.

- [s1] CORE TENSION: per-access %lo fold <=> phantom strand; CSE'd base <=> no phantom but wrong body. Target has BOTH per-access %lo AND no phantom.

- [s1] do/while loop is strictly worse (regs=6/0); keep the goto loop.

- [s1] src reverted to HEAD: HEAD's 10 frame-offset regfix substs correct the OLD phantom frame; with vH the frame is already correct so those substs would corrupt the oracle. DONE path = vH + retire the 10 substs + maspsx_label_nop_funcs.txt for the nop.

- [s1] The permuter is a VALID lever here (unlike the siblings) because vH's residual is NOT sp-offset-only; the frame is correct so the stack-normalized scorer sees the real reg/CSE diffs.

## s2 (structural, 2026-07-23) — per-access<->phantom coupling MAPPED across ~24 forms; floor firmly 11 on the structural axis

- [s2] Baseline reconfirmed: HEAD (base named-temp form) sandbox --disable all = 11, build_insns 69, target 70, 11 rules_dropped.

- [s2] **base's BODY is BYTE-PERFECT vs target** (2x per-access `lw D_801027BC(idx)` + in-place a0 reload `lw $4,0($4)` in $a0). Its ENTIRE 11 = the +8 phantom frame slot (10 diffs: sp-adjust x2 + 4 saves + 4 restores at wrong offsets) + 1 maspsx nop. Removing the phantom (keeping the body) would drop the floor to 1 (the nop, retirable via maspsx_label_nop_funcs.txt on the DONE path).

- [s2] **The phantom (reg100 strand) and per-access are COUPLED through the val1 sub-block + lifetime.** Measured .frame(vars=)/per-access(count of `lw D_801027BC`) across ~24 structural forms:
  * val1 loaded AFTER switch, BEFORE reload, in a sub-block (base/vFLAT/vFLAT2/vARR/vQ/vPTR): 2x per-access + vars=8 (STRAND). Right instruction ORDER.
  * val1 loaded AFTER reload/idx2 (vW/vMIRROR/vK1/vU/vY/vP/vH/vSPLIT): CSE'd `la $5` base (0-1x per-access) + vars=0. Frame byte-matches target.
  * val1 loaded BEFORE the switch (vPRESW): 2x per-access + vars=0 — the ONLY form with BOTH — but hoisting reorders the whole chain and pushes a0 off $a0 => real sandbox = 28. Rejected.

- [s2] **The reg100 strand mechanism (combine note-relocation wart).** cse2 has reg95=symbol_ref(D_801027BC), reg99=idx1*20, reg102: reg100=reg99+reg95, reg104: val1=*reg100. combine folds reg104 into `lw val1,D_801027BC(reg99)` (per-access) and DELETES reg100's def, but strands a bare `(insn 163 (use (reg:SI 100)) REG_DEAD)` placed UPSTREAM of reg100's own def — right after switch-merge code_label 85, before the val1 sub-block's NOTE_INSN_BLOCK_BEG (note 87). reg100 is thus in greg's "6 regs to allocate: 100 80 75 72 74 73" list, gets NO hard reg (absent from Register dispositions), => alter_reg reserves an unreferenced slot => vars=8. Only val1 (long lifetime, loaded early in a sub-block, USED in the outer-scope call) strands; val2 (loaded late in the same block, consumed in-block) folds cleanly.

- [s2] **Frame-correct CSE form == base's floor.** vMIRROR (target-order, both CSE, vars=0) and vSPLIT (val2 per-access + val1 CSE, vars=0) BOTH = sandbox 11, build_insns 64. The CSE'd base + a0-reload-in-$v0 + nop sum to 11 — no better than base's phantom. Neither regime is < 11.

- [s2] The per-access difference is a genuine fork cse2 divergence: standalone AND full-TU our-fork cse2 keeps &D_801027BC in one reg (`la`); cc1psx re-materializes %hi/%lo per load. base gets per-access only via register-pressure SEPARATION (val1's `la` reg reused across the long gap), NOT a cse2 barrier — and that separation is exactly what strands reg100.

- [s2] KILLED forms not previously banked: val1-before-switch (vars0+per-access, reorder=28); val1 in if-body no-subblock (vSPLIT/vSPLIT2/vSPLIT3 => CSE); flat single-block (vFLAT/vFLAT2 => strand; vFLAT3 => CSE); array-typed D_801027BC (vARR => strand, decl type irrelevant); explicit displaced pointer p1=&(&D)[idx1*5] (vPTR => strand); named scaled-index k1 (vK1/vMIRROR => CSE); idx1-before-switch val1-after (vIDXSW => val1 CSE).

- [s2] Artifacts: tmp/grind/func_80022F34/s2/{probe.sh, show.sh, _prelude.h, base.c, vH.c, vP.c vU.c vW.c vY.c vK1.c vMIRROR.c vSPLIT*.c vFLAT*.c vPRESW.c vARR.c vPTR.c + their .s}. rejected/val1-before-switch-per-access-vars0-but-reorder-28.c, rejected/frame-correct-cse-base-still-11.c.

- [s2] Baseline sandbox --disable all = 11 (build 69 / target 70), 11 rules_dropped, verdict C.

- [s2] base's body is BYTE-PERFECT vs target (2x per-access `lw D_801027BC(idx)` + in-place a0 reload `lw $4,0($4)` in $a0); the whole 11 = +8 phantom frame slot (10 diffs) + 1 maspsx nop.

- [s2] The phantom = reg100 (val1's folded address) stranded by combine placing a bare `(use reg:SI 100)` upstream of its def at switch-merge code_label 85 (before the val1 sub-block's NOTE_INSN_BLOCK_BEG); reg100 gets no hard reg -> unreferenced frame slot -> vars=8.

- [s2] per-access and phantom are COUPLED via val1's sub-block lifetime: val1 loaded early-in-subblock => per-access + strand (base); val1 loaded late => cse2 shares `la` base => vars=0 but not per-access (vH/vMIRROR); val1 before switch => vars=0 + per-access but gross reorder => sandbox 28.

- [s2] All non-reordered structural regimes floor at exactly 11: base (phantom body) 11, vMIRROR/vSPLIT (frame-correct CSE) 11.

- [s2] Only val1 strands (long lifetime, used in outer-scope call); val2 (loaded late, consumed in-block) folds clean => the strand is a lifetime/scope-crossing artifact, not inherent to the D_801027BC access.

- [s2] Type of D_801027BC (scalar+& vs array[]) is irrelevant to the strand (vARR strands identically); explicit displaced pointer (vPTR) strands too.

- [s2] The per-access gap is a genuine fork cse2 divergence: standalone AND full-TU our-fork cse2 keeps &D_801027BC in one reg (`la`); cc1psx re-materializes %hi/%lo per load. base gets per-access only via register-pressure separation, which is exactly what strands reg100.

## s3 (structural, 2026-07-23) — control-boundary re-association axis KILLED; reg100 strand is CFG-invariant, not statement-order

- [s3] Baseline reconfirmed: current src/code6cac.c sandbox --disable all = 11 (build_insns 69, target 70, rules_dropped 11, verdict C). base probe: vars=8, regs=4/0, sp=-40, D_801027BC-refs x2 (per-access, phantom strand). Unchanged from s1/s2.

- [s3] **NEW structural axis measured: control-boundary re-association (do-while(0) wrapping).** s2 exhausted the val1-PLACEMENT axis (where val1 loads relative to switch/reload); s3 attacked the ORTHOGONAL axis — inserting a control boundary between the switch-merge label and the val1 sub-block to relocate combine's stranded `(use reg100)` note off the merge label (the H-C frontier's two untried probes).
  * **vTAIL** (switch wrapped in do{}while(0)): vars=8, per-access x2 — STRAND PERSISTS. asm is base-equivalent modulo an arbitrary tbl/i register-name swap ($17<->$18); no change to the strand or frame.
  * **vVALDW** (val1 sub-block wrapped in do{}while(0)): vars=8, per-access x2 — STRAND PERSISTS, and STRICTLY WORSE: the scheduler no longer fills the `lh $2,74($4)` load-delay slot with the a0 reload (`lw $4,0($4)` moves 3 insns later, leaving a maspsx `#nop`). base's tighter schedule dominates.

- [s3] **CONCLUSION: the reg100 combine strand is CFG/scope-invariant.** do-while(0) is optimized away before combine, so it creates NO real CFG edge/label between the switch merge and the val1 def — combine still places `(use reg100)` on the switch-merge code_label. To actually separate them at the CFG level requires a REAL branch/label between switch and val1, which necessarily adds a body insn (breaks the byte-perfect body). The strand is therefore not removable by any statement-order OR control-boundary structural lever. This CONFIRMS + EXTENDS s2's KILL: BOTH structural axes (placement + control-boundary) are dead.

- [s3] The only vars=0+per-access datapoint remains vPRESW (val1 def before switch merge → no strand) which reorders the whole chain → sandbox 28. That the strand vanishes ONLY when val1's def is lexically before the merge label (not when merely wrapped in a scope after it) pins the mechanism to def-vs-label ordering, which is fixed by the a0-reload data dependency (val1 must live across the reload) — an algorithmic, not stylistic, constraint.

- [s3] Structural axis is exhausted. Remaining live axes are NON-structural (next session's modality): (a) permuter from base targeting the reg100 strand specifically (residual not sp-offset-only → scorer valid); (b) cse.c symbol_ref-cost study to force per-access re-materialization on a frame-correct base without the long val1 lifetime. Artifacts: tmp/grind/func_80022F34/s3/{probe.sh, _prelude.h, base2.c/.s, vTAIL.c/.s, vVALDW.c/.s}.

- [s3] Baseline reconfirmed: src/code6cac.c sandbox --disable all = 11 (build_insns 69, target 70, rules_dropped 11, verdict C); base probe vars=8 regs=4/0 sp=-40 per-access x2.

- [s3] vTAIL (switch in do-while(0)): vars=8, per-access x2 - strand persists; asm base-equivalent modulo an arbitrary tbl/i register-name swap.

- [s3] vVALDW (val1 block in do-while(0)): vars=8, per-access x2 - strand persists AND worse (a0 reload lw $4,0($4) moves 3 insns later, leaving a maspsx #nop where base fills the lh load-delay slot).

- [s3] do-while(0) is optimized away before combine, so it creates no real CFG edge between switch-merge and the val1 def; combine still anchors (use reg100) on the switch-merge code_label. The reg100 strand is CFG/scope-invariant.

- [s3] The strand vanishes ONLY when val1's def is lexically before the merge label (vPRESW: vars=0+per-access) which reorders the whole chain to sandbox 28 - and that ordering is fixed by the a0-reload data dependency (val1 must live across the reload), an algorithmic not stylistic constraint.

- [s3] CONCLUSION: both structural axes are dead - s2 killed val1-placement across ~24 forms; s3 kills control-boundary re-association. No statement-order or scope lever removes the +8 phantom without breaking the byte-perfect body.

## s4 (permuter, 2026-07-23) — H-A permuter axis KILLED on TWO chassis; corroborates the s1-s3 structural kills

- [s4] Built a codegen-faithful decomp-permuter workspace (tmp/perm_22F34) using the real code6cac pipeline (cpp full-defs | cc1 | prologue_fix | maspsx --expand-div | fix_lwl | multu_pad; regfix/asmfix cheat stages omitted since sandbox --disable all strips them). GOTCHA banked: the repo abs-path contains spaces ("Bushido Blade 2 Decompile") so cc1's `.file` directive splits to >3 tokens and crashes maspsx line 940 (`_, num, filename = line.split()`); strip the `.file` line before maspsx. target.o built at offset 0 from asm/funcs. base.o validated: frame -40 vs target -32 (+8 phantom), save/restore offsets +8, +1 maspsx nop — the exact expected residual.

- [s4] Campaigns run --stack-diffs (DEFAULT) so the +8 phantom frame stays VISIBLE to the scorer (without it the sp-offset residual normalizes to score 0 and the permuter couldn't gradient on the phantom).

- [s4] **base chassis (weighted base score 174, byte-perfect body + phantom frame): 30640 iterations, ZERO output dirs (nothing beat 174).** Random codegen mutation over the byte-perfect base CANNOT remove the reg100 combine strand. This is the CLOSEST chassis (weighted 174) and the permuter found no sub-174 form.

- [s4] **vH chassis (weighted base score 1250, frame-correct CSE'd body): best find = score 224** (rejected/permuter-vH-basin-best-still-224.c). The permuter only descends toward and re-finds the KNOWN base/vSPLIT/vH classes (named val1 temp + inlined arg3 => CSE'd `la` base, frame-correct but NOT per-access); it NEVER produces target's (per-access %hi/%lo AND vars=0) form. 224 > base 174, so vH is a strictly worse basin.

- [s4] **CONCLUSION: the permuter (H-A top frontier lead) is KILLED.** Neither the byte-perfect base (weighted 174) nor the frame-correct vH (weighted 1250) yields any form approaching a match; the absolute best find across ~35k iters on both chassis is 224, worse than base's own 174. This mechanically corroborates s1-s3: the per-access<->phantom coupling is a fork-level cse2+combine interaction, not reachable by any C the permuter can spell. Both campaigns harvested + stopped (0 orphans); src unchanged (floor 11). Artifacts under tmp/grind/func_80022F34/s4/ + tmp/perm_22F34{,_vH}/.

- [s4] Remaining live axis is H-D (non-permuter): read tools/gcc-2.7.2/cse.c symbol_ref/constant CSE cost model to find the C-visible condition that makes our fork's cse2 re-materialize %hi/%lo per load WITHOUT the long val1 lifetime (which is what strands reg100). s2 already noted per-access arises ONLY via register-pressure separation of the two `la` loads, and that separation IS what strands reg100 — so H-D must find a cse2 cost lever that decouples them; this is a cse.c-forensics session, not permuter/structural.

- [s4] Built a codegen-faithful permuter workspace using the real code6cac pipeline (cpp full-defs | cc1 | prologue_fix | maspsx --expand-div | fix_lwl | multu_pad); regfix/asmfix omitted (sandbox --disable all strips them). base.o validated: frame -40 vs target -32 (+8 phantom), save/restore offsets +8, +1 maspsx nop - the exact expected residual.

- [s4] GOTCHA (banked): the repo abs-path contains spaces ('Bushido Blade 2 Decompile'), so cc1's .file directive splits to >3 whitespace tokens and crashes maspsx line 940 (`_, num, filename = line.split()`); strip the .file line before maspsx in any standalone/permuter pipeline for this repo.

- [s4] Campaigns run --stack-diffs (default) so the +8 phantom frame stays scorer-visible; without it the sp-offset residual normalizes to 0 and the permuter could not gradient on the phantom.

- [s4] base chassis (weighted 174, byte-perfect body + phantom frame): 30640 iterations, ZERO sub-174 finds. This is the CLOSEST chassis and the permuter found no improvement.

- [s4] vH chassis (weighted 1250, frame-correct CSE'd body): best find score 224 (rejected/permuter-vH-basin-best-still-224.c) = base's named-temp class re-derived from vH's chassis; never target's per-access+no-phantom form.

- [s4] Mechanically corroborates s1-s3: the per-access<->phantom coupling is a fork-level cse2+combine interaction, not reachable by any C the permuter can spell. Both campaigns harvested+stopped (0 orphans); src unchanged (sandbox --disable all still 11, git status clean).

- [s4] structural (s2 placement across ~24 forms + s3 control-boundary) AND permuter (s4 two chassis) axes are all now measured dead; only H-D (cse.c symbol_ref cost forensics, non-permuter) remains un-measured.

## s5 (permuter, 2026-07-23) — vPRESW chassis (the ONLY both-properties form) KILLED; all THREE permuter chassis now dead

- [s5] Fresh-seed discipline required a structurally-DIFFERENT chassis than s4 (which used base=weighted174 and vH=weighted1250, NEITHER having both target properties). Seeded a NEW chassis from **vPRESW** — the only structural form (s2) with BOTH per-access %hi/%lo AND vars=0 (no phantom); its sole defect is scheduling (val1 hoisted before the switch => a0 pushed off $a0 => reorder => sandbox 28). Hypothesis: the reorder is pure schedule/reg = the permuter's domain, so it might recover the schedule while keeping per-access+vars=0.

- [s5] Built codegen-faithful workspace tmp/perm_22F34_presw (reused s4's compile.sh/target.o; base.c = vPRESW body, self-contained). Campaign vPRESW-schedule-recover, -j8, --stack-diffs, base_score 760 (between base's 174 and vH's 1250 — reflects vPRESW's reordering penalty at weight 60/reorder).

- [s5] **KILLED. 13367 iterations, best find = 590.** The basin descends 760 -> 590 within ~30s then OSCILLATES 590-760 (finds at 590/658/690/695/710/760) indefinitely; NEVER approaches base's byte-perfect chassis (174), let alone target (0). The 590 find is cosmetic only (permuter new_var + do-while(0) textual mutations; diff in tmp/grind/func_80022F34/s5/best-590/). The permuter cannot undo vPRESW's val1-hoist reorder without moving val1's def back past the switch merge — which re-strands reg100 (vars=8, base) or shares the la base (not per-access, vH). This IS s2's coupling; there is no permuter-reachable middle.

- [s5] **CONCLUSION: all THREE candidate permuter chassis are now measured dead** — s4 base(174) + vH(1250), s5 vPRESW(760, the one already holding 2 of 3 target properties). The per-access <-> phantom <-> schedule coupling is a fork-level cse2+combine interaction with no C spelling reachable by any permuter mutation from any chassis. The permuter axis (H-A) is fully and finally exhausted.

- [s5] The ONLY remaining un-measured sanctioned axis is H-D (cse.c symbol_ref/CSE-of-constants cost forensics, NON-permuter): find the C-visible condition that makes our fork's cse2 re-materialize %hi/%lo per load WITHOUT the long val1 lifetime that strands reg100. If cse.c shows no such C-visible lever, that is the OWNER-ESCALATION evidence (cc1psx-vs-fork cse2 divergence with no pure-C bridge, mirroring siblings func_80049A2C/func_80037540). s5 does NOT escalate: H-D remains grindable.

- [s5] campaign harvested + stopped (stopped:true, elapsed 534s, 0 orphans); src/code6cac.c unchanged (git clean, floor 11). Artifacts: tmp/grind/func_80022F34/s5/{build_ws_presw.sh, presw_campaign.log, best-590/}. Rejected: rejected/permuter-vPRESW-chassis-plateaus-590.c.

- [s5] vPRESW is the only structural form (s2) with BOTH target properties (per-access %hi/%lo + vars=0); its lone defect is a val1-hoist reorder scoring sandbox 28.

- [s5] s5 permuter campaign on the vPRESW chassis (base_score 760, -j8, --stack-diffs): 13367 iterations, best find 590, cosmetic mutations only; basin plateaus at 590, oscillating 590-760, never approaching base 174 or target 0.

- [s5] All THREE candidate permuter chassis are now measured dead: s4 base(174) + vH(1250), s5 vPRESW(760, the one already holding 2 of 3 target properties). The permuter axis (H-A) is fully exhausted.

- [s5] Mechanism (extends s2): un-hoisting val1 to fix the schedule re-strands reg100 (vars=8) or CSE-shares the la base (not per-access); no permuter mutation reaches the target's per-access + vars=0 + correct-schedule form from any chassis.

- [s5] Campaign harvested + stopped (stopped:true, elapsed 534s, 0 orphans); src/code6cac.c unchanged (git clean, floor 11).

## s6 (forensics, 2026-07-23) — mechanism pinned to combine.c distribute_notes REG_DEAD->CODE_LABEL; endgame-lock escalation filed

- [s6] Baseline reconfirmed: sandbox --disable all = 11 (build_insns 69, target 70, rules_dropped 11, verdict C). Fresh s6 dumps reproduce base: .frame vars=8, regs=4/0, subu $sp,$sp,40.

- [s6] **EXACT PASS/DECISION NAMED (fresh instrumented cc1, tmp/grind/func_80022F34/s6/dumps/):** the +8 phantom is GCC 2.7.2 `combine` distribute_notes, combine.c:10836-10846 (the "REG_DEAD note is still homeless AND we hit a CODE_LABEL -> emit a (use regN) after the label, to protect caller-save state tracking" branch). combine folds val1=*reg100 (reg100 = idx1*20 + symbol_ref(D_801027BC)) into the a1 arg load (insn 128, per-access mem(reg99+sym)), DELETES reg100's def (insn 102), and its orphaned REG_DEAD note walks back from insn 128 to the switch-merge code_label 85 and emits `(insn 163 (use (reg:SI 100)))` there (verbatim in fresh s6 base.i.combine).

- [s6] **ROOT LIMITATION cited at source:** combine.c:52-59 documents "reg_n_refs is not adjusted in the rare case when a register is no longer required ... a REG_DEAD note is lost." So reg100 keeps stale reg_n_refs; lreg = "Register 100 used 2 times across 1 insns; ST_REGS or none; pointer" (use, no reaching def). greg: reg100 in allocate list ("6 regs to allocate: 100 80 75 72 74 73"), EMPTY conflict set, ABSENT from Register dispositions => reload/alter_reg homes it to a stack slot => one 4-byte slot, 8-aligned => vars=8. val2's address pseudo (reg108) folds late/adjacent to i3 and gets a hard reg ("108 in 2") — only val1 strands.

- [s6] **Fold<->strand are the SAME combine event (inseparable):** GCC 2.7.2 MIPS has no explicit %hi/%lo relocs, so the full symbol_ref lives in the folded mem and reg100 is never referenced by i3 (insn 128) — the note can't anchor on i3 (combine.c:10735). The 4-way switch on D_800A38DC forces code_label 85; val1's dep (index from ORIGINAL a0, consumed in the call AFTER the a0=*a0 reload) forces reg100 to live across the reload so its death lands past the label. Per-access requires the fold (=strand); no-fold gives the CSE'd la base (vars=0 but not per-access). Target holds BOTH — an unreachable fixpoint for this fork's combine.

- [s6] **Two NEW forensic C forms KILLED** (rejected/): byte-pointer address arith `*(s32*)((u8*)&D_801027BC + idx1*20)` (vNH) strands identically (vars=8, reg100, phantom-strand=1); named live address pointer `s32 *q=&(&D)[idx1*5]; val1=*q` (vSECOND) relocates the phantom to pseudo reg83 (empty conflicts, no disposition, vars=8) — the phantom is spelling-invariant.

- [s6] scan_hand_coded.py --single func_80022F34 = tier=LOW score 1/8 (only S4 front-loads) -> ordinary compiled C, canonical-asm NOT supportable (endgame-lock gate 1 fails). No C construct removes a phantom stack slot -> no spelling family to sanction (gate 2 fails). Both AND-gates fail.

- [s6] OWNER-ESCALATION filed in docs/grind/decisions.md (2026-07-23, names func_80022F34) under the 2026-07-20 endgame-lock-disposition policy. Byte held by 11 regfix substs; honest floor 1 insn from clean; RA/combine artifact; LOW hand-coded; no SOTN-precedent closing construct (none exists — residual is a compiler-ADDED slot, inverse of func_80037540/func_80049A2C). Options: (a) canonical-asm NOT supportable; (b) OWNER-ACCEPTED INCOMPLETE (retain substs, park, re-attempt on a novel whole-function reshape). Outcome: owner-gated. Artifacts: tmp/grind/func_80022F34/s6/{base.c,vNH.c,vSECOND.c,dump.sh,probe.sh,escalation_entry.md,dumps/base.i.{combine,greg,lreg,flow,...}}.

- [s6] sandbox --disable all = 11 (build_insns 69, target 70, 11 rules_dropped, verdict C); floor flat at 11 since s1.

- [s6] base body is BYTE-PERFECT vs target (2x per-access lw %lo(D_801027BC), in-place a0 reload in $a0); entire 11 = +8 phantom stack slot (10 diffs) + 1 maspsx .L-label load-delay nop (retirable on DONE path).

- [s6] Exact pass/decision (fresh s6 dumps): combine.c:10836-10846 distribute_notes emits (insn 163 (use (reg:SI 100))) at switch-merge code_label 85 for reg100 (val1's folded address); combine.c:52-59 documents the reg_n_refs staleness this triggers.

- [s6] greg: reg100 in allocate list ('6 regs to allocate: 100 80 75 72 74 73'), EMPTY conflicts, ABSENT from Register dispositions -> reload/alter_reg reserves a stack slot -> vars=8. val2's pseudo (reg108) folds late/adjacent and gets a hard reg ('108 in 2') — only val1 strands.

- [s6] Fold==strand are one inseparable combine event: GCC 2.7.2 MIPS keeps the full symbol_ref in the folded mem (no explicit %hi/%lo relocs), so reg100 is never referenced by i3 and the note can't anchor on i3; the 4-way switch forces code_label 85; val1's dependency forces reg100 to live across the a0 reload. Not folding gives the CSE'd la base (vars=0 but not per-access) — the s2 coupling.

- [s6] This is the INVERSE of siblings func_80037540/func_80049A2C (they ADD a slot cc1psx reserves; this func must REMOVE a slot our fork reserves that target lacks). No C construct removes a phantom stack slot.

- [s6] scan_hand_coded.py --single func_80022F34 = tier=LOW 1/8 (only S4) -> ordinary compiled C; canonical-asm NOT supportable (endgame-lock gate 1 fails). No SOTN-precedent spelling family removes a phantom slot (gate 2 fails).

- [s6] Two new forensic forms KILLED and banked in rejected/ (bytepointer-address-arith-strand-persists.c, named-address-pointer-strand-moves-to-reg83-vars8.c).

- [s6] OWNER-ESCALATION filed in docs/grind/decisions.md (2026-07-23), names func_80022F34, under the 2026-07-20 endgame-lock-disposition policy; byte held by 11 regfix substs; both AND-gates fail; options (a) canonical-asm NOT supportable, (b) OWNER-ACCEPTED INCOMPLETE.

## s7 (solver, 2026-08-26) — solver axis FORECLOSED (PRE-RA); the s2/s3/s6 "placement + switch-label" mechanism is CORRECTED; a SECOND 11-floor chassis (vORIG) with target-exact frame is banked

- [s7] Chassis re-measured with candidate.c applied to src/code6cac.c: sandbox --disable all = **11** (build_insns 69, target 70, rules_dropped 0, cheat_asm_stripped 27, verdict C). Floor unchanged since s1. NOTE the post-migration chassis: HEAD is `INCLUDE_ASM("asm/funcs", func_80022F34);` and carries ZERO regfix rules (rules_dropped 0, not 11) — the 11 is now entirely honest distance, not rule-masked.

- [s7] **SOLVER VERDICT (the owner's 2026-08-24 directive, executed): PRE-RA — FORECLOSED for both ra_solver and sched_solver.** `inverse_compose.py classify` cannot be used here (its text path needs a `<stem>.tgt.s`, which an INCLUDE_ASM-routed function cannot produce; mkasm_honest.sh's own header says so). The correct object-level tool is `goal_from_tgt.py classify code6cac func_80022F34`, which reports `FIRST DIVERGENCE: PRE-RA / next tool: none — the residual is upstream of every model`. On the base chassis the one-stream-only shapes are exactly the frame set (ours addiu -40 / sw 32,36 / lw 32,36 / addiu 40; target addiu -32 / sw 16,20 / lw 16,20 / addiu 32) plus target's `nop`. **This mechanically closes the RA and scheduler model axes**: the two streams do not differ by a register assignment or an emission order, they differ by a stack-frame size decided upstream of global.c. No RA/sched lever vector exists to spell.

- [s7] **s6's mechanism claim is CORRECTED (falsified as stated).** s6 asserted "the 4-way switch on D_800A38DC forces code_label 85" as a NECESSARY condition of the strand. Measured with fresh -mel `-da` probes (tmp/grind/func_80022F34/s7/probe.sh, probe2.sh):
  * **vIF** (switch -> `if (val==0) ... else if (val!=3) ...` chain, jump table GONE): vars=8, per-access x2, strand=1.
  * **vIF2** (`if (val==3) {} else if (val==0) ... else ...`): vars=8, per-access x2, strand=1.
  * **vTERN** (single store, `if (val!=3) *(s16*)(a0+8) = (val==0) ? ... : *tbl;`): vars=8, per-access x2, strand=1.
  * **vMIN** (the ENTIRE switch/val block deleted — diagnostic, semantics intentionally broken): vars=8, regs=3/0, strand=1.
  The strand survives with no switch, no jump table, and no case-merge label at all. The CODE_LABEL that distribute_notes lands on is therefore NOT switch-specific (the loop's own label suffices), so "eliminate the switch-merge label" — the s6 frontier's headline reshape lever — is DEAD and must not be re-attempted.

- [s7] **The real discriminator, measured across 13 forms: STATEMENT SEPARATION of the two D_801027BC loads, not val1 placement.** s2/s3 framed this as a val1-PLACEMENT axis. It is not. The law:
  * Any form in which **either** `(&D_801027BC)[idx*5]` load is lifted out of the call expression into its own named `s32` temp => cse2 does NOT share a base => **per-access fold x2 => exactly one stranded address pseudo => vars=8, frame -40**. Measured: base, vO2 (val1 named, defined AFTER the a0 reload), vO4 (both named), vO6, vO7 (**val2 named ONLY** — the strand follows whichever load is separated, it is not a property of val1), vO13, vO16 (array-typed). 7/7.
  * Any form in which **both** loads stay inside the single call expression => cse2 shares one `la $5,D_801027BC` => **no fold, no strand, vars=0, frame -32 (target-exact)**. Measured: vORIG, vO3, vO5, vO11 (second load via byte-pointer arith), vO12 (both via byte-pointer arith), vO14 (array-typed). 6/6.
  Placement is inert (vO2 defines val1 after the reload and still strands; vO5 keeps idx1 live across the reload and does not). Symbol TYPE is inert (vO14/vO16: `extern s32 D_801027BC[];` + `D_801027BC[idx*5]` reproduces its class exactly). Byte-pointer address arithmetic is inert (vO11/vO12 stay in the la class; s6's vNH stayed in the fold class — consistent, because vNH was a separated statement).

- [s7] **NEW SECOND CHASSIS BANKED — vORIG (memory/grind/func_80022F34/chassis-vORIG.c), sandbox --disable all = 11, with a COMPLEMENTARY residual to base.** Body: `u8 *nxt = *(u8**)a0; s16 idx1 = *(s16*)(a0+0x4A); s16 idx2 = *(s16*)(nxt+0x4A); single_game_SetStatusUpData(i, (&D_801027BC)[idx1*5], (&D_801027BC)[idx2*5]);` (the a0 reload is read into a named `nxt` instead of being self-assigned into a0, so both loads can sit in one expression). Measured: vars=0, regs=4/0, **subu $sp,$sp,32 with saves at 16/20/24/28 — byte-identical to target's prologue AND epilogue**; the first 39 normalized insns (prologue, loop head, guard, whole switch incl. the jump-free compare chain, the sh store, the loop tail, the epilogue) are byte-identical to target. The ENTIRE 11 is localized to one 15-insn block: ours materializes `lui a1;addiu a1,a1,0` (= `la`) once and does `addu v1,v1,a1 / lw a1,0(v1)` + `addu v0,v0,a1 / lw a2,0(v0)`; target does `lui at;addu at,at,v1;lw a1,0(at)` twice. Multiset delta is literally ONE insn shape (ours `addiu #,#,0` vs target `lui #,0x0`) plus target's `nop`; goal_from_tgt classify on this chassis reports PRE-RA with only those three one-stream-only shapes.

- [s7] **Both chassis floor at exactly 11 with EXACTLY complementary residuals.** base = target's body + a compiler-ADDED +8 frame slot (10 frame diffs + nop). vORIG = target's frame/prologue/epilogue/switch + a shared `la` base where target re-materializes per access (10 body diffs + nop). Target needs base's address block AND vORIG's frame simultaneously; the 13-form law above says the two are the same cse2/combine decision, so no C form holds both. This is the s2 coupling, but now stated as a mechanically testable invariant (separation <=> fold <=> strand) rather than as a placement heuristic, and with a second, better-localized chassis in hand.

- [s7] Why vORIG is the better chassis to hand the next modality even though the score ties: (i) its residual is 15 contiguous insns in one basic block, not a frame-wide offset shift, so a permuter/solver gradient on it is not diluted by --stack-diffs bookkeeping; (ii) it contains ZERO RA artifacts (no unallocated pseudo, no phantom slot) — the only question left on it is a single cse2 cost decision ("hold the symbol base in a reg vs re-materialize it per load"), which is a pure PRE-RA question with a real cse.c cost model behind it (`CSE_ADDRESS_COST` / `rtx_cost` on symbol_ref+index); (iii) it needs one MORE insn than we emit (target 70 vs ours 69), i.e. the target form is strictly more expensive, which is exactly the direction a cost-model lever moves.

- [s7] Artifacts: tmp/grind/func_80022F34/s7/{probe.sh,probe2.sh,base.c,vIF.c,vIF2.c,vTERN.c,vMIN.c,vORIG.c,vO2..vO7,vO11..vO14,vO16 (.c/.i/.s + -da dumps)}, tmp/grind/func_80022F34/s7/classify_base.txt, classify_vORIG.txt, objdiff_vORIG.txt. src/code6cac.c reverted to HEAD (INCLUDE_ASM) at end of session.

- [s7] Chassis re-measured this session with candidate.c applied to src/code6cac.c: sandbox --disable all = 11 (build_insns 69, target 70, rules_dropped 0, cheat_asm_stripped 27, verdict C). Post-migration HEAD is INCLUDE_ASM with ZERO regfix rules, so the 11 is now fully honest distance, not rule-masked (the ledger's older 'rules_dropped 11' framing is stale).

- [s7] inverse_compose.py classify is NOT usable on an INCLUDE_ASM-routed function: its text path requires <stem>.tgt.s, which mkasm_honest.sh cannot produce once src carries no C body. The correct entry point is tools/ra_solver/goal_from_tgt.py classify <stem> <func> (object-level). Banked as a pipeline gotcha for every future solver session on a migrated function.

- [s7] SOLVER VERDICT: FIRST DIVERGENCE: PRE-RA on both chassis, 'next tool: none - the residual is upstream of every model'. The ra_solver and sched_solver axes are mechanically closed for func_80022F34; no RA seat and no emission-order tie exists to invert.

- [s7] s6's stated mechanism is CORRECTED: the 4-way switch's merge CODE_LABEL is NOT a necessary condition for the orphaned (use regN). vIF / vIF2 / vTERN (no jump table, no case-merge label) and vMIN (switch deleted entirely) all still measure vars=8, strand=1. The loop's own CODE_LABEL is enough for combine.c distribute_notes to land on, so 'remove the switch-merge label' - the s6 frontier's headline reshape lever - is dead.

- [s7] The residual's real discriminator, measured over 13 forms with zero exceptions: STATEMENT SEPARATION of the two (&D_801027BC)[idx*5] loads. Lift EITHER load into its own named s32 temp -> cse2 stops sharing -> per-access fold x2 -> exactly one stranded address pseudo -> vars=8, frame -40. Keep BOTH inside the single call expression -> one shared la -> no fold, no strand -> vars=0, frame -32. val1's placement is inert (vO2 defines it AFTER the a0 reload and still strands); which load is separated is inert (vO7 separates val2 only and strands); the symbol's declared type is inert (vO14/vO16 with extern s32 D_801027BC[]); byte-pointer address arithmetic is inert (vO11/vO12).

- [s7] NEW SECOND CHASSIS banked at memory/grind/func_80022F34/chassis-vORIG.c, sandbox --disable all = 11, with a residual EXACTLY COMPLEMENTARY to candidate.c's. vORIG matches target's frame, prologue, epilogue, loop and switch byte-for-byte (39 leading normalized insns identical, vars=0, sp -32, saves 16/20/24/28, strand=0) and its entire 11 lives in one 15-insn block where cse2 shares the symbol base instead of re-materialising %hi/%lo per access.

- [s7] Target needs base's per-access address block AND vORIG's zero-vars frame at the same time; the 13-form dichotomy says those are the same cse2 sharing decision, which is why both chassis tie at exactly 11.

- [s7] vORIG is the better handoff chassis even at a tied score: its residual is 15 contiguous insns in one basic block (a permuter/solver gradient on it is not diluted by --stack-diffs frame bookkeeping), it contains ZERO RA artifacts (no unallocated pseudo, no phantom slot), and the only open question on it is one cse2 cost decision. Target's form is one insn MORE expensive than ours (70 vs 69), which is the direction a cost-model lever moves.
