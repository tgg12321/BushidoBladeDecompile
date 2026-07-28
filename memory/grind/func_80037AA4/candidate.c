/* floor=4 (s1, re-verified s2 2026-07-28). Pure C, no pins, no dummy. Residual =
   the phantom 8-byte frame only (addiu sp,-8/+8 + final jr-delay nop; build 20
   vs target 23 insns). Loop, tail, and ALL register assignments match target.
   s1 lever: POINTER DECLARED FIRST flips the a0<->v1 rename (works here because
   sum 10/15 and p 7/7 allocno priorities TIE at 20000 and the tie-break picks
   the lower pseudo number = the pointer).
   s2: the frame IS naturally inducible — guard spelled `if (var_a1 < var_a2)`
   orphans an slt pseudo that reload gives a dead 4-byte slot (vars=8, all 23
   insns, sandbox 11) — but the orphan's cse-canonicalized operand charges sum
   +1 ref (11/15 = 22000 > 20000), un-tying the priorities and swapping
   sum<->p ($3/$4) throughout. See rejected/orphan-guard-sum-p-swap.c and
   evidence.md s2 for the full arithmetic + the len>=17 flip condition. */
s32 func_80037AA4(void) {
    s8 *var_v1;
    s32 var_a1;
    s32 var_a2;
    s32 var_a0;
    s32 var_v0;

    var_a1 = 0;
    var_a0 = 0;
    var_a2 = D_800A38C8;
    if (var_a2 > 0) {
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
