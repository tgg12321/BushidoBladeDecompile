void func_80055B60(s32 arg0, PadState *arg1) {
    PracticeMenuRec *rec;
    PracticeMenuRec *me;
    PracticeMenuRec *opp;
    PadState pad;
    PadBitTable bits;
    s32 lo, hi;
    /* work holds two values: D_800A387C - unk_43E, then its sign (work >> 31).
       One local, not two: ordinary-c-judge-decidable.md Ruling 11; (D) record in
       memory/grind/func_80055B60/evidence.md s3 (probes/r11/ there). */
    s32 work;
    /* temp holds six values: the unk_148 distance, a slot count minus one
       (clamped at 0), the slot increment (4 or 8, then plus the old count,
       clamped at 0xFF), the 0x20/0x60 flag, the wrapped ratan2() angle
       difference and the poll result of func_80055948/func_80058580.
       One local, not six: Ruling 11 (per-branch constants: Q20); (D) record in
       memory/grind/func_80055B60/evidence.md s3 (probes/r11/ there). */
    s32 temp;
    /* temp2 holds three values: the unk7 * 25 >> 3 limit, the func_80056FE8()
       result and the SquareRoot0() distance. One local, not three: Ruling 11;
       (D) record in memory/grind/func_80055B60/evidence.md s3 (probes/r11/ there). */
    s32 temp2;
    /* temp3 holds two values: the least slot count seen (starting at 0x100) and
       the func_80056FE8() result plus 800. One local, not two: Ruling 11; (D)
       record in memory/grind/func_80055B60/evidence.md s3 (probes/r11/ there). */
    s32 temp3;
    /* the counter of each of the four loops, reused loop to loop the way
       SOTN's AddToInventory reuses i for its two loops (Q51) */
    s32 i; /* SOTN: src/dra/5D5BC.c:173 @aa53500 */

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

    rec->unk_430 = (rec->unk_430 & 0x40060) | ((rand() & 0xFFF) < ((s16)rec->unk_438 >> 3) && rec->unk_3E8 >= 0x3D) |
                   ((D_80099D88[rec->unk_443].flags & 0xFF00)
                        ? (rec->unk_3F6 < rec->unk_3F5) << 2
                        : ((rand() & 0xFFF) < ((s16)rec->unk_438 >> 3) && rec->unk_3E8 >= 0x3D) << 2) |
                   (((rand() & 0xFFF) < (s16)rec->unk_438 || rec->unk_3E8 < ((s16)rec->unk_438 >> 3)) << 3) | (((rand() & 0xFFF) < ((s16)rec->unk_438 >> 1) || rec->unk_3E8 < ((s16)rec->unk_438 >> 4)) << 4) | (((u16)rec->unk_00->unk_6A == 2 || (u16)rec->unk_00->unk_6A == 0x1B || (u16)rec->unk_00->unk_6A == 0x28 || (u16)rec->unk_00->unk_6A == 0x26 || ((u16)rec->unk_6A == 0x11 && rec->unk_50[8] != rec->unk_58[1] - 1)) << 7) | (((u16)rec->unk_6A == 0x13 || (u16)rec->unk_6A == 0x1B || (u16)rec->unk_6A == 0x30) << 8) | (((u16)rec->unk_00->unk_6A == 0x13 || (u16)rec->unk_00->unk_6A == 0x1B || (u16)rec->unk_00->unk_6A == 0x30) << 9) | (((u16)rec->unk_6A == 6 || (u16)rec->unk_6A == 4 || (u16)rec->unk_6A == 0x14) << 10) | (((u16)rec->unk_00->unk_6A == 6 || (u16)rec->unk_00->unk_6A == 4 || (u16)rec->unk_00->unk_6A == 0x14) << 11) |
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
    work = D_800A387C - rec->unk_43E;
    if (temp2 < (work >= 0 ? work : -work)) {
        work >>= 31;
        if (work != (rec->unk_3F0 >> 15)) {
            rec->unk_3F0 = 0;
        }
        rec->unk_3F0 += work ? -1 : 1;
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

                temp3 = 0x100;
                i = 0;
                slot = -1;
                found = -1;

                for (; i < 8; i++) {
                    if (rec->unk_414[i][0] == rec->unk_428) {
                        found = i;
                    } else {
                        temp = rec->unk_414[i][1] - 1;
                        if (rec->unk_414[i][1] < temp3) {
                            slot = i;
                            temp3 = rec->unk_414[i][1];
                        }
                        if (temp < 0) {
                            temp = 0;
                        }
                        rec->unk_414[i][1] = temp;
                    }
                }
                temp = (u16)rec->unk_6A == 0x11 ? 8 : 4;
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
                rec->unk_428 = (rec->unk_00->unk_6C != 0xE && rec->unk_00->unk_6C != 0x2C) ? rec->unk_00->unk_5C : 0xFE;
                rec->unk_427 = lo;
                temp2 = func_80056FE8((s32)rec);
                temp3 = temp2 + 800;
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
                    } else if (temp3 >= D_800A387C) {
                        rec->unk_426 = 2;
                    } else {
                        rec->unk_426 = 3;
                    }
                }
                rec->unk_42A = rec->unk_00->unk_F4.x;
                rec->unk_42C = rec->unk_00->unk_F4.z;
                rec->unk_42E = temp3;
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
            u8 *obj = (u8 *)&D_80106A78 + i * 0x64;

            temp2 = SquareRoot0((rec->unk_F4.x - *(s32 *)(obj + 0x2C)) * (rec->unk_F4.x - *(s32 *)(obj + 0x2C)) +
                              (rec->unk_F4.z - *(s32 *)(obj + 0x34)) * (rec->unk_F4.z - *(s32 *)(obj + 0x34)));
            if (*(s16 *)(obj + 2) != -1 && obj[4] != 0 && obj[6] != rec->unk_04) {
                temp = (ratan2(rec->unk_F4.x - *(s32 *)(obj + 0x2C), rec->unk_F4.z - *(s32 *)(obj + 0x34)) -
                      ratan2(*(s32 *)(obj + 0x2C) - *(s32 *)(obj + 0x38), *(s32 *)(obj + 0x34) - *(s32 *)(obj + 0x40))) & 0xFFF;
                if (temp > 0x800) {
                    temp -= 0x1000;
                }
                if ((temp < 0 ? -temp : temp) < 0x80) {
                    if ((rec->unk_B8.vy - *(s32 *)(obj + 0x30) >= 0) ? (rec->unk_B8.vy - *(s32 *)(obj + 0x30) < 2000)
                                                              : (*(s32 *)(obj + 0x30) - rec->unk_B8.vy < 2000)) {
                        rec->unk_425 = temp2 < 3000 ? 2 : 1;
                    }
                }
            }
        }
    }

    if ((rec->unk_430 & 1) && !(D_80099D88[rec->unk_443].flags & 0xFF00) &&
        rec->unk_426 != 1 && rec->unk_426 != 2 && rec->unk_425 != 1 && rec->unk_425 != 2 &&
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
            ((rec->unk_00->unk_330 == 0 && rec->unk_425 != 1 && rec->unk_425 != 2) || rec->unk_00->unk_26C == 0)))) ||
         ((D_80099D88[rec->unk_443].flags & 0x20) &&
          ((u16)rec->unk_00->unk_6A == 0x25 || (u16)rec->unk_00->unk_6A == 9 ||
           (u16)rec->unk_00->unk_6A == 0x16 || (u16)rec->unk_00->unk_6A == 0x17 ||
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
            temp = func_80058580((u8 *)rec);
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
            pad.held |= 1 << bits.bit[rec->unk_441];
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
