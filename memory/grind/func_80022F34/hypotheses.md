# Hypothesis ledger — func_80022F34

## Frontier (grind s1, recon 2026-07-23) — floor 11, but re-characterized

The prior "s3/s4/s5 over-promotion" frame is DEAD (false diagnosis). The real
floor is: [SOLVED path] a +8 phantom frame slot (combine reload stale-ref bug on
the %lo(D_801027BC) fold at the switch merge label) + [remaining] a base-CSE and
the maspsx nop. vH (candidate.c) already dissolves the phantom (frame byte-matches
target). Start the next session from candidate.c, NOT HEAD.

H-A (TOP LEAD): from the vH base (frame already correct), defeat GCC's CSE of
&D_801027BC so each `(&D_801027BC)[idx*5]` re-materializes %hi/%lo per access like
target. Mechanism: cse2/combine share the symbol_ref across the two inlined
accesses. Levers: [[defeat-combine-symbol-fold]], [[store-const-reload-cse]];
try making the two accesses less simultaneously-visible (named per-arm temps,
different-looking base expressions). NB the permuter is a VALID tool here (unlike
the siblings) because vH's residual is NOT sp-offset-only — the frame is correct,
so decomp-permuter's stack-normalized scorer will see the real reg/CSE diffs.
next_probe: permuter from vH base + [[defeat-combine-symbol-fold]] variants;
score with engine sandbox to be safe.

H-B: fix the a0 self-reload register ($v0 vs target $a0). In vH the double-
indirection `*(u8**)a0` for arg3 lands in $v0 then `lh a0,74(v0)`; target reuses
$a0 (`lw a0,0(a0); lh a0,74(a0)`). Likely coupled to arg evaluation order /
a0 liveness. Lever: reorder so a0 is dead at the reload; [[walking-pointer-
serializes-parallel-loads]]. next_probe: vary arg2/arg3 eval order and a0 reload
placement, measure objdump distance in the reload region.

H-C (blend the tension): find a form with per-access %lo AND no phantom. HEAD's
named-temp form gives per-access %lo but strands reg 100 at the switch merge
label; vD's shared base gives no strand but promotes a reg; vH gives no strand +
CSE. The strand is combine putting a stray `(use reg100)` at code_label 85. Probe
whether isolating the idx1/val1 block from the switch merge (e.g. hoist the whole
if-body's tail out of the switch's control region, or an intervening statement
that blocks the mis-placed use) removes the strand while keeping per-access %lo.

DONE-path (for the session that lands the C match): vH body + retire the 10
frame-offset regfix substs (now redundant — frame is correct without them) +
add func_80022F34 to maspsx_label_nop_funcs.txt for the 1 load-delay nop
(store-value-consumer variant, [[maspsx-label-nop-gate]]). These are queue
done/retire actions outside a grind session's surface.

## [s1] The pure-C build over-promotes 3 values to callee-saved s3/s4/s5 with a -48 frame (prior ledger diagnosis).
- mechanism: claimed 7 callee-saved regs; frame too big from register over-promotion
- probe: objdump the honest cheat-stripped sandbox object for func_80022F34
- result: Build uses ONLY s0,s1,s2,ra with a -0x28 frame; body BYTE-IDENTICAL to target. Register allocation matches; the entire gap is an 8-byte-too-big frame (10 diffs) + 1 maspsx nop.
- verdict: KILLED

## [s1] The +8 frame is a phantom slot from GCC 2.7.2's combine reload stale-ref bug, triggered by the %lo(D_801027BC) address fold.
- mechanism: combine folds reg100 (=&D_801027BC[idx1*5] address) into per-access lw %lo(...)(at) but leaves a stray (use reg100) at the switch merge label -> stale reg_n_refs -> ST_REGS -> alter_reg reserves an unreferenced frame slot (vars=8)
- probe: cc1 -da lreg/combine dumps of a standalone reproducer + .frame comment (vars=V) as gradient
- result: lreg: 'Register 100 ... block 10; ST_REGS or none; pointer.' combine: (note 102 DELETED) + (insn 163 (use reg100) at code_label 85). Confirmed the phantom pseudo and its stranding site.
- verdict: CONFIRMED

## [s1] The phantom is dissolvable in pure C without an extra callee-saved register (fully-inlined m2c-faithful form).
- mechanism: inlining both D_801027BC accesses into the call + a0-reload as double-indirection makes GCC materialize the base into a caller-saved reg (no %lo strand), so vars=0
- probe: standalone .frame probe of vH_m2c.c; apply to src + full-TU sandbox --disable all; objdump the prologue
- result: vH -> vars=0, regs=4/0, frame 0x20 (byte-matches target prologue/epilogue). Full-TU sandbox = 11, but the 11 moved entirely into the D_801027BC access region (base-CSE + a0-reload register).
- verdict: CONFIRMED

## [s2] H-C (structural blend: per-access %lo AND no phantom) — KILLED on the val1-placement axis.
- mechanism: per-access requires val1 loaded early in a sub-block (long lifetime, register-pressure separation of the two &D_801027BC loads); that exact structure strands reg100 via combine's misplaced (use) note at the switch-merge label => vars=8. Loading val1 late collapses the separation => cse2 shares the base (`la`) => vars=0 but not per-access.
- probe: ~24 structural forms, .frame vars= + per-access `lw D_801027BC` count + real sandbox on base(11)/vSPLIT(11)/vMIRROR(11)/vPRESW(28).
- result: NO structural form has right-order + 2x per-access + vars=0. The only vars0+per-access form (val1 before switch, vPRESW) reorders to 28. All frame-correct forms CSE and floor at 11; all per-access forms strand and floor at 11.
- verdict: KILLED (structural / val1-placement axis). The coupling is a fork-level cse2+combine interaction, not a C-statement-order artifact.

## Revised frontier (grind s2) — floor 11; structural val1-placement axis exhausted

H-A (TOP LEAD, NON-structural): permuter from a frame-correct base. NB s2 shows the
byte-perfect body (base) needs only the reg100 strand removed to reach floor 1 — the
permuter base choice is now a real fork: (a) vH/vMIRROR (frame-correct, defeat the
&D_801027BC cse2 share to get per-access), OR (b) base (byte-perfect body, defeat the
reg100 combine strand to get vars=0). Option (b) is 1 combine-artifact away from a
near-match and is the higher-value permuter target. Levers: [[defeat-combine-symbol-fold]]
(inverse direction), register-pressure injection to force symbol re-materialization.
next_probe: permuter both bases with PERM_* directives; the residual is NOT sp-offset-only
so decomp-permuter's stack-normalized scorer sees the real reg/CSE/strand diffs.

H-D (NEW, non-structural — cse2/combine internals): the target's per-access comes from
cc1psx NOT sharing the &D_801027BC symbol_ref; our fork's cse2 shares it (`la`). Read
tools/gcc-2.7.2/cse.c around symbol_ref cost / CSE of constants to find what C-visible
condition (register pressure, intervening clobber) makes our cse2 re-materialize instead
of share. This is the root of both the per-access gap AND (via the separation it forces)
the reg100 strand — a form that makes cse2 re-materialize WITHOUT the long val1 lifetime
would break the coupling.

H-B (still open): a0 self-reload lands in $v0 (CSE forms) vs target $a0. Sub-issue worth
~1-2 diffs; only matters once the per-access/frame is resolved.

DONE-path unchanged: whichever base lands the match needs the 10 frame-offset regfix substs
retired (redundant once frame is correct) + func_80022F34 in maspsx_label_nop_funcs.txt for
the 1 load-delay nop.

## [s2] A structural form exists with target's instruction order AND per-access %lo on both D_801027BC loads AND no phantom frame slot (vars=0).
- mechanism: per-access requires val1 loaded early in a sub-block (long lifetime => register-pressure separation of the two &D_801027BC loads); that exact structure strands reg100 via combine's misplaced (use) note at the switch-merge label => vars=8. Loading val1 late collapses the separation => cse2 shares the base (la $5) => vars=0 but not per-access.
- probe: ~24 structural forms measured by cc1 .frame vars= + per-access `lw D_801027BC` count, plus real sandbox --disable all on base(11)/vSPLIT(11)/vMIRROR(11)/vPRESW(28).
- result: No form achieves all three. The ONLY vars=0 + 2x per-access form loads val1 before the switch (vPRESW) and reorders the whole chain => sandbox 28 (worse). Every frame-correct form CSEs the symbol; every per-access form strands reg100. All non-reordered regimes floor at exactly 11.
- verdict: KILLED

## [s2] base (HEAD named-temp) body is byte-perfect vs target and its entire 11 is a single defect (the +8 phantom frame slot), not a codegen-body difference.
- mechanism: cse2: reg100 = idx1*20 + symbol_ref(D_801027BC); val1 = *reg100. combine folds reg104 into `lw val1,D_801027BC(reg99)` (per-access) and deletes reg100's def, but strands `(insn 163 (use (reg:SI 100)) REG_DEAD)` UPSTREAM of the def at switch-merge code_label 85. reg100 gets no hard reg (greg allocate-list, absent from dispositions) => alter_reg reserves an unreferenced slot => vars=8 => sp-adjust x2 + 4 saves + 4 restores at +8 offsets (10 diffs) + 1 maspsx nop = 11.
- probe: objdump of base.s body vs asm/funcs/func_80022F34.s; cc1 -da greg/combine/cse2 dumps; sandbox --disable all = 11, build_insns 69, target 70.
- result: Body (both per-access lw D_801027BC + in-place `lw $4,0($4)` a0 reload in $a0) is identical to target; only the frame differs. Remove the reg100 strand and floor would drop to 1 (the nop, retirable via maspsx_label_nop_funcs.txt).
- verdict: CONFIRMED

## [s3] The reg100 combine strand can be relocated off the switch-merge label by inserting a control boundary (do-while(0)) between the switch and the val1 sub-block.
- mechanism: combine strands `(use reg100)` on the switch-merge code_label because no real insn/label sits between the merge and reg100's def; a control boundary might give it a different anchor or split the region.
- probe: vTAIL (switch in do{}while(0)) and vVALDW (val1 block in do{}while(0)); cc1 .frame vars= + per-access D_801027BC count + asm diff vs base.
- result: BOTH keep vars=8 + per-access x2 (strand intact). do-while(0) is optimized away before combine → no real CFG edge → note stays on the merge label. vVALDW additionally wastes the `lh` load-delay slot (extra maspsx #nop) → strictly worse. A real CFG separation would require a branch that adds a body insn (breaks the byte-perfect body).
- verdict: KILLED. The strand is CFG/scope-invariant; both structural axes (s2 placement + s3 control-boundary) are now dead. Escape is non-structural only.

## [s4] The permuter (random codegen mutation) removes the reg100 combine strand from base's byte-perfect body, or defeats vH's CSE to get per-access %lo without the phantom => a form scoring below floor 11.
- mechanism: register allocation / CSE / scheduling are the permuter's domain; a randomized transform keeping reg100 out of greg's allocate-list (or letting combine attach its note to a real insn) removes vars=8 without touching the body. Residual is not sp-offset-only, and with --stack-diffs the phantom frame is scorer-visible, so the permuter can gradient on it.
- probe: two codegen-faithful decomp-permuter chassis (base = byte-perfect body weighted 174; vH = frame-correct CSE'd weighted 1250), ~35k iters total, --stack-diffs, campaign telemetry via tools/permuter_campaign.py.
- result: base 30640 iters => ZERO sub-174 finds (nothing beat the byte-perfect-body chassis). vH best find = 224 (> base 174), only re-finds known base/vSPLIT/vH classes, never target's per-access+no-phantom. Absolute best across both chassis = 224, worse than base's own 174, nowhere near 0.
- verdict: KILLED. Random codegen mutation over either chassis cannot produce target's (per-access %hi/%lo AND vars=0) form. Mechanically corroborates the s1-s3 finding that the coupling is a fork-level cse2+combine interaction, not C-reachable.

## Revised frontier (grind s4) — floor 11; structural (s2/s3) AND permuter (s4) axes exhausted

Only ONE sanctioned axis remains un-measured: **H-D (cse.c forensics, non-permuter)**. All
statement-order / control-boundary (structural) and random-codegen (permuter) axes are now
dead. H-D: read tools/gcc-2.7.2/cse.c around symbol_ref / CSE-of-constants cost to find the
C-visible condition (register pressure, intervening clobber, cost threshold) that makes our
fork's cse2 RE-MATERIALIZE %hi/%lo(D_801027BC) per load instead of sharing the `la` reg —
WITHOUT the long val1 lifetime that strands reg100. The tension (s2): per-access arises ONLY
via register-pressure separation of the two `la` loads, and that separation is EXACTLY what
strands reg100, so H-D must find a cse2-cost lever that decouples the two. If cse.c shows no
such C-visible lever exists (the share-vs-rematerialize decision is driven only by inputs our
C cannot vary), that is the evidence for OWNER-ESCALATION — the target's per-access form would
then be a cc1psx-vs-our-fork cse2 divergence with no pure-C bridge, mirroring the sibling walls.

## [s2] A frame-correct (vars=0) CSE form scores below 11 because CSE costs fewer instructions than the +8 phantom.
- mechanism: CSE'd shared `la $5` base for both accesses is shorter than per-access %hi/%lo; if the CSE + a0-reload-in-$v0 diffs total < 10, a frame-correct form would beat the phantom.
- probe: Applied vMIRROR (target-order both-CSE) and vSPLIT (val2 per-access, val1 CSE) to src; sandbox --disable all.
- result: vMIRROR = 11 (build_insns 64), vSPLIT = 11 (build_insns 64). The CSE'd base + a0-reload-in-$v0 (vs target $a0) + nop sum to exactly 11. No frame-correct structural form beats base.
- verdict: KILLED

## [s3] A control boundary (do-while(0)) between the switch and the val1 sub-block relocates combine's stranded (use reg100) note off the switch-merge label, removing the +8 phantom frame slot (vars 8 -> 0) while keeping per-access %lo.
- mechanism: combine strands (use reg100) on the switch-merge code_label because no real insn sits between the merge and reg100's def; a control boundary was hypothesized to give it a real anchor or split the region.
- probe: vTAIL (switch wrapped in do{}while(0)) and vVALDW (val1 sub-block wrapped in do{}while(0)); cc1 .frame vars= + per-access D_801027BC-ref count + asm diff vs base; sandbox anchor on src = 11.
- result: Both keep vars=8 + per-access x2 (strand intact). do-while(0) is folded before combine -> no real CFG edge/label -> the note stays on the switch-merge code_label. vVALDW additionally wastes the lh 74($4) load-delay slot (extra maspsx #nop), strictly worse than base's schedule. A real CFG separation would need a branch that adds a body insn, breaking the byte-perfect body.
- verdict: KILLED

## [s4] The permuter (random codegen mutation) removes the reg100 combine strand from base's byte-perfect body, or defeats vH's CSE to get per-access %lo without the phantom, yielding a form below floor 11.
- mechanism: RA/CSE/scheduling are the permuter's domain; a randomized transform that keeps reg100 out of greg's allocate-list (or lets combine attach its note to a real insn) removes vars=8 without touching the byte-perfect body. Residual is not sp-offset-only and with --stack-diffs the +8 phantom frame is scorer-visible, so the permuter can gradient on it.
- probe: Two codegen-faithful decomp-permuter chassis (base = byte-perfect body, weighted base score 174; vH = frame-correct CSE'd body, weighted 1250), ~35k iters total, --stack-diffs default, telemetry via tools/permuter_campaign.py; harvested + stopped both.
- result: base: 30640 iters, ZERO output dirs (nothing beat 174) - random mutation over the byte-perfect base cannot remove the reg100 strand. vH: best find score 224 (>base 174), only re-finds the known base/vSPLIT/vH classes, never target's (per-access %hi/%lo AND vars=0). Absolute best across both chassis = 224, worse than base's own 174, nowhere near 0.
- verdict: KILLED

## [s5] The permuter recovers vPRESW's schedule while keeping per-access %lo + vars=0, yielding a form below floor 11.
- mechanism: vPRESW (val1 hoisted before the switch) is the ONLY structural form with BOTH per-access %hi/%lo AND vars=0 (no phantom); its only defect is scheduling (val1 lives across the switch => a0 off $a0 => reorder => sandbox 28). Reorder = pure schedule/reg = the permuter's domain; s4's chassis (base, vH) never had both properties, so vPRESW is a structurally-distinct fresh seed whose residual is exactly what the permuter mutates.
- probe: codegen-faithful workspace tmp/perm_22F34_presw seeded from vPRESW, base_score 760; permuter_campaign -j8 --stack-diffs, 13367 iterations; harvest --stop.
- result: best find = 590 (cosmetic new_var/do-while(0) mutations). Basin descends 760->590 in ~30s then oscillates 590-760 indefinitely; never approaches base's 174, let alone 0. The permuter cannot un-hoist val1 without re-strands reg100 (base, vars=8) or CSE-share (vH, not per-access) — s2's coupling has no permuter-reachable middle.
- verdict: KILLED. Extends s4's permuter kill to the third and only remaining candidate chassis (the one already holding 2/3 target properties). All three permuter chassis (base 174 / vH 1250 / vPRESW 760) are now dead; the permuter axis (H-A) is fully exhausted.

## Revised frontier (grind s5) — floor 11; structural (s2/s3) AND permuter (s4 base/vH + s5 vPRESW, all three chassis) exhausted

Only ONE sanctioned axis remains un-measured: **H-D (cse.c forensics, non-permuter)**. Read
tools/gcc-2.7.2/cse.c symbol_ref / CSE-of-constants cost model to find the C-visible condition
(register pressure, intervening clobber, cost threshold) that makes our fork's cse2 RE-MATERIALIZE
%hi/%lo(D_801027BC) per load instead of sharing the `la` reg — WITHOUT the long val1 lifetime that
strands reg100. s2/s5 tension: per-access arises ONLY via register-pressure separation of the two
`la` loads, and that separation is EXACTLY what strands reg100 (base) or, when hoisted to decouple,
wrecks the schedule (vPRESW/28). H-D must find a cse2-cost lever that re-materializes WITHOUT either
side effect. If cse.c shows no such C-visible lever exists, that is the OWNER-ESCALATION evidence — a
cc1psx-vs-fork cse2 divergence with no pure-C bridge (mirrors siblings func_80049A2C/func_80037540).

## [s5] A directed permuter seeded from vPRESW (the ONLY structural form with BOTH per-access %hi/%lo AND vars=0, whose sole defect is scheduling) recovers the schedule while keeping both target properties, yielding a form below floor 11.
- mechanism: vPRESW hoists val1's def before the switch => val1 lives across the switch merge => per-access + vars=0 but a0 pushed off $a0 => reorder => sandbox 28. Reorder is pure schedule/reg = the permuter's domain. s4 used base(174)/vH(1250), NEITHER holding both properties; vPRESW(760) is a structurally-distinct fresh seed whose residual is exactly what the permuter mutates.
- probe: Built codegen-faithful workspace tmp/perm_22F34_presw seeded from vPRESW (base_score 760); permuter_campaign vPRESW-schedule-recover -j8 --stack-diffs, 13367 iterations; wait windows in-turn; harvest --stop.
- result: Best find = 590 (cosmetic new_var/do-while(0) mutations only). Basin descends 760->590 in ~30s then oscillates 590-760 indefinitely (finds at 590/658/690/695/710/760); never approaches base's byte-perfect chassis 174, let alone target 0. The permuter cannot un-hoist val1 without re-stranding reg100 (base, vars=8) or CSE-sharing the la base (vH, not per-access) — s2's coupling has no permuter-reachable middle.
- verdict: KILLED

## [s6] H-D (cse.c/combine forensics): a C-visible lever makes our fork produce target's per-access + vars=0 fixpoint.
- mechanism: H-D premise (make cse2 re-materialize per-access) is MOOT — `base` ALREADY re-materializes per-access (byte-perfect body). The real irreducible defect is the +8 phantom slot from combine.c:10836-10846 (distribute_notes REG_DEAD->CODE_LABEL USE-emit) + combine.c:52-59 stale reg_n_refs, leaving reg100 unallocatable in greg -> reload stack slot.
- probe: fresh instrumented cc1 (-da) on base + two new fold-anchor spellings (byte-pointer arith vNH; named live address pointer vSECOND); combine/greg/lreg/flow dumps; scan_hand_coded; sandbox.
- result: base = insn 163 (use reg100) at code_label 85, greg empty-conflict no-disposition -> vars=8. vNH strands identically. vSECOND relocates the phantom to reg83 (still vars=8). The fold (=per-access) and the note-orphan (=strand) are the same combine event; not decoupleable by any C for this dataflow (switch-merge label forced; val1 lives across the a0 reload). scan_hand_coded LOW 1/8.
- verdict: KILLED. All sanctioned pure-C axes (structural s2/s3, permuter s4/s5, forensics s6) now measured dead; floor flat at 11 since s1. Endgame-lock AND-gates both fail -> OWNER-ESCALATION filed (owner-gated).

## Revised frontier (grind s6) — floor 11; ALL grind-advanceable axes dead; owner-gated
No un-measured sanctioned axis remains. The residual is a compiler-ADDED phantom stack slot (combine.c distribute_notes + stale reg_n_refs), the INVERSE of siblings func_80037540/func_80049A2C. There is no C construct that removes a phantom slot, and canonical-asm is refused (LOW 1/8, byte-perfect pure-C body exists). Disposition is now an OWNER POLICY CALL under the 2026-07-20 endgame-lock rule (docs/grind/decisions.md 2026-07-23, names func_80022F34): (a) canonical-asm — NOT supportable; (b) OWNER-ACCEPTED INCOMPLETE (retain the 11 regfix substs, park, re-attempt only on a novel whole-function reshape that changes reg100's fold/live-range vs the switch-merge label). Do NOT re-spin structural/permuter/forensic modalities — all three are dead.

## [s6] H-D: a C-visible lever makes our fork produce target's per-access + vars=0 fixpoint (make cse2 re-materialize %hi/%lo per load without the long val1 lifetime).
- mechanism: H-D's premise is moot: `base` ALREADY re-materializes per-access (byte-perfect body). The real defect is the +8 phantom slot: combine folds val1's address pseudo reg100 (=idx1*20+symbol_ref(D_801027BC)) into the a1 arg mem (insn 128), deleting reg100's def; distribute_notes (combine.c:10836-10846) can't anchor the orphaned REG_DEAD note and, hitting the switch-merge code_label 85, emits (insn 163 (use reg100)); combine.c:52-59 leaves reg_n_refs stale; greg gives reg100 empty conflicts + NO hard reg; reload/alter_reg homes it to a stack slot -> vars=8.
- probe: Fresh instrumented cc1 (-da) dumps on base + two new fold-anchor spellings (byte-pointer arith vNH; named live address pointer vSECOND); combine/greg/lreg/flow inspection; scan_hand_coded.py; sandbox --disable all.
- result: base: (use reg100) at code_label 85, greg empty-conflict/no-disposition, vars=8, sandbox 11. vNH strands identically (vars=8, reg100). vSECOND relocates the phantom to pseudo reg83 (still empty-conflict, no disposition, vars=8). The per-access fold and the note-orphan are the SAME combine event; the switch-merge label is forced by the 4-way switch and val1 must live across the a0 reload, so no C statement structure decouples them. scan_hand_coded LOW 1/8.
- verdict: KILLED

## [s6] A fold-anchor spelling (byte-pointer address arithmetic, or a named live address pointer) keeps per-access while giving combine a real insn to anchor reg100's death note, removing the phantom.
- mechanism: If val1's address were referenced by a real insn between code_label 85 and insn 128, distribute_notes would place the note there (combine.c:10735/10806) instead of emitting the USE-at-label.
- probe: vNH = *(s32*)((u8*)&D_801027BC + idx1*20); vSECOND = s32 *q=&(&D_801027BC)[idx1*5]; val1=*q. cc1 .frame vars= + combine/greg dumps.
- result: Both KILLED. vNH: identical strand (vars=8, reg100). vSECOND: phantom persists as pseudo reg83 (empty conflicts, no greg disposition) -> vars=8. The phantom is spelling-invariant.
- verdict: KILLED

## [s7] The residual is an RA seat or a scheduler emission-order tie, so ra_solver/sched_solver produce a ranked C-lever vector for it (the owner's 2026-08-24 solver directive).
- mechanism: The endgame-lock residual is a frame-size/allocation artifact (reg100 unallocated -> alter_reg stack slot), which reads like global.c/reload territory; the solver suite converts exactly that class into typed REACHABLE/FORECLOSED verdicts with ranked lever vectors.
- probe: `inverse_compose.py classify code6cac func_80022F34` FIRST (per the solver playbook). It refused: its text path needs `<stem>.tgt.s`, which an INCLUDE_ASM-routed function cannot produce (mkasm_honest.sh header documents this). Re-ran on the object-level path `goal_from_tgt.py classify code6cac func_80022F34` with candidate.c applied to src and `sandbox --disable all` (=11) run to produce the honest .o. Repeated on the s7 vORIG chassis.
- result: **FIRST DIVERGENCE: PRE-RA** on BOTH chassis; `next tool: none — the residual is upstream of every model`. base chassis one-stream-only shapes = exactly the frame set (ours addiu -40 / sw+lw 32,36 / addiu 40 vs target addiu -32 / sw+lw 16,20 / addiu 32) + target `nop`. vORIG chassis one-stream-only shapes = ours `addiu #,#,0` x1 vs target `lui #,0x0` x1 + `nop` x1. Neither stream differs from target by a register assignment or an emission order, so there is no RA seat and no scheduler tie to invert.
- verdict: KILLED (FORECLOSED). The ra_solver and sched_solver axes are closed mechanically for this function; do not re-run them, and do not spend measurements on RA/scheduler lever vectors. Artifacts: tmp/grind/func_80022F34/s7/{classify_base.txt,classify_vORIG.txt,objdiff_vORIG.txt}.

## [s7] Eliminating the switch-merge CODE_LABEL (the s6 frontier's headline whole-function reshape) removes the orphaned (use regN) and the +8 phantom slot.
- mechanism: s6 pinned the phantom to combine.c:10836-10846 distribute_notes emitting `(use reg100)` when the homeless REG_DEAD note walks back and hits a CODE_LABEL, and asserted the 4-way switch on D_800A38DC forces that label. If the label were gone the note would have nowhere to land.
- probe: four fresh -mel `-da` forms (tmp/grind/func_80022F34/s7/probe.sh): vIF (if/else-if chain, jump table gone), vIF2 (reordered chain), vTERN (single store via a conditional expression), vMIN (the entire switch/val block DELETED — diagnostic, semantics intentionally broken). Read .frame vars=, sp, and the `(use (reg:SI N))` count in each .i.combine dump.
- result: vIF vars=8 strand=1; vIF2 vars=8 strand=1; vTERN vars=8 strand=1; vMIN vars=8 regs=3/0 strand=1. The strand survives with NO switch, NO jump table and NO case-merge label whatsoever — the loop's own CODE_LABEL is sufficient for distribute_notes. (vIF/vIF2/vTERN also emit 5 fewer body insns than base's byte-perfect switch, so they are worse on the body too.)
- verdict: KILLED — and it CORRECTS s6: the switch-merge label is not a necessary condition, so "remove the switch-merge label" is not a live lever and must not be re-attempted. Banked: rejected/switch-to-ifchain-strand-persists.c.

## [s7] The strand is a property of val1 (its long lifetime across the a0 reload), so a spelling that shortens or relocates that lifetime removes the phantom while keeping per-access.
- mechanism: s2/s3/s6 all framed the residual as a val1-PLACEMENT / val1-LIFETIME axis ("only val1 strands; val2 folds clean").
- probe: 13 forms measured on one harness (tmp/grind/func_80022F34/s7/probe2.sh), varying WHICH load is separated, WHERE it is defined relative to the a0 reload, the symbol's declared TYPE, and the address spelling: base, vORIG, vO2 (val1 named, defined AFTER the reload), vO3, vO4 (both named), vO5 (idx1 named only, lives across the reload), vO6, vO7 (val2 named ONLY), vO11/vO12 (byte-pointer address arithmetic), vO13, vO14/vO16 (`extern s32 D_801027BC[];`).
- result: A clean 7/6 dichotomy with ZERO exceptions. Separate EITHER load into its own named s32 temp -> per-access fold x2, exactly one stranded address pseudo, vars=8, frame -40 (base, vO2, vO4, vO6, vO7, vO13, vO16). Keep BOTH loads inside the single call expression -> cse2 shares one `la`, no fold, no strand, vars=0, frame -32 (vORIG, vO3, vO5, vO11, vO12, vO14). Placement is inert (vO2 defines val1 after the reload and still strands). Which load is separated is inert (vO7 separates val2 only and strands). Type is inert. Byte-pointer arithmetic is inert.
- verdict: KILLED as stated, and REPLACED by a sharper invariant: the discriminator is STATEMENT SEPARATION of the two D_801027BC loads (a cse2 sharing decision), not val1's identity, lifetime or placement. Banked: rejected/val2-only-temp-strand-follows-the-separated-load.c, rejected/array-typed-global-inert-to-cse-dichotomy.c.

## Revised frontier (grind s7) — floor 11; TWO complementary 11-chassis in hand; the ONE live question is a cse2 cost decision
base (memory/grind/func_80022F34/candidate.c) = target's BODY byte-perfect + a compiler-added +8 frame slot.
vORIG (memory/grind/func_80022F34/chassis-vORIG.c, NEW this session) = target's FRAME, PROLOGUE, EPILOGUE,
LOOP and SWITCH byte-perfect (39 leading normalized insns identical, vars=0, sp -32, saves 16/20/24/28,
strand=0) + a shared `la D_801027BC` base where target re-materialises %hi/%lo per access. Both score 11.
The remaining question is now single and PRE-RA: **make cse2 re-materialise the symbol base per load
WITHOUT lifting a load into its own statement** (lifting it is what re-introduces the fold, the strand and
the +8 slot — the s7 dichotomy). Read tools/gcc-2.7.2/cse.c's address/rtx_cost model for symbol_ref+index
(CSE_ADDRESS_COST, the `cse_gen_binary`/`fold_rtx` address path, and the -G0 / no-explicit-%hi-%lo MIPS
lowering) and look for a C-visible cost input. Note target's form is one insn MORE expensive than ours
(70 vs 69), which is the direction a cost lever moves. Do NOT re-run: ra_solver/sched_solver (s7 PRE-RA
FORECLOSED), permuter (s4/s5, all three chassis), val1-placement or control-boundary structure (s2/s3),
switch-label elimination (s7).

## [s7] The residual is an RA seat or a scheduler emission-order tie, so ra_solver/sched_solver yield a ranked C-lever vector (the owner's 2026-08-24 solver directive).
- mechanism: reg100 goes unallocated in greg and reload/alter_reg homes it to a stack slot (+8 frame) - that reads like global.c/reload territory, exactly the class the solver suite types as REACHABLE/FORECLOSED.
- probe: inverse_compose.py classify code6cac func_80022F34 FIRST (per the playbook) - it refused: its text path needs <stem>.tgt.s, which an INCLUDE_ASM-routed function cannot produce (documented in mkasm_honest.sh). Re-ran the object-level path goal_from_tgt.py classify with candidate.c applied to src and sandbox --disable all (=11) run to build the honest .o; then repeated on the new vORIG chassis.
- result: FIRST DIVERGENCE: PRE-RA on BOTH chassis, 'next tool: none - the residual is upstream of every model'. base one-stream-only shapes = exactly the frame set (ours addiu -40 / sw+lw 32,36 / addiu 40 vs target addiu -32 / sw+lw 16,20 / addiu 32) plus target nop. vORIG one-stream-only shapes = ours 'addiu #,#,0' x1 vs target 'lui #,0x0' x1 plus nop x1. Neither differs from target by a register assignment or an emission order.
- verdict: KILLED

## [s7] Eliminating the switch-merge CODE_LABEL (s6's headline whole-function reshape lever) removes the orphaned (use regN) and the +8 phantom slot.
- mechanism: s6 pinned the phantom to combine.c:10836-10846 distribute_notes emitting (use reg100) when the homeless REG_DEAD note walks back and hits a CODE_LABEL, and asserted the 4-way switch on D_800A38DC forces that label.
- probe: Four fresh -mel -da forms via tmp/grind/func_80022F34/s7/probe.sh: vIF (switch -> if/else-if chain, jump table gone), vIF2 (reordered chain), vTERN (single store via a conditional expression), vMIN (entire switch/val block deleted - diagnostic, semantics intentionally broken). Read .frame vars=, sp, and the (use (reg:SI N)) count in each .i.combine.
- result: vIF vars=8 strand=1; vIF2 vars=8 strand=1; vTERN vars=8 strand=1; vMIN vars=8 regs=3/0 strand=1. The strand survives with no switch, no jump table and no case-merge label at all - the loop's own CODE_LABEL suffices for distribute_notes. vIF/vIF2/vTERN also emit 5 fewer body insns than base's byte-perfect switch, so they are worse on the body besides.
- verdict: KILLED

## [s7] The strand is a property of val1 (its long lifetime across the a0 reload), so a spelling that shortens or relocates that lifetime removes the phantom while keeping per-access.
- mechanism: s2/s3/s6 all framed the residual as a val1-PLACEMENT / val1-LIFETIME axis ('only val1 strands; val2 folds clean').
- probe: 13 forms on one harness (tmp/grind/func_80022F34/s7/probe2.sh) varying WHICH load is separated, WHERE it is defined relative to the a0 reload, the declared TYPE of D_801027BC, and the address spelling: base, vORIG, vO2 (val1 named but defined AFTER the reload), vO3, vO4 (both named), vO5 (idx1 named only, live across the reload), vO6, vO7 (val2 named ONLY), vO11/vO12 (byte-pointer address arithmetic), vO13, vO14/vO16 (extern s32 D_801027BC[]).
- result: Clean 7/6 dichotomy, zero exceptions. Separate EITHER load into its own named s32 temp -> per-access fold x2, exactly one stranded address pseudo, vars=8, frame -40 (base, vO2, vO4, vO6, vO7, vO13, vO16). Keep BOTH loads inside the single call expression -> cse2 shares one la, no fold, no strand, vars=0, frame -32 (vORIG, vO3, vO5, vO11, vO12, vO14). Placement inert, which-load inert, type inert, byte-pointer arithmetic inert.
- verdict: KILLED

## [s7] Reading the a0 reload into a named pointer so BOTH D_801027BC loads can sit in one call expression removes the phantom frame slot outright.
- mechanism: With no load lifted into its own statement, cse2 shares one symbol base, combine never folds an address into a mem, no REG_DEAD note is orphaned, no pseudo is left unallocated, and alter_reg reserves nothing.
- probe: vORIG body: u8 *nxt = *(u8**)a0; s16 idx1 = *(s16*)(a0+0x4A); s16 idx2 = *(s16*)(nxt+0x4A); single_game_SetStatusUpData(i, (&D_801027BC)[idx1*5], (&D_801027BC)[idx2*5]);  Applied to src/code6cac.c, ran sandbox --disable all, goal_from_tgt classify, and an objdump-level normalized diff vs build/src/code6cac.o.
- result: CONFIRMED for the frame: vars=0, regs=4/0, subu $sp,$sp,32 with saves at 16/20/24/28 - byte-identical to target's prologue AND epilogue; strand=0; the first 39 normalized insns (prologue, loop head, guard, whole switch, sh store, loop tail, epilogue) match target exactly. Score still 11 because the whole residual relocates into one 15-insn block: ours materialises la (lui+addiu) once and indexes it twice, target re-materialises lui/%lo per access. Multiset delta is literally one shape (ours addiu #,#,0 vs target lui #,0x0) plus target's maspsx nop.
- verdict: CONFIRMED

## [s8] Definition order, argument order, which value is named, or naming the SCALED offset instead of the loaded value crosses the s7 separation dichotomy and yields per-access folds without the +8 phantom slot.
- mechanism: s7's law was measured over 13 forms that varied WHICH load was separated and WHERE it was defined. It had not varied the ORDER of two named temps (val2 before val1), a temp defined as the very last statement before the call (so the fold's i3 is adjacent to the definition), the in-place `a0 = *(u8**)a0` reload combined with a fully inline `idx2`, or hoisting the scaled offset `idx*5` rather than the loaded value. Any of those could plausibly give combine's orphaned REG_DEAD note a nearer anchor, or change which pseudo cse elects to share.
- probe: six new forms generated by tmp/grind/func_80022F34/s8/gen.py and measured with tmp/grind/func_80022F34/s8/probe.sh (cc1 -O2 -G0 -mel -da; reads `.frame vars=`, `subu $sp`, the `la $N,D_801027BC` count, the per-access `D_801027BC($N)` count, and the `(use (reg:SI N))` count in `.i.combine`): vP1 (val1 named last), vP2 (both named, reverse order), vP3 (in-place a0 reload + fully inline idx2), vP4 (in-place a0 reload + named idx2, both loads inline), vP5 (scaled offsets o1/o2 named), vP6 (val1 named + val2 named with idx2 inlined).
- result: vP1 vars=8 sp-40 per-access=2 strand=1; vP2 vars=8 sp-40 per-access=2 strand=1; vP6 vars=8 sp-40 per-access=2 strand=1; vP3/vP4/vP5 vars=0 sp-32 la=1 per-access=0. Exactly the two classes s7 measured, with zero exceptions — 19 forms total now. Every "separated statement" form is in the fold/strand class regardless of order or of which quantity is named; every "both loads inside the one call expression" form is in the shared-`la` class.
- verdict: KILLED. Banked: rejected/arg-order-reversal-inert-to-separation-law.c, rejected/val-temp-defined-last-still-strands.c, rejected/scaled-offset-temps-still-share-la.c, rejected/inplace-a0-reload-inline-idx2-still-shares-la.c.

## [s8] The +8 phantom stack slot is caused by combine's `distribute_notes` CODE_LABEL fallback, and is therefore unavoidable for ANY loop-containing spelling in the per-access fold class.
- mechanism: `tools/gcc-2.7.2/combine.c:10829-10846`. When a REG_DEAD note has no home, distribute_notes walks back from i3 looking for an insn that `reg_referenced_p` the dying register; on hitting a `CODE_LABEL` it emits `(use (reg N))` after that label ("This prevents problems with call-state tracking in caller-save.c"). The pseudo then appears ONLY inside a bare USE, so regclass never records a real class for it (lreg: "ST_REGS or none"), greg lists it to allocate with an empty conflict set but omits it from Register dispositions, and reload/alter_reg homes it to a stack slot → vars=8 → sp -40 vs target's -32.
- probe: two deliberately semantics-broken diagnostics on the s8 harness, chosen to vary ONLY the presence of a preceding CODE_LABEL. vDIAG = the loop, the `*(s16*)(a0+6)` guard and the switch all deleted, leaving a straight-line function with no label at all. vDIAG2 = everything deleted EXCEPT the loop, so the loop's own label is the only CODE_LABEL in the function. Both keep the two separated `(&D_801027BC)[idx*5]` loads (fold class).
- result: **vDIAG: per-access=2, strand=0, `subu $sp,$sp,32`. vDIAG2: per-access=2, strand=1, `subu $sp,$sp,40`.** The strand — and with it the +8 slot — appears if and only if a CODE_LABEL precedes combine's i3. Supporting dump evidence: the per-pass symbol-ref census is identical on base and vORIG (4 in .rtl/.jump, 2 from .cse onward), so cse shares in BOTH forms and is NOT the discriminator (this corrects s7's "cse2 elects to share" framing); the discriminator is whether combine can substitute the shared constant into the mems (base: la deleted, both mems `(plus reg symbol_ref)`) or is blocked by the pseudo's second live use (vORIG: insn 100 survives, real `addu` kept).
- result (the closure it implies): the fold that produces target's per-access `lui/%lo` bytes is the same event that deletes the address pseudo's only definition. Giving the orphaned note a legitimate anchor would require a SECOND reference to that pseudo — which is exactly what prevents combine from folding (demonstrated by vORIG's surviving insn 100). Fold and clean note placement are mutually exclusive by construction, and a loop always supplies the CODE_LABEL. Target's form (per-access fold AND vars=0, inside a loop) is therefore unreachable in this fork for every C spelling, not merely for the 19 measured.
- verdict: CONFIRMED (the mechanism) / the fold-class family is CLOSED. Do not re-attempt fold-anchor spellings, note-relocation reshapes, control-boundary insertions, or label elimination — the label is not the removable part; the missing second reference is, and it is unobtainable.

## Revised frontier (grind s8) — floor 11; disposition filed under the 2026-07-27 standing ruling; ONE theoretical re-open route recorded
Both endgame-lock AND-gates re-measured and both FAIL (scan_hand_coded LOW 1/8, only S4; no PSX
entry in docs/reference/sotn-construct-index.md for a phantom / compiler-added stack slot,
distribute_notes, or reg_n_refs staleness). Entry appended to docs/grind/decisions.md
(2026-08-26, names func_80022F34) applying the owner's 2026-07-27 standing auto-ruling:
REFUSED / OWNER-ACCEPTED INCOMPLETE. No family grant requested (2026-08-24 auto-reject class).

The single route that would change the answer, recorded for completeness and NOT reachable by any
of the 19 measured forms: a C spelling in which `expand_expr` emits the address directly as
`(mem (plus (reg) (symbol_ref)))` rather than forcing the symbol into a pseudo that combine later
folds back. `GO_IF_LEGITIMATE_ADDRESS` (tools/gcc-2.7.2/config/mips/mips.h:2325-2349) explicitly
accepts that address form — the "pretend the MIPS supports a constant address + a register" clause,
whose own comment observes "On the other hand, CSE is not as effective." With no pseudo there is no
fold, no orphaned note and no slot, and the bytes are target's. Every measured form (including
array-typed `extern s32 D_801027BC[];` and byte-pointer address arithmetic) forces the pseudo — the
`.rtl` dumps show `(set (reg) (symbol_ref))` in all of them. This is an EXPAND-path question. Do NOT
re-run: ra_solver/sched_solver (s7 PRE-RA FORECLOSED), permuter (s4/s5, three chassis, ~48k iters),
structural placement / control boundaries (s2/s3), switch-label elimination (s7), fold-anchor
spellings or temp-ordering variants (s6/s8).

## [s8] Definition order, argument order, which value is named, or naming the SCALED offset instead of the loaded value crosses s7's separation dichotomy and yields per-access folds without the +8 phantom slot.
- mechanism: s7's 13-form law varied WHICH load was separated and WHERE it was defined, but never the order of two named temps, a temp defined as the very last statement before the call (so combine's i3 is adjacent to the definition), the in-place `a0 = *(u8**)a0` reload combined with a fully inline idx2, or hoisting the scaled offset idx*5 rather than the loaded value. Any of those could plausibly give the orphaned REG_DEAD note a nearer anchor or change which pseudo cse elects to share.
- probe: Six new forms generated by tmp/grind/func_80022F34/s8/gen.py and measured with tmp/grind/func_80022F34/s8/probe.sh (cc1 -O2 -G0 -mel -da; reads .frame vars=, subu $sp, the `la $N,D_801027BC` count, the per-access `D_801027BC($N)` count, and the `(use (reg:SI N))` count in .i.combine): vP1 val1-named-last, vP2 both-named-reverse-order, vP3 in-place-a0-reload+inline-idx2, vP4 in-place-a0-reload+named-idx2, vP5 scaled-offsets-named, vP6 val1-named+val2-named-with-inline-idx2.
- result: vP1 vars=8 sp-40 per-access=2 strand=1; vP2 vars=8 sp-40 per-access=2 strand=1; vP6 vars=8 sp-40 per-access=2 strand=1; vP3/vP4/vP5 vars=0 sp-32 la=1 per-access=0. Exactly s7's two classes, zero exceptions - 19 forms total now. Every separated-statement form is in the fold/strand class regardless of order or of which quantity is named; every both-loads-in-one-call-expression form is in the shared-la class.
- verdict: KILLED

## [s8] The +8 phantom stack slot is caused by combine's distribute_notes CODE_LABEL fallback, and is therefore unavoidable for ANY loop-containing spelling in the per-access fold class.
- mechanism: tools/gcc-2.7.2/combine.c:10829-10846: when a REG_DEAD note has no home, distribute_notes walks back from i3 for an insn that reg_referenced_p the dying register; on hitting a CODE_LABEL it emits `(use (reg N))` after that label ('This prevents problems with call-state tracking in caller-save.c'). The pseudo then appears only inside a bare USE, so regclass never records a real class (lreg: 'ST_REGS or none'), greg lists it to allocate with an empty conflict set but omits it from Register dispositions, and reload/alter_reg homes it to a stack slot -> vars=8 -> sp -40 vs target's -32.
- probe: Two deliberately semantics-broken diagnostics on the s8 harness, varying ONLY the presence of a preceding CODE_LABEL. vDIAG = loop, guard and switch all deleted (straight-line, no label anywhere). vDIAG2 = everything deleted except the loop (its label is the only CODE_LABEL). Both keep the two separated (&D_801027BC)[idx*5] loads, i.e. the fold class. Supporting: per-pass `grep -c D_801027BC` census across .rtl/.jump/.cse/.loop/.cse2/.flow/.combine on base and vORIG, plus reading the combine dumps at the la and the two argument-load insns.
- result: vDIAG: per-access=2, strand=0, subu $sp,$sp,32. vDIAG2: per-access=2, strand=1, subu $sp,$sp,40. The strand - and with it the +8 slot - appears iff a CODE_LABEL precedes combine's i3. The census is IDENTICAL on base and vORIG (4 symbol_refs in .rtl/.jump, 2 from .cse onward), so cse shares in BOTH forms and is NOT the discriminator (this corrects s7's 'cse2 elects to share' framing); the discriminator is whether combine can substitute the shared constant into the mems (base: la deleted, both mems `(plus reg symbol_ref)`) or is blocked by the pseudo's second live use (vORIG: insn 100 `(set (reg 96) (symbol_ref))` survives with uses at insns 108 and 120, so the real addu is kept and the mems stay `(mem (reg))`).
- verdict: CONFIRMED

## [s8] A fold-anchor spelling can give the orphaned REG_DEAD note a legitimate home while keeping the per-access fold (the family s6 opened and s7 continued).
- mechanism: If a real insn between the CODE_LABEL and i3 referenced the address pseudo, distribute_notes would place the note there instead of taking the USE-at-label fallback.
- probe: Deduced from the confirmed mechanism above and cross-checked against the vORIG combine dump: the anchor would have to be a SECOND reference to the address pseudo, and combine only folds a definition it can delete - vORIG shows exactly what happens when a second use exists (insn 100 survives, no fold, mems stay `(mem (reg))`, vars=0 but the body is wrong).
- result: Fold and clean note placement are mutually exclusive by construction, not by accident of spelling. A loop always supplies the preceding CODE_LABEL (vDIAG2). Target's form - per-access fold AND vars=0, inside a loop - is unreachable in this fork for every C spelling, not merely for the 19 measured.
- verdict: KILLED

## [s9] A C spelling exists in which expand_expr builds the D_801027BC access address directly as `(mem (plus (reg) (symbol_ref)))`, bypassing the address pseudo and therefore the whole combine fold / orphaned-note / phantom-slot chain (s8's last live frontier).
- mechanism (as banked by s8): GO_IF_LEGITIMATE_ADDRESS (tools/gcc-2.7.2/config/mips/mips.h:2325-2349) accepts CONSTANT_ADDRESS + REG under the "pretend that the MIPS supports an address mode of a constant address + a register" clause, so such a mem is valid at expand time; it is only because expand forces `(set (reg) (symbol_ref))` that combine has to fold the constant back in later, and that fold is what deletes the pseudo's definition and orphans the REG_DEAD note.
- probe: Read the expand output itself instead of guessing — `pwsh tools/grinder/dump.ps1 func_80022F34` on the candidate chassis (floor re-measured 11 this session), then the `.rtl` (post-expand) slice for func_80022F34, banked at tmp/grind/func_80022F34/s9/rtl-expand-addr-slice.txt (dump lines 23315-23420). Then traced the three source paths that decide the shape: expr.c:4563 (INDIRECT_REF -> EXPAND_SUM -> memory_address), expr.c:5286-5291 + 6323-6332 (PLUS_EXPR both_summands / ADDR_EXPR under EXPAND_SUM), and explow.c:385-470 (memory_address) + explow.c:274-291 (break_out_memory_refs).
- result: KILLED, at compiler-source level and spelling-invariantly. The frontier's premise about expand_expr is actually CORRECT — ADDR_EXPR under EXPAND_SUM returns the bare symbol_ref (expr.c:6323-6332) and both_summands returns an UNFORCED `(plus (reg) (symbol_ref))` (expr.c:5286-5291). The kill is one level below, inside memory_address: explow.c:414-416 runs `if (! cse_not_expected && GET_CODE (x) != REG) x = break_out_memory_refs (x);` BEFORE it ever consults GO_IF_LEGITIMATE_ADDRESS, and break_out_memory_refs (explow.c:274-291) recurses through PLUS and unconditionally `force_reg`s any operand that is `CONSTANT_P && CONSTANT_ADDRESS_P && GET_MODE != VOIDmode`. A SYMBOL_REF satisfies all three (it always carries Pmode), so the symbol is materialised into a pseudo before the legitimacy test; the residual `(plus (reg) (reg))` is not a legitimate MIPS address, so the tail of memory_address force_operand()s it into the second pseudo. The .rtl confirms exactly this: insn 94 `(set (reg 95) (symbol_ref "D_801027BC"))`, insn 102 `(set (reg 100) (plus (reg 99) (reg 95)))`, insn 104 `(set (reg 92) (mem (reg 100)))` — both pseudos present at EXPAND, before any optimisation pass, and the mem is `(mem (reg))`, never `(mem (plus reg symbol_ref))`. The mips.h "pretend" clause is unreachable at expand for every symbol+runtime-variable address in this fork; it is only reachable when a later pass hands rtl to recog, i.e. precisely the combine fold we already have. The only gate is `cse_not_expected`, a compiler-internal flag that is 0 during expand for every function here and that no C source can flip.
- verdict: KILLED

## Frontier after s9 — EMPTY

s9 removes the last item from the frontier. The chain is now pinned to compiler
source at every link, and its FIRST link is spelling-invariant:
  symbol + runtime-variable address
    => address pseudo, forced at expand, unconditionally (explow.c:274-291)
  per-access lui/%lo bytes (target's form)
    => combine must fold that pseudo away and delete its only definition
  deleting the only definition
    => the pseudo's REG_DEAD note has no home (combine.c:10829-10846)
  homeless note + any preceding CODE_LABEL (a loop always supplies one; vDIAG2)
    => `(use (reg N))` after the label => no hard reg => reload homes it
    => vars=8 => sp -40 vs target's -32 => the 10 frame-offset diffs
  + 1 maspsx load-delay nop = the honest floor of 11.
An anchor for the note requires a second reference to the pseudo, and a second
reference is exactly what blocks the fold (vORIG, s8). Mutually exclusive.
Do NOT re-open with: ra_solver/sched_solver (s7, PRE-RA FORECLOSED), permuter
(s4/s5, three chassis, ~48k iters), structural placement or control boundaries
(s2/s3), switch-label elimination (s7), fold-anchor / temp-ordering / naming
variants (s6/s8), or the expand-path lever (s9, this entry).

## [s9] A C spelling exists in which expand_expr builds the D_801027BC access address directly as (mem (plus (reg) (symbol_ref))) rather than forcing the symbol into a pseudo, giving per-access bytes with no combine fold, no orphaned REG_DEAD note and no phantom frame slot (s8's last live frontier).
- mechanism: GO_IF_LEGITIMATE_ADDRESS (tools/gcc-2.7.2/config/mips/mips.h:2325-2349) accepts CONSTANT_ADDRESS + REG under the 'pretend that the MIPS supports an address mode of a constant address + a register' clause, so such a mem is valid at expand time; it is only because expand forces (set (reg) (symbol_ref)) that combine has to fold the constant back in later, and that fold is what deletes the pseudo's definition and orphans the REG_DEAD note.
- probe: pwsh tools/grinder/dump.ps1 func_80022F34 on the candidate chassis, then READ the post-expand .rtl slice for func_80022F34 (tmp/grind/func_80022F34/s9/rtl-expand-addr-slice.txt, dump lines 23315-23420) instead of guessing pass attribution; then traced the three deciding source paths: expr.c:4563 (INDIRECT_REF -> EXPAND_SUM -> memory_address), expr.c:5286-5291 + 6323-6332 (PLUS_EXPR both_summands / ADDR_EXPR under EXPAND_SUM), explow.c:385-470 (memory_address) and explow.c:274-291 (break_out_memory_refs).
- result: KILLED spelling-invariantly. The frontier's premise about expand_expr is correct as far as it goes: ADDR_EXPR under EXPAND_SUM returns the bare symbol_ref, and both_summands returns an UNFORCED (plus (reg) (symbol_ref)). The kill is one level down. memory_address runs break_out_memory_refs at explow.c:414-416 BEFORE it consults GO_IF_LEGITIMATE_ADDRESS, and break_out_memory_refs (explow.c:274-291) recurses through the PLUS and unconditionally force_regs any operand that is CONSTANT_P && CONSTANT_ADDRESS_P && GET_MODE != VOIDmode, which every SYMBOL_REF is (it always carries Pmode). The residual (plus (reg) (reg)) is not a legitimate MIPS address, so the tail of memory_address force_operands it into a second pseudo. The .rtl confirms both pseudos exist at EXPAND, before any optimisation pass: insn 94 (set (reg 95) (symbol_ref "D_801027BC")), insn 102 (set (reg 100) (plus (reg 99) (reg 95))), insn 104 (set (reg 92) (mem (reg 100))). The mips.h const+reg 'pretend' clause is therefore unreachable at expand for ANY symbol+runtime-variable address in this fork; its only gate is the compiler-internal cse_not_expected flag, which no C source can flip.
- verdict: KILLED

## [s9] The endgame-lock canonical-asm gate (a) is open for func_80022F34 on the current chassis.
- mechanism: scan_hand_coded's STRONG signals (S1 multu pacing / S2 empty branch / S6 BIOS jumptable) would route the function to the canonical-asm grant path instead of a terminal refusal.
- probe: python3 tools/scan_hand_coded.py --single func_80022F34
- result: tier=LOW, score 1/8. Only S4 (front loads: 4 loads in an 8-insn window at insn 35) fires; S1, S2, S3, S5, S6, S7, S8 all negative. Identical to the s6 and s8 measurements. Gate FAILS - the canonical-asm grant path is not open.
- verdict: KILLED

## [s9] An in-hand SOTN-master (PSX / GCC 2.7.2) precedent exists for the closing construct, i.e. endgame-lock gate (b) is open.
- mechanism: A citable file:line precedent for a construct that removes a compiler-added frame slot would let the closing spelling be argued under an existing sanctioned family.
- probe: Census of docs/reference/sotn-construct-index.md for /frame/ and for /phantom|frame slot|frame size/.
- result: 2 lines match /frame/; zero PSX entries match the frame-slot patterns (the single hit, src/dra/8C600.c:180, is a PSP-tagged comment). There is also no closing construct to cite a precedent FOR: the slot is created by reload downstream of combine, so nothing expressible in C removes it. Gate FAILS - a negative census is a failed gate, not an open question.
- verdict: KILLED

## [s10] The 11-point residual is an OBJECT-MODEL defect, not a codegen defect: declaring D_801027BC as the array of 20-byte records that the target's own addressing proves it to be, and spelling the two accesses as a 2-D ARRAY_REF `D_801027BC[idx][0]`, makes expand emit the mem address directly as `(mem (plus (symbol_ref) (reg)))` with no address pseudo — and therefore with no combine fold, no orphaned REG_DEAD note, no `(use (reg N))` strand and no phantom frame slot.
- mechanism: s9 killed "reach the mips.h:2325-2349 CONSTANT_ADDRESS+REG clause at expand" by showing memory_address (explow.c:414-416) runs break_out_memory_refs (explow.c:274-291) first, which unconditionally force_regs any CONSTANT_P && CONSTANT_ADDRESS_P operand of a PLUS. That is true of a PLUS tree BUILT FROM `&scalar` — which is what every s1-s9 form had, because the header declared `extern s32 D_801027BC;` and the only way to index a scalar is the pointer pun `(&D_801027BC)[idx * 5]`. It is NOT true of an ARRAY_REF whose outer element type is itself an array: expand builds that address symbolically and emits the mem directly, never handing a symbol-bearing PLUS to memory_address. s9's kill was therefore correct within the scalar-declaration object model and void outside it.
- probe: (1) `include/code6cac.h:467` `extern s32 D_801027BC;` -> `extern s32 D_801027BC[][5];` and `:450` `extern u8 D_80102782;` -> `extern u8 D_80102782[];`; (2) body spelled `D_801027BC[idx1][0]` / `D_801027BC[idx2][0]` / `D_80102782[i]` (tmp/grind/func_80022F34/s10/vCLEAN2.c, otherwise IDENTICAL to the s2-s9 base chassis — same loop, same switch, same temps, same order); (3) `sandbox func_80022F34 --disable all`; (4) append `func_80022F34` to maspsx_label_nop_funcs.txt and re-measure; (5) `verify-oracle` on the full tree; (6) read the post-expand `.rtl` slice at tmp/grind/func_80022F34/s10/rtl-arrayref-no-address-pseudo.txt.
- result: CONFIRMED, decisively. Header + body alone: score 1 (down from the 9-session floor of 11), rules_dropped 0, build_insns 69 / target 70 — the lone diff is a missing load-delay nop. Plus the maspsx gate line: score 0, build_insns 70 == target 70, rules_dropped 0, zero cheat-asm. Full-tree `verify-oracle`: build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa == original_sha1_locked, build_matches true — so the two declaration corrections perturb no other translation unit and the per-function maspsx gate causes no index cascade in code6cac.c. The `.rtl` shows the mechanism directly: insn 104 `(set (reg/v:SI 92) (mem/s:SI (plus:SI (symbol_ref:SI ("D_801027BC")) (reg:SI 98))))` at EXPAND, with no `(set (reg) (symbol_ref))` feeding it (insns 100/102 are dead array-base decays that DCE removes) — versus the base chassis's insn 94/102/104 pseudo chain banked by s9. vars=0, `subu $sp,$sp,32`.
- verdict: CONFIRMED

## [s10] A FLAT 1-D array declaration (`extern s32 D_801027BC[];` + `D_801027BC[idx * 5]`) is equivalent to the 2-D form for this purpose — i.e. the win is "declare it as an array", not "declare it 2-D".
- mechanism: If merely giving the symbol array type were enough to keep expand off the memory_address path, the 1-D spelling with the stride folded into the index would match too, and the 2-D shape would be cosmetic.
- probe: `extern s32 D_801027BC[];` in include/code6cac.h + tmp/grind/func_80022F34/s10/vFLAT1D.c (vCLEAN2 with `D_801027BC[idx1 * 5]` / `D_801027BC[idx2 * 5]`), maspsx gate line PRESENT so the nop is not part of the score. `sandbox func_80022F34 --disable all`.
- result: score 10, build_insns 70, rules_dropped 0 — the +8 phantom frame slot and all 10 frame-offset diffs are back. A 1-D ARRAY_REF with a runtime index and a unit element type collapses to the same symbol-bearing PLUS tree as the `&scalar` pun, re-enters memory_address / break_out_memory_refs, and reproduces the s8 fold/strand chain exactly. The 2-D declaration is load-bearing, not cosmetic: the outer ARRAY_REF must have ARRAY type for expand to keep the address symbolic.
- kill_scope: instance
- measured_on: base chassis (vCLEAN2 body, s2-s9 control flow), maspsx label-nop gate ON, zero FAKE constructs, honest sandbox rules_dropped 0
- verdict: KILLED

## [s10] s9's banked kill of the mips.h:2325-2349 CONSTANT_ADDRESS+REG clause, as measured on the scalar-declaration pun chassis, also holds on an array-typed-declaration chassis.
- mechanism: s9 read explow.c:274-291 / 414-416 and concluded break_out_memory_refs force_regs the SYMBOL_REF before GO_IF_LEGITIMATE_ADDRESS is consulted, spelling-invariantly.
- probe: The s10 2-D ARRAY_REF form's post-expand `.rtl` (tmp/grind/func_80022F34/s10/rtl-arrayref-no-address-pseudo.txt).
- result: FALSIFIED. insn 104 is `(mem/s:SI (plus:SI (symbol_ref "D_801027BC") (reg 98)))` at expand — the clause IS reached. s9's reasoning was sound about the code path it read, but that path is only taken for an address expression that arrives at memory_address as a PLUS containing the symbol; an ARRAY_REF on an array-typed declaration never constructs one. The kill should have been scoped to the scalar-declaration object model. LESSON for the ledger: a compiler-source class kill is only as wide as the SOURCE-LEVEL shapes that can reach the code path read, and the DATA MODEL signal in the dispatch brief ("D_801027BC is indexed with a computed register but declared `extern s32`; declare it as an array") named the escape nine sessions before it was taken.
- kill_scope: instance
- measured_on: s10 2-D array chassis, zero FAKE constructs, honest sandbox rules_dropped 0
- verdict: KILLED

## [s10-rerun] The s10 score-0 configuration reproduces on a freshly-restored clean HEAD chassis (the discarded s10 run's measurements were not an artefact of its own dirty tree).

- mechanism: The prior s10 run was discarded by the driver validator for outcome-JSON wording only, and its tree was reverted. If the score-0 result had depended on any leftover state (a stale sandbox object, a partially-applied header edit, a `build/` reference built from the fix), a clean re-application from committed HEAD would not reproduce it.
- probe: From a clean `git status` at HEAD, re-applied the four edits mechanically via `tmp/grind/func_80022F34/s10/apply_s10b.py apply` (header `extern s32 D_801027BC[][5];` + `extern u8 D_80102782[];`; body from `memory/grind/func_80022F34/candidate.c` replacing the `INCLUDE_ASM` line; one appended line in `maspsx_label_nop_funcs.txt`), then `& tools/wteng.ps1 main sandbox func_80022F34 --disable all`, then `verify-oracle --rebuild --allow-dirty` followed by `verify-oracle`. Then reverted all three build files and re-ran `verify-oracle --rebuild` to restore the canonical reference.
- result: CONFIRMED. Sandbox: `"score": 0`, `"target_insns": 70`, `"build_insns": 70`, `"scorable": true`, `"rules_dropped": 0`, `"strip_cheat_asm": true` (the `cheat_asm_stripped: 23` count is other functions in code6cac.c; func_80022F34 itself carries none). Full-tree oracle in that configuration: `"ok": true`, `"build_matches": true`, `"build_sha1": "62efab4f73f992798c43e8c730aa43baa10bb4fa"` == `"original_sha1_locked"`. After revert, `verify-oracle --rebuild` again returned `"ok": true` with the same SHA1, so `build/` is left consistent with committed HEAD and the next session inherits a clean reference. Matched artefacts banked at `tmp/grind/func_80022F34/s10/code6cac.c.matched.s10b` and `.../maspsx_label_nop_funcs.txt.matched`.
- verdict: CONFIRMED

## [s10-rerun] A grind session can stage the whole remedy itself, i.e. this is a `candidate-ready` rather than an integration handoff.

- mechanism: If `include/code6cac.h` and `maspsx_label_nop_funcs.txt` were inside the session's allowed staging surface, the fix could be landed in-session and submitted to layer-1/Judge directly.
- probe: Checked `memory/grind/func_80022F34/` for a per-function `scope_allow` grant (none present; `state.json` carries no scope widening) and read `.claude/rules/integration-handoff-self-serve.md`, which reserves `add-scope-allow` to the DRIVER on a Judge `ESCALATE(integration-handoff)` and places the maspsx fidelity-gate lists on that grant's permanent denylist.
- result: KILLED. Neither surface is stageable by a session on the current grant state: `include/*.h` requires a driver-issued scope line (grantable, pipeline-executable), and `maspsx_label_nop_funcs.txt` is denylisted for `add-scope-allow` and needs an operator hand-apply of one line. Staging them anyway would make the session's diff foreign dirt and get it discarded by the SCOPE CHECK. The correct disposition is therefore `owner-gated` with an INTEGRATION HANDOFF entry, not `candidate-ready` — which is exactly what this session filed at `docs/grind/decisions.md` (2026-09-05, "INTEGRATION HANDOFF (bytes RE-PROVEN this session)").
- kill_scope: instance
- measured_on: clean HEAD chassis at commit cf645f90, no per-function scope_allow grant present, zero FAKE constructs
- verdict: KILLED

## [s10] The s10 score-0 configuration (2-D array declaration of D_801027BC + array declaration of D_80102782 + the banked body + the maspsx label-nop gate line) reproduces from a freshly-restored clean HEAD, so the discarded s10 run's measurements were not an artefact of its own tree state.
- mechanism: The prior s10 run was discarded by the driver validator for outcome-JSON wording only and its tree was reverted. If score 0 had depended on leftover state (a stale sandbox object, a partially-applied header edit, or a build/ reference built from the fix), a clean mechanical re-application from committed HEAD would not reproduce it.
- probe: From clean git status at HEAD cf645f90, applied the four edits via tmp/grind/func_80022F34/s10/apply_s10b.py (header include/code6cac.h:467 -> 'extern s32 D_801027BC[][5];' and :450 -> 'extern u8 D_80102782[];'; src/code6cac.c:2467 INCLUDE_ASM replaced by the memory/grind/func_80022F34/candidate.c body; one appended line 'func_80022F34' in maspsx_label_nop_funcs.txt), then '& tools/wteng.ps1 main sandbox func_80022F34 --disable all', then 'verify-oracle --rebuild --allow-dirty' followed by 'verify-oracle'; then reverted all three build files and re-ran 'verify-oracle --rebuild' to restore the canonical reference.
- result: CONFIRMED. Sandbox: score 0, target_insns 70, build_insns 70, scorable true, rules_dropped 0, strip_cheat_asm true (the cheat_asm_stripped count of 23 is other functions in code6cac.c; func_80022F34 itself carries zero cheat-asm and zero FAKE constructs). Full-tree oracle in that configuration: ok true, build_matches true, build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == original_sha1_locked. After revert, verify-oracle --rebuild returned ok true with the same SHA1, so build/ is left consistent with committed HEAD. Matched artefacts banked at tmp/grind/func_80022F34/s10/code6cac.c.matched.s10b and maspsx_label_nop_funcs.txt.matched.
- verdict: CONFIRMED

## [s10] A grind session can stage the whole remedy itself on the current grant state, making this a candidate-ready rather than an integration handoff.
- mechanism: If include/code6cac.h and maspsx_label_nop_funcs.txt were inside the session's allowed staging surface, the fix could be landed in-session and submitted to layer-1 and the Judge directly instead of being routed through a handoff record.
- probe: Checked memory/grind/func_80022F34/ for a per-function scope_allow grant (none present; state.json carries no scope widening) and read .claude/rules/integration-handoff-self-serve.md, which reserves grindlib.py add-scope-allow to the DRIVER on a Judge ESCALATE(integration-handoff) verdict and places the maspsx fidelity-gate lists on that grant's permanent denylist.
- result: KILLED. Neither surface is stageable by a session at the current grant state: include/*.h requires a driver-issued scope line (grantable, pipeline-executable, scope_paths=["include/code6cac.h", "src/code6cac.c"]), and maspsx_label_nop_funcs.txt is denylisted for add-scope-allow and needs an operator hand-apply of one line. Staging them anyway would make the session diff foreign dirt and get the session discarded by the SCOPE CHECK. Disposition is therefore owner-gated with an INTEGRATION HANDOFF entry, which this session filed.
- verdict: KILLED
- kill_scope: instance
- measured_on: clean HEAD chassis at commit cf645f90, no per-function scope_allow grant present for func_80022F34, zero FAKE constructs
