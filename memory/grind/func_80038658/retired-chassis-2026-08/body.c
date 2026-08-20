void func_80038658(void) {
    register s32 var_v1 asm("v1");
    register s32 var_v0 asm("v0");
    s32 var_s0;

    var_v1 = D_800A31F4;
    if (var_v1 == 4) {
        goto block_4;
    }
    if (var_v1 == 6) {
        goto block_6;
    }
    return;
block_4:
    var_s0 = func_800378A8();
    var_v0 = 1;
    if (var_s0 == 0) {
        goto block_store;
    }
    close(D_800A3794);
    var_v0 = 1;
    if (var_s0 != var_v0) {
        var_v0 = 3;
        goto block_store_clear;
    }
    var_v0 = 2;
    goto block_store_clear;
block_6:
    var_s0 = func_800378A8();
    var_v0 = 4;
    if (var_s0 == 0) {
        goto block_store;
    }
    close(D_800A3794);
    var_v0 = 1;
    if (var_s0 != var_v0) {
        var_v0 = 6;
        goto block_store_clear;
    }
    var_v0 = 5;
    D_800A379E = (s16)var_v0;
    if (func_8003800C(&D_800F34D8) != 0) {
        goto block_clear;
    }
    var_v0 = 0xF;
block_store_clear:
    D_800A379E = (s16)var_v0;
block_clear:
    D_800A31F4 = 0;
    return;
block_store:
    D_800A379E = (s16)var_v0;
}
