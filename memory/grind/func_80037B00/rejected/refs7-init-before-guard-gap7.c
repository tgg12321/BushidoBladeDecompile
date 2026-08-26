s32 func_80037B00(u8 *arg0) {
    s32 var_t1;
    s32 var_t2;
    s8 *var_a3;
    s8 *var_a1;
    s8 *var_a2;
    s8 *var_t0;
    s32 var_v1;
    s32 var_v0;
    s32 var_t3;



    var_t1 = 0;
    if (D_800A38C8 > 0) {
        var_t3 = D_800A38C8;
        var_a3 = (s8 *)&D_80102810;
        do {
            var_t2 = 0;
            var_a1 = var_a3;
            var_a2 = (s8 *)arg0;
            var_t0 = var_a3 + 0x15;
        loop_inner:
            var_v1 = (u8)*var_a2;
            if (var_v1 == 0) {
                goto block_5c;
            }
            var_v0 = (u8)*var_a1;
            if (var_v1 != var_v0) {
                goto block_6c;
            }
            var_a1 += 1;
            var_a2 += 1;
            if ((s32)var_a1 < (s32)var_t0) {
                goto loop_inner;
            }
        block_5c:
            var_t1 += 1;
            if (var_t2 != 0) {
                goto block_74;
            }
            return 1;
        block_6c:
            var_t2 = 1;
            goto block_5c;
        block_74:
            var_a3 += 0x28;
        } while (var_t1 < var_t3);
    }
    return 0;
}
