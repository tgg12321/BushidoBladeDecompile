void func_80055138(s32 arg0, u16 *arg1, u16 *arg2) {
    PracticeMenuRec *p = &g_practice_menu_table[arg0];
    CpuLevelEntry *src;
    u8 *pair;
    u8 base;
    /* idx counts two loops: the eight bytes cleared at 0x444, then the two
     * players (0 = this record, 1 = the opponent's). Admitted under Ruling 11
     * (.claude/rules/ordinary-c-judge-decidable.md); allocator-dump proof in
     * pre-slim-2026-10-01:memory/grind/func_80055138/ruling11.md. */
    s32 idx;
    s32 sec;
    PracticeMenuRec *rec;
    u16 *cursor;
    u16 *list;
    s32 chr;
    s32 bit;
    s32 lo, hi1, hi2;
    /* temp holds six values in turn; each is read before temp is written again:
     * case 2's level D_800A37D2 / 5; case 2's practice level D_800A37D2 / 3
     * (0 once it reaches 3); case 3's row in D_8009A9B4; a move entry's
     * byte-assembled character mask; the entry's stat bytes e[1] and e[2].
     * Admitted under Ruling 11 (.claude/rules/ordinary-c-judge-decidable.md);
     * allocator-dump proof in pre-slim-2026-10-01:memory/grind/func_80055138/ruling11.md. */
    s32 temp;
    u32 cat;
    s32 lo_val, hi1_val, hi2_val;
    PracticeMenuRec *other;

    p->unk_443 = p->unk_0A;
    p->unk_438 = p->unk_08;
    switch (D_800A38DC) {
    case 1:
        p->unk_438 = (D_800A3783 - 1) / 5 * 0x300 + 0x400;
        if (p->unk_438 > 0xD00) {
            p->unk_438 = 0xD00;
        }
        break;
    case 0:
        if (D_800A3680 == D_800A3671) {
            func_8005509C(p->unk_04);
        }
        if (D_80099D88[p->unk_443].flags & 0x300) {
            src = &D_8009A8C8[p->unk_86][D_800A37A0 - 1];
            p->unk_424 = src->unk0;
            p->unk_3F6 = src->unk1;
        }
        break;
    case 2:
        if (D_800A389A) {
            temp = D_800A37D2 / 5;
            p->unk_438 = temp * 0x180 + 0x280;
            if (p->unk_438 > 0x1000) {
                p->unk_438 = 0x1000;
            }
            if (D_800A37D2 % 5 == 0) {
                func_8005509C(p->unk_04);
            }
        } else {
            temp = D_800A37D2 / 3;
            if (temp >= 3) {
                D_800A37D2 = 0;
                temp = 0;
            }
            p->unk_443 = 0x19;
            p->unk_1C = (temp + 2) << 10;
            p->unk_438 = 0;
            p->unk_424 = 0;
            p->unk_3F6 = 0x3C - temp * 15;
        }
        break;
    case 3:
        p->unk_443 = cpu_practice_honmokuroku_data_tbl[D_800A38E2 - 1][0] + 0x1B;
        base = D_800A38E2 / 10;
        p->unk_438 = base * 16 + 0x80;
        if (D_80099D88[p->unk_443].flags & 0x3000) {
            p->unk_438 = base * 16 + 0x180;
        }
        if (D_80099D88[p->unk_443].flags & 0x4000) {
            p->unk_438 += 0x200;
        }
        if ((D_800A38E2 - 1) % 10 == 0) {
            func_8005509C(p->unk_04);
        }
        temp = D_800A38E2 / 10 * 2;
        if (D_800A38E2 % 10 == 0) {
            temp--;
        }
        pair = D_8009A9B4[temp];
        p->unk_424 = pair[0];
        p->unk_3F6 = pair[1];
        break;
    }
    if (file_GetFlag1() && D_800A38DC != 3) {
        p->unk_438 = p->unk_438 * 11 >> 4;
    }
    if (!(D_80099D88[p->unk_443].flags & 0xFF00)) {
        p->unk_424 = 0x11 - (p->unk_438 >> 8);
    }
    p->unk_39A = 0x8000 / p->unk_1C;
    p->unk_3BD = 0x10 - (p->unk_438 >> 8);
    if (D_80099D88[p->unk_443].flags & 0x100) {
        D_80099D88[p->unk_443].unk3 = (rand() & 3) + 1;
    }
    for (idx = 0; idx < sizeof(p->unk_444); idx++) {
        p->unk_444[idx] = 0;
    }
    p->unk_3A4 = arg1;
    for (idx = 0; idx < 2; idx++) {
        if (idx) {
            rec = p->unk_00;
            cursor = arg2;
            list = arg2;
            chr = rec->unk_0A;
        } else {
            rec = p;
            cursor = arg1;
            list = arg1;
            chr = p->unk_443;
        }
        rec->unk_40A = (rec->unk_1A - 0x1000) * 225 >> 11;
        for (sec = 0; sec < 3; sec++) {
            bit = 1 << chr;
            lo = 0xFFFF;
            hi2 = 0;
            hi1 = 0;
            if (idx == 0) {
                p->unk_3A8[sec] = cursor;
            }
            while (*cursor != 0) {
                u8 *e = (u8 *)list + *cursor;
                if (e[4] == 0x40) {
                    temp = (e[8] << 24) | (e[7] << 16) | (e[6] << 8) | e[5];
                    if (!(temp & bit)) {
                        goto next;
                    }
                }
                if (e[1] != 0 && e[1] != 0xFF) {
                    temp = e[1];
                    if (temp < lo) {
                        lo = temp;
                    }
                }
                if (e[2] != 0 && e[2] != 0xFF) {
                    temp = e[2];
                    if (hi1 < temp) {
                        hi1 = temp;
                    }
                    cat = e[0] & 7;
                    if (hi2 < temp && (e[3] & 0xF) * 4 < 0x10 && (cat < 2 || cat == 7)) {
                        hi2 = temp;
                    }
                }
            next:
                cursor++;
            }
            if (rec->unk_0E >= 6) {
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
                 * pre-slim-2026-10-01:memory/grind/func_80055138/ruling11.md. */
                hi2_val = rec->unk_40A + 100;
                lo_val = hi2_val + lo * 40;
                hi1_val = hi2_val + hi1 * 40;
                hi2_val += hi2 * 40;
            }
            cursor++;
            rec->unk_3F8[sec] = lo_val;
            rec->unk_3FE[sec] = hi1_val;
            rec->unk_404[sec] = hi2_val;
        }
    }
    other = p->unk_00;
    p->unk_40D = -1;
    p->unk_40C = -1;
    p->unk_428 = -1;
    p->unk_425 = 0;
    p->unk_426 = 0;
    p->unk_3B4 = 0;
    p->unk_3E0 = 0;
    p->unk_3DC = 0;
    p->unk_3D8 = 0;
    other->unk_440 = 0;
    p->unk_440 = 0;
    other->unk_441 = 0;
    p->unk_441 = 0;
    other->unk_43C = 0;
    other->unk_43A = 0;
    p->unk_43C = 0;
    p->unk_43A = 0;
    p->unk_362 = 0;
    p->unk_39D = 0;
    p->unk_3F5 = 0;
    p->unk_3F4 = 0;
    p->unk_3F3 = 0;
    p->unk_3F2 = 0;
    p->unk_3EE = 0;
    p->unk_3F0 = 0;
    p->unk_3E8 = 0;
    p->unk_39C = 0;
    p->unk_394 = 0;
    p->unk_398 = 0;
    p->unk_3C4 = 0;
    p->unk_3C2 = 0;
    p->unk_3C1 = 0;
    p->unk_3C0 = 0;
    p->unk_440 = 0;
    p->unk_430 = 0;
    p->unk_3E4 = -1;
}
