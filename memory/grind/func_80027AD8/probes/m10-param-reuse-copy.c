/* PROBE, NOT A CANDIDATE (slotK s2 2026-09-26): param `rec` reused for the R6a same-side flag + local copy `hd`
 * passed to func_800278C0. Banned (Ruling 11 (A) parameter; (C)(3) bare copy). Sandbox 55 vs candidate 97:
 * the only family measured that reproduces frame 0x50, opp in s6, `move fp,<rec>` and thresh spilled. */
extern void func_800203B4(u8 *, s32, s16 *);
extern u8 D_8008EB74[3][2][2];
s32 func_80027AD8(s32 pass, u8 *ch, s32 limb, s32 thresh, s32 flag, Tbl8008E194 *rec, s32 arg6, s32 *out) {
    s32 hd;
    s16 *vec;
    u8 *scr;
    s32 player;
    u8 *opp;
    s32 ang;
    u16 st;
    s32 diff;
    s32 cat;
    u32 sign;
    s32 code;

    hd = (s32)rec;
    vec = &D_800A37E8;
    player = *(s16 *)(ch + 4);
    opp = *(u8 **)ch;
    scr = (u8 *)0x1F8000A8 + limb * 12 + player * 0x108;
    *out = 0;
    if (*(s16 *)(ch + 0xC) == 0x1C) {
        ang = ((*(s32 *)(ch + 0x1EC) * D_800A37E8 + *(s32 *)(ch + 0x1F0) * D_800A37EA +
                *(s32 *)(ch + 0x1F4) * D_800A37EC) >> 12) & 0x1FFF;
        if (ang >= 0x1000) {
            ang -= 0x2000;
        }
        if (!((0xE >> limb) & 1) || ang < 0x400) {
            if (*(s16 *)(ch + 0x96) == 0) {
                *(s16 *)(ch + 0x286) = 0x1E;
            }
            if (pass == 0 && *(s16 *)(opp + 0x286) == -1) {
                code = rec ? 0x19 : 0xB;
                *(s16 *)(opp + 0x286) = code;
            }
            func_80032854(player, 0x12, scr, 0);
            return 0;
        }
    }
    if (*(s16 *)(ch + 0xC) == 0xD && *(s16 *)(ch + 0x96) == 0 && *(u16 *)(ch + 0x6A) != 0x2E) {
        func_80027640((s32)ch);
        return 2;
    }
    st = *(u16 *)(ch + 0x6A);
    if (st == 4 || st == 0x14) {
        func_800203B4(ch, limb, vec);
        func_800278C0(pass, (s32 *)ch, limb, hd, scr, arg6);
        return 1;
    }
    if (*(s16 *)(ch + 0xC) == 0x1F) {
        func_800203B4(ch, limb, vec);
        func_800278C0(pass, (s32 *)ch, limb, hd, scr, arg6);
        *(s16 *)(ch + 0x286) = 7;
        return 1;
    }
    if ((st == 2 || st == 0x1B || st == 0x28 || st == 0x26) && *(s16 *)(ch + 0x40) < *(u8 *)(ch + 0xA0)) {
        if (pass == 0) {
            diff = *(s16 *)(opp + 0x20) - *(s16 *)(ch + 0x20);
            cat = func_800272FC(diff);
            sign = (u32)diff >> 31;
            rec = (Tbl8008E194 *)(ch[0xAF] == opp[0xAF]);
            ch[0xB4] = opp[0xB4] = (s32)rec;
            func_80032854(player, rec ? 0xF : 2, scr, 0);
            code = D_8008EB74[cat][sign][(s32)rec];
            *(s16 *)(ch + 0x286) = code;
            if (code == 0) {
                func_80032854(player == 0, 0x25, scr, 0);
                func_80032854(player, 0x25, scr, 0);
            } else if (code == 1) {
                func_80032854(player == 0, 0x26, scr, 0);
                func_80032854(player, 0x25, scr, 0);
                func_80032854(player, 0x2D, scr, 0);
            } else if (code == 2) {
                func_80032854(player == 0, 0x26, scr, 0);
                func_80032854(player, 0x26, scr, 0);
                func_80032854(player, 0x2D, scr, 0);
                func_80027A58((s32 *)ch);
            } else {
                return 0;
            }
            if (*(s16 *)(ch + 0x286) == 2) {
                if (vec[1] * vec[1] < vec[0] * vec[0] + vec[2] * vec[2]) {
                    func_8001F860((s16 *)ch, ratan2(vec[0], vec[2]));
                    *(s32 *)(ch + 0x134) -= vec[0] / 16;
                    *(s32 *)(ch + 0x13C) -= vec[2] / 16;
                }
            }
            return 0;
        }
        if (pass == 1) {
            if (rec->unkC == 0) {
                func_80032854(player, 2, scr, 0);
                func_80032854(player, 0x25, scr, 0);
                *(s16 *)(ch + 0x286) = 0;
                *out = pass;
                return 0;
            }
            if (rec->unkC == 1) {
                func_80032854(player, 2, scr, 0);
                func_80032854(player, 0x25, scr, 0);
                *(s16 *)(ch + 0x286) = 1;
                func_8001F860((s16 *)ch, ratan2(vec[0], vec[2]));
                *(s32 *)(ch + 0x134) -= vec[0] / 16;
                *(s32 *)(ch + 0x13C) -= vec[2] / 16;
                *out = pass;
                return 0;
            }
        }
    }
    if (D_800A38DC == 0 && player == 0) {
        func_8002738C(0, limb);
    }
    func_800278C0(pass, (s32 *)ch, limb, hd, scr, arg6);
    st = *(u16 *)(ch + 0x6A);
    if (st == 6 || st == 9) {
        if (flag) {
            *(s16 *)(ch + 0x286) = *(s16 *)(ch + 0x1DA) ? 6 : 7;
            func_800203B4(ch, limb, vec);
            return 1;
        }
        *(s16 *)(ch + 0x286) = *(s16 *)(ch + 0x1DA) ? 3 : 4;
        func_80032854(player, 3, scr, 0);
        func_80027438(ch, limb, 1);
        return 0;
    }
    if (pass == 1 && rec->unk0 == 4) {
        s16 kind = *(s16 *)(ch + 0xA);
        if (kind == 2 || kind == 4 || kind == 0xE || kind == 0xF) {
            func_80027438(ch, limb, rec->unkD == 2 ? 2 : 1);
            *(s16 *)(ch + 0x286) = 0x1B;
            func_80032854(player, 3, scr, 0);
            return 0;
        }
    }
    switch (limb) {
    case 0:
        if (flag) {
            *(s16 *)(ch + 0x286) = thresh > 0x400 ? 6 : 9;
            func_800203B4(ch, limb, vec);
            func_80027A58((s32 *)ch);
            return 1;
        }
    case 1:
    case 2:
    case 3:
        if (flag) {
            *(s16 *)(ch + 0x286) = thresh > 0x400 ? 7 : 9;
            func_800203B4(ch, limb, vec);
            func_80027A58((s32 *)ch);
            return 1;
        }
    case 4:
    case 5:
        if (!flag) {
            if (pass == 1 && rec->unkD == 2) {
                (*(u16 *)(ch + 0x272))++;
            }
            if (D_800A38DC != 5) {
                (*(u16 *)(ch + 0x272))++;
            }
            *(s16 *)(ch + 0x286) = 5;
            func_80032854(player, 3, scr, 0);
            func_80027A58((s32 *)ch);
            return 0;
        }
        *(s16 *)(ch + 0x286) = thresh > 0x400 ? 8 : 9;
        func_800203B4(ch, limb, vec);
        func_80027A58((s32 *)ch);
        return 1;
    case 6:
    case 7:
    case 8:
    case 9:
        if (D_800A38DC != 5) {
            *(s16 *)(ch + 0x26C) = 0;
        }
        *(s16 *)(ch + 0x286) = 4;
        *(s16 *)(ch + 0x90) = 0;
        func_80032854(player, 3, scr, 0);
        func_80027A58((s32 *)ch);
        *(s16 *)(ch + 0x8A) = 0;
        return 0;
    case 10:
    case 11:
    case 12:
    case 13:
        if (D_800A38DC != 5) {
            (*(u16 *)(ch + 0x270))++;
        }
        *(s16 *)(ch + 0x286) = 5;
        func_80032854(player, 3, scr, 0);
        func_80027A58((s32 *)ch);
        return 0;
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
        if (D_800A38DC != 5) {
            (*(u16 *)(ch + 0x26E))++;
        }
        *(s16 *)(ch + 0x286) = 3;
        func_80032854(player, 3, scr, 0);
        func_80027A58((s32 *)ch);
        return 0;
    }
}
