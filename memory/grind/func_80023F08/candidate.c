typedef struct {
    s16 unk0;
    u16 unk2;
    u16 unk4;
    u16 unk6;
    u16 unk8;
    u16 unkA;
    u16 unkC[0x3C];
} Pose23;

typedef struct {
    u16 unk0;
    u16 unk2;
    u16 unk4;
    u8 unk6;
    u8 unk7;
    u8 unk8;
    u8 unk9;
    u16 unkA[1];
} Move23;

typedef struct {
    LeafPos unk00[2][3];
    LeafPos unk48[2][2];
    u8 unk78[0xA8 - 0x78];
    LeafPos unkA8[2][22];
} ScrPad23;
#define SPAD23 ((ScrPad23 *)0x1F800000)

typedef struct R23 {
    struct R23 *unk_00;
    s16 unk_04;
    s16 unk_06;
    s16 unk_08;
    s16 unk_0A;
    s16 unk_0C;
    s16 unk_0E;
    u8  unk_10[2];
    s16 unk_12;
    s16 unk_14;
    u8  unk_16[4];
    s16 unk_1A;
    s16 unk_1C;
    s16 unk_1E;
    s16 unk_20;
    u8  unk_22[2];
    PadState unk_24;
    s32 unk_3C;
    s16 unk_40;
    s16 unk_42;
    s16 unk_44;
    s16 unk_46;
    u8  unk_48[2];
    s16 unk_4A;
    s16 unk_4C;
    u8  unk_4E[2];
    Move23 *unk_50;
    u16 *unk_54;
    u8 *unk_58;
    u16 unk_5C;
    s16 unk_5E;
    u8  unk_60[2];
    u8  unk_62;
    u8  unk_63;
    u16 unk_64;
    u16 unk_66;
    s16 unk_68;
    u16 unk_6A;
    s16 unk_6C;
    u8  unk_6E[2];
    s16 unk_70;
    s16 unk_72;
    s32 unk_74;
    s16 unk_78;
    s16 unk_7A;
    Move23 *unk_7C;
    s16 unk_80;
    u16 unk_82;
    s16 unk_84;
    s16 unk_86;
    s16 unk_88;
    s16 unk_8A;
    s16 unk_8C;
    s16 unk_8E;
    s16 unk_90;
    s16 unk_92;
    s16 unk_94;
    s16 unk_96;
    SVec4i16 unk_98;
    u8  unk_A0;
    u8  unk_A1[2];
    u8  unk_A3[2];
    u8  unk_A5;
    u8  unk_A6;
    u8  unk_A7;
    u8  unk_A8;
    u8  unk_A9;
    u8  unk_AA;
    u8  unk_AB;
    u8  unk_AC;
    u8  unk_AD;
    u8  unk_AE;
    u8  unk_AF;
    u8  unk_B0;
    u8  unk_B1;
    u8  unk_B2;
    u8  unk_B3;
    u8  unk_B4;
    u8  unk_B5[3];
    Vec4i32 unk_B8;
    Vec4i32 unk_C8;
    Vec3i32 unk_D8;
    u8  unk_E4[4];
    Vec3i32 unk_E8;
    Vec3i32 unk_F4;
    u8  unk_100[4];
    Vec4i32 unk_104;
    Vec4i32 unk_114[2];
    Vec4i32 unk_134;
    s32 unk_144;
    s32 unk_148;
    s16 unk_14C;
    s16 unk_14E;
    s16 unk_150;
    s16 unk_152;
    s16 unk_154;
    s16 unk_156;
    s16 unk_158;
    s16 unk_15A;
    u8  unk_15C[2];
    s16 unk_15E;
    s16 unk_160;
    s16 unk_162;
    u8  unk_164[4];
    Vec3i32 unk_168;
    Vec3i32 unk_174;
    Vec3i32 unk_180;
    Vec3i32 unk_18C;
    u8  unk_198[0x1C8 - 0x198];
    SVec4i16 unk_1C8;
    SVec4i16 unk_1D0;
    s16 unk_1D8;
    s16 unk_1DA;
    s16 unk_1DC;
    u8  unk_1DE[0x1F8 - 0x1DE];
    Vec3i32 unk_1F8;
    u8  unk_204[0x210 - 0x204];
    LeafPos unk_210[3];
    LeafPos unk_234[2];
    Vec4i32 unk_24C;
    LeafPos unk_25C;
    s32 unk_268;
    s16 unk_26C;
    s16 unk_26E;
    s16 unk_270;
    s16 unk_272;
    u8  unk_274[0x286 - 0x274];
    s16 unk_286;
    u16 unk_288[2];
    s32 unk_28C;
    Pose23 unk_290;
    u16 unk_314;
    u16 unk_316;
    s16 unk_318;
    s16 unk_31A;
    s16 unk_31C;
    u8  unk_31E[2];
    Vec3i32 unk_320;
    u8  unk_32C[4];
    s16 unk_330;
    s16 unk_332[12];
    u8  unk_34A;
    u8  unk_34B;
    u8  unk_34C;
    u8  unk_34D;
    u8  unk_34E[0x44C - 0x34E];
} R23;

extern s16 D_8008E0BC[][4];
extern u8 D_8008D90C[][8];


extern void cpu_check_same_dir_timer(s32 *);
extern void cpu_set_move_command_and_dir(PracticeMenuRec *, s32, Vec3i32 *);
extern s32 func_8002798C(u8 *);
extern s32 func_8002FDB0(s32 *);
extern s32 func_800307D0(PracticeMenuRec *);
extern s32 func_80030BA8(PracticeMenuRec *);
extern u8 *func_80032064(u8 *, s32);
extern void func_80039680(u8 *);
extern void func_80040304(s32, s32);
extern void func_80040D48(s32, s32, s32 *, s16 *, s16 *, s32);
extern void func_80041188(s32, u8 *, u8 *, s32, s32 *);
extern void func_80049718(s32, s32, s32 *, s16 *);
extern void func_80049A2C(s32, s32, s32);
extern void scratchpad_Save(void);
extern void scratchpad_Restore(void);
extern void func_800204C0(u8 *);
extern void func_800207C8(u8 *, LeafPos *, LeafPos *, LeafPos *);

void func_80023F08(s32 arg0, s32 arg1) {
    Pose23 pose[2];
    Vec3i32 v[2];
    s32 pos[3];
    s16 ang[4];
    s32 dir;
    s16 sid;
    R23 *rec;
    u16 flags;
    u16 bits;
    u16 *ent;
    Move23 *mr;
    u16 *tbl;
    s32 d;
    s32 t;
    s32 frame;
    s32 nf;
    s32 base;
    s32 nfr;
    s32 obj;
    s32 add;
    s32 f0;
    s32 f1;
    s16 prev;
    s16 sub;
    s32 u;
    s32 state;
    s32 a;
    s32 b;

    rec = (R23 *)&g_practice_menu_table[arg0];
    rec->unk_3C++;
    rec->unk_24 = *(PadState *)arg1;
    if (D_800A38BA != 0 && (arg0 == 0 || (arg0 == 1 && rec->unk_06 == 0))) {
        rec->unk_24.held = (rec->unk_24.held & 0xFFF) | ((rec->unk_24.held & 0x8000) >> 3) | ((rec->unk_24.held & 0x7000) << 1);
        rec->unk_24.pressed = (rec->unk_24.pressed & 0xFFF) | ((rec->unk_24.pressed & 0x8000) >> 3) | ((rec->unk_24.pressed & 0x7000) << 1);
        rec->unk_24.released = (rec->unk_24.released & 0xFFF) | ((rec->unk_24.released & 0x8000) >> 3) | ((rec->unk_24.released & 0x7000) << 1);
        rec->unk_24.unheld = (rec->unk_24.unheld & 0xFFF) | ((rec->unk_24.unheld & 0x8000) >> 3) | ((rec->unk_24.unheld & 0x7000) << 1);
    }
    if (D_800A38DC == 2 || D_800A38DC == 5 || D_800A38DC == 3 || (D_800A38DC == 0 && D_800A385C != 0)) {
        rec->unk_24.pressed &= ~0x100;
    }
    if (rec->unk_3C < 2) {
        rec->unk_24.held = rec->unk_24.pressed = rec->unk_24.released = 0;
        rec->unk_24.unheld = 0xFFFF;
    }
    rec->unk_20 = (rec->unk_1E * D_8008E0BC[rec->unk_0A][rec->unk_272 >= 4 ? 3 : rec->unk_272]) >> 12;
    if (rec->unk_7A == 1) {
        rec->unk_7A = 0;
    }
    func_8001F938((u8 *)rec);
    sub = rec->unk_42 + rec->unk_44;
    rec->unk_42 = sub;
    if (sub >= 0x1000) {
        rec->unk_46 = 0;
        rec->unk_40 += sub >> 12;
        rec->unk_42 &= 0xFFF;
    } else {
        rec->unk_46 = 1;
    }
    if (rec->unk_286 >= 0) {
        if (rec->unk_6A == 0xA || rec->unk_6A == 0x17 || rec->unk_6A == 0x18) {
            rec->unk_72 = 1;
        }
        rec->unk_31A = 0;
        func_80021A98(arg0, func_80021424((u8 *)rec, ((u16 *)func_80021424((u8 *)rec, rec->unk_50->unk0, (u8 *)&rec->unk_5E))[rec->unk_286], (u8 *)&rec->unk_5E), rec->unk_5E);
        rec->unk_286 = -1;
    }
    if ((rec->unk_24.held & 8) && (rec->unk_24.held & 0xF000)) {
        if ((rec->unk_6A == 0x15 || rec->unk_6A == 0x19 || rec->unk_6A == 0x1A || rec->unk_6A == 0x13 || rec->unk_6A == 0x30 || rec->unk_6A == 0x31) && func_800233AC((u8 *)rec, &dir) != 0) {
            func_8001F860((s16 *)rec, dir);
            rec->unk_31A = 0;
            func_80021A98(arg0, func_80021424((u8 *)rec, ((u16 *)func_80021424((u8 *)rec, rec->unk_50->unk0, (u8 *)&rec->unk_5E))[0xD], (u8 *)&rec->unk_5E), rec->unk_5E);
            rec->unk_104.vx = 0;
            rec->unk_104.vy = 0;
            rec->unk_104.vz = 0;
        }
    }
    if (rec->unk_50->unk7 < rec->unk_40) {
        if (rec->unk_7C != NULL) {
            rec->unk_5E = rec->unk_80;
            if (rec->unk_82 & 0x1000) {
                rec->unk_4C = 1;
            }
            func_80021A98(arg0, (u8 *)rec->unk_7C, rec->unk_80);
        } else {
            if ((rec->unk_50->unkA[0] & 0x2000) && D_800A38AE != arg0) {
                if (rec->unk_50->unkA[0] & 0x1000) {
                    rec->unk_4C = 1;
                }
                func_80021A98(arg0, func_80021424((u8 *)rec, rec->unk_50->unkA[1], (u8 *)&rec->unk_5E), rec->unk_5E);
            } else {
                if (rec->unk_50->unk9 & 0x20) {
                    rec->unk_4C = 1;
                }
                func_80021A98(arg0, func_80021424((u8 *)rec, rec->unk_50->unk2, (u8 *)&rec->unk_5E), rec->unk_5E);
            }
        }
    }
    if (rec->unk_7C != NULL) {
        if (rec->unk_50->unk8 < rec->unk_40) {
            if (rec->unk_6A == 0x11) {
                D_800A376E = 1;
                D_800A36D8 = (s32)rec->unk_7C;
                D_800A381C = rec->unk_80;
                D_800A36CA = rec->unk_82;
            } else {
                rec->unk_5E = rec->unk_80;
                if (rec->unk_82 & 0x1000) {
                    rec->unk_4C = 1;
                }
                func_80021A98(arg0, (u8 *)rec->unk_7C, rec->unk_80);
            }
        }
    } else if (rec->unk_50->unk8 >= rec->unk_40) {
        ent = rec->unk_50->unkA;
        while ((flags = ent[0]) != 0) {
            if (!(flags & 0x8000) || ((ent[2] | ((u32)ent[3] << 16)) & (1 << rec->unk_0A))) {
                switch (flags & 0x30) {
                case 0x00:
                    bits = rec->unk_24.held;
                    break;
                case 0x10:
                    bits = rec->unk_24.pressed;
                    break;
                case 0x20:
                    bits = rec->unk_24.unheld;
                    break;
                case 0x30:
                    bits = rec->unk_24.released;
                    break;
                }
                if (flags & 0x2000) {
                    bits = 0;
                }
                if (rec->unk_6A == 0x11 && D_800A38AE == arg0) {
                    bits = 0;
                }
                if ((flags & 0xF) == 9) {
                    flags = (flags & 0xFFF0) | 3;
                    if (rec->unk_00->unk_6A != 6 && rec->unk_00->unk_40 < rec->unk_00->unk_AB) {
                        bits = 0;
                    }
                } else if ((flags & 0xF) == 0xA) {
                    flags = (flags & 0xFFF0) | 5;
                    if (rec->unk_26C == 0) {
                        bits = 0;
                    }
                }
                if ((bits >> (flags & 0xF)) & 1) {
                    rec->unk_B0 = flags;
                    if (flags & 0x1000) {
                        rec->unk_4C = 1;
                    }
                    mr = func_80021424((u8 *)rec, ent[1], (u8 *)&sid);
                    if (rec->unk_50->unk9 & 0x80) {
                        rec->unk_7C = mr;
                        rec->unk_82 = flags;
                        rec->unk_4C = 0;
                        rec->unk_80 = sid;
                    } else if (rec->unk_6A == 0x11) {
                        D_800A376E = 1;
                        D_800A36D8 = (s32)mr;
                        D_800A36CA = flags;
                        D_800A381C = sid;
                    } else {
                        rec->unk_5E = sid;
                        func_80021A98(arg0, (u8 *)mr, sid);
                    }
                    break;
                }
            }
            if (flags & 0xC000) {
                ent += 4;
            } else {
                ent += 2;
            }
        }
    }
    if (rec->unk_6A == 0xA || rec->unk_72 != 0) {
        if (func_80023DB8((u8 *)rec) != 0) {
            if (rec->unk_74 + 0xFA0 < rec->unk_B8.vy) {
                tbl = func_80021424((u8 *)rec, rec->unk_50->unk0, (u8 *)&rec->unk_5E);
                func_80021A98(arg0, func_80021424((u8 *)rec, tbl[rec->unk_94 != 0 ? 0x17 : 0x18], (u8 *)&rec->unk_5E), rec->unk_5E);
            } else if (rec->unk_72 == 0) {
                func_80021A98(arg0, func_80021424((u8 *)rec, rec->unk_50->unk2, (u8 *)&rec->unk_5E), rec->unk_5E);
            }
            rec->unk_72 = 0;
        }
    } else if (rec->unk_7A != 0 && (rec->unk_6C == 0x17 || rec->unk_6C == 0x18) && !(rec->unk_6A == 0x17 || rec->unk_6A == 0x18) && rec->unk_74 + 0xFA0 < rec->unk_B8.vy) {
        func_80021A98(arg0, func_80021424((u8 *)rec, ((u16 *)func_80021424((u8 *)rec, rec->unk_50->unk0, (u8 *)&rec->unk_5E))[0x18], (u8 *)&rec->unk_5E), rec->unk_5E);
    }
    if (rec->unk_70 == 1) {
        func_8001F860((s16 *)rec, rec->unk_1D8);
    }
    if (rec->unk_7A != 0) {
        if (rec->unk_6A == 2 || rec->unk_6A == 0x1B || rec->unk_6A == 0x28 || rec->unk_6A == 0x26 || rec->unk_6A == 0x24) {
            func_8001F860((s16 *)rec, rec->unk_1D8);
            if (rec->unk_6A == 2 || rec->unk_6A == 0x24) {
                d = (rec->unk_58[2] >> 4) * 0x88;
                if (rec->unk_14C > d) {
                    rec->unk_14C = d;
                } else if (rec->unk_14C < -d) {
                    rec->unk_14C = -d;
                }
            }
        }
    }
    if (rec->unk_46 == 0 && rec->unk_6A == 9 && rec->unk_A9 == rec->unk_40) {
        func_8001F860((s16 *)rec, rec->unk_1D8);
    }
    if (rec->unk_7A != 0 && rec->unk_6C != 0x18 && rec->unk_6A == 0x18) {
        rec->unk_104.vx += (Judge[rec->unk_1C8.vy & 0xFFF] * (&D_8008DA94)[rec->unk_0A]) >> 12;
        rec->unk_104.vz += (Judge[(rec->unk_1C8.vy + 0x400) & 0xFFF] * (&D_8008DA94)[rec->unk_0A]) >> 12;
        rec->unk_104.vy += (&D_8008DA50)[rec->unk_0A];
        rec->unk_74 = rec->unk_B8.vy;
    }
    if (rec->unk_7A != 0 && rec->unk_6C != 0x17 && rec->unk_6A == 0x17) {
        rec->unk_104.vy += (&D_8008DAD8)[rec->unk_0A];
        rec->unk_74 = rec->unk_B8.vy;
    }
    if (rec->unk_7A != 0 && rec->unk_6A == 0x28) {
        s32 dv[3];
        s32 tgt[3];

        d = (rec->unk_1D8 - rec->unk_1C8.vy) & 0xFFF;
        if (d >= 0x800) {
            d = 0x1000 - d;
        }
        tgt[0] = rec->unk_00->unk_18C.x - ((Judge[rec->unk_1D8 & 0xFFF] * d) >> 14);
        tgt[1] = rec->unk_00->unk_18C.y;
        tgt[2] = rec->unk_00->unk_18C.z - ((Judge[(rec->unk_1D8 + 0x400) & 0xFFF] * d) >> 14);
        rec->unk_104.vy -= 0xFA;
        func_800200DC((s32 *)&rec->unk_180, tgt, rec->unk_104.vy, 0x1F, dv);
        rec->unk_104.vx += dv[0];
        rec->unk_104.vz += dv[2];
        rec->unk_72 = 1;
        rec->unk_74 = rec->unk_B8.vy;
    }
    if (rec->unk_6A == 0x17 || rec->unk_6A == 0x18 || rec->unk_6A == 0x28 || rec->unk_6A == 0xA) {
        if (rec->unk_40 == rec->unk_50->unk7 && rec->unk_104.vy > 0) {
            rec->unk_40 = rec->unk_50->unk7 - 1;
        }
    }
    if (rec->unk_7A != 0 && rec->unk_6A == 0x23) {
        rec->unk_B8.vy -= 0x898;
        rec->unk_D8.y -= 0x898;
        rec->unk_1F8.y += 0x898;
    }
    state = rec->unk_6A;
    if (rec->unk_7A != 0 && state == 6) {
        rec->unk_86 = rec->unk_84;
    }
    d = (rec->unk_14C * rec->unk_44) / 24576;
    rec->unk_1C8.vy += d;
    rec->unk_14C -= d;
    if (rec->unk_7A != 0) {
        if (rec->unk_78 != 0) {
            func_80023D08((s32)rec);
        } else if (rec->unk_6A == 0x1C) {
            func_80023CB4((s16 *)rec, 0x400);
        } else {
            if (rec->unk_6C == 0x1C || rec->unk_6A == 0x11 || rec->unk_6C == 0x11 || rec->unk_6A == 0x15 || rec->unk_6A == 0xA || rec->unk_6A == 0x2D) {
                func_80023D08((s32)rec);
            } else {
                switch ((rec->unk_6C << 8) | rec->unk_6A) {
                case 0x1525:
                case 0x1502:
                case 0x1322:
                case 0x1519:
                case 0x3022:
                case 0x1913:
                case 0x3122:
                    func_80023D08((s32)rec);
                    break;
                }
            }
        }
    }
    nf = rec->unk_40 + 1;
    if (nf >= rec->unk_58[1]) {
        nf = rec->unk_58[1] - 1;
    }
    if (rec->unk_50->unk9 & 0x40) {
        obj = rec->unk_5E == 0 ? ((R23 *)g_practice_menu_table)[D_800A38AE].unk_4A + 1 : 0;
        if (rec->unk_6A == 0x11 && D_800A38AE != arg0) {
            add = rec->unk_58[1];
        } else {
            add = 0;
        }
    } else {
        obj = rec->unk_5E == 0 ? rec->unk_4A + 1 : 0;
        if (rec->unk_6A == 0x15 && D_8008D9EC[rec->unk_0A] != 0) {
            add = rec->unk_58[1];
        } else {
            add = 0;
        }
    }
    base = add + *rec->unk_54;
    frame = base + rec->unk_40;
    nfr = base + nf;
    rec->unk_64 = (obj << 14) | frame;
    rec->unk_66 = (obj << 14) | nfr;
    if (rec->unk_6A == 0x33) {
        pose[1] = ((Pose23 **)&D_800A3888)[arg0][rec->unk_40];
        pose[0] = pose[1];
    } else {
        func_800198D0(obj, frame, (u32 *)&pose[0], (u16 *)0x1F8001B0);
        func_800198D0(obj, nfr, (u32 *)&pose[1], (u16 *)0x1F8001B0);
    }
    if (rec->unk_7A != 0 && (rec->unk_6A == 6 || rec->unk_6A == 0x14) && rec->unk_6C == 6) {
        MATRIX m1;
        MATRIX m2;
        s32 r;

        math_RotMatrixZYXAngles(rec->unk_290.unk6, rec->unk_290.unk8, rec->unk_290.unkA, m1.m[0]);
        math_RotMatrixZYXAngles(pose[0].unk6, pose[0].unk8, pose[0].unkA, m2.m[0]);
        r = ratan2(m1.m[0][0], m1.m[2][0]);
        d = ratan2(m2.m[0][0], m2.m[2][0]) - r;
        rec->unk_1C8.vy += d;
        func_8001B690(arg0, d);
    }
    func_8001F2E4((u8 *)rec, (u8 *)&pose[0], (u8 *)&pose[1]);
    if (rec->unk_6A == 0x11) {
        rec->unk_154 = rec->unk_1C8.vy;
    }
    if (rec->unk_152 != 0) {
        a = rec->unk_154;
    } else {
        a = rec->unk_1C8.vy;
    }
    ang[0] = ang[1] = a;
    a += 0x400;
    v[0].x = (pose[0].unk4 * Judge[(pose[0].unk2 - a + 0x400) & 0xFFF]) >> 12;
    v[0].y = -pose[0].unk0;
    v[0].z = (pose[0].unk4 * Judge[(pose[0].unk2 - a) & 0xFFF]) >> 12;
    v[1].x = (pose[1].unk4 * Judge[(pose[1].unk2 - a + 0x400) & 0xFFF]) >> 12;
    v[1].y = -pose[1].unk0;
    v[1].z = (pose[1].unk4 * Judge[(pose[1].unk2 - a) & 0xFFF]) >> 12;
    v[0].x = (v[0].x * rec->unk_1A) >> 12;
    v[0].z = (v[0].z * rec->unk_1A) >> 12;
    v[1].x = (v[1].x * rec->unk_1A) >> 12;
    v[1].z = (v[1].z * rec->unk_1A) >> 12;
    if (rec->unk_6A != 8) {
        v[0].y = (v[0].y * rec->unk_1A) >> 12;
        v[1].y = (v[1].y * rec->unk_1A) >> 12;
    }
    if (rec->unk_31A != 0) {
        pose[1] = pose[0];
        ang[1] = ang[0];
        pose[0] = rec->unk_290;
        ang[0] = rec->unk_314;
        t = rec->unk_318;
        rec->unk_66 = rec->unk_64;
        rec->unk_64 = rec->unk_316;
    } else {
        t = rec->unk_42;
    }
    u = 0x1000 - t;
    rec->unk_68 = t;
    if (rec->unk_31A != 0) {
        Vec3i32 vc;

        if (rec->unk_7A != 0) {
            rec->unk_D8.x += rec->unk_1F8.x - v[0].x;
            rec->unk_D8.z += rec->unk_1F8.z - v[0].z;
            rec->unk_320.x += rec->unk_1F8.x - v[0].x;
            rec->unk_320.z += rec->unk_1F8.z - v[0].z;
        }
        b = ang[0] + 0x400;
        vc.x = (pose[0].unk4 * Judge[(pose[0].unk2 - b + 0x400) & 0xFFF]) >> 12;
        vc.y = -pose[0].unk0;
        vc.z = (pose[0].unk4 * Judge[(pose[0].unk2 - b) & 0xFFF]) >> 12;
        vc.x = (vc.x * rec->unk_1A) >> 12;
        vc.y = (vc.y * rec->unk_1A) >> 12;
        vc.z = (vc.z * rec->unk_1A) >> 12;
        vc.x -= rec->unk_320.x;
        vc.z -= rec->unk_320.z;
        rec->unk_E8.x = (vc.x * u + v[0].x * t) >> 12;
        rec->unk_E8.y = (vc.y * u + v[0].y * t) >> 12;
        rec->unk_E8.z = (vc.z * u + v[0].z * t) >> 12;
    } else {
        rec->unk_E8.x = (v[0].x * u + v[1].x * t) >> 12;
        rec->unk_E8.y = (v[0].y * u + v[1].y * t) >> 12;
        rec->unk_E8.z = (v[0].z * u + v[1].z * t) >> 12;
    }
    if (rec->unk_7A != 0 && rec->unk_31A == 0) {
        rec->unk_D8.x += rec->unk_1F8.x - rec->unk_E8.x;
        rec->unk_D8.z += rec->unk_1F8.z - rec->unk_E8.z;
        if (rec->unk_6A == 0x23 || rec->unk_6A == 0xA) {
            rec->unk_D8.y += rec->unk_1F8.y - rec->unk_E8.y;
        }
    }
    func_80023648((u8 *)rec);
    func_800238C4((u8 *)rec);
    if (rec->unk_6A != 8 && rec->unk_6A != 0x22) {
        rec->unk_104.vy += 0x15;
    }
    state = rec->unk_6A;
    if (state == 8 || state == 0x22) {
        rec->unk_134.vx -= rec->unk_98.vx / 256;
        rec->unk_134.vz -= rec->unk_98.vz / 256;
    }
    if (rec->unk_7A == 0 || rec->unk_6A != 0x28) {
        rec->unk_104.vx = (rec->unk_104.vx * rec->unk_156) >> 12;
        rec->unk_104.vy = (rec->unk_104.vy * rec->unk_158) >> 12;
        rec->unk_104.vz = (rec->unk_104.vz * rec->unk_15A) >> 12;
    }
    if (rec->unk_104.vx >= -15 && rec->unk_104.vx <= 15) {
        rec->unk_104.vx /= 2;
    }
    if (rec->unk_104.vy >= -15 && rec->unk_104.vy <= 15) {
        rec->unk_104.vy /= 2;
    }
    if (rec->unk_104.vz >= -15 && rec->unk_104.vz <= 15) {
        rec->unk_104.vz /= 2;
    }
    rec->unk_D8.x += rec->unk_104.vx;
    rec->unk_D8.y += rec->unk_104.vy;
    rec->unk_D8.z += rec->unk_104.vz;
    if ((rec->unk_6A == 7 || rec->unk_6A == 0xD) && rec->unk_B4 == 0) {
        d = (rec->unk_24.held & 0x1000) ? 1 : -((rec->unk_24.held & 0x4000) != 0);
        if (d != 0) {
            rec->unk_134.vx /= 2;
            rec->unk_134.vz /= 2;
            rec->unk_134.vx += (Judge[rec->unk_1D8 & 0xFFF] - Judge[(rec->unk_1D8 + 0x400) & 0xFFF] * d * 3) / 128;
            rec->unk_134.vz += (Judge[(rec->unk_1D8 + 0x400) & 0xFFF] + Judge[rec->unk_1D8 & 0xFFF] * d * 3) / 128;
        }
    }
    rec->unk_134.vx = (rec->unk_134.vx * rec->unk_15E) >> 12;
    if (rec->unk_134.vx >= -3 && rec->unk_134.vx <= 3) {
        rec->unk_134.vx = 0;
    }
    rec->unk_134.vy = (rec->unk_134.vy * rec->unk_160) >> 12;
    if (rec->unk_134.vy >= -3 && rec->unk_134.vy <= 3) {
        rec->unk_134.vy = 0;
    }
    rec->unk_134.vz = (rec->unk_134.vz * rec->unk_162) >> 12;
    if (rec->unk_134.vz >= -3 && rec->unk_134.vz <= 3) {
        rec->unk_134.vz = 0;
    }
    if (rec->unk_6A == 4 || rec->unk_6A == 0x14 || rec->unk_0C == 0x1F) {
        rec->unk_134.vx = 0;
        rec->unk_134.vy = 0;
        rec->unk_134.vz = 0;
    } else {
        rec->unk_D8.x += rec->unk_134.vx;
        rec->unk_D8.y += rec->unk_134.vy;
        rec->unk_D8.z += rec->unk_134.vz;
    }
    pos[0] = rec->unk_D8.x + rec->unk_E8.x;
    pos[1] = rec->unk_D8.y;
    if (rec->unk_6A == 8 || rec->unk_6A == 0x22) {
        pos[1] += rec->unk_E8.y;
    }
    pos[2] = rec->unk_D8.z + rec->unk_E8.z;
    rec->unk_C8 = rec->unk_B8;
    func_8002304C((u8 *)rec, (s32 *)&rec->unk_B8, pos, (s32 *)&rec->unk_104);
    if (rec->unk_B1 != 7 && rec->unk_B1 != 0) {
        rec->unk_B2 = ((0x2A >> rec->unk_B1) ^ 1) & 1;
    }
    rec->unk_D8.x = rec->unk_B8.vx - rec->unk_E8.x;
    rec->unk_D8.y = rec->unk_B8.vy;
    if (rec->unk_6A == 8 || rec->unk_6A == 0x22) {
        rec->unk_D8.y -= rec->unk_E8.y;
    }
    rec->unk_D8.z = rec->unk_B8.vz - rec->unk_E8.z;
    rec->unk_F4.x = rec->unk_D8.x + rec->unk_E8.x;
    rec->unk_F4.y = rec->unk_D8.y + rec->unk_E8.y;
    rec->unk_F4.z = rec->unk_D8.z + rec->unk_E8.z;
    rec->unk_1F8 = rec->unk_E8;
    rec->unk_168.x = rec->unk_F4.x;
    rec->unk_168.y = rec->unk_D8.y - 0x384;
    rec->unk_168.z = rec->unk_F4.z;
    func_80023E40((u8 *)rec);
    func_80041188(arg0, (u8 *)&pose[0], (u8 *)&pose[1], rec->unk_68, (s32 *)0x1F8001B0);
    scratchpad_Save();
    func_80040D48(arg0, 1, (s32 *)&rec->unk_F4, (s16 *)&rec->unk_1C8, 0, rec->unk_148);
    scratchpad_Restore();
    if (rec->unk_86 == rec->unk_8E && (rec->unk_6A == 0x15 || rec->unk_6A == 0x19 || rec->unk_6A == 0x1A || rec->unk_6A == 0x16 || rec->unk_6A == 0x30 || rec->unk_6A == 0x13 || rec->unk_6A == 0x31)) {
        rec->unk_92 = 0;
    } else {
        rec->unk_92 = rec->unk_92 != 0 ? 1 : 2;
    }
    rec->unk_62 = 0;
    if (rec->unk_0C != 0x1F && rec->unk_96 == 0 && rec->unk_92 != 0) {
        rec->unk_62 = 1;
    }
    if ((D_800A38DC != 3 || arg0 != 1 || D_800A384C == 4) && (u16)rec->unk_0E < 2 && (D_800A38DC != 2 || D_800A389A != 0) && D_800A38DC != 5) {
        rec->unk_62 |= 2;
    }
    if (rec->unk_86 == rec->unk_88 && rec->unk_8A != 0) {
        if (func_8002798C((u8 *)rec) == 0) {
            goto clear_8c;
        }
        goto set_8c;
    }
    if (((rec->unk_0C == 0x1D || rec->unk_0C == 0xE) && (rec->unk_6A == 2 || rec->unk_6A == 0x1B || rec->unk_6A == 0x28 || rec->unk_6A == 0x26) && rec->unk_A1[1] < 0xFF && rec->unk_26C != 0 && rec->unk_40 >= rec->unk_A5 && rec->unk_40 <= rec->unk_A6)
        || (rec->unk_0A == 0xE && rec->unk_6A == 0x11 && D_800A38AE == arg0 && rec->unk_40 >= rec->unk_A5 && rec->unk_A6 >= rec->unk_40)) {
    set_8c:
        rec->unk_8C = rec->unk_8C != 0 ? 1 : 2;
        rec->unk_62 |= 4;
    } else {
    clear_8c:
        rec->unk_8C = 0;
    }
    if (D_800A38DC != 3 || arg0 != 1 || D_800A384C == 4) {
        if (D_800A38DC == 3 && arg0 == 1 && D_800A384C == 4) {
            if (rec->unk_00->unk_0A == 1 || rec->unk_00->unk_0A == 3 || rec->unk_00->unk_0A == 4 || rec->unk_00->unk_0A == 9 || rec->unk_00->unk_0A == 0x11) {
                rec->unk_62 |= 8;
            }
            if (rec->unk_6A != 4 && rec->unk_6A != 0x14 && rec->unk_92 == 0) {
                rec->unk_62 |= 0x10;
            }
            if (rec->unk_330 > 0 && rec->unk_332[0] == rec->unk_14 && rec->unk_8C == 0) {
                rec->unk_62 |= 0x20;
            }
        } else if ((D_800A38DC != 2 || D_800A389A != 0) && D_800A38DC != 5) {
            if (rec->unk_0A == 1 || rec->unk_0A == 3 || rec->unk_0A == 4 || rec->unk_0A == 9 || rec->unk_0A == 0x11) {
                rec->unk_62 |= 8;
            }
            if (rec->unk_6A != 4 && rec->unk_6A != 0x14 && rec->unk_92 == 0) {
                rec->unk_62 |= 0x10;
            }
            if (rec->unk_330 > 0 && rec->unk_332[0] == rec->unk_14 && rec->unk_8C == 0) {
                rec->unk_62 |= 0x20;
            }
        }
    }
    if ((D_800A38DC == 2 && D_800A389A == 0) || D_800A38DC == 5) {
        if ((u16)rec->unk_0E < 2 && rec->unk_6A != 4 && rec->unk_6A != 0x14 && rec->unk_92 == 0) {
            rec->unk_62 = (rec->unk_62 | 2) & ~0x10;
        }
        if (rec->unk_330 > 0 && rec->unk_332[0] == rec->unk_14 && rec->unk_8C == 0) {
            rec->unk_62 = (rec->unk_62 | 8) & ~0x20;
            func_80049A2C(D_8008EB80[rec->unk_14], (arg0 * 2) | 1, 0);
        }
    }
    if (rec->unk_62 & 1) {
        func_80049718(rec->unk_12, arg0 * 2 + 0x8000, 0, 0);
    }
    if (rec->unk_62 & 4) {
        func_80049718(D_8008EB80[rec->unk_14], arg0 * 2 + 0x8001, 0, 0);
    }
    if (rec->unk_62 & 2) {
        func_80049A2C(rec->unk_12, arg0 * 2, (rec->unk_62 >> 4) & 1);
    }
    if (rec->unk_62 & 8) {
        func_80049A2C(D_8008EB80[rec->unk_14], (arg0 * 2) | 1, (rec->unk_62 >> 5) & 1);
    }
    rec->unk_290 = pose[0];
    rec->unk_314 = ang[0];
    rec->unk_316 = rec->unk_64;
    if (rec->unk_31A != 0) {
        if (rec->unk_31A >= 2) {
            rec->unk_31A = 1;
        }
        rec->unk_318 += rec->unk_31C;
        if (rec->unk_318 >= 0x1000) {
            rec->unk_31A = 0;
            rec->unk_290 = pose[1];
            rec->unk_314 = ang[1];
            rec->unk_316 = rec->unk_66;
            rec->unk_1F8 = v[1];
        }
    }
    rec->unk_1D8 = ratan2(rec->unk_00->unk_F4.x - rec->unk_F4.x, rec->unk_00->unk_F4.z - rec->unk_F4.z);
    rec->unk_24C = rec->unk_104;
    func_80023D28((u8 *)rec);
    func_800207C8((u8 *)rec, SPAD23->unkA8[arg0], SPAD23->unk00[arg0], SPAD23->unk48[arg0]);
    if (rec->unk_3C == 1) {
        rec->unk_210[0] = SPAD23->unk00[arg0][0];
    }
    if (rec->unk_8C == 2) {
        rec->unk_8C = 1;
        rec->unk_234[0] = SPAD23->unk48[arg0][0];
        rec->unk_234[1] = SPAD23->unk48[arg0][1];
    }
    if (rec->unk_92 == 2) {
        rec->unk_92 = 1;
        rec->unk_234[0] = SPAD23->unk48[arg0][0];
        rec->unk_234[1] = SPAD23->unk48[arg0][1];
    }
    if ((rec->unk_0E == 6 || rec->unk_0E == 7) && (rec->unk_62 & 1)) {
        rec->unk_25C = SPAD23->unk00[arg0][0];
        rec->unk_268 = 1;
    } else {
        rec->unk_268 = 0;
    }
    rec->unk_114[0].vx = SPAD23->unk00[arg0][0].x - rec->unk_210[0].x;
    rec->unk_114[0].vy = SPAD23->unk00[arg0][0].y - rec->unk_210[0].y;
    rec->unk_114[0].vz = SPAD23->unk00[arg0][0].z - rec->unk_210[0].z;
    rec->unk_114[1].vx = SPAD23->unk48[arg0][0].x - rec->unk_234[0].x;
    rec->unk_114[1].vy = SPAD23->unk48[arg0][0].y - rec->unk_234[0].y;
    rec->unk_114[1].vz = SPAD23->unk48[arg0][0].z - rec->unk_234[0].z;
    rec->unk_1DA = func_8002FDB0((s32 *)rec);
    f0 = 0;
    if ((rec->unk_6A == 2 || rec->unk_6A == 0x1B || rec->unk_6A == 0x28 || rec->unk_6A == 0x26) && rec->unk_AD != 0) {
        if ((rec->unk_40 >= rec->unk_A1[0] && rec->unk_40 <= rec->unk_A3[0]) || (rec->unk_40 >= rec->unk_A1[1] && rec->unk_40 <= rec->unk_A3[1])) {
            f0 = 1;
        }
    }
    rec->unk_AE = f0;
    rec->unk_288[0] = 0;
    rec->unk_288[1] = 0;
    if (rec->unk_AD != 0) {
        if (rec->unk_6A == 2 || rec->unk_6A == 0x1B || rec->unk_6A == 0x28 || rec->unk_6A == 0x26) {
            rec->unk_288[0] += 1;
            rec->unk_288[1] += 1;
            if (rec->unk_40 < rec->unk_A1[0]) {
                rec->unk_288[0] += 2;
            } else if (rec->unk_40 <= rec->unk_A3[0]) {
                rec->unk_288[0] += 4;
            }
            if (rec->unk_40 < rec->unk_A1[1]) {
                rec->unk_288[1] += 2;
            } else if (rec->unk_40 <= rec->unk_A3[1]) {
                rec->unk_288[1] += 4;
            }
        }
    }
    if (rec->unk_6A == 6) {
        rec->unk_15E = 0x600;
        rec->unk_160 = 0x600;
        rec->unk_162 = 0x600;
    } else if (rec->unk_6A == 8) {
        rec->unk_15E = 0x100;
        rec->unk_160 = 0x100;
        rec->unk_162 = 0x100;
    } else {
        rec->unk_15E = 0xC00;
        rec->unk_160 = 0xC00;
        rec->unk_162 = 0xC00;
    }
    if (rec->unk_1DC != 0) {
        rec->unk_156 = 0xD00;
        rec->unk_158 = 0xFC0;
        rec->unk_15A = 0xD00;
    } else {
        rec->unk_156 = 0xFC0;
        rec->unk_158 = 0xFC0;
        rec->unk_15A = 0xFC0;
    }
    if (rec->unk_96 == 0
        && ((rec->unk_7A != 0 && rec->unk_6C != 4 && rec->unk_6C != 0x14 && (rec->unk_6A == 4 || rec->unk_6A == 0x14))
            || (rec->unk_6A == 0x11 && arg0 != D_800A38AE && rec->unk_AA == rec->unk_40))) {
        MATRIX **pd = (MATRIX **)game_GetPlayerData(arg0);
        if (D_800A38DC != 0 || D_8008D9EC[g_practice_menu_table[0].unk_0A] == 0 || D_800A37A0 != 1 || arg0 != D_800A37A0) {
            func_80032854(arg0, 0x2E, (s32 *)&rec->unk_F4, 0);
        }
        if (rec->unk_0C != 0x1F) {
            cpu_set_move_command_and_dir((PracticeMenuRec *)rec, D_8008D90C[D_8008D9EC[rec->unk_0A]][rec->unk_0E], (Vec3i32 *)pd[0x12]->t);
        }
        rec->unk_96 = 1;
        if (D_800A3748 == -1) {
            D_800A3748 = arg0 == 0;
        }
    }
    if (rec->unk_7A != 0 && rec->unk_0C == 0x1B && rec->unk_6A == 0xB && rec->unk_34D == 0) {
        rec->unk_286 = 0x1F;
    }
    if (rec->unk_46 == 0) {
        if (D_800A38DC != 5 && D_800A38DC != 2 && D_800A38DC != 3 && (D_800A38DC != 0 || D_800A385C == 0)) {
            if (rec->unk_6A == 0xB && rec->unk_40 == rec->unk_A7) {
                if (rec->unk_0C == 0x1B) {
                    if (rec->unk_34D != 0) {
                        rec->unk_34D--;
                    }
                } else if (rec->unk_26C != 0 && func_800307D0((PracticeMenuRec *)rec) == rec->unk_14) {
                    rec->unk_8A = 0;
                }
            }
        }
        if (rec->unk_6A == 6) {
            if (func_80030BA8((PracticeMenuRec *)rec) == rec->unk_14) {
                rec->unk_8A = rec->unk_26C;
            }
        } else if (rec->unk_26C != 0 && (rec->unk_6A == 0xC || rec->unk_6A == 0x2A) && rec->unk_40 == rec->unk_A8 && func_80030BA8((PracticeMenuRec *)rec) == rec->unk_14) {
            rec->unk_8A = 1;
        }
        if (rec->unk_6A == 0x12 && rec->unk_40 == rec->unk_A7 && ((0x78 >> rec->unk_B1) & 1) && rec->unk_26C != 0) {
            func_80032064((u8 *)rec, ((0x18 >> rec->unk_B1) & 1) ? 1 : 2);
        }
        if (rec->unk_6A == 0x10 && rec->unk_40 == rec->unk_AC && rec->unk_26C != 0 && rec->unk_34B != 0) {
            rec->unk_34A = 0xA;
            rec->unk_34B--;
            func_80032854(arg0, 0x31, (s32 *)&rec->unk_F4, 0);
        }
        if (rec->unk_46 == 0 && (rec->unk_6A != 0x11 || arg0 == D_800A38AE)) {
            rec->unk_62 |= 0x40;
            cpu_check_same_dir_timer((s32 *)rec);
        }
    }
    if (rec->unk_26C != 0) {
        rec->unk_62 |= 0x80;
    }
    if (rec->unk_96 != 0) {
        if (D_800A3834 != 0xD) {
            rec->unk_B3 = 4;
        }
    } else {
        rec->unk_B3 = 0;
    }
    if (rec->unk_0C == 0x1F) {
        rec->unk_B3 = 4;
    }
    func_80040304(arg0, rec->unk_B3);
    func_800204C0((u8 *)rec);
    if (rec->unk_7A == 2) {
        rec->unk_7A = 0;
    }
    func_80039680((u8 *)rec);
}
