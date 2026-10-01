void func_80030D7C(void) {
    u8 *scr;
    Obj80106A78 *obj;
    s32 i;
    s32 half;
    s32 dot;
    s32 spd;
    s16 state;
    s16 *nrm;

    scr = (u8 *)0x1F8002B8;
    obj = D_80106A78;
    for (i = 0; i < 12; i++, obj++) {
        s32 temp;

        if (obj->unk_02 == -1) {
            obj->unk_0A = 0xFF;
            continue;
        }
        if (obj->unk_08 != 0) {
            continue;
        }
        obj->unk_38 = obj->unk_2C;
        if (obj->unk_50 != 0) {
            obj->unk_00++;
        }
        if (obj->unk_02 == 0x10 && obj->unk_50 != 0 && obj->unk_05 == 0
            && obj->unk_00 >= 14) {
            s32 amt;

            temp = ratan2(obj->unk_44.x, obj->unk_44.z);
            amt = (0x4E - obj->unk_00) * 96 / 64;
            if (amt < 0) {
                amt = 0;
            } else if (amt > 0x42) {
                amt = 0x42;
            }
            *(s32 *)(scr + 0x10) = (Judge[(temp + 0x400) & 0xFFF] * obj->unk_44.x
                                    - Judge[temp & 0xFFF] * obj->unk_44.z) >> 12;
            *(s32 *)(scr + 0x14) = obj->unk_44.y;
            *(s32 *)(scr + 0x18) = (Judge[temp & 0xFFF] * obj->unk_44.x
                                    + Judge[(temp + 0x400) & 0xFFF] * obj->unk_44.z) >> 12;
            *(s32 *)(scr + 0x20) = *(s32 *)(scr + 0x10);
            half = amt / 2;
            *(s32 *)(scr + 0x24) = (Judge[(half + 0x400) & 0xFFF] * *(s32 *)(scr + 0x14)
                                    - Judge[half & 0xFFF] * *(s32 *)(scr + 0x18)) >> 12;
            *(s32 *)(scr + 0x28) = (Judge[half & 0xFFF] * *(s32 *)(scr + 0x14)
                                    + Judge[(half + 0x400) & 0xFFF] * *(s32 *)(scr + 0x18)) >> 12;
            obj->unk_44.x = (Judge[(amt - temp + 0x400) & 0xFFF] * *(s32 *)(scr + 0x20)
                                    - Judge[(amt - temp) & 0xFFF] * *(s32 *)(scr + 0x28)) >> 12;
            obj->unk_44.y = *(s32 *)(scr + 0x24);
            obj->unk_44.z = (Judge[(amt - temp) & 0xFFF] * *(s32 *)(scr + 0x20)
                                    + Judge[(amt - temp + 0x400) & 0xFFF] * *(s32 *)(scr + 0x28)) >> 12;
            obj->unk_5C[0] = obj->unk_5C[0] * 63 / 64;
            obj->unk_5C[1] = obj->unk_5C[1] * 63 / 64;
            obj->unk_5C[2] = obj->unk_5C[2] * 63 / 64;
        } else {
            obj->unk_5C[0] = obj->unk_5C[0] * 15 / 16;
            obj->unk_5C[1] = obj->unk_5C[1] * 15 / 16;
            obj->unk_5C[2] = obj->unk_5C[2] * 15 / 16;
        }
        obj->unk_54[0] += obj->unk_5C[0];
        obj->unk_54[1] += obj->unk_5C[1];
        obj->unk_54[2] += obj->unk_5C[2];
        obj->unk_44.y += 13;
        *(s32 *)(scr + 0x0) = obj->unk_2C.x + obj->unk_44.x;
        *(s32 *)(scr + 0x4) = obj->unk_2C.y + obj->unk_44.y;
        *(s32 *)(scr + 0x8) = obj->unk_2C.z + obj->unk_44.z;
        obj->unk_2C.y -= 8;
        nrm = (s16 *)(scr + 0x30);
        temp = func_8005344C(&obj->unk_2C.x, (s32 *)scr, (s32 *)(scr + 0x10), (s32 *)nrm, (s32)(scr + 0x38));
        if (temp != 0 && func_80054434() == 7) {
            temp = 0;
        }
        if (temp != 0) {
            s32 rest;

            if (obj->unk_02 == 0xF) {
                func_80032854(obj->unk_06, 0xE, scr + 0x10, nrm);
                obj->unk_02 = -1;
                continue;
            }
            dot = (obj->unk_44.x * nrm[0]
                   + obj->unk_44.y * nrm[1]
                   + obj->unk_44.z * nrm[2]) / 2048;
            obj->unk_44.x -= nrm[0] * dot / 4096;
            obj->unk_44.y -= nrm[1] * dot / 4096;
            obj->unk_44.z -= nrm[2] * dot / 4096;
            obj->unk_2C = *(Vec3i32 *)(scr + 0x10);
            state = obj->unk_02;
            rest = D_8008E194[state].unkA;
            if (obj->unk_50 != 0) {
                obj->unk_44.x = obj->unk_44.x * rest / 4096;
                obj->unk_44.y = obj->unk_44.y * rest / 4096;
                obj->unk_44.z = obj->unk_44.z * rest / 4096;
                spd = obj->unk_44.x * obj->unk_44.x
                    + obj->unk_44.z * obj->unk_44.z;
                if (*(s16 *)(scr + 0x32) >= -0x7FF) {
                    obj->unk_5C[1] += (rng_Next() & 1) ? spd / 64 : -spd / 64;
                    if (obj->unk_02 != 0xE && obj->unk_04 != 0) {
                        func_80032854(obj->unk_06, 1, (u8 *)&obj->unk_2C, 0);
                    }
                }
                if (obj->unk_02 == 0xE) {
                    if (obj->unk_04 != 0) {
                        func_80032854((obj->unk_02 ^ D_800A36F2[0]) != 0, 0x2F, (u8 *)&obj->unk_2C, 0);
                    }
                } else if (obj->unk_02 < 0x12) {
                    if (spd > 0x10) {
                        func_80032854((obj->unk_02 ^ D_800A36F2[0]) != 0, 0x2C, (u8 *)&obj->unk_2C, 0);
                    }
                } else if ((u16)(obj->unk_02 - 0x12) < 12) {
                    if (spd > 0x10) {
                        func_80032854(obj->unk_06, 0x29, (u8 *)&obj->unk_2C, 0);
                    }
                }
            } else if (*(s16 *)(scr + 0x32) >= -0x7FF && state == 0xE) {
                if (obj->unk_05 == 1) {
                    obj->unk_50 = 1;
                    obj->unk_54[1] += 0x780 + (rng_Next() & 0xFF);
                    obj->unk_44.x = Judge[obj->unk_54[1] & 0xFFF] / 64;
                    obj->unk_44.y = -150;
                    obj->unk_44.z = Judge[(obj->unk_54[1] + 0x400) & 0xFFF] / 64;
                    obj->unk_05 = 2;
                    func_80032854((obj->unk_02 ^ D_800A36F2[0]) != 0, 0x2F, (u8 *)&obj->unk_2C, 0);
                }
            }
            obj->unk_04 = 0;
            if (obj->unk_07 == 1) {
                obj->unk_07 = 2;
            }
            if (obj->unk_05 == 2) {
                obj->unk_05 = 3;
            }
        } else {
            obj->unk_2C = *(Vec3i32 *)scr;
        }
        if ((u32)(obj->unk_44.y + 15) < 31 && (u32)(obj->unk_44.x + 3) < 7
            && (u32)(obj->unk_44.z + 3) < 7) {
            if (temp != 0) {
                if (D_800A38DC == 3) {
                    obj->unk_02 = -1;
                } else {
                    obj->unk_50 = 0;
                    obj->unk_05 = 1;
                    obj->unk_44.x = 0;
                    obj->unk_44.y = 0;
                    obj->unk_44.z = 0;
                }
            }
        } else {
            obj->unk_50 = 1;
        }
        if (obj->unk_02 == 0xE) {
            if (obj->unk_05 == 1 && !(rng_Next() & 0x133)) {
                obj->unk_50 = 1;
                obj->unk_5C[1] += (rng_Next() & 0x7F) - 0x40;
                obj->unk_44.x = Judge[obj->unk_54[1] & 0xFFF] / 64;
                obj->unk_44.y = -150;
                obj->unk_44.z = Judge[(obj->unk_54[1] + 0x400) & 0xFFF] / 64;
                obj->unk_05 = 2;
                func_80032854((obj->unk_02 ^ D_800A36F2[0]) != 0, 0x2F, (u8 *)&obj->unk_2C, 0);
            }
        }
        if (obj->unk_07 == 2) {
            obj->unk_54[1] = math_LerpAngle(obj->unk_54[1], 0, 0x800);
            obj->unk_54[2] = math_LerpAngle(obj->unk_54[2], 0x400, 0xE00);
            obj->unk_5C[1] = obj->unk_5C[1] * 3 / 4;
        } else if (obj->unk_04 == 0 && D_8008E194[obj->unk_02].unk0 == 2) {
            obj->unk_54[0] = math_LerpAngle(obj->unk_54[0], 0, 0xE00);
            obj->unk_54[1] = math_LerpAngle(obj->unk_54[1], 0, 0x600);
            obj->unk_54[2] = math_LerpAngle(obj->unk_54[2], -0x400, 0xE00);
        }
        if (obj->unk_2C.y > 0x3A98) {
            obj->unk_02 = -1;
        }
    }
    func_80030208();
}
