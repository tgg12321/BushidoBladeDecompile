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
