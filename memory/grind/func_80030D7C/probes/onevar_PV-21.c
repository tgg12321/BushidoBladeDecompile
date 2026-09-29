/* Per-frame update of the twelve 0x64-byte records at D_80106A78: velocity turn,
 * spin decay, gravity, collision test through func_8005344C, reflection off the
 * returned normal, func_80032854 cues, and the rest / re-hop logic. Scratch
 * vectors live in the scratchpad record at 0x1F8002B8. */
void func_80030D7C(void) {
    u8 *scr;
    u8 *obj;
    s32 i;
    s32 amt;
    s32 rest;
    s32 half;
    s32 ang;
    s32 hit;
    s32 dot;
    s32 spd;
    s16 state;
    s16 *nrm;

    scr = (u8 *)0x1F8002B8;
    obj = (u8 *)&D_80106A78;
    for (i = 0; i < 12; i++, obj += 0x64) {
        if (*(s16 *)(obj + 2) == -1) {
            *(u8 *)(obj + 0xA) = 0xFF;
            continue;
        }
        if (*(u8 *)(obj + 8) != 0) {
            continue;
        }
        *(Vec3i32 *)(obj + 0x38) = *(Vec3i32 *)(obj + 0x2C);
        if (*(s32 *)(obj + 0x50) != 0) {
            (*(s16 *)obj)++;
        }
        if (*(s16 *)(obj + 2) == 0x10 && *(s32 *)(obj + 0x50) != 0 && *(u8 *)(obj + 5) == 0
            && *(s16 *)obj >= 14) {
            ang = ratan2(*(s32 *)(obj + 0x44), *(s32 *)(obj + 0x4C));
            amt = (0x4E - *(s16 *)obj) * 96 / 64;
            if (amt < 0) {
                amt = 0;
            } else if (amt > 0x42) {
                amt = 0x42;
            }
            *(s32 *)(scr + 0x10) = ((&Judge)[(ang + 0x400) & 0xFFF] * *(s32 *)(obj + 0x44)
                                    - (&Judge)[ang & 0xFFF] * *(s32 *)(obj + 0x4C)) >> 12;
            *(s32 *)(scr + 0x14) = *(s32 *)(obj + 0x48);
            *(s32 *)(scr + 0x18) = ((&Judge)[ang & 0xFFF] * *(s32 *)(obj + 0x44)
                                    + (&Judge)[(ang + 0x400) & 0xFFF] * *(s32 *)(obj + 0x4C)) >> 12;
            *(s32 *)(scr + 0x20) = *(s32 *)(scr + 0x10);
            half = amt / 2;
            *(s32 *)(scr + 0x24) = ((&Judge)[(half + 0x400) & 0xFFF] * *(s32 *)(scr + 0x14)
                                    - (&Judge)[half & 0xFFF] * *(s32 *)(scr + 0x18)) >> 12;
            *(s32 *)(scr + 0x28) = ((&Judge)[half & 0xFFF] * *(s32 *)(scr + 0x14)
                                    + (&Judge)[(half + 0x400) & 0xFFF] * *(s32 *)(scr + 0x18)) >> 12;
            *(s32 *)(obj + 0x44) = ((&Judge)[(amt - ang + 0x400) & 0xFFF] * *(s32 *)(scr + 0x20)
                                    - (&Judge)[(amt - ang) & 0xFFF] * *(s32 *)(scr + 0x28)) >> 12;
            *(s32 *)(obj + 0x48) = *(s32 *)(scr + 0x24);
            *(s32 *)(obj + 0x4C) = ((&Judge)[(amt - ang) & 0xFFF] * *(s32 *)(scr + 0x20)
                                    + (&Judge)[(amt - ang + 0x400) & 0xFFF] * *(s32 *)(scr + 0x28)) >> 12;
            *(s16 *)(obj + 0x5C) = *(s16 *)(obj + 0x5C) * 63 / 64;
            *(s16 *)(obj + 0x5E) = *(s16 *)(obj + 0x5E) * 63 / 64;
            *(s16 *)(obj + 0x60) = *(s16 *)(obj + 0x60) * 63 / 64;
        } else {
            *(s16 *)(obj + 0x5C) = *(s16 *)(obj + 0x5C) * 15 / 16;
            *(s16 *)(obj + 0x5E) = *(s16 *)(obj + 0x5E) * 15 / 16;
            *(s16 *)(obj + 0x60) = *(s16 *)(obj + 0x60) * 15 / 16;
        }
        *(s16 *)(obj + 0x54) += *(s16 *)(obj + 0x5C);
        *(s16 *)(obj + 0x56) += *(s16 *)(obj + 0x5E);
        *(s16 *)(obj + 0x58) += *(s16 *)(obj + 0x60);
        *(s32 *)(obj + 0x48) += 13;
        *(s32 *)(scr + 0x0) = *(s32 *)(obj + 0x2C) + *(s32 *)(obj + 0x44);
        *(s32 *)(scr + 0x4) = *(s32 *)(obj + 0x30) + *(s32 *)(obj + 0x48);
        *(s32 *)(scr + 0x8) = *(s32 *)(obj + 0x34) + *(s32 *)(obj + 0x4C);
        *(s32 *)(obj + 0x30) -= 8;
        nrm = (s16 *)(scr + 0x30);
        hit = func_8005344C((s32 *)(obj + 0x2C), (s32 *)scr, (s32 *)(scr + 0x10), (s32 *)nrm, (s32)(scr + 0x38));
        if (hit != 0 && func_80054434() == 7) {
            hit = 0;
        }
        if (hit != 0) {
            if (*(s16 *)(obj + 2) == 0xF) {
                func_80032854(*(u8 *)(obj + 6), 0xE, scr + 0x10, nrm);
                *(s16 *)(obj + 2) = -1;
                continue;
            }
            dot = (*(s32 *)(obj + 0x44) * nrm[0]
                   + *(s32 *)(obj + 0x48) * nrm[1]
                   + *(s32 *)(obj + 0x4C) * nrm[2]) / 2048;
            *(s32 *)(obj + 0x44) -= nrm[0] * dot / 4096;
            *(s32 *)(obj + 0x48) -= nrm[1] * dot / 4096;
            *(s32 *)(obj + 0x4C) -= nrm[2] * dot / 4096;
            *(Vec3i32 *)(obj + 0x2C) = *(Vec3i32 *)(scr + 0x10);
            state = *(s16 *)(obj + 2);
            rest = D_8008E194[state].unkA;
            if (*(s32 *)(obj + 0x50) != 0) {
                *(s32 *)(obj + 0x44) = *(s32 *)(obj + 0x44) * rest / 4096;
                *(s32 *)(obj + 0x48) = *(s32 *)(obj + 0x48) * rest / 4096;
                *(s32 *)(obj + 0x4C) = *(s32 *)(obj + 0x4C) * rest / 4096;
                spd = *(s32 *)(obj + 0x44) * *(s32 *)(obj + 0x44)
                    + *(s32 *)(obj + 0x4C) * *(s32 *)(obj + 0x4C);
                if (*(s16 *)(scr + 0x32) >= -0x7FF) {
                    *(s16 *)(obj + 0x5E) += (rng_Next() & 1) ? spd / 64 : -spd / 64;
                    if (*(s16 *)(obj + 2) != 0xE && *(u8 *)(obj + 4) != 0) {
                        func_80032854(*(u8 *)(obj + 6), 1, obj + 0x2C, 0);
                    }
                }
                if (*(s16 *)(obj + 2) == 0xE) {
                    if (*(u8 *)(obj + 4) != 0) {
                        func_80032854((*(s16 *)(obj + 2) ^ D_800A36F2) != 0, 0x2F, obj + 0x2C, 0);
                    }
                } else if (*(s16 *)(obj + 2) < 0x12) {
                    if (spd > 0x10) {
                        func_80032854((*(s16 *)(obj + 2) ^ D_800A36F2) != 0, 0x2C, obj + 0x2C, 0);
                    }
                } else if ((u16)(*(s16 *)(obj + 2) - 0x12) < 12) {
                    if (spd > 0x10) {
                        func_80032854(*(u8 *)(obj + 6), 0x29, obj + 0x2C, 0);
                    }
                }
            } else if (*(s16 *)(scr + 0x32) >= -0x7FF && state == 0xE) {
                if (*(u8 *)(obj + 5) == 1) {
                    *(s32 *)(obj + 0x50) = 1;
                    *(s16 *)(obj + 0x56) += 0x780 + (rng_Next() & 0xFF);
                    *(s32 *)(obj + 0x44) = (&Judge)[*(s16 *)(obj + 0x56) & 0xFFF] / 64;
                    *(s32 *)(obj + 0x48) = -150;
                    *(s32 *)(obj + 0x4C) = (&Judge)[(*(s16 *)(obj + 0x56) + 0x400) & 0xFFF] / 64;
                    *(u8 *)(obj + 5) = 2;
                    func_80032854((*(s16 *)(obj + 2) ^ D_800A36F2) != 0, 0x2F, obj + 0x2C, 0);
                }
            }
            *(u8 *)(obj + 4) = 0;
            if (*(u8 *)(obj + 7) == 1) {
                *(u8 *)(obj + 7) = 2;
            }
            if (*(u8 *)(obj + 5) == 2) {
                *(u8 *)(obj + 5) = 3;
            }
        } else {
            *(Vec3i32 *)(obj + 0x2C) = *(Vec3i32 *)scr;
        }
        if ((u32)(*(s32 *)(obj + 0x48) + 15) < 31 && (u32)(*(s32 *)(obj + 0x44) + 3) < 7
            && (u32)(*(s32 *)(obj + 0x4C) + 3) < 7) {
            if (hit != 0) {
                if (D_800A38DC == 3) {
                    *(s16 *)(obj + 2) = -1;
                } else {
                    *(s32 *)(obj + 0x50) = 0;
                    *(u8 *)(obj + 5) = 1;
                    *(s32 *)(obj + 0x44) = 0;
                    *(s32 *)(obj + 0x48) = 0;
                    *(s32 *)(obj + 0x4C) = 0;
                }
            }
        } else {
            *(s32 *)(obj + 0x50) = 1;
        }
        if (*(s16 *)(obj + 2) == 0xE) {
            if (*(u8 *)(obj + 5) == 1 && !(rng_Next() & 0x133)) {
                *(s32 *)(obj + 0x50) = 1;
                *(s16 *)(obj + 0x5E) += (rng_Next() & 0x7F) - 0x40;
                *(s32 *)(obj + 0x44) = (&Judge)[*(s16 *)(obj + 0x56) & 0xFFF] / 64;
                *(s32 *)(obj + 0x48) = -150;
                *(s32 *)(obj + 0x4C) = (&Judge)[(*(s16 *)(obj + 0x56) + 0x400) & 0xFFF] / 64;
                *(u8 *)(obj + 5) = 2;
                func_80032854((*(s16 *)(obj + 2) ^ D_800A36F2) != 0, 0x2F, obj + 0x2C, 0);
            }
        }
        if (*(u8 *)(obj + 7) == 2) {
            *(s16 *)(obj + 0x56) = math_LerpAngle(*(s16 *)(obj + 0x56), 0, 0x800);
            *(s16 *)(obj + 0x58) = math_LerpAngle(*(s16 *)(obj + 0x58), 0x400, 0xE00);
            *(s16 *)(obj + 0x5E) = *(s16 *)(obj + 0x5E) * 3 / 4;
        } else if (*(u8 *)(obj + 4) == 0 && D_8008E194[*(s16 *)(obj + 2)].unk0 == 2) {
            *(s16 *)(obj + 0x54) = math_LerpAngle(*(s16 *)(obj + 0x54), 0, 0xE00);
            *(s16 *)(obj + 0x56) = math_LerpAngle(*(s16 *)(obj + 0x56), 0, 0x600);
            *(s16 *)(obj + 0x58) = math_LerpAngle(*(s16 *)(obj + 0x58), -0x400, 0xE00);
        }
        if (*(s32 *)(obj + 0x30) > 0x3A98) {
            *(s16 *)(obj + 2) = -1;
        }
    }
    func_80030208();
}
