/* REJECTED s2 (2026-07-28): sandbox 11 (build 23/23 insns == target count!).
   The `if (var_a1 < var_a2)` guard (a1 just zeroed) is the NATURAL vars=8 trigger:
   expand emits a reg-reg slt pseudo; cse folds (a1=0); combine folds slt+branch
   -> blez a2 and orphans the slt pseudo (ST_REGS, stale refs); reload alter_reg
   allocates it a dead 4-byte spill slot -> addiu sp,-8/+8 + final jr-delay nop,
   all three missing insns of the floor-4 form, WITH byte-perfect structure.
   FATAL: the orphan slt's surviving operand is cse-canonicalized to var_a0 (sum)
   -- cse.c make_regs_eqv picks the LONGEST-LIVED member of the zero-equivalence
   class, and sum's last use is the function's final insn, so sum is ALWAYS the
   canonical rep no matter which zero-var the guard names. That +1 ref makes
   sum's global.c allocno priority 3*11/15=22000 vs pointer 2*7/7=20000
   (g0 ties 10/15 -> 20000=20000, tie-break by lower allocno = pointer wins),
   so global-alloc allocates sum FIRST -> sum=$3/p=$4, swapped vs target
   ($4/$3) in all 11 sum/p-touching insns. Swap is invariant across: all 120
   decl orders, all init orders, guard operand identity (a1/a0/v0/reversed),
   loop stmt order, address-temp splits (cse-folds), split-addiu chains
   (cse-folds), nested/&& double guards (2nd test either cse-deduped, survives
   as a second blez -- jump2 does NOT dedup -- or lands in a separate bb where
   combine can't fold it). */
s32 func_80037AA4(void) {
    s8 *var_v1;
    s32 var_a1;
    s32 var_a2;
    s32 var_a0;
    s32 var_v0;

    var_a1 = 0;
    var_a0 = 0;
    var_a2 = D_800A38C8;
    if (var_a1 < var_a2) {
        var_v1 = (s8 *)&D_80102810;
        do {
            var_v0 = *(s32 *)(var_v1 + 0x18);
            var_a1 += 1;
            var_a0 += var_v0;
            var_v1 += 0x28;
        } while (var_a1 < var_a2);
    }
    var_v0 = var_a0;
    if (var_a0 < 0) {
        var_v0 = var_a0 + 0x1FFF;
    }
    var_a0 = var_v0 >> 0xD;
    return 0xF - var_a0;
}
