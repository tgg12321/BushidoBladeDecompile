# Hypothesis ledger — func_8003B10C

## s2 [structural, 2026-07-23]
- KILLED-BY-SOLVING: web split (v9 move copy). Recompute-inline of arg0*1100 removes the variable pseudo -> single CSE temp -> no copy, frame -32, delay nop. CONFIRMED via RTL (insn34 copy 74<-85) + sandbox asm.
- CONFIRMED: floor 8 achievable in pure C (hoisted family) but capped (61 insns vs target 64; ED6 not duplicated -> never 0).
- OPEN (0-path): duplicated chassis (dup13_form.c, score 13). Residual = tbl->a1/ED6->a0 register assignment driven by sched1 hoisting the ED6 load above the ED2 mult. Structural levers exhausted (all inert or worse). Next: SCHEDULING/PERMUTER modality to seat ED6 in v1 (read after the mult) so tbl keeps a0.

## [s2] v9's `move s2,s0` copy is a cse.c canon_reg EBB web split of the arg0*1100 offset; removing the offset variable (recompute arg0*1100 inline at every use) unifies it to one pseudo and kills the copy.
- mechanism: expand emits mult result into temp pseudo 85, insn34 copies it into reg/v variable pseudo 74 (s0=...); cse canon_reg makes 85 canonical in the fall-through EBB (if-arm+EDA) while the else-arm keeps 74 -> two co-live pseudos -> local-alloc cannot coalesce -> move copy survives to codegen (frame -40, +s3 save).
- probe: Reconstructed v9 (score 36 with the copy), took cc1 -da RTL dumps (confirmed insn34 copy 74<-85 in base.i.rtl/.greg), then deleted the s0 variable and inlined arg0*1100 at all 5 offset uses; sandbox + read the emitted asm.
- result: Recompute-inline: single CSE temp (s0), move copy GONE, frame -32, 493E4 jal delay-slot NOP restored. Prologue+mult+EDA+branch (29 insns) byte-match target exactly.
- verdict: CONFIRMED

## [s2] A pure-C form can beat HEAD's floor of 10.
- mechanism: recompute-inline + `s32 e2` staging ED2 through its own local (fixes if-arm ED2->v1 and the *3 mult register order) + inline ED6 read; all vars live, no cheats.
- probe: sandbox --disable all on the clean hoisted form.
- result: score 8 at 61 insns (pure C). But HOISTED family is a matching dead-end: target is 64 insns with ED6 duplicated per-arm, so a 61-insn single-read form is 3 insns short of 0 permanently.
- verdict: CONFIRMED

## [s2] The duplicated 0-path chassis's residual is a small register-assignment gap fixable by structural levers (var split / decl order / type narrow / statement reorder).
- mechanism: splitting the if-arm *6 as `v0=ED2*3; v1=ED6; v0*=2;` keeps arm-specific code after the ED6 read so jump2 does not cross-jump-merge it -> ED6 stays per-arm (64 insns). Residual = tbl->a1 (target a0) + ED6->a0 (target v1).
- probe: swept split-init (inert), decl-order (inert), statement-reorder (inert), branch-swap (worse 19-38), tbl-inline (worse 17), asymmetric-e2 (16) on the duplicated chassis.
- result: Best duplicated = 13 (cleaner than historic v9's 13: frame -32, no copy). The 2-register residual is a sched1 decision, NOT structural: sched1 hoists the independent ED6 load ABOVE the ED2 mult (ED2 still live in v1 -> ED6 forced to a0, tbl bumped to a1); target reads ED6 AFTER the mult so it reuses the freed v1 and tbl keeps a0. No structural lever moves the schedule.
- verdict: KILLED
