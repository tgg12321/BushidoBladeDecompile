/* s3 (2026-07-28): the entire guard-chain spelling space is double-walled.
   Representative below: `(var_a1 < var_a2) & var_v0` with var_v0 = 1.
   (a) TREE-FOLD wall: a2>=1, 1<=a2, !(a2<1), (..)!=0, (..)==1, &1, ^1, |0,
       <<31<0, <<1!=0, neg<0, *2!=0 — ALL canonicalize at tree/expand level
       to the plain compare (byte-identical to g0 or g1, measured).
   (b) CSE wall: routing the constant through a var defeats tree-fold, but
       cse.c fold_rtx const-props and simplifies every and/xor/eq/ge-of-
       compare once the constant is visible: lreg stats byte-identical to g1
       (sum 11/15) for the whole y-family.
   (c) Even a chain SURVIVING to flow is worthless: combine DECREMENTS
       reg_live_length for insns it deletes (flow dump sum 11/18 -> lreg
       11/15 in g1; z1's ashift present at flow, deleted by combine, len
       unchanged). The s2 frontier premise "combine-deleted bb0 insns add
       live_length" is impossible by construction. */
s32 func_80037AA4(void) {
    s8 *var_v1;
    s32 var_a1;
    s32 var_a2;
    s32 var_a0;
    s32 var_v0;

    var_a1 = 0;
    var_a0 = 0;
    var_a2 = D_800A38C8;
    var_v0 = 1;
    if ((var_a1 < var_a2) & var_v0) {
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
