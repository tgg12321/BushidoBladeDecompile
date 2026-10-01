/* BEGIN func_80055B60 */
typedef struct {
    u8 b[4];
} Bytes800A3258;
typedef struct {
    u8 unk0[2];
    s16 unk2;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7[0x2C - 0x7];
    Vec3i32 unk2C;
    Vec3i32 unk38;
    u8 unk44[0x64 - 0x44];
} Obj80106A78;
extern Obj80106A78 D_80106A78[12];
extern Bytes800A3258 D_800A3258;
extern u8 D_8009A088[];
extern s32 ratan2(s32, s32);
extern s32 SquareRoot0(s32);
extern s32 func_80058580(PracticeMenuRec *);
extern void func_80056CB8(s32);
extern s32 func_80056FE8(s32);
void func_80055B60(s32 arg0, PadState *arg1) {
    PracticeMenuRec *rec;
    PracticeMenuRec *me;
    PracticeMenuRec *opp;
    PadState pad;
    Bytes800A3258 bits;
    s32 b3, b4, b7, b8, b9, b10, b11;
    s32 lo, hi;
    s32 flags;
    s32 ang;
    s32 diff;
    s32 temp;
    s32 temp2;
    s32 far;
    s32 i;

    rec = &g_practice_menu_table[arg0];
    rec->unk_3CC = 0;
    if (rec->unk_3E8 & 1) {
        opp = rec->unk_00;
        me = rec;
    } else {
        me = rec->unk_00;
        opp = rec;
    }
    me->unk_441 = me->unk_58[2] & 0xF;
    me->unk_43A = (ratan2(opp->unk_F4.x - me->unk_F4.x, opp->unk_F4.z - me->unk_F4.z) - me->unk_1C8.vy) & 0xFFF;
    if (me->unk_43A > 0x800) {
        me->unk_43A -= 0x1000;
    }
    me->unk_43C = me->unk_43A < 0 ? -me->unk_43A : me->unk_43A;
    if ((u16)me->unk_6A == 0x15) {
        me->unk_440 = me->unk_441;
    }

    flags = (rec->unk_430 & 0x40060) | ((rand() & 0xFFF) < ((s16)rec->unk_438 >> 3) && rec->unk_3E8 >= 0x3D);
    b3 = ((rand() & 0xFFF) < (s16)rec->unk_438 || rec->unk_3E8 < ((s16)rec->unk_438 >> 3)) << 3;
    b4 = ((rand() & 0xFFF) < ((s16)rec->unk_438 >> 1) || rec->unk_3E8 < ((s16)rec->unk_438 >> 4)) << 4;
    b7 = ((u16)rec->unk_00->unk_6A == 2 || (u16)rec->unk_00->unk_6A == 0x1B ||
          (u16)rec->unk_00->unk_6A == 0x28 || (u16)rec->unk_00->unk_6A == 0x26 ||
          ((u16)rec->unk_6A == 0x11 && rec->unk_50[8] != rec->unk_58[1] - 1)) << 7;
    b8 = ((u16)rec->unk_6A == 0x13 || (u16)rec->unk_6A == 0x1B || (u16)rec->unk_6A == 0x30) << 8;
    b9 = ((u16)rec->unk_00->unk_6A == 0x13 || (u16)rec->unk_00->unk_6A == 0x1B ||
          (u16)rec->unk_00->unk_6A == 0x30) << 9;
    b10 = ((u16)rec->unk_6A == 6 || (u16)rec->unk_6A == 4 || (u16)rec->unk_6A == 0x14) << 10;
    b11 = ((u16)rec->unk_00->unk_6A == 6 || (u16)rec->unk_00->unk_6A == 4 ||
           (u16)rec->unk_00->unk_6A == 0x14) << 11;
    rec->unk_430 = flags |
                   ((D_80099D88[rec->unk_443].flags & 0xFF00)
                        ? (rec->unk_3F6 < rec->unk_3F5) << 2
                        : ((rand() & 0xFFF) < ((s16)rec->unk_438 >> 3) && rec->unk_3E8 >= 0x3D) << 2) |
                   b3 | b4 | b7 | b8 | b9 | b10 | b11 |
                   ((u16)rec->unk_6A == 0x15 ? 0x1000 : 0) |
                   ((u16)rec->unk_00->unk_6A == 0x15 ? 0x2000 : 0) |
                   ((u16)rec->unk_6A == 0x19 ? 0x4000 : 0) |
                   ((u16)rec->unk_00->unk_6A == 0x19 ? 0x8000 : 0) |
                   ((u16)rec->unk_6A == 0x1A ? 0x10000 : 0);
    rec->unk_3E8++;

    if ((u16)rec->unk_6A != 2 && (u16)rec->unk_6A != 0x1B && (u16)rec->unk_6A != 0x28 &&
        (u16)rec->unk_6A != 0x26 && (u16)rec->unk_6A != 0x2C && (u16)rec->unk_6A != 3 &&
        (u16)rec->unk_6A != 7) {
        if (rec->unk_3F5 != 0xFF) {
            rec->unk_3F5++;
        }
    } else {
        rec->unk_3F5 = 0;
    }

    if (((D_8009A088[rec->unk_00->unk_0E] >> rec->unk_00->unk_440) & 1) &&
        rec->unk_00->unk_43C < ((s16)rec->unk_438 >> 4) && rec->unk_0E < 6 &&
        (rec->unk_430 & 0x2000) && !(D_80099D88[rec->unk_443].flags & 0xBF00)) {
        rec->unk_430 |= 0x20000;
    } else {
        rec->unk_430 &= ~0x20000;
    }

    rec->unk_43E = rec->unk_3F8[rec->unk_86] +
                   (((rec->unk_404[rec->unk_86] - rec->unk_3F8[rec->unk_86]) * D_80099D88[rec->unk_443].unk5) >> 8);
    temp2 = (D_80099D88[rec->unk_443].unk7 * 25u) >> 3;
    diff = D_800A387C - rec->unk_43E;
    if (temp2 < (diff >= 0 ? diff : -diff)) {
        diff >>= 31;
        if (diff != (rec->unk_3F0 >> 15)) {
            rec->unk_3F0 = 0;
        }
        rec->unk_3F0 += diff ? -1 : 1;
    } else {
        rec->unk_3F0 = 0;
    }

    temp = rec->unk_00->unk_148 - rec->unk_148;
    if (temp < -1000) {
        rec->unk_442 = 1;
    } else if (temp > 1000) {
        rec->unk_442 = 2;
    } else {
        rec->unk_442 = 0;
    }
    if (rec->unk_430 & 0x15500) {
        func_80056CB8((s32)rec);
    }
    if ((rec->unk_40 == 0 && ((u16)rec->unk_6A == 3 || (u16)rec->unk_6A == 0x2C)) ||
        (rec->unk_00->unk_40 == 0 && ((u16)rec->unk_00->unk_6A == 0xD || (u16)rec->unk_00->unk_6A == 0x2C))) {
        if (rec->unk_3F4 != 0xFF) {
            rec->unk_3F4++;
        }
    }

    if (rec->unk_430 & 0x80) {
        lo = rec->unk_00->unk_A1 != 0xFF ? rec->unk_00->unk_A1 : rec->unk_00->unk_A2;
        hi = rec->unk_00->unk_A3 != 0xFF ? rec->unk_00->unk_A3 : rec->unk_00->unk_A4;
    }
    if (!(rec->unk_430 & 0x80) ||
        ((u16)rec->unk_6A != 0x11 ? (hi < rec->unk_00->unk_40 || lo - rec->unk_00->unk_40 >= 9)
                                  : rec->unk_50[8] < rec->unk_40)) {
        if (rec->unk_428 != -1) {
            if (rec->unk_424 != 0 && (file_GetFlag1() == 0 || D_800A38DC == 3)) {
                s32 slot;
                s32 found;

                far = 0x100;
                i = 0;
                slot = -1;
                found = -1;

                for (; i < 8; i++) {
                    if (rec->unk_414[i][0] == rec->unk_428) {
                        found = i;
                    } else {
                        temp = rec->unk_414[i][1] - 1;
                        if (rec->unk_414[i][1] < far) {
                            slot = i;
                            far = rec->unk_414[i][1];
                        }
                        if (temp < 0) {
                            temp = 0;
                        }
                        rec->unk_414[i][1] = temp;
                    }
                }
                temp = 4;
                if ((u16)rec->unk_6A == 0x11) {
                    temp = 8;
                }
                if (found == -1) {
                    rec->unk_414[slot][0] = rec->unk_428;
                    rec->unk_414[slot][1] = temp;
                } else {
                    temp += rec->unk_414[found][1];
                    if (temp > 0xFF) {
                        temp = 0xFF;
                    }
                    rec->unk_414[found][1] = temp;
                }
            }
            rec->unk_428 = -1;
            rec->unk_426 = 0;
            rec->unk_430 &= ~0x60;
        }
    } else {
        if (rec->unk_428 != rec->unk_00->unk_5C) {
            if ((u16)rec->unk_6A == 0x11) {
                rec->unk_428 = 0xFF;
            } else {
                far = (rec->unk_00->unk_6C != 0xE && rec->unk_00->unk_6C != 0x2C) ? rec->unk_00->unk_5C : 0xFE;
                rec->unk_428 = far;
                rec->unk_427 = lo;
                temp2 = func_80056FE8((s32)rec);
                far = temp2 + 800;
                if (rec->unk_442 != 0 || ((rec->unk_430 & 0x100) && rec->unk_43C > 0x400)) {
                    if (temp2 / 2 >= D_800A387C) {
                        rec->unk_426 = 1;
                    } else if (temp2 >= D_800A387C) {
                        rec->unk_426 = 2;
                    } else {
                        rec->unk_426 = 3;
                    }
                } else {
                    if (temp2 >= D_800A387C) {
                        rec->unk_426 = 1;
                    } else if (far >= D_800A387C) {
                        rec->unk_426 = 2;
                    } else {
                        rec->unk_426 = 3;
                    }
                }
                rec->unk_42A = rec->unk_00->unk_F4.x;
                rec->unk_42C = rec->unk_00->unk_F4.z;
                rec->unk_42E = far;
            }
            for (i = 0; i < 8; i++) {
                if (rec->unk_414[i][0] == rec->unk_428 && rec->unk_414[i][1] != 0 &&
                    rec->unk_414[i][1] >= rec->unk_424) {
                    temp = 0x20;
                    if (rec->unk_414[i][1] >= rec->unk_424 * 4) {
                        temp = 0x60;
                    }
                    rec->unk_430 |= temp;
                    break;
                }
            }
        }
        if (rec->unk_430 & 0x20) {
            rec->unk_430 |= 0x18;
        }
    }

    if ((u16)rec->unk_00->unk_6A == 0x12 && rec->unk_00->unk_43C < 0x80 && D_800A387C < 0x1194) {
        rec->unk_425 = 1;
    } else {
        rec->unk_425 = 0;
        for (i = 0; i < 12; i++) {
            Obj80106A78 *obj = &D_80106A78[i];

            temp2 = SquareRoot0((rec->unk_F4.x - obj->unk2C.x) * (rec->unk_F4.x - obj->unk2C.x) +
                              (rec->unk_F4.z - obj->unk2C.z) * (rec->unk_F4.z - obj->unk2C.z));
            if (obj->unk2 != -1 && obj->unk4 != 0 && obj->unk6 != rec->unk_04) {
                temp = (ratan2(rec->unk_F4.x - obj->unk2C.x, rec->unk_F4.z - obj->unk2C.z) -
                      ratan2(obj->unk2C.x - obj->unk38.x, obj->unk2C.z - obj->unk38.z)) & 0xFFF;
                if (temp > 0x800) {
                    temp -= 0x1000;
                }
                if ((temp < 0 ? -temp : temp) < 0x80) {
                    if ((rec->unk_B8.vy - obj->unk2C.y >= 0) ? (rec->unk_B8.vy - obj->unk2C.y < 2000)
                                                              : (obj->unk2C.y - rec->unk_B8.vy < 2000)) {
                        rec->unk_425 = temp2 < 3000 ? 2 : 1;
                    }
                }
            }
        }
    }

    if ((rec->unk_430 & 1) && !(D_80099D88[rec->unk_443].flags & 0xFF00) &&
        (u8)(rec->unk_426 - 1) >= 2 && (u8)(rec->unk_425 - 1) >= 2 &&
        (((rec->unk_430 & 0x80) &&
          ((rec->unk_00->unk_7C == 0 && hi < rec->unk_00->unk_40) || lo - rec->unk_00->unk_40 >= 9)) ||
         (u16)rec->unk_00->unk_6A == 0x10 || (u16)rec->unk_00->unk_6A == 3 ||
         (u16)rec->unk_00->unk_6A == 7 || (u16)rec->unk_00->unk_6A == 0x2C ||
         (u16)rec->unk_00->unk_6A == 0x24 ||
         (!(D_80099D88[rec->unk_443].flags & 0x20) &&
          (((rec->unk_430 & 0x80) && rec->unk_426 == 3) ||
           ((u16)rec->unk_00->unk_6A == 0x2A && rec->unk_00->unk_26C == 0) ||
           ((u16)rec->unk_00->unk_6A == 0x12 &&
            (!((0x78 >> rec->unk_00->unk_B1) & 1) || rec->unk_00->unk_26C == 0)) ||
           ((u16)rec->unk_00->unk_6A == 0xB &&
            ((rec->unk_00->unk_330 == 0 && (u8)(rec->unk_425 - 1) >= 2) || rec->unk_00->unk_26C == 0)))) ||
         ((D_80099D88[rec->unk_443].flags & 0x20) &&
          ((u16)rec->unk_00->unk_6A == 0x25 || (u16)rec->unk_00->unk_6A == 9 ||
           ((u16)rec->unk_00->unk_6A >= 0x16 && (u16)rec->unk_00->unk_6A <= 0x17) ||
           (u16)rec->unk_00->unk_6A == 0xA ||
           ((u16)rec->unk_00->unk_6A == 0x22 && rec->unk_442 == 0) ||
           rec->unk_00->unk_43C > 0x400)))) {
        rec->unk_430 |= 2;
    } else {
        rec->unk_430 &= ~2;
    }

    if (D_80099D88[rec->unk_443].flags & 0x8000) {
        rec->unk_430 &= ~0x78;
        if ((u16)rec->unk_6A != 0x25 && (D_80102788.held & 0x100)) {
            rec->unk_430 |= 0x40000;
            rec->unk_3F2 = 0;
        }
    }
    if (rec->unk_0E >= 6) {
        rec->unk_430 |= 0x78;
    }

    i = 0;
    do {
        if (rec->unk_3B4 != 0) {
            temp = func_80055948((u8 *)rec);
        } else {
            temp = func_80058580(rec);
        }
        i++;
    } while (temp == -1 && i < 4);
    if (temp != -1) {
        pad.held = temp;
    } else {
        pad.held = 0;
    }
    if (pad.held & 0x660) {
        bits = D_800A3258;
        if ((u16)rec->unk_6A == 0x19) {
            pad.held |= 1 << bits.b[rec->unk_441];
        } else if ((u16)rec->unk_6A == 0x13) {
            pad.held |= 4;
        }
    }
    pad.held &= 0xFFFF;
    pad.pressed = pad.held & ~rec->unk_3D0.held;
    pad.unheld = ~pad.held & 0xFFFF;
    pad.released = ~pad.held & rec->unk_3D0.held;
    pad.unk_00[arg0] = 4;
    rec->unk_3D0 = pad;
    *arg1 = rec->unk_3D0;
}
/* END func_80055B60 */
