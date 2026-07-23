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

## [s2] A frame-correct (vars=0) CSE form scores below 11 because CSE costs fewer instructions than the +8 phantom.
- mechanism: CSE'd shared `la $5` base for both accesses is shorter than per-access %hi/%lo; if the CSE + a0-reload-in-$v0 diffs total < 10, a frame-correct form would beat the phantom.
- probe: Applied vMIRROR (target-order both-CSE) and vSPLIT (val2 per-access, val1 CSE) to src; sandbox --disable all.
- result: vMIRROR = 11 (build_insns 64), vSPLIT = 11 (build_insns 64). The CSE'd base + a0-reload-in-$v0 (vs target $a0) + nop sum to exactly 11. No frame-correct structural form beats base.
- verdict: KILLED
