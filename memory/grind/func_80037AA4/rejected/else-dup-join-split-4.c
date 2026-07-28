/* s3 (2026-07-28): the else-arm dup form — the ONLY measured construct that
   achieves the full register flip (vars=8, sum=$4, p=$3, blez $6, sra $4,$2:
   every one of the 21 cc1 insns byte-correct) by giving sum live_length 17
   (refs 11 -> pri 19411 < p 20000). sandbox = 4 (ties the floor, residual is
   now 2-3 INSERTED insns instead of the missing frame): the then-arm copy
   needs `j .L35` around the else arm, and the else-arm `var_v0 = var_a0`
   const-folds to `move $2,$0` (cse follows the taken-branch path and knows
   sum==0 there — any skip-path copy folds; both-arms identity for cross-jump
   merging is therefore unreachable). Neither insn is deleted post-greg:
   jump2's noop_moves only kills reg-to-same-reg moves, and the j is only
   deletable if the move dies first. Banked as the best-understood permuter
   seed: one 2-insn deletion event from 0. */
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
        var_v0 = var_a0;
    } else {
        var_v0 = var_a0;
    }
    if (var_a0 < 0) {
        var_v0 = var_a0 + 0x1FFF;
    }
    var_a0 = var_v0 >> 0xD;
    return 0xF - var_a0;
}
