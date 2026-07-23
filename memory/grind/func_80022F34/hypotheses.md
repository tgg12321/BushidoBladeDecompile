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
