M2C_UNK cpu_check_same_dir_timer(void *);           /* extern */
M2C_UNK cpu_set_move_command_and_dir(void *, u8, s32); /* extern */
M2C_UNK func_800198D0(s32, s32, s16 *, M2C_UNK);    /* extern */
M2C_UNK func_8001B690(s32, s32);                    /* extern */
M2C_UNK func_8001F2E4(void *, s16 *, s16 *);        /* extern */
M2C_UNK func_8001F860(void *, s32);                 /* extern */
M2C_UNK func_8001F938(void *, s16, s32);            /* extern */
M2C_UNK func_800200DC(void *, s32 *, s32, M2C_UNK, s32 *); /* extern */
M2C_UNK func_800204C0(void *);                      /* extern */
M2C_UNK func_800207C8(void *, s32, void *, s32);    /* extern */
void *func_80021424(void *, u16, u16 *);            /* extern */
M2C_UNK func_80021A98(s32, void *, s16);            /* extern */
M2C_UNK func_8002304C(void *, void *, s32 *, void *); /* extern */
s32 func_800233AC(void *, s32 *);                   /* extern */
M2C_UNK func_80023648(void *, s32, u16, s16);       /* extern */
M2C_UNK func_800238C4(void *);                      /* extern */
M2C_UNK func_80023CB4(void *, M2C_UNK);             /* extern */
M2C_UNK func_80023D08(void *, s32);                 /* extern */
M2C_UNK func_80023D28(void *, s32);                 /* extern */
s32 func_80023DB8(void *);                          /* extern */
M2C_UNK func_80023E40(void *, s32);                 /* extern */
s32 func_8002798C(void *);                          /* extern */
s16 func_8002FDB0(void *);                          /* extern */
s32 func_800307D0(void *);                          /* extern */
s32 func_80030BA8(void *);                          /* extern */
M2C_UNK func_80032064(void *, M2C_UNK);             /* extern */
M2C_UNK func_80032854(s32, M2C_UNK, void *, M2C_UNK); /* extern */
M2C_UNK func_80039680(void *);                      /* extern */
M2C_UNK func_80040304(s32, u8);                     /* extern */
M2C_UNK func_80040D48(s32, M2C_UNK, void *, void *, s32, s32); /* extern */
M2C_UNK func_80041188(s32, s16 *, s16 *, s16, s32); /* extern */
M2C_UNK func_80049718(s16, s32, M2C_UNK, M2C_UNK);  /* extern */
M2C_UNK func_80049A2C(u8, s32, s32);                /* extern */
void *game_GetPlayerData(s32);                      /* extern */
M2C_UNK math_RotMatrixZYXAngles(u16, u16, u16, s16 *); /* extern */
s16 ratan2(s16, s16, s16 *, void *);                /* extern */
M2C_UNK scratchpad_Restore();                       /* extern */
M2C_UNK scratchpad_Save();                          /* extern */
extern M2C_UNK D_8008D90C;
extern M2C_UNK D_8008D9EC;
extern M2C_UNK D_8008DA50;
extern M2C_UNK D_8008DA94;
extern M2C_UNK D_8008DAD8;
extern M2C_UNK D_8008E0BC;
extern M2C_UNK D_8008EB80;
extern u16 D_800A36CA;
extern void *D_800A36D8;
extern s8 D_800A3748;
extern s16 D_800A376E;
extern u8 D_800A37A0;
extern u16 D_800A381C;
extern s16 D_800A3834;
extern u8 D_800A384C;
extern s32 D_800A385C;
extern M2C_UNK D_800A3888;
extern u8 D_800A389A;
extern s16 D_800A38AE;
extern s16 D_800A38BA;
extern s16 D_800A38DC;
extern M2C_UNK D_80101EC8;
extern s16 D_80101ED2;
extern M2C_UNK D_80101F12;
extern M2C_UNK Judge;

void func_80023F08(s32 arg0, void *arg1) {
    s32 sp1C0;
    s32 sp1BC;
    s32 sp1B8;
    s16 sp198;
    s16 sp178;
    s32 sp170;
    s32 sp16C;
    s32 sp168;
    s32 sp158;
    u16 sp154;
    s32 sp150;
    u16 sp14A;
    u16 sp148;
    s32 sp140;
    s32 sp13C;
    s32 sp138;
    s32 sp134;
    s32 sp130;
    s32 sp12C;
    s32 sp128;
    s32 sp124;
    s32 sp120;
    M2C_UNK sp11C;
    s16 sp9C;
    M2C_UNK sp98;
    s16 sp18;
    M2C_UNK var_a1_9;
    s16 *var_a2_5;
    s16 *var_a2_7;
    s16 *var_a2_8;
    s16 *var_a3;
    s16 *var_a3_2;
    s16 *var_a3_3;
    s16 *var_a3_4;
    s16 *var_v0;
    s16 temp_a0_11;
    s16 temp_a0_21;
    s16 temp_a0_22;
    s16 temp_a0_23;
    s16 temp_a0_2;
    s16 temp_a0_9;
    s16 temp_a1_2;
    s16 temp_a1_4;
    s16 temp_a1_9;
    s16 temp_s0_4;
    s16 temp_t0_2;
    s16 temp_t1;
    s16 temp_v0_10;
    s16 temp_v0_13;
    s16 temp_v0_3;
    s16 temp_v1_28;
    s16 temp_v1_34;
    s16 temp_v1_36;
    s16 temp_v1_37;
    s16 temp_v1_38;
    s16 temp_v1_44;
    s16 temp_v1_46;
    s16 temp_v1_9;
    s16 var_a0;
    s16 var_a1_6;
    s16 var_a2;
    s16 var_a3_5;
    s16 var_v0_10;
    s16 var_v0_7;
    s16 var_v1_5;
    s16 var_v1_7;
    s16 var_v1_8;
    s32 temp_a0;
    s32 temp_a0_13;
    s32 temp_a0_14;
    s32 temp_a0_15;
    s32 temp_a0_16;
    s32 temp_a0_18;
    s32 temp_a0_19;
    s32 temp_a0_4;
    s32 temp_a1;
    s32 temp_a1_10;
    s32 temp_a1_11;
    s32 temp_a1_5;
    s32 temp_a1_6;
    s32 temp_a1_7;
    s32 temp_a1_8;
    s32 temp_a2;
    s32 temp_a3;
    s32 temp_s0_5;
    s32 temp_s2;
    s32 temp_t0;
    s32 temp_t0_3;
    s32 temp_v0_12;
    s32 temp_v0_4;
    s32 temp_v0_5;
    s32 temp_v0_7;
    s32 temp_v0_8;
    s32 temp_v0_9;
    s32 temp_v1;
    s32 temp_v1_11;
    s32 temp_v1_13;
    s32 temp_v1_15;
    s32 temp_v1_16;
    s32 temp_v1_17;
    s32 temp_v1_18;
    s32 temp_v1_19;
    s32 temp_v1_20;
    s32 temp_v1_23;
    s32 temp_v1_24;
    s32 temp_v1_25;
    s32 temp_v1_27;
    s32 temp_v1_2;
    s32 temp_v1_33;
    s32 temp_v1_41;
    s32 temp_v1_42;
    s32 temp_v1_4;
    s32 var_a0_2;
    s32 var_a1_5;
    s32 var_a1_7;
    s32 var_a1_8;
    s32 var_a2_3;
    s32 var_a2_4;
    s32 var_a2_6;
    s32 var_lo;
    s32 var_s0_2;
    s32 var_v0_2;
    s32 var_v0_3;
    s32 var_v0_4;
    s32 var_v0_6;
    s32 var_v1;
    s32 var_v1_2;
    s32 var_v1_4;
    s32 var_v1_6;
    s8 var_a0_3;
    u16 *temp_s0;
    u16 *temp_s0_2;
    u16 *temp_s0_3;
    u16 *var_a2_2;
    u16 *var_v0_5;
    u16 temp_a0_17;
    u16 temp_a0_20;
    u16 temp_a0_3;
    u16 temp_a0_8;
    u16 temp_v0;
    u16 temp_v0_14;
    u16 temp_v1_10;
    u16 temp_v1_12;
    u16 temp_v1_14;
    u16 temp_v1_21;
    u16 temp_v1_22;
    u16 temp_v1_26;
    u16 temp_v1_29;
    u16 temp_v1_30;
    u16 temp_v1_32;
    u16 temp_v1_35;
    u16 temp_v1_39;
    u16 temp_v1_40;
    u16 temp_v1_43;
    u16 temp_v1_45;
    u16 temp_v1_5;
    u16 temp_v1_6;
    u16 temp_v1_7;
    u16 temp_v1_8;
    u16 var_a1_2;
    u16 var_a1_4;
    u16 var_s0;
    u16 var_s2;
    u16 var_v0_8;
    u16 var_v0_9;
    u32 temp_v0_6;
    u8 temp_a0_10;
    u8 temp_a0_12;
    u8 temp_a2_2;
    u8 temp_v0_15;
    u8 temp_v1_31;
    u8 temp_v1_47;
    u8 var_v1_3;
    void *temp_a0_5;
    void *temp_a0_6;
    void *temp_a0_7;
    void *temp_a1_3;
    void *temp_s0_6;
    void *temp_s1;
    void *temp_s2_2;
    void *temp_v0_11;
    void *temp_v0_2;
    void *temp_v1_3;
    void *var_a1;
    void *var_a1_3;
    void *var_a3_6;

    var_s2 = saved_reg_s2;
    temp_s1 = (arg0 * 0x44C) + &D_80101EC8;
    M2C_FIELD(temp_s1, s32 *, 0x3C) = (s32) (M2C_FIELD(temp_s1, s32 *, 0x3C) + 1);
    temp_a2 = M2C_FIELD(arg1, s32 *, 0xC);
    M2C_FIELD(temp_s1, s32 *, 0x24) = (s32) M2C_FIELD(arg1, s32 *, 0);
    M2C_FIELD(temp_s1, s32 *, 0x28) = (s32) M2C_FIELD(arg1, s32 *, 4);
    M2C_FIELD(temp_s1, s32 *, 0x2C) = (s32) M2C_FIELD(arg1, s32 *, 8);
    M2C_FIELD(temp_s1, s32 *, 0x30) = temp_a2;
    M2C_FIELD(temp_s1, s32 *, 0x34) = (s32) M2C_FIELD(arg1, s32 *, 0x10);
    M2C_FIELD(temp_s1, s32 *, 0x38) = (s32) M2C_FIELD(arg1, s32 *, 0x14);
    if ((D_800A38BA != 0) && ((arg0 == 0) || ((arg0 == 1) && (M2C_FIELD(temp_s1, s16 *, 6) == 0)))) {
        temp_v1 = M2C_FIELD(temp_s1, s32 *, 0x2C);
        temp_a0 = M2C_FIELD(temp_s1, s32 *, 0x30);
        M2C_FIELD(temp_s1, s32 *, 0x2C) = (s32) ((temp_v1 & 0xFFF) | ((u32) (temp_v1 & 0x8000) >> 3) | ((temp_v1 & 0x7000) * 2));
        temp_a1 = M2C_FIELD(temp_s1, s32 *, 0x34);
        M2C_FIELD(temp_s1, s32 *, 0x30) = (s32) ((temp_a0 & 0xFFF) | ((u32) (temp_a0 & 0x8000) >> 3) | ((temp_a0 & 0x7000) * 2));
        temp_v1_2 = M2C_FIELD(temp_s1, s32 *, 0x38);
        M2C_FIELD(temp_s1, s32 *, 0x34) = (s32) ((temp_a1 & 0xFFF) | ((u32) (temp_a1 & 0x8000) >> 3) | ((temp_a1 & 0x7000) * 2));
        M2C_FIELD(temp_s1, s32 *, 0x38) = (s32) ((temp_v1_2 & 0xFFF) | ((u32) (temp_v1_2 & 0x8000) >> 3) | ((temp_v1_2 & 0x7000) * 2));
    }
    if ((D_800A38DC == 2) || (D_800A38DC == 5) || (D_800A38DC == 3) || ((D_800A38DC == 0) && (D_800A385C != 0))) {
        M2C_FIELD(temp_s1, s32 *, 0x30) = (s32) (M2C_FIELD(temp_s1, s32 *, 0x30) & ~0x100);
    }
    if (M2C_FIELD(temp_s1, s32 *, 0x3C) < 2) {
        M2C_FIELD(temp_s1, s32 *, 0x34) = 0;
        M2C_FIELD(temp_s1, s32 *, 0x30) = 0;
        M2C_FIELD(temp_s1, s32 *, 0x2C) = 0;
        M2C_FIELD(temp_s1, s32 *, 0x38) = 0xFFFF;
    }
    temp_a0_2 = M2C_FIELD(temp_s1, s16 *, 0x272);
    temp_a1_2 = M2C_FIELD(temp_s1, s16 *, 0x1E);
    temp_v1_3 = (M2C_FIELD(temp_s1, s16 *, 0xA) * 8) + &D_8008E0BC;
    if (temp_a0_2 < 4) {
        var_v0 = temp_v1_3 + (temp_a0_2 * 2);
    } else {
        var_v0 = temp_v1_3 + 6;
    }
    M2C_FIELD(temp_s1, s16 *, 0x20) = (s16) ((s32) (temp_a1_2 * *var_v0) >> 0xC);
    if (M2C_FIELD(temp_s1, s16 *, 0x7A) == 1) {
        M2C_FIELD(temp_s1, s16 *, 0x7A) = 0;
    }
    func_8001F938(temp_s1, temp_a1_2, temp_a2);
    temp_v0 = M2C_FIELD(temp_s1, u16 *, 0x42) + M2C_FIELD(temp_s1, u16 *, 0x44);
    M2C_FIELD(temp_s1, u16 *, 0x42) = temp_v0;
    if ((s16) temp_v0 >= 0x1000) {
        M2C_FIELD(temp_s1, s16 *, 0x46) = 0;
        M2C_FIELD(temp_s1, u16 *, 0x40) = (u16) (M2C_FIELD(temp_s1, u16 *, 0x40) + ((s32) (temp_v0 << 0x10) >> 0x1C));
        M2C_FIELD(temp_s1, u16 *, 0x42) = (u16) (M2C_FIELD(temp_s1, u16 *, 0x42) & 0xFFF);
    } else {
        M2C_FIELD(temp_s1, s16 *, 0x46) = 1;
    }
    if (M2C_FIELD(temp_s1, s16 *, 0x286) >= 0) {
        temp_a0_3 = M2C_FIELD(temp_s1, u16 *, 0x6A);
        if (((temp_a0_3 & 0xFFFF) == 0xA) || ((u32) (temp_a0_3 - 0x17) < 2U)) {
            M2C_FIELD(temp_s1, s16 *, 0x72) = 1;
        }
        temp_s0 = temp_s1 + 0x5E;
        M2C_FIELD(temp_s1, s16 *, 0x31A) = 0;
        func_80021A98(arg0, func_80021424(temp_s1, *((M2C_FIELD(temp_s1, s16 *, 0x286) * 2) + func_80021424(temp_s1, M2C_FIELD(M2C_FIELD(temp_s1, void **, 0x50), u16 *, 0), temp_s0)), temp_s0), M2C_FIELD(temp_s1, s16 *, 0x5E));
        M2C_FIELD(temp_s1, s16 *, 0x286) = -1;
    }
    temp_v1_4 = M2C_FIELD(temp_s1, s32 *, 0x2C);
    if ((temp_v1_4 & 8) && (temp_v1_4 & 0xF000) && ((temp_v1_5 = M2C_FIELD(temp_s1, u16 *, 0x6A), temp_a0_4 = temp_v1_5 & 0xFFFF, (temp_a0_4 == 0x15)) || ((u32) (temp_v1_5 - 0x19) < 2U) || (temp_a0_4 == 0x13) || ((u32) (temp_v1_5 - 0x30) < 2U)) && (func_800233AC(temp_s1, &sp150) != 0)) {
        func_8001F860(temp_s1, sp150);
        temp_s0_2 = temp_s1 + 0x5E;
        M2C_FIELD(temp_s1, s16 *, 0x31A) = 0;
        func_80021A98(arg0, func_80021424(temp_s1, M2C_FIELD(func_80021424(temp_s1, M2C_FIELD(M2C_FIELD(temp_s1, void **, 0x50), u16 *, 0), temp_s0_2), u16 *, 0x1A), temp_s0_2), M2C_FIELD(temp_s1, s16 *, 0x5E));
        M2C_FIELD(temp_s1, s32 *, 0x104) = 0;
        M2C_FIELD(temp_s1, s32 *, 0x108) = 0;
        M2C_FIELD(temp_s1, s32 *, 0x10C) = 0;
    }
    temp_a0_5 = M2C_FIELD(temp_s1, void **, 0x50);
    if ((s32) M2C_FIELD(temp_a0_5, u8 *, 7) < (s16) M2C_FIELD(temp_s1, u16 *, 0x40)) {
        if (M2C_FIELD(temp_s1, void **, 0x7C) != NULL) {
            M2C_FIELD(temp_s1, s16 *, 0x5E) = (s16) M2C_FIELD(temp_s1, u16 *, 0x80);
            if (M2C_FIELD(temp_s1, u16 *, 0x82) & 0x1000) {
                M2C_FIELD(temp_s1, s16 *, 0x4C) = 1;
            }
            var_a1 = M2C_FIELD(temp_s1, void **, 0x7C);
            var_a2 = (s16) M2C_FIELD(temp_s1, u16 *, 0x80);
        } else {
            temp_v1_6 = M2C_FIELD(temp_a0_5, u16 *, 0xA);
            if ((temp_v1_6 & 0x2000) && (D_800A38AE != arg0)) {
                if (temp_v1_6 & 0x1000) {
                    M2C_FIELD(temp_s1, s16 *, 0x4C) = 1;
                }
                var_a1_2 = M2C_FIELD(M2C_FIELD(temp_s1, void **, 0x50), u16 *, 0xC);
            } else {
                if (M2C_FIELD(M2C_FIELD(temp_s1, void **, 0x50), u8 *, 9) & 0x20) {
                    M2C_FIELD(temp_s1, s16 *, 0x4C) = 1;
                }
                var_a1_2 = M2C_FIELD(M2C_FIELD(temp_s1, void **, 0x50), u16 *, 2);
            }
            var_a2 = M2C_FIELD(temp_s1, s16 *, 0x5E);
            var_a1 = func_80021424(temp_s1, var_a1_2, temp_s1 + 0x5E);
        }
        func_80021A98(arg0, var_a1, var_a2);
    }
    if (M2C_FIELD(temp_s1, void **, 0x7C) != NULL) {
        if ((s32) M2C_FIELD(M2C_FIELD(temp_s1, void **, 0x50), u8 *, 8) < (s16) M2C_FIELD(temp_s1, u16 *, 0x40)) {
            if (M2C_FIELD(temp_s1, u16 *, 0x6A) == 0x11) {
                D_800A376E = 1;
                D_800A36D8 = M2C_FIELD(temp_s1, void **, 0x7C);
                D_800A381C = M2C_FIELD(temp_s1, u16 *, 0x80);
                D_800A36CA = M2C_FIELD(temp_s1, u16 *, 0x82);
            } else {
                M2C_FIELD(temp_s1, s16 *, 0x5E) = (s16) M2C_FIELD(temp_s1, u16 *, 0x80);
                if (M2C_FIELD(temp_s1, u16 *, 0x82) & 0x1000) {
                    M2C_FIELD(temp_s1, s16 *, 0x4C) = 1;
                }
                func_80021A98(arg0, M2C_FIELD(temp_s1, void **, 0x7C), (s16) M2C_FIELD(temp_s1, u16 *, 0x80));
            }
        }
    } else {
        temp_a0_6 = M2C_FIELD(temp_s1, void **, 0x50);
        if ((s32) M2C_FIELD(temp_a0_6, u8 *, 8) >= (s16) M2C_FIELD(temp_s1, u16 *, 0x40)) {
            var_s0 = M2C_FIELD(temp_a0_6, u16 *, 0xA);
            var_a2_2 = temp_a0_6 + 0xA;
            if (var_s0 != 0) {
                var_a1_3 = temp_a0_6 + 0xC;
                var_v0_2 = var_s0 & 0x8000;
loop_61:
                var_v1 = var_s0 & 0x30;
                if ((var_v0_2 == 0) || (var_v1 = var_s0 & 0x30, (((M2C_FIELD(var_a1_3, u16 *, 2) | (M2C_FIELD(var_a1_3, u16 *, 4) << 0x10)) & (1 << M2C_FIELD(temp_s1, s16 *, 0xA))) != 0))) {
                    if (var_v1 != 0x10) {
                        if (var_v1 < 0x11) {
                            var_v0_3 = var_s0 & 0x2000;
                            if (var_v1 != 0) {

                            } else {
                                var_s2 = (u16) M2C_FIELD(temp_s1, s32 *, 0x2C);
                            }
                        } else if (var_v1 != 0x20) {
                            var_v0_3 = var_s0 & 0x2000;
                            if (var_v1 != 0x30) {

                            } else {
                                var_s2 = (u16) M2C_FIELD(temp_s1, s32 *, 0x34);
                            }
                        } else {
                            var_s2 = (u16) M2C_FIELD(temp_s1, s32 *, 0x38);
                            var_v0_3 = var_s0 & 0x2000;
                        }
                    } else {
                        var_s2 = (u16) M2C_FIELD(temp_s1, s32 *, 0x30);
                        var_v0_3 = var_s0 & 0x2000;
                    }
                    if (var_v0_3 != 0) {
                        var_s2 = 0;
                    }
                    if (M2C_FIELD(temp_s1, u16 *, 0x6A) == 0x11) {
                        var_v1_2 = var_s0 & 0xF;
                        if (D_800A38AE == arg0) {
                            var_s2 = 0;
                            goto block_79;
                        }
                    } else {
block_79:
                        var_v1_2 = var_s0 & 0xF;
                    }
                    if (var_v1_2 == 9) {
                        temp_a0_7 = M2C_FIELD(temp_s1, void **, 0);
                        var_s0 = (var_s0 & 0xFFF0) | 3;
                        var_v0_4 = var_s2 & 0xFFFF;
                        if (M2C_FIELD(temp_a0_7, u16 *, 0x6A) != 6) {
                            var_v0_4 = var_s2 & 0xFFFF;
                            if (M2C_FIELD(temp_a0_7, s16 *, 0x40) < (s32) M2C_FIELD(temp_a0_7, u8 *, 0xAB)) {
                                var_s2 = 0;
                                goto block_87;
                            }
                        }
                    } else {
                        var_v0_4 = var_s2 & 0xFFFF;
                        if (var_v1_2 == 0xA) {
                            var_s0 = (var_s0 & 0xFFF0) | 5;
                            if (M2C_FIELD(temp_s1, s16 *, 0x26C) == 0) {
                                var_s2 = 0;
                            }
block_87:
                            var_v0_4 = var_s2 & 0xFFFF;
                        }
                    }
                    if ((var_v0_4 >> (var_s0 & 0xF)) & 1) {
                        M2C_FIELD(temp_s1, s8 *, 0xB0) = (s8) var_s0;
                        if (var_s0 & 0x1000) {
                            M2C_FIELD(temp_s1, s16 *, 0x4C) = 1;
                        }
                        temp_a1_3 = func_80021424(temp_s1, M2C_FIELD(var_a1_3, u16 *, 0), &sp154);
                        if (!(M2C_FIELD(M2C_FIELD(temp_s1, void **, 0x50), u8 *, 9) & 0x80)) {
                            if (M2C_FIELD(temp_s1, u16 *, 0x6A) != 0x11) {
                                M2C_FIELD(temp_s1, s16 *, 0x5E) = (s16) sp154;
                                func_80021A98(arg0, temp_a1_3, (s16) sp154);
                            } else {
                                D_800A376E = 1;
                                D_800A36D8 = temp_a1_3;
                                D_800A36CA = var_s0;
                                D_800A381C = sp154;
                            }
                        } else {
                            M2C_FIELD(temp_s1, void **, 0x7C) = temp_a1_3;
                            M2C_FIELD(temp_s1, u16 *, 0x82) = var_s0;
                            M2C_FIELD(temp_s1, s16 *, 0x4C) = 0;
                            M2C_FIELD(temp_s1, u16 *, 0x80) = sp154;
                        }
                    } else {
                        goto block_94;
                    }
                } else {
block_94:
                    if (var_s0 & 0xC000) {
                        var_a1_3 += 8;
                        var_a2_2 += 8;
                    } else {
                        var_a1_3 += 4;
                        var_a2_2 += 4;
                    }
                    var_s0 = *var_a2_2;
                    var_v0_2 = var_s0 & 0x8000;
                    if (var_s0 != 0) {
                        goto loop_61;
                    }
                }
            }
        }
    }
    temp_a0_8 = M2C_FIELD(temp_s1, u16 *, 0x6A);
    if (((temp_a0_8 & 0xFFFF) == 0xA) || (M2C_FIELD(temp_s1, s16 *, 0x72) != 0)) {
        if (func_80023DB8(temp_s1) != 0) {
            if ((M2C_FIELD(temp_s1, s32 *, 0x74) + 0xFA0) < M2C_FIELD(temp_s1, s32 *, 0xBC)) {
                temp_v0_2 = func_80021424(temp_s1, M2C_FIELD(M2C_FIELD(temp_s1, void **, 0x50), u16 *, 0), temp_s1 + 0x5E);
                if (M2C_FIELD(temp_s1, s16 *, 0x94) != 0) {
                    var_v0_5 = temp_v0_2 + 0x2E;
                } else {
                    var_v0_5 = temp_v0_2 + 0x30;
                }
                var_a1_4 = *var_v0_5;
                goto block_108;
            }
            if (M2C_FIELD(temp_s1, s16 *, 0x72) == 0) {
                var_a1_4 = M2C_FIELD(M2C_FIELD(temp_s1, void **, 0x50), u16 *, 2);
block_108:
                func_80021A98(arg0, func_80021424(temp_s1, var_a1_4, temp_s1 + 0x5E), M2C_FIELD(temp_s1, s16 *, 0x5E));
            }
            M2C_FIELD(temp_s1, s16 *, 0x72) = 0;
        }
    } else if ((M2C_FIELD(temp_s1, s16 *, 0x7A) != 0) && ((u32) (M2C_FIELD(temp_s1, u16 *, 0x6C) - 0x17) < 2U) && ((u32) (temp_a0_8 - 0x17) >= 2U) && ((M2C_FIELD(temp_s1, s32 *, 0x74) + 0xFA0) < M2C_FIELD(temp_s1, s32 *, 0xBC))) {
        temp_s0_3 = temp_s1 + 0x5E;
        func_80021A98(arg0, func_80021424(temp_s1, M2C_FIELD(func_80021424(temp_s1, M2C_FIELD(M2C_FIELD(temp_s1, void **, 0x50), u16 *, 0), temp_s0_3), u16 *, 0x30), temp_s0_3), M2C_FIELD(temp_s1, s16 *, 0x5E));
    }
    if (M2C_FIELD(temp_s1, s16 *, 0x70) == 1) {
        func_8001F860(temp_s1, (s32) M2C_FIELD(temp_s1, s16 *, 0x1D8));
    }
    if ((M2C_FIELD(temp_s1, s16 *, 0x7A) != 0) && ((temp_v1_7 = M2C_FIELD(temp_s1, u16 *, 0x6A), (temp_v1_7 == 2)) || (temp_v1_7 == 0x1B) || (temp_v1_7 == 0x28) || (temp_v1_7 == 0x26) || (temp_v1_7 == 0x24)) && ((func_8001F860(temp_s1, (s32) M2C_FIELD(temp_s1, s16 *, 0x1D8)), temp_v1_8 = M2C_FIELD(temp_s1, u16 *, 0x6A), (temp_v1_8 == 2)) || (temp_v1_8 == 0x24))) {
        temp_a0_9 = M2C_FIELD(temp_s1, s16 *, 0x14C);
        temp_a1_4 = ((u8) M2C_FIELD(M2C_FIELD(temp_s1, void **, 0x58), u8 *, 2) >> 4) * 0x88;
        temp_v1_9 = -temp_a1_4;
        if (temp_a1_4 < temp_a0_9) {
            M2C_FIELD(temp_s1, s16 *, 0x14C) = temp_a1_4;
        } else if (temp_a0_9 < temp_v1_9) {
            M2C_FIELD(temp_s1, s16 *, 0x14C) = temp_v1_9;
        }
    }
    if ((M2C_FIELD(temp_s1, s16 *, 0x46) == 0) && (M2C_FIELD(temp_s1, u16 *, 0x6A) == 9) && (M2C_FIELD(temp_s1, u8 *, 0xA9) == (s16) M2C_FIELD(temp_s1, u16 *, 0x40))) {
        func_8001F860(temp_s1, (s32) M2C_FIELD(temp_s1, s16 *, 0x1D8));
    }
    if (M2C_FIELD(temp_s1, s16 *, 0x7A) != 0) {
        if (((s16) M2C_FIELD(temp_s1, u16 *, 0x6C) != 0x18) && (M2C_FIELD(temp_s1, u16 *, 0x6A) == 0x18)) {
            M2C_FIELD(temp_s1, s32 *, 0x104) = (s32) (M2C_FIELD(temp_s1, s32 *, 0x104) + ((s32) (*(&Judge + ((M2C_FIELD(temp_s1, u16 *, 0x1CA) & 0xFFF) * 2)) * *(&D_8008DA94 + (M2C_FIELD(temp_s1, s16 *, 0xA) * 2))) >> 0xC));
            M2C_FIELD(temp_s1, s32 *, 0x10C) = (s32) (M2C_FIELD(temp_s1, s32 *, 0x10C) + ((s32) (*(&Judge + ((((s16) M2C_FIELD(temp_s1, u16 *, 0x1CA) + 0x400) & 0xFFF) * 2)) * *(&D_8008DA94 + (M2C_FIELD(temp_s1, s16 *, 0xA) * 2))) >> 0xC));
            M2C_FIELD(temp_s1, s32 *, 0x74) = (s32) M2C_FIELD(temp_s1, s32 *, 0xBC);
            M2C_FIELD(temp_s1, s32 *, 0x108) = (s32) (M2C_FIELD(temp_s1, s32 *, 0x108) + *(&D_8008DA50 + (M2C_FIELD(temp_s1, s16 *, 0xA) * 2)));
        }
        if (M2C_FIELD(temp_s1, s16 *, 0x7A) != 0) {
            if (((s16) M2C_FIELD(temp_s1, u16 *, 0x6C) != 0x17) && (M2C_FIELD(temp_s1, u16 *, 0x6A) == 0x17)) {
                M2C_FIELD(temp_s1, s32 *, 0x74) = (s32) M2C_FIELD(temp_s1, s32 *, 0xBC);
                M2C_FIELD(temp_s1, s32 *, 0x108) = (s32) (M2C_FIELD(temp_s1, s32 *, 0x108) + *(&D_8008DAD8 + (M2C_FIELD(temp_s1, s16 *, 0xA) * 2)));
            }
            if ((M2C_FIELD(temp_s1, s16 *, 0x7A) != 0) && (M2C_FIELD(temp_s1, u16 *, 0x6A) == 0x28)) {
                temp_v0_3 = M2C_FIELD(temp_s1, s16 *, 0x1D8);
                var_a1_5 = (temp_v0_3 - (s16) M2C_FIELD(temp_s1, u16 *, 0x1CA)) & 0xFFF;
                var_v0_6 = temp_v0_3 & 0xFFF;
                if (var_a1_5 >= 0x800) {
                    var_a1_5 = 0x1000 - var_a1_5;
                    var_v0_6 = temp_v0_3 & 0xFFF;
                }
                sp168 = M2C_FIELD(M2C_FIELD(temp_s1, void **, 0), s32 *, 0x18C) - ((s32) (*(&Judge + (var_v0_6 * 2)) * var_a1_5) >> 0xE);
                sp16C = M2C_FIELD(M2C_FIELD(temp_s1, void **, 0), s32 *, 0x190);
                sp170 = M2C_FIELD(M2C_FIELD(temp_s1, void **, 0), s32 *, 0x194) - ((s32) (*(&Judge + (((M2C_FIELD(temp_s1, s16 *, 0x1D8) + 0x400) & 0xFFF) * 2)) * var_a1_5) >> 0xE);
                M2C_FIELD(temp_s1, s32 *, 0x108) = (s32) (M2C_FIELD(temp_s1, s32 *, 0x108) - 0xFA);
                func_800200DC(temp_s1 + 0x180, &sp168, M2C_FIELD(temp_s1, s32 *, 0x108), 0x1F, &sp158);
                M2C_FIELD(temp_s1, s32 *, 0x104) = (s32) (M2C_FIELD(temp_s1, s32 *, 0x104) + sp158);
                M2C_FIELD(temp_s1, s16 *, 0x72) = 1;
                M2C_FIELD(temp_s1, s32 *, 0x74) = (s32) M2C_FIELD(temp_s1, s32 *, 0xBC);
                M2C_FIELD(temp_s1, s32 *, 0x10C) = (s32) (M2C_FIELD(temp_s1, s32 *, 0x10C) + sp160);
            }
        }
    }
    temp_v1_10 = M2C_FIELD(temp_s1, u16 *, 0x6A);
    temp_v1_11 = temp_v1_10 & 0xFFFF;
    if (((u32) (temp_v1_10 - 0x17) < 2U) || (temp_v1_11 == 0x28) || (temp_v1_11 == 0xA)) {
        temp_a0_10 = M2C_FIELD(M2C_FIELD(temp_s1, void **, 0x50), u8 *, 7);
        if (((s16) M2C_FIELD(temp_s1, u16 *, 0x40) == temp_a0_10) && (M2C_FIELD(temp_s1, s32 *, 0x108) > 0)) {
            M2C_FIELD(temp_s1, u16 *, 0x40) = (u16) (temp_a0_10 - 1);
        }
    }
    if ((M2C_FIELD(temp_s1, s16 *, 0x7A) != 0) && (M2C_FIELD(temp_s1, u16 *, 0x6A) == 0x23)) {
        M2C_FIELD(temp_s1, s32 *, 0xBC) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xBC) - 0x898);
        M2C_FIELD(temp_s1, s32 *, 0x1FC) = (s32) (M2C_FIELD(temp_s1, s32 *, 0x1FC) + 0x898);
        M2C_FIELD(temp_s1, s32 *, 0xDC) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xDC) - 0x898);
    }
    if ((M2C_FIELD(temp_s1, s16 *, 0x7A) != 0) && (M2C_FIELD(temp_s1, u16 *, 0x6A) == 6)) {
        M2C_FIELD(temp_s1, u16 *, 0x86) = (u16) M2C_FIELD(temp_s1, u16 *, 0x84);
    }
    temp_a1_5 = (M2C_FIELD(temp_s1, s16 *, 0x14C) * (s16) M2C_FIELD(temp_s1, u16 *, 0x44)) / 24576;
    M2C_FIELD(temp_s1, u16 *, 0x1CA) = (u16) (M2C_FIELD(temp_s1, u16 *, 0x1CA) + temp_a1_5);
    M2C_FIELD(temp_s1, s16 *, 0x14C) = (s16) ((u16) M2C_FIELD(temp_s1, s16 *, 0x14C) - temp_a1_5);
    if (M2C_FIELD(temp_s1, s16 *, 0x7A) != 0) {
        if (M2C_FIELD(temp_s1, s16 *, 0x78) == 0) {
            temp_v1_12 = M2C_FIELD(temp_s1, u16 *, 0x6A);
            if (temp_v1_12 == 0x1C) {
                func_80023CB4(temp_s1, 0x400);
            } else {
                temp_a0_11 = (s16) M2C_FIELD(temp_s1, u16 *, 0x6C);
                if ((temp_a0_11 != 0x1C) && (temp_v1_12 != 0x11) && (temp_a0_11 != 0x11) && (temp_v1_12 != 0x15) && (temp_v1_12 != 0xA) && (temp_v1_12 != 0x2D) && (temp_v1_13 = (temp_a0_11 << 8) | temp_v1_12, (temp_v1_13 != 0x1525))) {
                    if (temp_v1_13 < 0x1526) {
                        if (temp_v1_13 != 0x1502) {
                            if (temp_v1_13 < 0x1503) {
                                if (temp_v1_13 != 0x1322) {

                                } else {
                                    goto block_181;
                                }
                            } else if (temp_v1_13 != 0x1519) {

                            } else {
                                goto block_181;
                            }
                        } else {
                            goto block_181;
                        }
                    } else if (temp_v1_13 != 0x3022) {
                        if (temp_v1_13 < 0x3023) {
                            if (temp_v1_13 != 0x1913) {

                            } else {
                                goto block_181;
                            }
                        } else if (temp_v1_13 == 0x3122) {
                            goto block_181;
                        }
                    } else {
                        goto block_181;
                    }
                } else {
                    goto block_181;
                }
            }
        } else {
block_181:
            func_80023D08(temp_s1, temp_a1_5);
        }
    }
    temp_a0_12 = M2C_FIELD(M2C_FIELD(temp_s1, void **, 0x58), u8 *, 1);
    var_a2_3 = (s16) M2C_FIELD(temp_s1, u16 *, 0x40) + 1;
    if (var_a2_3 >= (s32) temp_a0_12) {
        var_a2_3 = temp_a0_12 - 1;
    }
    if (M2C_FIELD(M2C_FIELD(temp_s1, void **, 0x50), u8 *, 9) & 0x40) {
        var_s0_2 = 0;
        if (M2C_FIELD(temp_s1, s16 *, 0x5E) == 0) {
            var_s0_2 = *(&D_80101F12 + (D_800A38AE * 0x44C)) + 1;
        }
        if (M2C_FIELD(temp_s1, u16 *, 0x6A) == 0x11) {
            var_v1_3 = 0;
            if (D_800A38AE == arg0) {

            } else {
                goto block_194;
            }
        } else {
            goto block_195;
        }
    } else {
        var_s0_2 = 0;
        if (M2C_FIELD(temp_s1, s16 *, 0x5E) == 0) {
            var_s0_2 = M2C_FIELD(temp_s1, s16 *, 0x4A) + 1;
        }
        if (M2C_FIELD(temp_s1, u16 *, 0x6A) == 0x15) {
            var_v1_3 = 0;
            if (*(&D_8008D9EC + M2C_FIELD(temp_s1, s16 *, 0xA)) != 0) {
block_194:
                var_v1_3 = M2C_FIELD(M2C_FIELD(temp_s1, void **, 0x58), u8 *, 1);
            }
        } else {
block_195:
            var_v1_3 = 0;
        }
    }
    temp_a0_13 = var_v1_3 + *M2C_FIELD(temp_s1, u16 **, 0x54);
    temp_a1_6 = temp_a0_13 + (s16) M2C_FIELD(temp_s1, u16 *, 0x40);
    temp_v0_4 = var_s0_2 << 0xE;
    temp_s2 = temp_a0_13 + var_a2_3;
    M2C_FIELD(temp_s1, u16 *, 0x64) = (u16) (temp_v0_4 | temp_a1_6);
    M2C_FIELD(temp_s1, u16 *, 0x66) = (u16) (temp_v0_4 | temp_s2);
    if (M2C_FIELD(temp_s1, u16 *, 0x6A) == 0x33) {
        var_a3 = &sp9C;
        var_a2_4 = ((s16) M2C_FIELD(temp_s1, u16 *, 0x40) * 0x84) + *(&D_800A3888 + (arg0 * 4));
        temp_t0 = var_a2_4 + 0x80;
        if ((var_a2_4 | (s32) var_a3) & 3) {
            do {
                M2C_FIELD(var_a3, M2C_UNK *, 3) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                M2C_FIELD(var_a3, M2C_UNK *, 7) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                M2C_FIELD(var_a3, M2C_UNK *, 0xB) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                M2C_FIELD(var_a3, M2C_UNK *, 0xF) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                var_a2_4 += 0x10;
                var_a3 += 0x10;
            } while (var_a2_4 != temp_t0);
        } else {
            do {
                M2C_FIELD(var_a3, s32 *, 0) = (s32) M2C_FIELD(var_a2_4, s32 *, 0);
                M2C_FIELD(var_a3, s32 *, 4) = (s32) M2C_FIELD(var_a2_4, s32 *, 4);
                M2C_FIELD(var_a3, s32 *, 8) = (s32) M2C_FIELD(var_a2_4, s32 *, 8);
                M2C_FIELD(var_a3, s32 *, 0xC) = (s32) M2C_FIELD(var_a2_4, s32 *, 0xC);
                var_a2_4 += 0x10;
                var_a3 += 0x10;
            } while (var_a2_4 != temp_t0);
        }
        M2C_FIELD(var_a3, M2C_UNK *, 3) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
        var_a3_2 = &sp18;
        var_a2_5 = &sp9C;
        if (((s32) var_a2_5 | (s32) var_a3_2) & 3) {
            do {
                M2C_FIELD(var_a3_2, M2C_UNK *, 3) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                M2C_FIELD(var_a3_2, M2C_UNK *, 7) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                M2C_FIELD(var_a3_2, M2C_UNK *, 0xB) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                M2C_FIELD(var_a3_2, M2C_UNK *, 0xF) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                var_a2_5 += 0x10;
                var_a3_2 += 0x10;
            } while (var_a2_5 != &sp11C);
        } else {
            do {
                M2C_FIELD(var_a3_2, s32 *, 0) = (s32) M2C_FIELD(var_a2_5, s32 *, 0);
                M2C_FIELD(var_a3_2, s32 *, 4) = (s32) M2C_FIELD(var_a2_5, s32 *, 4);
                M2C_FIELD(var_a3_2, s32 *, 8) = (s32) M2C_FIELD(var_a2_5, s32 *, 8);
                M2C_FIELD(var_a3_2, s32 *, 0xC) = (s32) M2C_FIELD(var_a2_5, s32 *, 0xC);
                var_a2_5 += 0x10;
                var_a3_2 += 0x10;
            } while (var_a2_5 != &sp11C);
        }
        M2C_FIELD(var_a3_2, M2C_UNK *, 3) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
    } else {
        func_800198D0(var_s0_2, temp_a1_6, &sp18, 0x1F8001B0);
        func_800198D0(var_s0_2, temp_s2, &sp9C, 0x1F8001B0);
    }
    if ((M2C_FIELD(temp_s1, s16 *, 0x7A) != 0) && ((temp_v1_14 = M2C_FIELD(temp_s1, u16 *, 0x6A), (temp_v1_14 == 6)) || (temp_v1_14 == 0x14))) {
        if ((s16) M2C_FIELD(temp_s1, u16 *, 0x6C) == 6) {
            math_RotMatrixZYXAngles(M2C_FIELD(temp_s1, u16 *, 0x296), M2C_FIELD(temp_s1, u16 *, 0x298), M2C_FIELD(temp_s1, u16 *, 0x29A), &sp178);
            math_RotMatrixZYXAngles(sp1E, sp20, sp22, &sp198);
            temp_s0_4 = ratan2(sp178, sp184);
            temp_a1_7 = ratan2(sp198, sp1A4) - temp_s0_4;
            M2C_FIELD(temp_s1, u16 *, 0x1CA) = (u16) (M2C_FIELD(temp_s1, u16 *, 0x1CA) + temp_a1_7);
            func_8001B690(arg0, temp_a1_7);
        }
    }
    func_8001F2E4(temp_s1, &sp18, &sp9C);
    if (M2C_FIELD(temp_s1, u16 *, 0x6A) == 0x11) {
        M2C_FIELD(temp_s1, u16 *, 0x154) = (u16) M2C_FIELD(temp_s1, u16 *, 0x1CA);
    }
    if (M2C_FIELD(temp_s1, s16 *, 0x152) != 0) {
        var_a1_6 = (s16) M2C_FIELD(temp_s1, u16 *, 0x154);
        var_v0_7 = var_a1_6;
    } else {
        var_a1_6 = (s16) M2C_FIELD(temp_s1, u16 *, 0x1CA);
        var_v0_7 = var_a1_6;
    }
    temp_a1_8 = var_a1_6 + 0x400;
    sp14A = (u16) var_v0_7;
    sp148 = (u16) var_v0_7;
    temp_v1_15 = sp1A - temp_a1_8;
    temp_t1 = -sp18;
    sp124 = (s32) temp_t1;
    temp_a3 = (s32) (sp1C * *(&Judge + (((temp_v1_15 + 0x400) & 0xFFF) * 2))) >> 0xC;
    sp120 = temp_a3;
    temp_v1_16 = sp9E - temp_a1_8;
    var_a2_6 = (s32) (sp1C * *(&Judge + ((temp_v1_15 & 0xFFF) * 2))) >> 0xC;
    sp128 = var_a2_6;
    temp_t0_2 = -sp9C;
    sp130 = (s32) temp_t0_2;
    var_a1_7 = (s32) (spA0 * *(&Judge + (((temp_v1_16 + 0x400) & 0xFFF) * 2))) >> 0xC;
    sp12C = var_a1_7;
    temp_v1_17 = (s32) (spA0 * *(&Judge + ((temp_v1_16 & 0xFFF) * 2))) >> 0xC;
    sp134 = temp_v1_17;
    sp120 = (s32) (temp_a3 * M2C_FIELD(temp_s1, s16 *, 0x1A)) >> 0xC;
    sp128 = (s32) (var_a2_6 * M2C_FIELD(temp_s1, s16 *, 0x1A)) >> 0xC;
    sp12C = (s32) (var_a1_7 * M2C_FIELD(temp_s1, s16 *, 0x1A)) >> 0xC;
    sp134 = (s32) (temp_v1_17 * M2C_FIELD(temp_s1, s16 *, 0x1A)) >> 0xC;
    if (M2C_FIELD(temp_s1, u16 *, 0x6A) != 8) {
        sp124 = (s32) (temp_t1 * M2C_FIELD(temp_s1, s16 *, 0x1A)) >> 0xC;
        sp130 = (s32) (temp_t0_2 * M2C_FIELD(temp_s1, s16 *, 0x1A)) >> 0xC;
    }
    var_a3_3 = &sp9C;
    if (M2C_FIELD(temp_s1, s16 *, 0x31A) != 0) {
        var_a2_7 = &sp18;
        if (((s32) var_a2_7 | (s32) var_a3_3) & 3) {
            do {
                M2C_FIELD(var_a3_3, M2C_UNK *, 3) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                M2C_FIELD(var_a3_3, M2C_UNK *, 7) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                M2C_FIELD(var_a3_3, M2C_UNK *, 0xB) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                M2C_FIELD(var_a3_3, M2C_UNK *, 0xF) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                var_a2_7 += 0x10;
                var_a3_3 += 0x10;
            } while (var_a2_7 != &sp98);
        } else {
            do {
                M2C_FIELD(var_a3_3, s32 *, 0) = (s32) M2C_FIELD(var_a2_7, s32 *, 0);
                M2C_FIELD(var_a3_3, s32 *, 4) = (s32) M2C_FIELD(var_a2_7, s32 *, 4);
                M2C_FIELD(var_a3_3, s32 *, 8) = (s32) M2C_FIELD(var_a2_7, s32 *, 8);
                M2C_FIELD(var_a3_3, s32 *, 0xC) = (s32) M2C_FIELD(var_a2_7, s32 *, 0xC);
                var_a2_7 += 0x10;
                var_a3_3 += 0x10;
            } while (var_a2_7 != &sp98);
        }
        M2C_FIELD(var_a3_3, M2C_UNK *, 3) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
        var_a3_4 = &sp18;
        var_a2_6 = (s32) (temp_s1 + 0x290);
        sp14A = sp148;
        if ((var_a2_6 | (s32) var_a3_4) & 3) {
            do {
                var_a1_7 = M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */);
                M2C_FIELD(var_a3_4, M2C_UNK *, 3) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                M2C_FIELD(var_a3_4, M2C_UNK *, 7) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                M2C_FIELD(var_a3_4, M2C_UNK *, 0xB) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                M2C_FIELD(var_a3_4, M2C_UNK *, 0xF) = M2C_UNALIGNED32(var_a1_7);
                var_a2_6 += 0x10;
                var_a3_4 += 0x10;
            } while (var_a2_6 != (temp_s1 + 0x310));
        } else {
            do {
                var_a1_7 = M2C_FIELD(var_a2_6, s32 *, 0xC);
                M2C_FIELD(var_a3_4, s32 *, 0) = (s32) M2C_FIELD(var_a2_6, s32 *, 0);
                M2C_FIELD(var_a3_4, s32 *, 4) = (s32) M2C_FIELD(var_a2_6, s32 *, 4);
                M2C_FIELD(var_a3_4, s32 *, 8) = (s32) M2C_FIELD(var_a2_6, s32 *, 8);
                M2C_FIELD(var_a3_4, s32 *, 0xC) = var_a1_7;
                var_a2_6 += 0x10;
                var_a3_4 += 0x10;
            } while (var_a2_6 != (temp_s1 + 0x310));
        }
        M2C_FIELD(var_a3_4, M2C_UNK *, 3) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
        sp148 = M2C_FIELD(temp_s1, u16 *, 0x314);
        var_a3_5 = M2C_FIELD(temp_s1, s16 *, 0x318);
        M2C_FIELD(temp_s1, u16 *, 0x66) = (u16) M2C_FIELD(temp_s1, u16 *, 0x64);
        M2C_FIELD(temp_s1, u16 *, 0x64) = (u16) M2C_FIELD(temp_s1, u16 *, 0x316);
    } else {
        var_a3_5 = (s16) M2C_FIELD(temp_s1, u16 *, 0x42);
    }
    temp_t0_3 = 0x1000 - var_a3_5;
    M2C_FIELD(temp_s1, s16 *, 0x68) = var_a3_5;
    if (M2C_FIELD(temp_s1, s16 *, 0x31A) != 0) {
        if (M2C_FIELD(temp_s1, s16 *, 0x7A) != 0) {
            M2C_FIELD(temp_s1, s32 *, 0xD8) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xD8) + (M2C_FIELD(temp_s1, s32 *, 0x1F8) - sp120));
            M2C_FIELD(temp_s1, s32 *, 0xE0) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xE0) + (M2C_FIELD(temp_s1, s32 *, 0x200) - sp128));
            M2C_FIELD(temp_s1, s32 *, 0x320) = (s32) (M2C_FIELD(temp_s1, s32 *, 0x320) + (M2C_FIELD(temp_s1, s32 *, 0x1F8) - sp120));
            M2C_FIELD(temp_s1, s32 *, 0x328) = (s32) (M2C_FIELD(temp_s1, s32 *, 0x328) + (M2C_FIELD(temp_s1, s32 *, 0x200) - sp128));
        }
        var_a2_6 = (s32) sp1C;
        temp_v1_18 = sp1A - ((s16) sp148 + 0x400);
        temp_a1_9 = -sp18;
        sp1BC = (s32) temp_a1_9;
        temp_a0_14 = (s32) (var_a2_6 * *(&Judge + (((temp_v1_18 + 0x400) & 0xFFF) * 2))) >> 0xC;
        sp1B8 = temp_a0_14;
        temp_v1_19 = (s32) (var_a2_6 * *(&Judge + ((temp_v1_18 & 0xFFF) * 2))) >> 0xC;
        sp1C0 = temp_v1_19;
        temp_a0_15 = (s32) (temp_a0_14 * M2C_FIELD(temp_s1, s16 *, 0x1A)) >> 0xC;
        sp1B8 = temp_a0_15;
        temp_v0_5 = (s32) (temp_a1_9 * M2C_FIELD(temp_s1, s16 *, 0x1A)) >> 0xC;
        sp1BC = temp_v0_5;
        temp_v1_20 = (s32) (temp_v1_19 * M2C_FIELD(temp_s1, s16 *, 0x1A)) >> 0xC;
        sp1C0 = temp_v1_20;
        temp_a0_16 = temp_a0_15 - M2C_FIELD(temp_s1, s32 *, 0x320);
        var_a1_7 = temp_a0_16 * temp_t0_3;
        sp1B8 = temp_a0_16;
        sp1C0 = temp_v1_20 - M2C_FIELD(temp_s1, s32 *, 0x328);
        M2C_FIELD(temp_s1, s32 *, 0xE8) = (s32) ((s32) (var_a1_7 + (sp120 * var_a3_5)) >> 0xC);
        M2C_FIELD(temp_s1, s32 *, 0xEC) = (s32) ((s32) ((temp_v0_5 * temp_t0_3) + (sp124 * var_a3_5)) >> 0xC);
        var_v1_4 = sp1C0 * temp_t0_3;
        var_lo = sp128 * var_a3_5;
    } else {
        M2C_FIELD(temp_s1, s32 *, 0xE8) = (s32) ((s32) ((sp120 * temp_t0_3) + (sp12C * var_a3_5)) >> 0xC);
        M2C_FIELD(temp_s1, s32 *, 0xEC) = (s32) ((s32) ((sp124 * temp_t0_3) + (sp130 * var_a3_5)) >> 0xC);
        var_v1_4 = sp128 * temp_t0_3;
        var_lo = sp134 * var_a3_5;
    }
    M2C_FIELD(temp_s1, s32 *, 0xF0) = (s32) ((s32) (var_v1_4 + var_lo) >> 0xC);
    if ((M2C_FIELD(temp_s1, s16 *, 0x7A) != 0) && (M2C_FIELD(temp_s1, s16 *, 0x31A) == 0) && ((M2C_FIELD(temp_s1, s32 *, 0xD8) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xD8) + (M2C_FIELD(temp_s1, s32 *, 0x1F8) - M2C_FIELD(temp_s1, s32 *, 0xE8))), temp_v1_21 = M2C_FIELD(temp_s1, u16 *, 0x6A), M2C_FIELD(temp_s1, s32 *, 0xE0) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xE0) + (M2C_FIELD(temp_s1, s32 *, 0x200) - M2C_FIELD(temp_s1, s32 *, 0xF0))), (temp_v1_21 == 0x23)) || (temp_v1_21 == 0xA))) {
        M2C_FIELD(temp_s1, s32 *, 0xDC) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xDC) + (M2C_FIELD(temp_s1, s32 *, 0x1FC) - M2C_FIELD(temp_s1, s32 *, 0xEC)));
    }
    func_80023648(temp_s1, var_a1_7, (u16) var_a2_6, var_a3_5);
    func_800238C4(temp_s1);
    temp_v1_22 = M2C_FIELD(temp_s1, u16 *, 0x6A);
    if (temp_v1_22 != 8) {
        if (temp_v1_22 != 0x22) {
            M2C_FIELD(temp_s1, s32 *, 0x108) = (s32) (M2C_FIELD(temp_s1, s32 *, 0x108) + 0x15);
        }
    }
    if ((M2C_FIELD(temp_s1, u16 *, 0x6A) == 8) || (M2C_FIELD(temp_s1, u16 *, 0x6A) == 0x22)) {
        var_v1_5 = M2C_FIELD(temp_s1, s16 *, 0x98);
        if (var_v1_5 < 0) {
            var_v1_5 += 0xFF;
        }
        var_a0 = M2C_FIELD(temp_s1, s16 *, 0x9C);
        M2C_FIELD(temp_s1, u32 *, 0x134) = (u32) (M2C_FIELD(temp_s1, u32 *, 0x134) - (var_v1_5 >> 8));
        if (var_a0 < 0) {
            var_a0 += 0xFF;
        }
        M2C_FIELD(temp_s1, s32 *, 0x13C) = (s32) (M2C_FIELD(temp_s1, s32 *, 0x13C) - (var_a0 >> 8));
    }
    if ((M2C_FIELD(temp_s1, s16 *, 0x7A) == 0) || (M2C_FIELD(temp_s1, u16 *, 0x6A) != 0x28)) {
        M2C_FIELD(temp_s1, s32 *, 0x104) = (s32) ((s32) (M2C_FIELD(temp_s1, s32 *, 0x104) * M2C_FIELD(temp_s1, s16 *, 0x156)) >> 0xC);
        M2C_FIELD(temp_s1, s32 *, 0x108) = (s32) ((s32) (M2C_FIELD(temp_s1, s32 *, 0x108) * M2C_FIELD(temp_s1, s16 *, 0x158)) >> 0xC);
        M2C_FIELD(temp_s1, s32 *, 0x10C) = (s32) ((s32) (M2C_FIELD(temp_s1, s32 *, 0x10C) * M2C_FIELD(temp_s1, s16 *, 0x15A)) >> 0xC);
    }
    temp_v1_23 = M2C_FIELD(temp_s1, s32 *, 0x104);
    if ((u32) (temp_v1_23 + 0xF) < 0x1FU) {
        M2C_FIELD(temp_s1, s32 *, 0x104) = (s32) (temp_v1_23 / 2);
    }
    temp_v1_24 = M2C_FIELD(temp_s1, s32 *, 0x108);
    if ((u32) (temp_v1_24 + 0xF) < 0x1FU) {
        M2C_FIELD(temp_s1, s32 *, 0x108) = (s32) (temp_v1_24 / 2);
    }
    temp_v1_25 = M2C_FIELD(temp_s1, s32 *, 0x10C);
    if ((u32) (temp_v1_25 + 0xF) < 0x1FU) {
        M2C_FIELD(temp_s1, s32 *, 0x10C) = (s32) (temp_v1_25 / 2);
    }
    M2C_FIELD(temp_s1, s32 *, 0xD8) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xD8) + M2C_FIELD(temp_s1, s32 *, 0x104));
    M2C_FIELD(temp_s1, s32 *, 0xE0) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xE0) + M2C_FIELD(temp_s1, s32 *, 0x10C));
    temp_v1_26 = M2C_FIELD(temp_s1, u16 *, 0x6A);
    M2C_FIELD(temp_s1, s32 *, 0xDC) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xDC) + M2C_FIELD(temp_s1, s32 *, 0x108));
    if (((temp_v1_26 == 7) || (temp_v1_26 == 0xD)) && (M2C_FIELD(temp_s1, u8 *, 0xB4) == 0)) {
        temp_v1_27 = M2C_FIELD(temp_s1, s32 *, 0x2C);
        var_a1_8 = 1;
        if (!(temp_v1_27 & 0x1000)) {
            var_a1_8 = -((temp_v1_27 & 0x4000) != 0);
        }
        if (var_a1_8 != 0) {
            temp_v0_6 = M2C_FIELD(temp_s1, u32 *, 0x134);
            temp_a0_17 = (u16) M2C_FIELD(temp_s1, s16 *, 0x1D8);
            M2C_FIELD(temp_s1, u32 *, 0x134) = (u32) ((s32) (temp_v0_6 + (temp_v0_6 >> 0x1F)) >> 1);
            M2C_FIELD(temp_s1, s32 *, 0x13C) = (s32) ((s32) M2C_FIELD(temp_s1, s32 *, 0x13C) / 2);
            var_a0_2 = *(&Judge + ((temp_a0_17 & 0xFFF) * 2)) - (*(&Judge + ((((s16) temp_a0_17 + 0x400) & 0xFFF) * 2)) * var_a1_8 * 3);
            if (var_a0_2 < 0) {
                var_a0_2 += 0x7F;
            }
            temp_v1_28 = M2C_FIELD(temp_s1, s16 *, 0x1D8);
            M2C_FIELD(temp_s1, u32 *, 0x134) = (u32) (M2C_FIELD(temp_s1, u32 *, 0x134) + (var_a0_2 >> 7));
            var_v1_6 = *((((temp_v1_28 + 0x400) & 0xFFF) * 2) + &Judge) + (*(((temp_v1_28 & 0xFFF) * 2) + &Judge) * var_a1_8 * 3);
            if (var_v1_6 < 0) {
                var_v1_6 += 0x7F;
            }
            M2C_FIELD(temp_s1, s32 *, 0x13C) = (s32) (M2C_FIELD(temp_s1, s32 *, 0x13C) + (var_v1_6 >> 7));
        }
    }
    temp_v0_7 = (s32) (M2C_FIELD(temp_s1, u32 *, 0x134) * M2C_FIELD(temp_s1, s16 *, 0x15E)) >> 0xC;
    M2C_FIELD(temp_s1, u32 *, 0x134) = (u32) temp_v0_7;
    if ((u32) (temp_v0_7 + 3) < 7U) {
        M2C_FIELD(temp_s1, u32 *, 0x134) = 0U;
    }
    temp_v0_8 = (s32) (M2C_FIELD(temp_s1, s32 *, 0x138) * M2C_FIELD(temp_s1, s16 *, 0x160)) >> 0xC;
    M2C_FIELD(temp_s1, s32 *, 0x138) = temp_v0_8;
    if ((u32) (temp_v0_8 + 3) < 7U) {
        M2C_FIELD(temp_s1, s32 *, 0x138) = 0;
    }
    temp_v0_9 = (s32) (M2C_FIELD(temp_s1, s32 *, 0x13C) * M2C_FIELD(temp_s1, s16 *, 0x162)) >> 0xC;
    M2C_FIELD(temp_s1, s32 *, 0x13C) = temp_v0_9;
    if ((u32) (temp_v0_9 + 3) < 7U) {
        M2C_FIELD(temp_s1, s32 *, 0x13C) = 0;
    }
    temp_v1_29 = M2C_FIELD(temp_s1, u16 *, 0x6A);
    if ((temp_v1_29 == 4) || (temp_v1_29 == 0x14) || (M2C_FIELD(temp_s1, s16 *, 0xC) == 0x1F)) {
        M2C_FIELD(temp_s1, u32 *, 0x134) = 0U;
        M2C_FIELD(temp_s1, s32 *, 0x138) = 0;
        M2C_FIELD(temp_s1, s32 *, 0x13C) = 0;
    } else {
        M2C_FIELD(temp_s1, s32 *, 0xD8) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xD8) + M2C_FIELD(temp_s1, u32 *, 0x134));
        M2C_FIELD(temp_s1, s32 *, 0xDC) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xDC) + M2C_FIELD(temp_s1, s32 *, 0x138));
        M2C_FIELD(temp_s1, s32 *, 0xE0) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xE0) + M2C_FIELD(temp_s1, s32 *, 0x13C));
    }
    sp138 = M2C_FIELD(temp_s1, s32 *, 0xD8) + M2C_FIELD(temp_s1, s32 *, 0xE8);
    temp_a0_18 = M2C_FIELD(temp_s1, s32 *, 0xDC);
    sp13C = temp_a0_18;
    temp_v1_30 = M2C_FIELD(temp_s1, u16 *, 0x6A);
    if ((temp_v1_30 == 8) || (temp_v1_30 == 0x22)) {
        sp13C = temp_a0_18 + M2C_FIELD(temp_s1, s32 *, 0xEC);
    }
    sp140 = M2C_FIELD(temp_s1, s32 *, 0xE0) + M2C_FIELD(temp_s1, s32 *, 0xF0);
    M2C_FIELD(temp_s1, s32 *, 0xC8) = (s32) M2C_FIELD(temp_s1, s32 *, 0xB8);
    M2C_FIELD(temp_s1, s32 *, 0xCC) = (s32) M2C_FIELD(temp_s1, s32 *, 0xBC);
    M2C_FIELD(temp_s1, s32 *, 0xD0) = (s32) M2C_FIELD(temp_s1, s32 *, 0xC0);
    M2C_FIELD(temp_s1, s32 *, 0xD4) = (s32) M2C_FIELD(temp_s1, s32 *, 0xC4);
    func_8002304C(temp_s1, temp_s1 + 0xB8, &sp138, temp_s1 + 0x104);
    temp_v1_31 = M2C_FIELD(temp_s1, u8 *, 0xB1);
    if ((temp_v1_31 != 7) && (temp_v1_31 != 0)) {
        M2C_FIELD(temp_s1, s8 *, 0xB2) = (s8) (((0x2A >> temp_v1_31) ^ 1) & 1);
    }
    temp_a0_19 = M2C_FIELD(temp_s1, s32 *, 0xBC);
    temp_v1_32 = M2C_FIELD(temp_s1, u16 *, 0x6A);
    M2C_FIELD(temp_s1, s32 *, 0xD8) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xB8) - M2C_FIELD(temp_s1, s32 *, 0xE8));
    M2C_FIELD(temp_s1, s32 *, 0xDC) = temp_a0_19;
    if ((temp_v1_32 == 8) || (temp_v1_32 == 0x22)) {
        M2C_FIELD(temp_s1, s32 *, 0xDC) = (s32) (temp_a0_19 - M2C_FIELD(temp_s1, s32 *, 0xEC));
    }
    M2C_FIELD(temp_s1, s32 *, 0xE0) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xC0) - M2C_FIELD(temp_s1, s32 *, 0xF0));
    M2C_FIELD(temp_s1, s32 *, 0xF4) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xD8) + M2C_FIELD(temp_s1, s32 *, 0xE8));
    temp_a1_10 = M2C_FIELD(temp_s1, s32 *, 0xF0);
    M2C_FIELD(temp_s1, s32 *, 0xF8) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xDC) + M2C_FIELD(temp_s1, s32 *, 0xEC));
    M2C_FIELD(temp_s1, s32 *, 0xFC) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xE0) + temp_a1_10);
    M2C_FIELD(temp_s1, s32 *, 0x1F8) = (s32) M2C_FIELD(temp_s1, s32 *, 0xE8);
    M2C_FIELD(temp_s1, s32 *, 0x1FC) = (s32) M2C_FIELD(temp_s1, s32 *, 0xEC);
    M2C_FIELD(temp_s1, s32 *, 0x200) = (s32) M2C_FIELD(temp_s1, s32 *, 0xF0);
    M2C_FIELD(temp_s1, s32 *, 0x168) = (s32) M2C_FIELD(temp_s1, s32 *, 0xF4);
    M2C_FIELD(temp_s1, s32 *, 0x170) = (s32) M2C_FIELD(temp_s1, s32 *, 0xFC);
    M2C_FIELD(temp_s1, s32 *, 0x16C) = (s32) (M2C_FIELD(temp_s1, s32 *, 0xDC) - 0x384);
    func_80023E40(temp_s1, temp_a1_10);
    func_80041188(arg0, &sp18, &sp9C, M2C_FIELD(temp_s1, s16 *, 0x68), 0x1F8001B0);
    scratchpad_Save();
    func_80040D48(arg0, 1, temp_s1 + 0xF4, temp_s1 + 0x1C8, 0, M2C_FIELD(temp_s1, s32 *, 0x148));
    scratchpad_Restore();
    if (((s16) M2C_FIELD(temp_s1, u16 *, 0x86) == M2C_FIELD(temp_s1, s16 *, 0x8E)) && ((temp_a0_20 = M2C_FIELD(temp_s1, u16 *, 0x6A), temp_v1_33 = temp_a0_20 & 0xFFFF, (temp_v1_33 == 0x15)) || ((u32) (temp_a0_20 - 0x19) < 2U) || (temp_v1_33 == 0x16) || (temp_v1_33 == 0x30) || (temp_v1_33 == 0x13) || (temp_v1_33 == 0x31))) {
        M2C_FIELD(temp_s1, s16 *, 0x92) = 0;
    } else {
        var_v1_7 = 2;
        if (M2C_FIELD(temp_s1, s16 *, 0x92) != 0) {
            var_v1_7 = 1;
        }
        M2C_FIELD(temp_s1, s16 *, 0x92) = var_v1_7;
    }
    M2C_FIELD(temp_s1, u8 *, 0x62) = 0U;
    if ((M2C_FIELD(temp_s1, s16 *, 0xC) != 0x1F) && (M2C_FIELD(temp_s1, s16 *, 0x96) == 0) && (M2C_FIELD(temp_s1, s16 *, 0x92) != 0)) {
        M2C_FIELD(temp_s1, u8 *, 0x62) = 1U;
    }
    if (((D_800A38DC != 3) || (arg0 != 1) || (D_800A384C == 4)) && ((u16) M2C_FIELD(temp_s1, u16 *, 0xE) < 2U) && ((D_800A38DC != 2) || (D_800A389A != 0)) && (D_800A38DC != 5)) {
        M2C_FIELD(temp_s1, u8 *, 0x62) = (u8) (M2C_FIELD(temp_s1, u8 *, 0x62) | 2);
    }
    if (((s16) M2C_FIELD(temp_s1, u16 *, 0x86) == M2C_FIELD(temp_s1, s16 *, 0x88)) && (M2C_FIELD(temp_s1, s16 *, 0x8A) != 0)) {
        if (func_8002798C(temp_s1) != 0) {
            goto block_336;
        }
        goto block_339;
    }
    temp_v1_34 = M2C_FIELD(temp_s1, s16 *, 0xC);
    if ((((temp_v1_34 == 0x1D) || (temp_v1_34 == 0xE)) && ((temp_v1_35 = M2C_FIELD(temp_s1, u16 *, 0x6A), (temp_v1_35 == 2)) || (temp_v1_35 == 0x1B) || (temp_v1_35 == 0x28) || (temp_v1_35 == 0x26)) && ((u8) M2C_FIELD(temp_s1, u8 *, 0xA2) < 0xFFU) && (M2C_FIELD(temp_s1, s16 *, 0x26C) != 0) && (temp_v1_36 = (s16) M2C_FIELD(temp_s1, u16 *, 0x40), ((temp_v1_36 < (s32) M2C_FIELD(temp_s1, u8 *, 0xA5)) == 0)) && ((s32) M2C_FIELD(temp_s1, u8 *, 0xA6) >= temp_v1_36)) || ((M2C_FIELD(temp_s1, s16 *, 0xA) == 0xE) && (M2C_FIELD(temp_s1, u16 *, 0x6A) == 0x11) && (D_800A38AE == arg0) && (temp_v1_37 = (s16) M2C_FIELD(temp_s1, u16 *, 0x40), ((temp_v1_37 < (s32) M2C_FIELD(temp_s1, u8 *, 0xA5)) == 0)) && ((s32) M2C_FIELD(temp_s1, u8 *, 0xA6) >= temp_v1_37))) {
block_336:
        var_v1_8 = 2;
        if (M2C_FIELD(temp_s1, s16 *, 0x8C) != 0) {
            var_v1_8 = 1;
        }
        M2C_FIELD(temp_s1, s16 *, 0x8C) = var_v1_8;
        M2C_FIELD(temp_s1, u8 *, 0x62) = (u8) (M2C_FIELD(temp_s1, u8 *, 0x62) | 4);
    } else {
block_339:
        M2C_FIELD(temp_s1, s16 *, 0x8C) = 0;
    }
    if (D_800A38DC == 3) {
        if (arg0 == 1) {
            if (D_800A384C == 4) {
                if ((arg0 == 1) && (D_800A384C == 4)) {
                    temp_v1_38 = M2C_FIELD(M2C_FIELD(temp_s1, void **, 0), s16 *, 0xA);
                    if ((temp_v1_38 != arg0) && ((u32) ((temp_v1_38 - 3) & 0xFFFF) >= 2U) && (temp_v1_38 != 9) && (temp_v1_38 != 0x11)) {

                    } else {
                        goto block_359;
                    }
                    goto block_360;
                }
                goto block_351;
            }
        } else {
            goto block_352;
        }
    } else {
block_351:
block_352:
        if (((D_800A38DC != 2) || (D_800A389A != 0)) && (D_800A38DC != 5)) {
            temp_a0_21 = M2C_FIELD(temp_s1, s16 *, 0xA);
            if ((temp_a0_21 == 1) || ((u32) ((temp_a0_21 - 3) & 0xFFFF) < 2U) || (temp_a0_21 == 9) || (temp_a0_21 == 0x11)) {
block_359:
                M2C_FIELD(temp_s1, u8 *, 0x62) = (u8) (M2C_FIELD(temp_s1, u8 *, 0x62) | 8);
            }
block_360:
            temp_v1_39 = M2C_FIELD(temp_s1, u16 *, 0x6A);
            if ((temp_v1_39 != 4) && (temp_v1_39 != 0x14) && (M2C_FIELD(temp_s1, s16 *, 0x92) == 0)) {
                M2C_FIELD(temp_s1, u8 *, 0x62) = (u8) (M2C_FIELD(temp_s1, u8 *, 0x62) | 0x10);
            }
            if ((M2C_FIELD(temp_s1, s16 *, 0x330) > 0) && (M2C_FIELD(temp_s1, s16 *, 0x332) == M2C_FIELD(temp_s1, s16 *, 0x14)) && (M2C_FIELD(temp_s1, s16 *, 0x8C) == 0)) {
                M2C_FIELD(temp_s1, u8 *, 0x62) = (u8) (M2C_FIELD(temp_s1, u8 *, 0x62) | 0x20);
            }
        }
    }
    if (((D_800A38DC == 2) && (D_800A389A == 0)) || (D_800A38DC == 5)) {
        if ((u16) M2C_FIELD(temp_s1, u16 *, 0xE) < 2U) {
            temp_v1_40 = M2C_FIELD(temp_s1, u16 *, 0x6A);
            if ((temp_v1_40 != 4) && (temp_v1_40 != 0x14) && (M2C_FIELD(temp_s1, s16 *, 0x92) == 0)) {
                M2C_FIELD(temp_s1, u8 *, 0x62) = (u8) ((M2C_FIELD(temp_s1, u8 *, 0x62) | 2) & 0xEF);
            }
        }
        if ((M2C_FIELD(temp_s1, s16 *, 0x330) > 0) && (M2C_FIELD(temp_s1, s16 *, 0x332) == M2C_FIELD(temp_s1, s16 *, 0x14)) && (M2C_FIELD(temp_s1, s16 *, 0x8C) == 0)) {
            M2C_FIELD(temp_s1, u8 *, 0x62) = (u8) ((M2C_FIELD(temp_s1, u8 *, 0x62) | 8) & 0xDF);
            func_80049A2C(*(&D_8008EB80 + M2C_FIELD(temp_s1, s16 *, 0x14)), (arg0 * 2) | 1, 0);
        }
    }
    if (M2C_FIELD(temp_s1, u8 *, 0x62) & 1) {
        func_80049718(M2C_FIELD(temp_s1, s16 *, 0x12), (arg0 * 2) + 0x8000, 0, 0);
    }
    if (M2C_FIELD(temp_s1, u8 *, 0x62) & 4) {
        func_80049718((s16) *(&D_8008EB80 + M2C_FIELD(temp_s1, s16 *, 0x14)), (arg0 * 2) + 0x8001, 0, 0);
    }
    temp_a2_2 = M2C_FIELD(temp_s1, u8 *, 0x62);
    if (temp_a2_2 & 2) {
        func_80049A2C((u8) M2C_FIELD(temp_s1, s16 *, 0x12), arg0 * 2, (temp_a2_2 >> 4) & 1);
    }
    if (M2C_FIELD(temp_s1, u8 *, 0x62) & 8) {
        func_80049A2C(*(&D_8008EB80 + M2C_FIELD(temp_s1, s16 *, 0x14)), (arg0 * 2) | 1, (M2C_FIELD(temp_s1, u8 *, 0x62) >> 5) & 1);
    }
    var_a3_6 = temp_s1 + 0x290;
    var_a2_8 = &sp18;
    if (((s32) var_a2_8 | (s32) var_a3_6) & 3) {
        do {
            M2C_FIELD(var_a3_6, M2C_UNK *, 3) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
            M2C_FIELD(var_a3_6, M2C_UNK *, 7) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
            M2C_FIELD(var_a3_6, M2C_UNK *, 0xB) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
            M2C_FIELD(var_a3_6, M2C_UNK *, 0xF) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
            var_a2_8 += 0x10;
            var_a3_6 += 0x10;
        } while (var_a2_8 != &sp98);
    } else {
        do {
            M2C_FIELD(var_a3_6, s32 *, 0) = (s32) M2C_FIELD(var_a2_8, s32 *, 0);
            M2C_FIELD(var_a3_6, s32 *, 4) = (s32) M2C_FIELD(var_a2_8, s32 *, 4);
            M2C_FIELD(var_a3_6, s32 *, 8) = (s32) M2C_FIELD(var_a2_8, s32 *, 8);
            M2C_FIELD(var_a3_6, s32 *, 0xC) = (s32) M2C_FIELD(var_a2_8, s32 *, 0xC);
            var_a2_8 += 0x10;
            var_a3_6 += 0x10;
        } while (var_a2_8 != &sp98);
    }
    M2C_FIELD(var_a3_6, M2C_UNK *, 3) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
    temp_a0_22 = M2C_FIELD(temp_s1, s16 *, 0x31A);
    M2C_FIELD(temp_s1, u16 *, 0x314) = sp148;
    M2C_FIELD(temp_s1, u16 *, 0x316) = (u16) M2C_FIELD(temp_s1, u16 *, 0x64);
    if (temp_a0_22 != 0) {
        if (temp_a0_22 >= 2) {
            M2C_FIELD(temp_s1, s16 *, 0x31A) = 1;
        }
        temp_v0_10 = (u16) M2C_FIELD(temp_s1, s16 *, 0x318) + M2C_FIELD(temp_s1, u16 *, 0x31C);
        M2C_FIELD(temp_s1, s16 *, 0x318) = temp_v0_10;
        var_a2_8 = &sp9C;
        if (temp_v0_10 >= 0x1000) {
            var_a3_6 = temp_s1 + 0x290;
            M2C_FIELD(temp_s1, s16 *, 0x31A) = 0;
            if (((s32) var_a2_8 | (s32) var_a3_6) & 3) {
                do {
                    M2C_FIELD(var_a3_6, M2C_UNK *, 3) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                    M2C_FIELD(var_a3_6, M2C_UNK *, 7) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                    M2C_FIELD(var_a3_6, M2C_UNK *, 0xB) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                    M2C_FIELD(var_a3_6, M2C_UNK *, 0xF) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
                    var_a2_8 += 0x10;
                    var_a3_6 += 0x10;
                } while (var_a2_8 != &sp11C);
            } else {
                do {
                    M2C_FIELD(var_a3_6, s32 *, 0) = (s32) M2C_FIELD(var_a2_8, s32 *, 0);
                    M2C_FIELD(var_a3_6, s32 *, 4) = (s32) M2C_FIELD(var_a2_8, s32 *, 4);
                    M2C_FIELD(var_a3_6, s32 *, 8) = (s32) M2C_FIELD(var_a2_8, s32 *, 8);
                    M2C_FIELD(var_a3_6, s32 *, 0xC) = (s32) M2C_FIELD(var_a2_8, s32 *, 0xC);
                    var_a2_8 += 0x10;
                    var_a3_6 += 0x10;
                } while (var_a2_8 != &sp11C);
            }
            M2C_FIELD(var_a3_6, M2C_UNK *, 3) = M2C_UNALIGNED32(M2C_ERROR(/* Unable to handle lwr; missing a corresponding lwl */));
            M2C_FIELD(temp_s1, u16 *, 0x314) = sp14A;
            M2C_FIELD(temp_s1, u16 *, 0x316) = (u16) M2C_FIELD(temp_s1, u16 *, 0x66);
            M2C_FIELD(temp_s1, s32 *, 0x1F8) = sp12C;
            M2C_FIELD(temp_s1, s32 *, 0x1FC) = sp130;
            M2C_FIELD(temp_s1, s32 *, 0x200) = sp134;
        }
    }
    temp_v0_11 = M2C_FIELD(temp_s1, void **, 0);
    M2C_FIELD(temp_s1, s16 *, 0x1D8) = ratan2(M2C_FIELD(temp_v0_11, s32 *, 0xF4) - M2C_FIELD(temp_s1, s32 *, 0xF4), M2C_FIELD(temp_v0_11, s32 *, 0xFC) - M2C_FIELD(temp_s1, s32 *, 0xFC), var_a2_8, var_a3_6);
    temp_a1_11 = M2C_FIELD(temp_s1, s32 *, 0x110);
    M2C_FIELD(temp_s1, s32 *, 0x24C) = (s32) M2C_FIELD(temp_s1, s32 *, 0x104);
    M2C_FIELD(temp_s1, s32 *, 0x250) = (s32) M2C_FIELD(temp_s1, s32 *, 0x108);
    M2C_FIELD(temp_s1, s32 *, 0x254) = (s32) M2C_FIELD(temp_s1, s32 *, 0x10C);
    M2C_FIELD(temp_s1, s32 *, 0x258) = temp_a1_11;
    func_80023D28(temp_s1, temp_a1_11);
    temp_v0_12 = arg0 * 0x24;
    temp_s2_2 = temp_v0_12 + 0x1F800000;
    temp_s0_5 = arg0 * 0x18;
    func_800207C8(temp_s1, (arg0 * 0x108) + 0x1F8000A8, temp_s2_2, temp_s0_5 + 0x1F800048);
    if (M2C_FIELD(temp_s1, s32 *, 0x3C) == 1) {
        M2C_FIELD(temp_s1, s32 *, 0x210) = (s32) M2C_FIELD(temp_v0_12, s32 *, 0x1F800000);
        M2C_FIELD(temp_s1, s32 *, 0x214) = (s32) M2C_FIELD(temp_s2_2, s32 *, 4);
        M2C_FIELD(temp_s1, s32 *, 0x218) = (s32) M2C_FIELD(temp_s2_2, s32 *, 8);
    }
    if (M2C_FIELD(temp_s1, s16 *, 0x8C) == 2) {
        M2C_FIELD(temp_s1, s16 *, 0x8C) = 1;
        M2C_FIELD(temp_s1, s32 *, 0x234) = (s32) M2C_FIELD((temp_s0_5 + 0x1F800000), s32 *, 0x48);
        M2C_FIELD(temp_s1, s32 *, 0x238) = (s32) M2C_FIELD((temp_s0_5 + 0x1F800000), s32 *, 0x4C);
        M2C_FIELD(temp_s1, s32 *, 0x23C) = (s32) M2C_FIELD((temp_s0_5 + 0x1F800000), s32 *, 0x50);
        M2C_FIELD(temp_s1, s32 *, 0x240) = (s32) M2C_FIELD((temp_s0_5 + 0x1F800000), s32 *, 0x54);
        M2C_FIELD(temp_s1, s32 *, 0x244) = (s32) M2C_FIELD((temp_s0_5 + 0x1F800000), s32 *, 0x58);
        M2C_FIELD(temp_s1, s32 *, 0x248) = (s32) M2C_FIELD((temp_s0_5 + 0x1F800000), s32 *, 0x5C);
    }
    if (M2C_FIELD(temp_s1, s16 *, 0x92) == 2) {
        M2C_FIELD(temp_s1, s16 *, 0x92) = 1;
        M2C_FIELD(temp_s1, s32 *, 0x234) = (s32) M2C_FIELD((temp_s0_5 + 0x1F800000), s32 *, 0x48);
        M2C_FIELD(temp_s1, s32 *, 0x238) = (s32) M2C_FIELD((temp_s0_5 + 0x1F800000), s32 *, 0x4C);
        M2C_FIELD(temp_s1, s32 *, 0x23C) = (s32) M2C_FIELD((temp_s0_5 + 0x1F800000), s32 *, 0x50);
        M2C_FIELD(temp_s1, s32 *, 0x240) = (s32) M2C_FIELD((temp_s0_5 + 0x1F800000), s32 *, 0x54);
        M2C_FIELD(temp_s1, s32 *, 0x244) = (s32) M2C_FIELD((temp_s0_5 + 0x1F800000), s32 *, 0x58);
        M2C_FIELD(temp_s1, s32 *, 0x248) = (s32) M2C_FIELD((temp_s0_5 + 0x1F800000), s32 *, 0x5C);
    }
    if (((u32) (M2C_FIELD(temp_s1, u16 *, 0xE) - 6) < 2U) && (M2C_FIELD(temp_s1, u8 *, 0x62) & 1)) {
        M2C_FIELD(temp_s1, s32 *, 0x25C) = (s32) M2C_FIELD(temp_v0_12, s32 *, 0x1F800000);
        M2C_FIELD(temp_s1, s32 *, 0x260) = (s32) M2C_FIELD(temp_s2_2, s32 *, 4);
        M2C_FIELD(temp_s1, s32 *, 0x264) = (s32) M2C_FIELD(temp_s2_2, s32 *, 8);
        M2C_FIELD(temp_s1, s32 *, 0x268) = 1;
    } else {
        M2C_FIELD(temp_s1, s32 *, 0x268) = 0;
    }
    temp_v1_41 = arg0 * 0x24;
    M2C_FIELD(temp_s1, s32 *, 0x114) = (s32) (M2C_FIELD(temp_v1_41, s32 *, 0x1F800000) - M2C_FIELD(temp_s1, s32 *, 0x210));
    M2C_FIELD(temp_s1, s32 *, 0x118) = (s32) (M2C_FIELD((temp_v1_41 + 0x1F800000), s32 *, 4) - M2C_FIELD(temp_s1, s32 *, 0x214));
    temp_v1_42 = arg0 * 0x18;
    M2C_FIELD(temp_s1, s32 *, 0x11C) = (s32) (M2C_FIELD((temp_v1_41 + 0x1F800000), s32 *, 8) - M2C_FIELD(temp_s1, s32 *, 0x218));
    M2C_FIELD(temp_s1, s32 *, 0x124) = (s32) (M2C_FIELD((temp_v1_42 + 0x1F800000), s32 *, 0x48) - M2C_FIELD(temp_s1, s32 *, 0x234));
    M2C_FIELD(temp_s1, s32 *, 0x128) = (s32) (M2C_FIELD((temp_v1_42 + 0x1F800000), s32 *, 0x4C) - M2C_FIELD(temp_s1, s32 *, 0x238));
    M2C_FIELD(temp_s1, s32 *, 0x12C) = (s32) (M2C_FIELD((temp_v1_42 + 0x1F800000), s32 *, 0x50) - M2C_FIELD(temp_s1, s32 *, 0x23C));
    temp_v1_43 = M2C_FIELD(temp_s1, u16 *, 0x6A);
    M2C_FIELD(temp_s1, s16 *, 0x1DA) = func_8002FDB0(temp_s1);
    var_a0_3 = 0;
    if ((temp_v1_43 == 2) || (temp_v1_43 == 0x1B) || (temp_v1_43 == 0x28) || (temp_v1_43 == 0x26)) {
        if (M2C_FIELD(temp_s1, u8 *, 0xAD) != 0) {
            temp_v1_44 = (s16) M2C_FIELD(temp_s1, u16 *, 0x40);
            if (((temp_v1_44 >= (s32) M2C_FIELD(temp_s1, u8 *, 0xA1)) && ((s32) M2C_FIELD(temp_s1, u8 *, 0xA3) >= temp_v1_44)) || ((temp_v1_44 >= (s32) M2C_FIELD(temp_s1, u8 *, 0xA2)) && ((s32) M2C_FIELD(temp_s1, u8 *, 0xA4) >= temp_v1_44))) {
                var_a0_3 = 1;
            }
        }
    }
    M2C_FIELD(temp_s1, s8 *, 0xAE) = var_a0_3;
    M2C_FIELD(temp_s1, u16 *, 0x288) = 0U;
    M2C_FIELD(temp_s1, u16 *, 0x28A) = 0U;
    if (M2C_FIELD(temp_s1, u8 *, 0xAD) != 0) {
        temp_v1_45 = M2C_FIELD(temp_s1, u16 *, 0x6A);
        if ((temp_v1_45 == 2) || (temp_v1_45 == 0x1B) || (temp_v1_45 == 0x28) || (temp_v1_45 == 0x26)) {
            temp_a0_23 = (s16) M2C_FIELD(temp_s1, u16 *, 0x40);
            M2C_FIELD(temp_s1, u16 *, 0x288) = (u16) (M2C_FIELD(temp_s1, u16 *, 0x288) + 1);
            M2C_FIELD(temp_s1, u16 *, 0x28A) = (u16) (M2C_FIELD(temp_s1, u16 *, 0x28A) + 1);
            if (temp_a0_23 < (s32) M2C_FIELD(temp_s1, u8 *, 0xA1)) {
                var_v0_8 = M2C_FIELD(temp_s1, u16 *, 0x288) + 2;
                goto block_433;
            }
            if ((s32) M2C_FIELD(temp_s1, u8 *, 0xA3) >= temp_a0_23) {
                var_v0_8 = M2C_FIELD(temp_s1, u16 *, 0x288) + 4;
block_433:
                M2C_FIELD(temp_s1, u16 *, 0x288) = var_v0_8;
            }
            temp_v1_46 = (s16) M2C_FIELD(temp_s1, u16 *, 0x40);
            if (temp_v1_46 < (s32) M2C_FIELD(temp_s1, u8 *, 0xA2)) {
                var_v0_9 = M2C_FIELD(temp_s1, u16 *, 0x28A) + 2;
                goto block_438;
            }
            if ((s32) M2C_FIELD(temp_s1, u8 *, 0xA4) >= temp_v1_46) {
                var_v0_9 = M2C_FIELD(temp_s1, u16 *, 0x28A) + 4;
block_438:
                M2C_FIELD(temp_s1, u16 *, 0x28A) = var_v0_9;
            }
        }
    }
    if (M2C_FIELD(temp_s1, u16 *, 0x6A) == 6) {
        var_v0_10 = 0x600;
    } else {
        var_v0_10 = 0xC00;
        if (M2C_FIELD(temp_s1, u16 *, 0x6A) == 8) {
            var_v0_10 = 0x100;
        }
    }
    M2C_FIELD(temp_s1, s16 *, 0x15E) = var_v0_10;
    M2C_FIELD(temp_s1, s16 *, 0x160) = var_v0_10;
    M2C_FIELD(temp_s1, s16 *, 0x162) = var_v0_10;
    if (M2C_FIELD(temp_s1, s16 *, 0x1DC) != 0) {
        M2C_FIELD(temp_s1, s16 *, 0x156) = 0xD00;
        M2C_FIELD(temp_s1, s16 *, 0x158) = 0xFC0;
        M2C_FIELD(temp_s1, s16 *, 0x15A) = 0xD00;
    } else {
        M2C_FIELD(temp_s1, s16 *, 0x156) = 0xFC0;
        M2C_FIELD(temp_s1, s16 *, 0x158) = 0xFC0;
        M2C_FIELD(temp_s1, s16 *, 0x15A) = 0xFC0;
    }
    if ((M2C_FIELD(temp_s1, s16 *, 0x96) == 0) && (((M2C_FIELD(temp_s1, s16 *, 0x7A) != 0) && (temp_v0_13 = (s16) M2C_FIELD(temp_s1, u16 *, 0x6C), (temp_v0_13 != 4)) && (temp_v0_13 != 0x14) && ((temp_v0_14 = M2C_FIELD(temp_s1, u16 *, 0x6A), (temp_v0_14 == 4)) || (temp_v0_14 == 0x14))) || ((M2C_FIELD(temp_s1, u16 *, 0x6A) == 0x11) && (arg0 != D_800A38AE) && (M2C_FIELD(temp_s1, u8 *, 0xAA) == (s16) M2C_FIELD(temp_s1, u16 *, 0x40))))) {
        temp_s0_6 = game_GetPlayerData(arg0);
        if ((D_800A38DC != 0) || (*(&D_8008D9EC + D_80101ED2) == 0) || (D_800A37A0 != 1) || (arg0 != D_800A37A0)) {
            func_80032854(arg0, 0x2E, temp_s1 + 0xF4, 0);
        }
        if (M2C_FIELD(temp_s1, s16 *, 0xC) != 0x1F) {
            cpu_set_move_command_and_dir(temp_s1, *((*(&D_8008D9EC + M2C_FIELD(temp_s1, s16 *, 0xA)) * 8) + &D_8008D90C + (s16) M2C_FIELD(temp_s1, u16 *, 0xE)), M2C_FIELD(temp_s0_6, s32 *, 0x48) + 0x14);
        }
        M2C_FIELD(temp_s1, s16 *, 0x96) = 1;
        if (D_800A3748 == -1) {
            D_800A3748 = arg0 == 0;
        }
    }
    if ((M2C_FIELD(temp_s1, s16 *, 0x7A) != 0) && (M2C_FIELD(temp_s1, s16 *, 0xC) == 0x1B) && (M2C_FIELD(temp_s1, u16 *, 0x6A) == 0xB) && (M2C_FIELD(temp_s1, u8 *, 0x34D) == 0)) {
        M2C_FIELD(temp_s1, s16 *, 0x286) = 0x1F;
    }
    if (M2C_FIELD(temp_s1, s16 *, 0x46) == 0) {
        if ((D_800A38DC != 5) && ((u32) ((D_800A38DC - 2) & 0xFFFF) >= 2U) && ((D_800A38DC != 0) || (D_800A385C == 0))) {
            if (M2C_FIELD(temp_s1, u16 *, 0x6A) == 0xB) {
                if ((s16) M2C_FIELD(temp_s1, u16 *, 0x40) == M2C_FIELD(temp_s1, u8 *, 0xA7)) {
                    if (M2C_FIELD(temp_s1, s16 *, 0xC) == 0x1B) {
                        temp_v0_15 = M2C_FIELD(temp_s1, u8 *, 0x34D);
                        if (temp_v0_15 != 0) {
                            M2C_FIELD(temp_s1, u8 *, 0x34D) = (u8) (temp_v0_15 - 1);
                        }
                    } else if ((M2C_FIELD(temp_s1, s16 *, 0x26C) != 0) && (func_800307D0(temp_s1) == M2C_FIELD(temp_s1, s16 *, 0x14))) {
                        M2C_FIELD(temp_s1, s16 *, 0x8A) = 0;
                    }
                }
            }
        }
        if (M2C_FIELD(temp_s1, u16 *, 0x6A) == 6) {
            if (func_80030BA8(temp_s1) == M2C_FIELD(temp_s1, s16 *, 0x14)) {
                M2C_FIELD(temp_s1, s16 *, 0x8A) = (s16) (u16) M2C_FIELD(temp_s1, s16 *, 0x26C);
            }
        } else if ((M2C_FIELD(temp_s1, s16 *, 0x26C) != 0) && ((M2C_FIELD(temp_s1, u16 *, 0x6A) == 0xC) || (M2C_FIELD(temp_s1, u16 *, 0x6A) == 0x2A)) && ((s16) M2C_FIELD(temp_s1, u16 *, 0x40) == M2C_FIELD(temp_s1, u8 *, 0xA8)) && (func_80030BA8(temp_s1) == M2C_FIELD(temp_s1, s16 *, 0x14))) {
            M2C_FIELD(temp_s1, s16 *, 0x8A) = 1;
        }
        if (M2C_FIELD(temp_s1, u16 *, 0x6A) == 0x12) {
            if ((s16) M2C_FIELD(temp_s1, u16 *, 0x40) == M2C_FIELD(temp_s1, u8 *, 0xA7)) {
                temp_v1_47 = M2C_FIELD(temp_s1, u8 *, 0xB1);
                if (((0x78 >> temp_v1_47) & 1) && (M2C_FIELD(temp_s1, s16 *, 0x26C) != 0)) {
                    var_a1_9 = 2;
                    if ((0x18 >> temp_v1_47) & 1) {
                        var_a1_9 = 1;
                    }
                    func_80032064(temp_s1, var_a1_9);
                }
            }
        }
        if ((M2C_FIELD(temp_s1, u16 *, 0x6A) == 0x10) && ((s16) M2C_FIELD(temp_s1, u16 *, 0x40) == M2C_FIELD(temp_s1, u8 *, 0xAC)) && (M2C_FIELD(temp_s1, s16 *, 0x26C) != 0) && (M2C_FIELD(temp_s1, u8 *, 0x34B) != 0)) {
            M2C_FIELD(temp_s1, s8 *, 0x34A) = 0xA;
            M2C_FIELD(temp_s1, u8 *, 0x34B) = (u8) (M2C_FIELD(temp_s1, u8 *, 0x34B) - 1);
            func_80032854(arg0, 0x31, temp_s1 + 0xF4, 0);
        }
        if ((M2C_FIELD(temp_s1, s16 *, 0x46) == 0) && ((M2C_FIELD(temp_s1, u16 *, 0x6A) != 0x11) || (arg0 == D_800A38AE))) {
            M2C_FIELD(temp_s1, u8 *, 0x62) = (u8) (M2C_FIELD(temp_s1, u8 *, 0x62) | 0x40);
            cpu_check_same_dir_timer(temp_s1);
        }
    }
    if (M2C_FIELD(temp_s1, s16 *, 0x26C) != 0) {
        M2C_FIELD(temp_s1, u8 *, 0x62) = (u8) (M2C_FIELD(temp_s1, u8 *, 0x62) | 0x80);
    }
    if (M2C_FIELD(temp_s1, s16 *, 0x96) != 0) {
        if (D_800A3834 != 0xD) {
            M2C_FIELD(temp_s1, u8 *, 0xB3) = 4U;
        }
    } else {
        M2C_FIELD(temp_s1, u8 *, 0xB3) = 0U;
    }
    if (M2C_FIELD(temp_s1, s16 *, 0xC) == 0x1F) {
        M2C_FIELD(temp_s1, u8 *, 0xB3) = 4U;
    }
    func_80040304(arg0, M2C_FIELD(temp_s1, u8 *, 0xB3));
    func_800204C0(temp_s1);
    if (M2C_FIELD(temp_s1, s16 *, 0x7A) == 2) {
        M2C_FIELD(temp_s1, s16 *, 0x7A) = 0;
    }
    func_80039680(temp_s1);
}
