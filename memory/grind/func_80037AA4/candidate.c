/* floor=4 (s1, 2026-07-28). Pure C, no pins, no dummy. The ONLY residual is the
   phantom 8-byte frame (target addiu sp,-8/+8 with no sp stores; cc1 vars=0 here
   vs target vars=8). Loop, tail, and ALL register assignments match target.
   Key lever found s1: POINTER DECLARED FIRST flips the a0<->v1 rename that the
   old WIP called blocked (its decl-reorder probe moved var_a0 first, which is inert;
   var_v1 first is the lever). */
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
