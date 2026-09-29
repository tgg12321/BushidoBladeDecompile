s32 func_800290B8(M2C_UNK, s32, M2C_UNK);           /* extern */
s32 func_8002DAD0(M2C_UNK);                         /* extern */
s32 func_8002DE20(M2C_UNK, void *, void *, void *); /* extern */
M2C_UNK func_80032854(s32, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
extern M2C_UNK D_1F800018;
extern M2C_UNK D_1F800024;
extern M2C_UNK D_80101EC8;
extern s32 D_80101F04;
extern s32 D_80102350;

s32 func_80029454(void) {
    M2C_UNK sp10;
    M2C_UNK *var_a3_2;
    M2C_UNK *var_s4_3;
    M2C_UNK *var_t0;
    M2C_UNK *var_t0_3;
    M2C_UNK *var_t0_4;
    M2C_UNK *var_t0_5;
    M2C_UNK *var_t0_6;
    M2C_UNK *var_t1;
    s16 temp_v1_2;
    s16 temp_v1_3;
    s16 var_v1;
    s32 temp_v1_10;
    s32 temp_v1_11;
    s32 temp_v1_12;
    s32 temp_v1_13;
    s32 temp_v1_14;
    s32 temp_v1_15;
    s32 temp_v1_16;
    s32 temp_v1_5;
    s32 temp_v1_6;
    s32 temp_v1_7;
    s32 temp_v1_8;
    s32 temp_v1_9;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a1;
    s32 var_a2_3;
    s32 var_a3;
    s32 var_a3_10;
    s32 var_a3_5;
    s32 var_a3_6;
    s32 var_a3_9;
    s32 var_fp;
    s32 var_s1;
    s32 var_s1_2;
    s32 var_s3;
    s32 var_s3_2;
    s32 var_s3_3;
    s32 var_s4;
    s32 var_s4_2;
    s32 var_s4_4;
    s32 var_s5;
    s32 var_s7;
    s32 var_t3;
    s32 var_v0_4;
    u16 temp_v1;
    void *temp_s1;
    void *temp_s1_2;
    void *temp_v0;
    void *temp_v0_2;
    void *temp_v0_3;
    void *temp_v0_4;
    void *temp_v1_4;
    void *var_a2;
    void *var_a2_2;
    void *var_a2_4;
    void *var_a2_5;
    void *var_a2_6;
    void *var_a2_7;
    void *var_a3_3;
    void *var_a3_4;
    void *var_a3_7;
    void *var_a3_8;
    void *var_s6;
    void *var_t0_2;
    void *var_v0;
    void *var_v0_2;
    void *var_v0_3;
    void *var_v0_5;

    if (D_80101F04 >= 4) {
        if (D_80102350 >= 4) {
            var_a3 = 0;
            var_t0 = &sp10;
            var_a2 = (void *)0x1F800000;
            do {
                M2C_FIELD(var_t0, s32 *, 0) = (s32) M2C_FIELD(var_a2, s32 *, 0xA8);
                M2C_FIELD(var_t0, s32 *, 4) = (s32) M2C_FIELD(var_a2, s32 *, 0xAC);
                M2C_FIELD(var_t0, s32 *, 8) = (s32) M2C_FIELD(var_a2, s32 *, 0xB0);
                var_t0 += 0xC;
                var_a3 += 1;
                var_a2 += 0xC;
            } while (var_a3 < 0x10);
            var_s3 = 0;
            var_s6 = (void *)0x1F800054;
            var_s4_3 = &D_1F800024;
            var_fp = 0;
            var_s7 = 0;
            do {
                temp_s1 = var_s7 + &D_80101EC8;
                if (((u32) (M2C_FIELD(temp_s1, u16 *, 0xE) - 6) >= 2U) && ((temp_v1 = M2C_FIELD(temp_s1, u16 *, 0x6A), (temp_v1 == 2)) || (temp_v1 == 0x1B) || (temp_v1 == 0x28) || (temp_v1 == 0x26))) {
                    if ((u32) (M2C_FIELD(temp_s1, u16 *, 0xE) - 4) < 2U) {
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0) = (s32) M2C_FIELD(var_s4_3, s32 *, 0);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 4) = (s32) M2C_FIELD(var_s4_3, s32 *, 4);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 8) = (s32) M2C_FIELD(var_s4_3, s32 *, 8);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0xC) = (s32) M2C_FIELD(var_s4_3, s32 *, 0x18);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x10) = (s32) M2C_FIELD(var_s4_3, s32 *, 0x1C);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x14) = (s32) M2C_FIELD(var_s4_3, s32 *, 0x20);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x18) = (s32) M2C_FIELD(temp_s1, s32 *, 0x210);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x1C) = (s32) M2C_FIELD(temp_s1, s32 *, 0x214);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x20) = (s32) M2C_FIELD(temp_s1, s32 *, 0x218);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x24) = (s32) M2C_FIELD(temp_s1, s32 *, 0x228);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x28) = (s32) M2C_FIELD(temp_s1, s32 *, 0x22C);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x2C) = (s32) M2C_FIELD(temp_s1, s32 *, 0x230);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x30) = (s32) M2C_FIELD(var_s4_3, s32 *, 0);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x34) = (s32) M2C_FIELD(var_s4_3, s32 *, 4);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x38) = (s32) M2C_FIELD(var_s4_3, s32 *, 8);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x3C) = (s32) M2C_FIELD(var_s4_3, s32 *, 0xC);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x40) = (s32) M2C_FIELD(var_s4_3, s32 *, 0x10);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x44) = (s32) M2C_FIELD(var_s4_3, s32 *, 0x14);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x48) = (s32) M2C_FIELD(temp_s1, s32 *, 0x210);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x4C) = (s32) M2C_FIELD(temp_s1, s32 *, 0x214);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x50) = (s32) M2C_FIELD(temp_s1, s32 *, 0x218);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x54) = (s32) M2C_FIELD(temp_s1, s32 *, 0x21C);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x58) = (s32) M2C_FIELD(temp_s1, s32 *, 0x220);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x5C) = (s32) M2C_FIELD(temp_s1, s32 *, 0x224);
                    } else {
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0) = (s32) M2C_FIELD(var_s4_3, s32 *, 0);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 4) = (s32) M2C_FIELD(var_s4_3, s32 *, 4);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 8) = (s32) M2C_FIELD(var_s4_3, s32 *, 8);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0xC) = (s32) M2C_FIELD((var_fp + 0x1F800000), s32 *, 0xC);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x10) = (s32) M2C_FIELD((var_fp + 0x1F800000), s32 *, 0x10);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x14) = (s32) M2C_FIELD((var_fp + 0x1F800000), s32 *, 0x14);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x18) = (s32) M2C_FIELD(temp_s1, s32 *, 0x210);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x1C) = (s32) M2C_FIELD(temp_s1, s32 *, 0x214);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x20) = (s32) M2C_FIELD(temp_s1, s32 *, 0x218);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x24) = (s32) M2C_FIELD(temp_s1, s32 *, 0x21C);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x28) = (s32) M2C_FIELD(temp_s1, s32 *, 0x220);
                        M2C_FIELD((void *)0x1F8000A8, s32 *, 0x2C) = (s32) M2C_FIELD(temp_s1, s32 *, 0x224);
                        if (M2C_FIELD(temp_s1, s16 *, 0x8C) != 0) {
                            M2C_FIELD((void *)0x1F8000A8, s32 *, 0x30) = (s32) M2C_FIELD(var_s6, s32 *, -0xC);
                            M2C_FIELD((void *)0x1F8000A8, s32 *, 0x34) = (s32) M2C_FIELD(var_s6, s32 *, -8);
                            M2C_FIELD((void *)0x1F8000A8, s32 *, 0x38) = (s32) M2C_FIELD(var_s6, s32 *, -4);
                            M2C_FIELD((void *)0x1F8000A8, s32 *, 0x3C) = (s32) M2C_FIELD(var_s6, s32 *, 0);
                            M2C_FIELD((void *)0x1F8000A8, s32 *, 0x40) = (s32) M2C_FIELD(var_s6, s32 *, 4);
                            M2C_FIELD((void *)0x1F8000A8, s32 *, 0x44) = (s32) M2C_FIELD(var_s6, s32 *, 8);
                            M2C_FIELD((void *)0x1F8000A8, s32 *, 0x48) = (s32) M2C_FIELD(temp_s1, s32 *, 0x234);
                            M2C_FIELD((void *)0x1F8000A8, s32 *, 0x4C) = (s32) M2C_FIELD(temp_s1, s32 *, 0x238);
                            M2C_FIELD((void *)0x1F8000A8, s32 *, 0x50) = (s32) M2C_FIELD(temp_s1, s32 *, 0x23C);
                            M2C_FIELD((void *)0x1F8000A8, s32 *, 0x54) = (s32) M2C_FIELD(temp_s1, s32 *, 0x240);
                            M2C_FIELD((void *)0x1F8000A8, s32 *, 0x58) = (s32) M2C_FIELD(temp_s1, s32 *, 0x244);
                            M2C_FIELD((void *)0x1F8000A8, s32 *, 0x5C) = (s32) M2C_FIELD(temp_s1, s32 *, 0x248);
                        }
                    }
                    temp_v1_2 = M2C_FIELD(temp_s1, s16 *, 0x40);
                    if ((temp_v1_2 >= (s32) M2C_FIELD(temp_s1, u8 *, 0xA1)) && ((s32) M2C_FIELD(temp_s1, u8 *, 0xA3) >= temp_v1_2) && (func_800290B8(0, (u32) (M2C_FIELD(temp_s1, u16 *, 0xE) - 4) < 2U, 0x1F8000A8) != 0)) {
                        func_80032854(var_s3, 1, 0x1F8003B8, 0);
                        func_80032854(var_s3, 0x26, 0x1F8003B8, 0);
                        func_80032854(var_s3, 0x2D, 0x1F8003B8, 0);
                        var_v1 = 0xB;
                        if (M2C_FIELD(temp_s1, s16 *, 0x8C) != 0) {
                            var_v1 = 0x19;
                        }
                        M2C_FIELD(temp_s1, s16 *, 0x286) = var_v1;
                        goto block_25;
                    }
                    if (M2C_FIELD(temp_s1, s16 *, 0x8C) != 0) {
                        temp_v1_3 = M2C_FIELD(temp_s1, s16 *, 0x40);
                        if ((temp_v1_3 >= (s32) M2C_FIELD(temp_s1, u8 *, 0xA2)) && ((s32) M2C_FIELD(temp_s1, u8 *, 0xA4) >= temp_v1_3) && (func_800290B8(1, 0, 0x1F8000A8) != 0)) {
                            func_80032854(var_s3, 1, 0x1F8003B8, 0);
                            func_80032854(var_s3, 0x26, 0x1F8003B8, 0);
                            func_80032854(var_s3, 0x2D, 0x1F8003B8, 0);
                            M2C_FIELD(temp_s1, s16 *, 0x286) = 0xB;
block_25:
                            M2C_FIELD(temp_s1, s8 *, 0xAD) = 0;
                        }
                    }
                }
                var_s6 += 0x18;
                var_fp += 0x24;
                var_s3 += 1;
                var_s7 += 0x44C;
            } while (var_s3 < 2);
            var_s3_2 = 0;
            var_a3_2 = &sp10;
            var_t1 = &D_1F800018;
            var_a2_2 = (void *)0x1F8000A8;
            var_t0_2 = (void *)0x1F800000;
            var_t3 = 0;
            do {
                temp_s1_2 = var_t3 + &D_80101EC8;
                M2C_FIELD(var_a2_2, s32 *, 0) = (s32) M2C_FIELD(var_t0_2, s32 *, 0);
                M2C_FIELD(var_a2_2, s32 *, 4) = (s32) M2C_FIELD(var_t0_2, s32 *, 4);
                M2C_FIELD(var_a2_2, s32 *, 8) = (s32) M2C_FIELD(var_t0_2, s32 *, 8);
                M2C_FIELD(var_a2_2, s32 *, 0xC) = (s32) M2C_FIELD(var_t0_2, s32 *, 0xC);
                M2C_FIELD(var_a2_2, s32 *, 0x10) = (s32) M2C_FIELD(var_t0_2, s32 *, 0x10);
                M2C_FIELD(var_a2_2, s32 *, 0x14) = (s32) M2C_FIELD(var_t0_2, s32 *, 0x14);
                M2C_FIELD(var_a2_2, s32 *, 0x18) = (s32) M2C_FIELD(temp_s1_2, s32 *, 0x210);
                M2C_FIELD(var_a2_2, s32 *, 0x1C) = (s32) M2C_FIELD(temp_s1_2, s32 *, 0x214);
                M2C_FIELD(var_a2_2, s32 *, 0x20) = (s32) M2C_FIELD(temp_s1_2, s32 *, 0x218);
                M2C_FIELD(var_a2_2, s32 *, 0x24) = (s32) M2C_FIELD(temp_s1_2, s32 *, 0x21C);
                M2C_FIELD(var_a2_2, s32 *, 0x28) = (s32) M2C_FIELD(temp_s1_2, s32 *, 0x220);
                M2C_FIELD(var_a2_2, s32 *, 0x2C) = (s32) M2C_FIELD(temp_s1_2, s32 *, 0x224);
                M2C_FIELD(var_a3_2, s32 *, 0xC0) = 2;
                if ((M2C_FIELD(temp_s1_2, s16 *, 0x96) != 0) || (M2C_FIELD(temp_s1_2, s16 *, 0x92) == 0) || (M2C_FIELD(temp_s1_2, s16 *, 0xC) == 0x1F)) {
                    M2C_FIELD(var_a2_2, s32 *, 4) = 0x186A0;
                    M2C_FIELD(var_a2_2, s32 *, 0x10) = 0x186A0;
                    M2C_FIELD(var_a2_2, s32 *, 0x1C) = 0x186A0;
                    M2C_FIELD(var_a2_2, s32 *, 0x28) = 0x186A0;
                }
                if (M2C_FIELD(temp_s1_2, s16 *, 0x8C) != 0) {
                    M2C_FIELD(var_a2_2, s32 *, 0x30) = (s32) M2C_FIELD(var_t1, s32 *, 0x48);
                    M2C_FIELD(var_a2_2, s32 *, 0x34) = (s32) M2C_FIELD(var_t1, s32 *, 0x4C);
                    M2C_FIELD(var_a2_2, s32 *, 0x38) = (s32) M2C_FIELD(var_t1, s32 *, 0x50);
                    M2C_FIELD(var_a2_2, s32 *, 0x3C) = (s32) M2C_FIELD(var_t1, s32 *, 0x54);
                    M2C_FIELD(var_a2_2, s32 *, 0x40) = (s32) M2C_FIELD(var_t1, s32 *, 0x58);
                    M2C_FIELD(var_a2_2, s32 *, 0x44) = (s32) M2C_FIELD(var_t1, s32 *, 0x5C);
                    M2C_FIELD(var_a2_2, s32 *, 0x48) = (s32) M2C_FIELD(temp_s1_2, s32 *, 0x234);
                    M2C_FIELD(var_a2_2, s32 *, 0x4C) = (s32) M2C_FIELD(temp_s1_2, s32 *, 0x238);
                    M2C_FIELD(var_a2_2, s32 *, 0x50) = (s32) M2C_FIELD(temp_s1_2, s32 *, 0x23C);
                    M2C_FIELD(var_a2_2, s32 *, 0x54) = (s32) M2C_FIELD(temp_s1_2, s32 *, 0x240);
                    M2C_FIELD(var_a2_2, s32 *, 0x58) = (s32) M2C_FIELD(temp_s1_2, s32 *, 0x244);
                    M2C_FIELD(var_a2_2, s32 *, 0x5C) = (s32) M2C_FIELD(temp_s1_2, s32 *, 0x248);
                    M2C_FIELD(var_a3_2, s32 *, 0xC0) = (s32) (M2C_FIELD(var_a3_2, s32 *, 0xC0) + 2);
                }
                var_a3_2 += 4;
                var_a2_2 += 0x60;
                var_s3_2 += 1;
                var_t3 += 0x44C;
            } while (var_s3_2 < 2);
            var_s3_3 = 0;
            var_a2_3 = 0;
            do {
                var_s4_4 = 0;
                var_a1 = var_a2_3;
loop_37:
                temp_v1_4 = var_a1 + 0x1F8000A8;
                var_a1 += 0x18;
                var_s4_4 += 1;
                M2C_FIELD(temp_v1_4, s32 *, 0) = (s32) ((s32) M2C_FIELD(temp_v1_4, s32 *, 0) >> 1);
                M2C_FIELD(temp_v1_4, s32 *, 4) = (s32) ((s32) M2C_FIELD(temp_v1_4, s32 *, 4) >> 1);
                M2C_FIELD(temp_v1_4, s32 *, 8) = (s32) ((s32) M2C_FIELD(temp_v1_4, s32 *, 8) >> 1);
                M2C_FIELD(temp_v1_4, s32 *, 0xC) = (s32) ((s32) M2C_FIELD(temp_v1_4, s32 *, 0xC) >> 1);
                M2C_FIELD(temp_v1_4, s32 *, 0x10) = (s32) ((s32) M2C_FIELD(temp_v1_4, s32 *, 0x10) >> 1);
                M2C_FIELD(temp_v1_4, s32 *, 0x14) = (s32) ((s32) M2C_FIELD(temp_v1_4, s32 *, 0x14) >> 1);
                if (var_s4_4 < 4) {
                    goto loop_37;
                }
                var_s3_3 += 1;
                var_a2_3 += 0x60;
            } while (var_s3_3 < 2);
            var_s5 = 0;
            var_s4_2 = 0;
            if (spD0 > 0) {
loop_41:
                switch (var_s4_2) {                 /* switch 1; irregular */
                case 0:                             /* switch 1 */
                    M2C_FIELD((void *)0x1F8002B8, void **, 0x60) = (void *) ((void *)0x1F8000A8 + 0x18);
                    M2C_FIELD((void *)0x1F8002B8, void **, 0x64) = (void *) ((void *)0x1F8000A8 + 0x24);
                    var_v0 = (void *)0x1F8000A8 + 0xC;
block_52:
                    M2C_FIELD((void *)0x1F8002B8, void **, 0x68) = var_v0;
                    break;
                case 1:                             /* switch 1 */
                    M2C_FIELD((void *)0x1F8002B8, void **, 0x64) = (void *) ((void *)0x1F8000A8 + 0xC);
                    var_v0 = (void *)0x1F8000A8 + 0x18;
                    M2C_FIELD((void *)0x1F8002B8, void **, 0x60) = (void *)0x1F8000A8;
                    goto block_52;
                case 2:                             /* switch 1 */
                    M2C_FIELD((void *)0x1F8002B8, void **, 0x60) = (void *) ((void *)0x1F8000A8 + 0x48);
                    M2C_FIELD((void *)0x1F8002B8, void **, 0x64) = (void *) ((void *)0x1F8000A8 + 0x54);
                    var_v0 = (void *)0x1F8000A8 + 0x3C;
                    goto block_52;
                case 3:                             /* switch 1 */
                    M2C_FIELD((void *)0x1F8002B8, void **, 0x60) = (void *) ((void *)0x1F8000A8 + 0x30);
                    M2C_FIELD((void *)0x1F8002B8, void **, 0x64) = (void *) ((void *)0x1F8000A8 + 0x3C);
                    var_v0 = (void *)0x1F8000A8 + 0x48;
                    goto block_52;
                }
                var_a3_3 = (void *)0x1F8002B8 + 4;
                if (func_8002DAD0(0x1F8002B8) == 0) {
                    var_s5 |= 1 << var_s4_2;
                    goto block_109;
                }
                temp_v0 = M2C_FIELD((void *)0x1F8002B8, void **, 0x60);
                M2C_FIELD((void *)0x1F8002B8, s32 *, 0x84) = (s32) M2C_FIELD(temp_v0, s32 *, 0);
                M2C_FIELD((void *)0x1F8002B8, s32 *, 0x88) = (s32) M2C_FIELD(temp_v0, s32 *, 4);
                M2C_FIELD((void *)0x1F8002B8, s32 *, 0x8C) = (s32) M2C_FIELD(temp_v0, s32 *, 8);
                M2C_FIELD((void *)0x1F8002B8, s32 *, 0x78) = (s32) M2C_FIELD((void *)0x1F8002B8, s32 *, 0x84);
                M2C_FIELD((void *)0x1F8002B8, s32 *, 0x7C) = (s32) M2C_FIELD((void *)0x1F8002B8, s32 *, 0x88);
                M2C_FIELD((void *)0x1F8002B8, s32 *, 0x80) = (s32) M2C_FIELD((void *)0x1F8002B8, s32 *, 0x8C);
                do {
                    temp_v1_5 = M2C_FIELD(M2C_FIELD(var_a3_3, void **, 0x60), s32 *, 0);
                    if (temp_v1_5 < M2C_FIELD((void *)0x1F8002B8, s32 *, 0x78)) {
                        M2C_FIELD((void *)0x1F8002B8, s32 *, 0x78) = temp_v1_5;
                    } else if (M2C_FIELD((void *)0x1F8002B8, s32 *, 0x84) < temp_v1_5) {
                        M2C_FIELD((void *)0x1F8002B8, s32 *, 0x84) = temp_v1_5;
                    }
                    temp_v1_6 = M2C_FIELD(M2C_FIELD(var_a3_3, void **, 0x60), s32 *, 4);
                    if (temp_v1_6 < M2C_FIELD((void *)0x1F8002B8, s32 *, 0x7C)) {
                        M2C_FIELD((void *)0x1F8002B8, s32 *, 0x7C) = temp_v1_6;
                    } else if (M2C_FIELD((void *)0x1F8002B8, s32 *, 0x88) < temp_v1_6) {
                        M2C_FIELD((void *)0x1F8002B8, s32 *, 0x88) = temp_v1_6;
                    }
                    temp_v1_7 = M2C_FIELD(M2C_FIELD(var_a3_3, void **, 0x60), s32 *, 8);
                    if (temp_v1_7 < M2C_FIELD((void *)0x1F8002B8, s32 *, 0x80)) {
                        M2C_FIELD((void *)0x1F8002B8, s32 *, 0x80) = temp_v1_7;
                    } else if (M2C_FIELD((void *)0x1F8002B8, s32 *, 0x8C) < temp_v1_7) {
                        M2C_FIELD((void *)0x1F8002B8, s32 *, 0x8C) = temp_v1_7;
                    }
                    var_a3_3 += 4;
                } while ((s32) var_a3_3 < (s32) ((void *)0x1F8002B8 + 0xC));
                var_s1_2 = 0;
                if (spD4 > 0) {
loop_71:
                    if (var_s1_2 != 1) {
                        if (var_s1_2 < 2) {
                            if (var_s1_2 != 0) {
                                var_a3_4 = (void *)0x1F8002B8 + 4;
                            } else {
                                M2C_FIELD((void *)0x1F8002B8, void **, 0x6C) = (void *) ((void *)0x1F8000A8 + 0x78);
                                M2C_FIELD((void *)0x1F8002B8, void **, 0x70) = (void *) ((void *)0x1F8000A8 + 0x84);
                                var_v0_2 = (void *)0x1F8000A8 + 0x6C;
                                goto block_82;
                            }
                        } else if (var_s1_2 != 2) {
                            if (var_s1_2 != 3) {
                                var_a3_4 = (void *)0x1F8002B8 + 4;
                            } else {
                                M2C_FIELD((void *)0x1F8002B8, void **, 0x6C) = (void *) ((void *)0x1F8000A8 + 0x90);
                                M2C_FIELD((void *)0x1F8002B8, void **, 0x70) = (void *) ((void *)0x1F8000A8 + 0x9C);
                                var_v0_2 = (void *)0x1F8000A8 + 0xA8;
                                goto block_82;
                            }
                        } else {
                            M2C_FIELD((void *)0x1F8002B8, void **, 0x6C) = (void *) ((void *)0x1F8000A8 + 0xA8);
                            M2C_FIELD((void *)0x1F8002B8, void **, 0x70) = (void *) ((void *)0x1F8000A8 + 0xB4);
                            var_v0_2 = (void *)0x1F8000A8 + 0x9C;
                            goto block_82;
                        }
                    } else {
                        M2C_FIELD((void *)0x1F8002B8, void **, 0x6C) = (void *) ((void *)0x1F8000A8 + 0x60);
                        M2C_FIELD((void *)0x1F8002B8, void **, 0x70) = (void *) ((void *)0x1F8000A8 + 0x6C);
                        var_v0_2 = (void *)0x1F8000A8 + 0x78;
block_82:
                        M2C_FIELD((void *)0x1F8002B8, void **, 0x74) = var_v0_2;
                        var_a3_4 = (void *)0x1F8002B8 + 4;
                    }
                    temp_v0_2 = M2C_FIELD((void *)0x1F8002B8, void **, 0x6C);
                    M2C_FIELD((void *)0x1F8002B8, s32 *, 0x9C) = (s32) M2C_FIELD(temp_v0_2, s32 *, 0);
                    M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA0) = (s32) M2C_FIELD(temp_v0_2, s32 *, 4);
                    M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA4) = (s32) M2C_FIELD(temp_v0_2, s32 *, 8);
                    M2C_FIELD((void *)0x1F8002B8, s32 *, 0x90) = (s32) M2C_FIELD((void *)0x1F8002B8, s32 *, 0x9C);
                    M2C_FIELD((void *)0x1F8002B8, s32 *, 0x94) = (s32) M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA0);
                    M2C_FIELD((void *)0x1F8002B8, s32 *, 0x98) = (s32) M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA4);
                    do {
                        temp_v1_8 = M2C_FIELD(M2C_FIELD(var_a3_4, void **, 0x6C), s32 *, 0);
                        if (temp_v1_8 < M2C_FIELD((void *)0x1F8002B8, s32 *, 0x90)) {
                            M2C_FIELD((void *)0x1F8002B8, s32 *, 0x90) = temp_v1_8;
                        } else if (M2C_FIELD((void *)0x1F8002B8, s32 *, 0x9C) < temp_v1_8) {
                            M2C_FIELD((void *)0x1F8002B8, s32 *, 0x9C) = temp_v1_8;
                        }
                        temp_v1_9 = M2C_FIELD(M2C_FIELD(var_a3_4, void **, 0x6C), s32 *, 4);
                        if (temp_v1_9 < M2C_FIELD((void *)0x1F8002B8, s32 *, 0x94)) {
                            M2C_FIELD((void *)0x1F8002B8, s32 *, 0x94) = temp_v1_9;
                        } else if (M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA0) < temp_v1_9) {
                            M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA0) = temp_v1_9;
                        }
                        temp_v1_10 = M2C_FIELD(M2C_FIELD(var_a3_4, void **, 0x6C), s32 *, 8);
                        if (temp_v1_10 < M2C_FIELD((void *)0x1F8002B8, s32 *, 0x98)) {
                            M2C_FIELD((void *)0x1F8002B8, s32 *, 0x98) = temp_v1_10;
                        } else if (M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA4) < temp_v1_10) {
                            M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA4) = temp_v1_10;
                        }
                        var_a3_4 += 4;
                    } while ((s32) var_a3_4 < (s32) ((void *)0x1F8002B8 + 0xC));
                    var_a0 = 0;
                    if ((M2C_FIELD((void *)0x1F8002B8, s32 *, 0x9C) >= M2C_FIELD((void *)0x1F8002B8, s32 *, 0x78)) && (M2C_FIELD((void *)0x1F8002B8, s32 *, 0x84) >= M2C_FIELD((void *)0x1F8002B8, s32 *, 0x90)) && (M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA0) >= M2C_FIELD((void *)0x1F8002B8, s32 *, 0x7C)) && (M2C_FIELD((void *)0x1F8002B8, s32 *, 0x88) >= M2C_FIELD((void *)0x1F8002B8, s32 *, 0x94)) && (M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA4) >= M2C_FIELD((void *)0x1F8002B8, s32 *, 0x80))) {
                        var_a0 = M2C_FIELD((void *)0x1F8002B8, s32 *, 0x8C) >= M2C_FIELD((void *)0x1F8002B8, s32 *, 0x98);
                    }
                    if (var_a0 != 0) {
                        var_a3_5 = 0;
                        if (func_8002DE20(0x1F8002B8, M2C_FIELD((void *)0x1F8002B8, void **, 0x6C), M2C_FIELD((void *)0x1F8002B8, void **, 0x70), M2C_FIELD((void *)0x1F8002B8, void **, 0x74)) != 0) {
                            var_t0_3 = &sp10;
                            var_a2_4 = (void *)0x1F800000;
                            do {
                                M2C_FIELD(var_a2_4, s32 *, 0xA8) = (s32) M2C_FIELD(var_t0_3, s32 *, 0);
                                M2C_FIELD(var_a2_4, s32 *, 0xAC) = (s32) M2C_FIELD(var_t0_3, s32 *, 4);
                                M2C_FIELD(var_a2_4, s32 *, 0xB0) = (s32) M2C_FIELD(var_t0_3, s32 *, 8);
                                var_t0_3 += 0xC;
                                var_a3_5 += 1;
                                var_a2_4 += 0xC;
                            } while (var_a3_5 < 0x10);
                            return ((var_s1_2 >> 1) * 2) | (var_s4_2 >> 1);
                        }
                    }
                    var_s1_2 += 1;
                    if (var_s1_2 >= spD4) {
                        goto block_109;
                    }
                    goto loop_71;
                }
block_109:
                var_s4_2 += 1;
                if (var_s4_2 >= spD0) {
                    goto block_110;
                }
                goto loop_41;
            }
block_110:
            var_a3_6 = 0;
            if (var_s5 == 0) {
                var_t0_4 = &sp10;
                var_a2_5 = (void *)0x1F800000;
                do {
                    M2C_FIELD(var_a2_5, s32 *, 0xA8) = (s32) M2C_FIELD(var_t0_4, s32 *, 0);
                    M2C_FIELD(var_a2_5, s32 *, 0xAC) = (s32) M2C_FIELD(var_t0_4, s32 *, 4);
                    M2C_FIELD(var_a2_5, s32 *, 0xB0) = (s32) M2C_FIELD(var_t0_4, s32 *, 8);
                    var_t0_4 += 0xC;
                    var_a3_6 += 1;
                    var_a2_5 += 0xC;
                } while (var_a3_6 < 0x10);
                return -1;
            }
            var_s4 = 0;
            if (spD4 > 0) {
loop_116:
                switch (var_s4) {                   /* switch 2; irregular */
                case 0:                             /* switch 2 */
                    M2C_FIELD((void *)0x1F8002B8, void **, 0x60) = (void *) ((void *)0x1F8000A8 + 0x78);
                    M2C_FIELD((void *)0x1F8002B8, void **, 0x64) = (void *) ((void *)0x1F8000A8 + 0x84);
                    var_v0_3 = (void *)0x1F8000A8 + 0x6C;
block_127:
                    M2C_FIELD((void *)0x1F8002B8, void **, 0x68) = var_v0_3;
                    break;
                case 1:                             /* switch 2 */
                    M2C_FIELD((void *)0x1F8002B8, void **, 0x60) = (void *) ((void *)0x1F8000A8 + 0x60);
                    M2C_FIELD((void *)0x1F8002B8, void **, 0x64) = (void *) ((void *)0x1F8000A8 + 0x6C);
                    var_v0_3 = (void *)0x1F8000A8 + 0x78;
                    goto block_127;
                case 2:                             /* switch 2 */
                    M2C_FIELD((void *)0x1F8002B8, void **, 0x60) = (void *) ((void *)0x1F8000A8 + 0xA8);
                    M2C_FIELD((void *)0x1F8002B8, void **, 0x64) = (void *) ((void *)0x1F8000A8 + 0xB4);
                    var_v0_3 = (void *)0x1F8000A8 + 0x9C;
                    goto block_127;
                case 3:                             /* switch 2 */
                    M2C_FIELD((void *)0x1F8002B8, void **, 0x60) = (void *) ((void *)0x1F8000A8 + 0x90);
                    M2C_FIELD((void *)0x1F8002B8, void **, 0x64) = (void *) ((void *)0x1F8000A8 + 0x9C);
                    var_v0_3 = (void *)0x1F8000A8 + 0xA8;
                    goto block_127;
                }
                var_a3_7 = (void *)0x1F8002B8 + 4;
                if (func_8002DAD0(0x1F8002B8) != 0) {
                    temp_v0_3 = M2C_FIELD((void *)0x1F8002B8, void **, 0x60);
                    M2C_FIELD((void *)0x1F8002B8, s32 *, 0x84) = (s32) M2C_FIELD(temp_v0_3, s32 *, 0);
                    M2C_FIELD((void *)0x1F8002B8, s32 *, 0x88) = (s32) M2C_FIELD(temp_v0_3, s32 *, 4);
                    M2C_FIELD((void *)0x1F8002B8, s32 *, 0x8C) = (s32) M2C_FIELD(temp_v0_3, s32 *, 8);
                    M2C_FIELD((void *)0x1F8002B8, s32 *, 0x78) = (s32) M2C_FIELD((void *)0x1F8002B8, s32 *, 0x84);
                    M2C_FIELD((void *)0x1F8002B8, s32 *, 0x7C) = (s32) M2C_FIELD((void *)0x1F8002B8, s32 *, 0x88);
                    M2C_FIELD((void *)0x1F8002B8, s32 *, 0x80) = (s32) M2C_FIELD((void *)0x1F8002B8, s32 *, 0x8C);
                    do {
                        temp_v1_11 = M2C_FIELD(M2C_FIELD(var_a3_7, void **, 0x60), s32 *, 0);
                        if (temp_v1_11 < M2C_FIELD((void *)0x1F8002B8, s32 *, 0x78)) {
                            M2C_FIELD((void *)0x1F8002B8, s32 *, 0x78) = temp_v1_11;
                        } else if (M2C_FIELD((void *)0x1F8002B8, s32 *, 0x84) < temp_v1_11) {
                            M2C_FIELD((void *)0x1F8002B8, s32 *, 0x84) = temp_v1_11;
                        }
                        temp_v1_12 = M2C_FIELD(M2C_FIELD(var_a3_7, void **, 0x60), s32 *, 4);
                        if (temp_v1_12 < M2C_FIELD((void *)0x1F8002B8, s32 *, 0x7C)) {
                            M2C_FIELD((void *)0x1F8002B8, s32 *, 0x7C) = temp_v1_12;
                        } else if (M2C_FIELD((void *)0x1F8002B8, s32 *, 0x88) < temp_v1_12) {
                            M2C_FIELD((void *)0x1F8002B8, s32 *, 0x88) = temp_v1_12;
                        }
                        temp_v1_13 = M2C_FIELD(M2C_FIELD(var_a3_7, void **, 0x60), s32 *, 8);
                        if (temp_v1_13 < M2C_FIELD((void *)0x1F8002B8, s32 *, 0x80)) {
                            M2C_FIELD((void *)0x1F8002B8, s32 *, 0x80) = temp_v1_13;
                        } else if (M2C_FIELD((void *)0x1F8002B8, s32 *, 0x8C) < temp_v1_13) {
                            M2C_FIELD((void *)0x1F8002B8, s32 *, 0x8C) = temp_v1_13;
                        }
                        var_a3_7 += 4;
                    } while ((s32) var_a3_7 < (s32) ((void *)0x1F8002B8 + 0xC));
                    var_s1 = 0;
                    if (spD0 > 0) {
                        var_v0_4 = 1 << 0;
loop_145:
                        if (var_s5 & var_v0_4) {
                            if (var_s1 != 1) {
                                if (var_s1 < 2) {
                                    if (var_s1 != 0) {
                                        var_a3_8 = (void *)0x1F8002B8 + 4;
                                    } else {
                                        M2C_FIELD((void *)0x1F8002B8, void **, 0x6C) = (void *) ((void *)0x1F8000A8 + 0x18);
                                        M2C_FIELD((void *)0x1F8002B8, void **, 0x70) = (void *) ((void *)0x1F8000A8 + 0x24);
                                        var_v0_5 = (void *)0x1F8000A8 + 0xC;
                                        goto block_157;
                                    }
                                } else if (var_s1 != 2) {
                                    if (var_s1 != 3) {
                                        var_a3_8 = (void *)0x1F8002B8 + 4;
                                    } else {
                                        M2C_FIELD((void *)0x1F8002B8, void **, 0x6C) = (void *) ((void *)0x1F8000A8 + 0x30);
                                        M2C_FIELD((void *)0x1F8002B8, void **, 0x70) = (void *) ((void *)0x1F8000A8 + 0x3C);
                                        var_v0_5 = (void *)0x1F8000A8 + 0x48;
                                        goto block_157;
                                    }
                                } else {
                                    M2C_FIELD((void *)0x1F8002B8, void **, 0x6C) = (void *) ((void *)0x1F8000A8 + 0x48);
                                    M2C_FIELD((void *)0x1F8002B8, void **, 0x70) = (void *) ((void *)0x1F8000A8 + 0x54);
                                    var_v0_5 = (void *)0x1F8000A8 + 0x3C;
                                    goto block_157;
                                }
                            } else {
                                M2C_FIELD((void *)0x1F8002B8, void **, 0x70) = (void *) ((void *)0x1F8000A8 + 0xC);
                                var_v0_5 = (void *)0x1F8000A8 + 0x18;
                                M2C_FIELD((void *)0x1F8002B8, void **, 0x6C) = (void *)0x1F8000A8;
block_157:
                                M2C_FIELD((void *)0x1F8002B8, void **, 0x74) = var_v0_5;
                                var_a3_8 = (void *)0x1F8002B8 + 4;
                            }
                            temp_v0_4 = M2C_FIELD((void *)0x1F8002B8, void **, 0x6C);
                            M2C_FIELD((void *)0x1F8002B8, s32 *, 0x9C) = (s32) M2C_FIELD(temp_v0_4, s32 *, 0);
                            M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA0) = (s32) M2C_FIELD(temp_v0_4, s32 *, 4);
                            M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA4) = (s32) M2C_FIELD(temp_v0_4, s32 *, 8);
                            M2C_FIELD((void *)0x1F8002B8, s32 *, 0x90) = (s32) M2C_FIELD((void *)0x1F8002B8, s32 *, 0x9C);
                            M2C_FIELD((void *)0x1F8002B8, s32 *, 0x94) = (s32) M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA0);
                            M2C_FIELD((void *)0x1F8002B8, s32 *, 0x98) = (s32) M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA4);
                            do {
                                temp_v1_14 = M2C_FIELD(M2C_FIELD(var_a3_8, void **, 0x6C), s32 *, 0);
                                if (temp_v1_14 < M2C_FIELD((void *)0x1F8002B8, s32 *, 0x90)) {
                                    M2C_FIELD((void *)0x1F8002B8, s32 *, 0x90) = temp_v1_14;
                                } else if (M2C_FIELD((void *)0x1F8002B8, s32 *, 0x9C) < temp_v1_14) {
                                    M2C_FIELD((void *)0x1F8002B8, s32 *, 0x9C) = temp_v1_14;
                                }
                                temp_v1_15 = M2C_FIELD(M2C_FIELD(var_a3_8, void **, 0x6C), s32 *, 4);
                                if (temp_v1_15 < M2C_FIELD((void *)0x1F8002B8, s32 *, 0x94)) {
                                    M2C_FIELD((void *)0x1F8002B8, s32 *, 0x94) = temp_v1_15;
                                } else if (M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA0) < temp_v1_15) {
                                    M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA0) = temp_v1_15;
                                }
                                temp_v1_16 = M2C_FIELD(M2C_FIELD(var_a3_8, void **, 0x6C), s32 *, 8);
                                if (temp_v1_16 < M2C_FIELD((void *)0x1F8002B8, s32 *, 0x98)) {
                                    M2C_FIELD((void *)0x1F8002B8, s32 *, 0x98) = temp_v1_16;
                                } else if (M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA4) < temp_v1_16) {
                                    M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA4) = temp_v1_16;
                                }
                                var_a3_8 += 4;
                            } while ((s32) var_a3_8 < (s32) ((void *)0x1F8002B8 + 0xC));
                            var_a0_2 = 0;
                            if ((M2C_FIELD((void *)0x1F8002B8, s32 *, 0x9C) >= M2C_FIELD((void *)0x1F8002B8, s32 *, 0x78)) && (M2C_FIELD((void *)0x1F8002B8, s32 *, 0x84) >= M2C_FIELD((void *)0x1F8002B8, s32 *, 0x90)) && (M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA0) >= M2C_FIELD((void *)0x1F8002B8, s32 *, 0x7C)) && (M2C_FIELD((void *)0x1F8002B8, s32 *, 0x88) >= M2C_FIELD((void *)0x1F8002B8, s32 *, 0x94)) && (M2C_FIELD((void *)0x1F8002B8, s32 *, 0xA4) >= M2C_FIELD((void *)0x1F8002B8, s32 *, 0x80))) {
                                var_a0_2 = M2C_FIELD((void *)0x1F8002B8, s32 *, 0x8C) >= M2C_FIELD((void *)0x1F8002B8, s32 *, 0x98);
                            }
                            if (var_a0_2 != 0) {
                                var_a3_9 = 0;
                                if (func_8002DE20(0x1F8002B8, M2C_FIELD((void *)0x1F8002B8, void **, 0x6C), M2C_FIELD((void *)0x1F8002B8, void **, 0x70), M2C_FIELD((void *)0x1F8002B8, void **, 0x74)) != 0) {
                                    var_t0_5 = &sp10;
                                    var_a2_6 = (void *)0x1F800000;
                                    do {
                                        M2C_FIELD(var_a2_6, s32 *, 0xA8) = (s32) M2C_FIELD(var_t0_5, s32 *, 0);
                                        M2C_FIELD(var_a2_6, s32 *, 0xAC) = (s32) M2C_FIELD(var_t0_5, s32 *, 4);
                                        M2C_FIELD(var_a2_6, s32 *, 0xB0) = (s32) M2C_FIELD(var_t0_5, s32 *, 8);
                                        var_t0_5 += 0xC;
                                        var_a3_9 += 1;
                                        var_a2_6 += 0xC;
                                    } while (var_a3_9 < 0x10);
                                    return ((var_s4 >> 1) * 2) | (var_s1 >> 1);
                                }
                            }
                            goto block_183;
                        }
block_183:
                        var_s1 += 1;
                        var_v0_4 = 1 << var_s1;
                        if (var_s1 >= spD0) {
                            goto block_184;
                        }
                        goto loop_145;
                    }
                    goto block_184;
                }
block_184:
                var_s4 += 1;
                if (var_s4 >= spD4) {
                    goto block_185;
                }
                goto loop_116;
            }
block_185:
            var_a3_10 = 0;
            var_t0_6 = &sp10;
            var_a2_7 = (void *)0x1F800000;
            do {
                M2C_FIELD(var_a2_7, s32 *, 0xA8) = (s32) M2C_FIELD(var_t0_6, s32 *, 0);
                M2C_FIELD(var_a2_7, s32 *, 0xAC) = (s32) M2C_FIELD(var_t0_6, s32 *, 4);
                M2C_FIELD(var_a2_7, s32 *, 0xB0) = (s32) M2C_FIELD(var_t0_6, s32 *, 8);
                var_t0_6 += 0xC;
                var_a3_10 += 1;
                var_a2_7 += 0xC;
            } while (var_a3_10 < 0x10);
            goto block_187;
        }
        /* Duplicate return node #188. Try simplifying control flow for better match */
        return -1;
    }
block_187:
    return -1;
}
