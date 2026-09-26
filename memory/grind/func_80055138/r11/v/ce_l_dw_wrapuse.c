extern u32 file_GetFlag1(void);
extern s32 rand(void);
void func_80055138(s32 arg0, u16 *arg1, u16 *arg2) {
    u8 *p = (u8 *)&D_80101EC8 + arg0 * 0x44C;
    u8 *src;
    u8 *pair;
    u8 (*row)[4];
    u8 base;
    /* idx counts two loops: the eight bytes cleared at 0x444, then the two
     * players (0 = this record, 1 = the opponent's). Admitted under Ruling 11
     * (.claude/rules/ordinary-c-judge-decidable.md); allocator-dump proof in
     * memory/grind/func_80055138/ruling11.md. */
    s32 idx;
    s32 sec;
    u8 *rec;
    u16 *cursor;
    u8 *list;
    s32 chr;
    s32 bit;
    s32 lo, hi1, hi2;
    u32 cat;
    s32 lo_val, hi1_val, hi2_val;
    u8 *other;

    p[0x443] = *(u16 *)(p + 0xA);
    *(s16 *)(p + 0x438) = *(u16 *)(p + 8);
    switch (D_800A38DC) {
        s32 row_idx;
    case 1:
        *(s16 *)(p + 0x438) = (D_800A3783 - 1) / 5 * 0x300 + 0x400;
        if (*(s16 *)(p + 0x438) > 0xD00) {
            *(s16 *)(p + 0x438) = 0xD00;
        }
        break;
    case 0:
        if (D_800A3680 == D_800A3671) {
            func_8005509C(*(s16 *)(p + 4));
        }
        if (D_80099D88[p[0x443]].flags & 0x300) {
            row = D_8009A8C4[*(s16 *)(p + 0x86)];
            src = row[D_800A37A0];
            p[0x424] = src[0];
            p[0x3F6] = src[1];
        }
        break;
    case 2:
        if (D_800A389A) {
            s32 lvl5;
            lvl5 = D_800A37D2 / 5;
            do { /* FAKE */
                *(s16 *)(p + 0x438) = lvl5 * 0x180 + 0x280;
            } while (0);
            if (*(s16 *)(p + 0x438) > 0x1000) {
                *(s16 *)(p + 0x438) = 0x1000;
            }
            if ((u8)(D_800A37D2 % 5) == 0) {
                func_8005509C(*(s16 *)(p + 4));
            }
        } else {
            s32 lvl3;
            lvl3 = D_800A37D2 / 3;
            if (lvl3 >= 3) {
                D_800A37D2 = 0;
                lvl3 = 0;
            }
            p[0x443] = 0x19;
            *(s16 *)(p + 0x1C) = (lvl3 + 2) << 10;
            *(s16 *)(p + 0x438) = 0;
            p[0x424] = 0;
            p[0x3F6] = 0x3C - lvl3 * 15;
        }
        break;
    case 3:
        p[0x443] = cpu_practice_honmokuroku_data_tbl[D_800A38E2 - 1][0] + 0x1B;
        base = D_800A38E2 / 10;
        *(s16 *)(p + 0x438) = base * 16 + 0x80;
        if (D_80099D88[p[0x443]].flags & 0x3000) {
            *(s16 *)(p + 0x438) = base * 16 + 0x180;
        }
        if (D_80099D88[p[0x443]].flags & 0x4000) {
            *(s16 *)(p + 0x438) += 0x200;
        }
        if ((D_800A38E2 - 1) % 10 == 0) {
            func_8005509C(*(s16 *)(p + 4));
        }
        row_idx = (u8)(D_800A38E2 / 10) * 2;
        if ((u8)(D_800A38E2 % 10) == 0) {
            row_idx--;
        }
        pair = D_8009A9B4[row_idx];
        p[0x424] = pair[0];
        p[0x3F6] = pair[1];
        break;
    }
    if (file_GetFlag1() && D_800A38DC != 3) {
        *(s16 *)(p + 0x438) = *(s16 *)(p + 0x438) * 11 >> 4;
    }
    if (!(D_80099D88[p[0x443]].flags & 0xFF00)) {
        p[0x424] = 0x11 - (*(s16 *)(p + 0x438) >> 8);
    }
    *(s16 *)(p + 0x39A) = 0x8000 / *(s16 *)(p + 0x1C);
    p[0x3BD] = 0x10 - (*(s16 *)(p + 0x438) >> 8);
    if (D_80099D88[p[0x443]].flags & 0x100) {
        D_80099D88[p[0x443]].unk3 = (rand() & 3) + 1;
    }
    for (idx = 0; idx < 8U; idx++) {
        (p + idx)[0x444] = 0;
    }
    *(u16 **)(p + 0x3A4) = arg1;
    for (idx = 0; idx < 2; idx++) {
        if (idx) {
            rec = *(u8 **)p;
            cursor = arg2;
            list = (u8 *)arg2;
            chr = *(s16 *)(rec + 0xA);
        } else {
            rec = p;
            cursor = arg1;
            list = (u8 *)arg1;
            chr = p[0x443];
        }
        *(s16 *)(rec + 0x40A) = (*(s16 *)(rec + 0x1A) - 0x1000) * 225 >> 11;
        for (sec = 0; sec < 3; sec++) {
            bit = 1 << chr;
            lo = 0xFFFF;
            hi2 = 0;
            hi1 = 0;
            if (idx == 0) {
                *(u16 **)(p + 0x3A8 + sec * 4) = cursor;
            }
            while (*cursor != 0) {
                u8 *e = list + *cursor;
                if (e[4] == 0x40) {
                    s32 move_mask;
                    move_mask = (e[8] << 24) | (e[7] << 16) | (e[6] << 8) | e[5];
                    if (!(move_mask & bit)) {
                        goto next;
                    }
                }
                if (e[1] != 0 && e[1] != 0xFF) {
                    s32 stat1;
                    stat1 = e[1];
                    if (stat1 < lo) {
                        lo = stat1;
                    }
                }
                if (e[2] != 0 && e[2] != 0xFF) {
                    s32 stat2;
                    stat2 = e[2];
                    if (hi1 < stat2) {
                        hi1 = stat2;
                    }
                    cat = e[0] & 7;
                    if (hi2 < stat2 && (e[3] & 0xF) * 4 < 0x10 && (cat < 2 || cat == 7)) {
                        hi2 = stat2;
                    }
                }
            next:
                cursor++;
            }
            if (*(s16 *)(rec + 0xE) >= 6) {
                lo_val = 0;
                hi2_val = 0x7530;
                hi1_val = 0x7530;
            } else {
                /* FAKE: the shared base (rec's 0x40A halfword + 100) is staged
                 * through hi2_val, whose own value (base + hi2 * 40) is
                 * completed below; staged-value-reused-variable. Mechanism: a
                 * separate base local lives in one basic block, so
                 * local-alloc.c combine_regs ties it to the dying lh result
                 * (lh v1; addiu v1,v1,100); hi2_val is set in both arms and
                 * read after the join, so it is global-allocated and untied
                 * (target: lh v0; addiu v1,v0,100). Lever exhaustion:
                 * memory/grind/func_80055138/ruling11.md. */
                hi2_val = *(s16 *)(rec + 0x40A) + 100;
                lo_val = hi2_val + lo * 40;
                hi1_val = hi2_val + hi1 * 40;
                hi2_val += hi2 * 40;
            }
            cursor++;
            ((s16 *)(rec + 0x3F8))[sec] = lo_val;
            ((s16 *)(rec + 0x3FE))[sec] = hi1_val;
            ((s16 *)(rec + 0x404))[sec] = hi2_val;
        }
    }
    other = *(u8 **)p;
    *(s8 *)(p + 0x40D) = -1;
    *(s8 *)(p + 0x40C) = -1;
    *(s16 *)(p + 0x428) = -1;
    p[0x425] = 0;
    p[0x426] = 0;
    *(s32 *)(p + 0x3B4) = 0;
    *(s32 *)(p + 0x3E0) = 0;
    *(s32 *)(p + 0x3DC) = 0;
    *(s32 *)(p + 0x3D8) = 0;
    other[0x440] = 0;
    p[0x440] = 0;
    other[0x441] = 0;
    p[0x441] = 0;
    *(s16 *)(other + 0x43C) = 0;
    *(s16 *)(other + 0x43A) = 0;
    *(s16 *)(p + 0x43C) = 0;
    *(s16 *)(p + 0x43A) = 0;
    p[0x362] = 0;
    p[0x39D] = 0;
    p[0x3F5] = 0;
    p[0x3F4] = 0;
    p[0x3F3] = 0;
    p[0x3F2] = 0;
    *(s16 *)(p + 0x3EE) = 0;
    *(s16 *)(p + 0x3F0) = 0;
    *(s16 *)(p + 0x3E8) = 0;
    p[0x39C] = 0;
    *(s32 *)(p + 0x394) = 0;
    *(s16 *)(p + 0x398) = 0;
    *(s32 *)(p + 0x3C4) = 0;
    *(s16 *)(p + 0x3C2) = 0;
    p[0x3C1] = 0;
    p[0x3C0] = 0;
    p[0x440] = 0;
    *(s32 *)(p + 0x430) = 0;
    *(s32 *)(p + 0x3E4) = -1;
}
