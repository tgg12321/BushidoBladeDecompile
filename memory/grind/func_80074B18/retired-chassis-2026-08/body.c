void func_80074B18(s32 arg0, s32 arg1, s32 arg2) {
    s32 var_v1;
    s32 var_s3;
    s32 var_fp;
    s32 var_s4;
    s32 var_s5;
    s32 var_s7;
    register s32 var_s6 asm("s6");
    s32 var_s2;
    register s32 var_s1 asm("s1");
    register s32 var_s0 asm("s0");
    s32 v0;
    s32 a0_loc;

    var_v1 = 5;
    if (arg2 != 0) {
        var_v1 = 8;
    }
    a0_loc = (s32)D_800A36A0;
    var_s3 = *(s32 *)(arg0 + 0x14);
    var_fp = 0;
    if (((s32) *(u8 *)(a0_loc + 0x65)) + 3 != 0) {
        s32 sp28 = var_v1;
        s32 sp30 = (var_v1 > 0);
        var_s6 = ((arg1 << 4) - arg1) << 4;
        do {
            var_s2 = *(s32 *)(*(s32 *)(a0_loc + 4) + 0x3C);
            var_s4 = 0;
            if (sp30 != 0) {
                var_s5 = (((var_fp << 16) >> 16) << 4) + ((var_fp << 16) >> 16);
                var_s7 = var_s5 << 1;
                var_s1 = var_s2 + 2;
                var_s0 = var_s3 + 0xA;
                do {
                    SetTile((GameObj *)var_s3);
                    *(u8 *)(var_s0 - 6) = *(u8 *)(var_s1 + 6);
                    *(u8 *)(var_s0 - 5) = *(u8 *)(var_s1 + 7);
                    *(u8 *)(var_s0 - 4) = *(u8 *)(var_s1 + 8);
                    *(u16 *)(var_s0 + 2) = *(u16 *)(var_s1 + 2);
                    *(u16 *)(var_s0 + 4) = *(u16 *)(var_s1 + 4);
                    SetSemiTrans((GameObj *)var_s3, 0);
                    if (arg2 != 0) {
                        *(s16 *)(var_s0 - 2) = *(u16 *)var_s2 + var_s6;
                        v0 = *(u16 *)var_s1 + var_s7 + 0x2B;
                    } else {
                        *(s16 *)(var_s0 - 2) = *(u16 *)var_s2 + var_s6;
                        v0 = *(u16 *)var_s1 + var_s5 + 0x7C;
                    }
                    *(s16 *)var_s0 = v0;
                    if (arg1 != 0) {
                        a0_loc = 0x15;
                    } else {
                        a0_loc = 0xB;
                    }
                    {
                        s32 a3_loc = var_s3;
                        var_s0 += 0x10;
                        var_s3 += 0x10;
                        var_s1 += 0xC;
                        var_s2 += 0xC;
                        AddPrim(D_800A374C + (a0_loc << 2), (GameObj *)a3_loc);
                    }
                    var_s4 += 1;
                } while ((s16)var_s4 < sp28);
            }
            var_fp += 1;
            a0_loc = (s32)D_800A36A0;
        } while ((s16)var_fp < (((s32) *(u8 *)(a0_loc + 0x65)) + 3));
    }
    *(s32 *)(arg0 + 0x14) = var_s3;
}
