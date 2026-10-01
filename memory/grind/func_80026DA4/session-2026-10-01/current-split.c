void func_80026DA4(void) {
    PracticeMenuRec *record;
    PracticeMenuRec *partner;
    PracticeMenuRec *current;
    PracticeMenuRec *selected;
    PracticeMenuRec *other;
    s32 timer;
    s32 i;
    s32 idx;
    s32 dir;
    s32 kind;
    s32 pos[3];

    timer = func_8002BEA0();
    record = g_practice_menu_table;
    D_800A3824 = -1;
    if ((u16)g_practice_menu_table[0].unk_6A == 0x1C) {
        if (g_practice_menu_table[0].unk_40 != 4) goto tail;
        idx = D_800A3876;
        if (idx == -1) goto tail;
        /* FAKE: reuse the record pointers for the selected pair, then restore
         * the fixed pair at the tail join. This keeps the selected pointers
         * in the call-preserved allocation used by the original.
         * SOTN: src/st/lib/unk_3B53C.c:41-55 @db41b28eee52969244a52cc269c8163d1ed8826a */
        selected = &g_practice_menu_table[idx];
        other = g_practice_menu_table;
        if (idx == 0) {
            other++;
        }
        selected->unk_286 = 3;
        other->unk_286 = 4;
    } else if ((u16)g_practice_menu_table[0].unk_6A != 0xF) {
        for (i = 0; i < 2; i++) {
            current = record + i;
            if (current->unk_30 & 0x20) {
                current->unk_28C += current->unk_20;
            }
            if (current->unk_30 & 0x40) {
                current->unk_28C += current->unk_20;
            }
            if ((u16)current->unk_6A == 0x1D || (u16)current->unk_6A == 0x1E ||
                (u16)current->unk_6A == 0x20) {
                if (current->unk_2C & 0x1000) {
                    dir = 1;
                } else if (current->unk_2C & 0x4000) {
                    dir = -1;
                } else {
                    dir = 0;
                }
                current->unk_134.vx -= (Judge[(current->unk_1C8.vy + 0x400) & 0xFFF] * dir) / 256;
                current->unk_134.vz += (Judge[(u16)current->unk_1C8.vy & 0xFFF] * dir) / 256;
            }
        }
        /* FAKE: re-materialize the same record base after the drift loop;
         * GCC cse.c's basic-block boundary retains the original base load.
         * SOTN: src/weapon/w_011.c:343-348 @db41b28eee52969244a52cc269c8163d1ed8826a */
        record = g_practice_menu_table;
        partner = record + 1;
        D_800A389C++;
        if ((s16)D_800A389C >= 0x2B) {
            if (g_practice_menu_table[0].unk_28C > g_practice_menu_table[1].unk_28C) {
                g_practice_menu_table[0].unk_286 = 3;
                g_practice_menu_table[1].unk_286 = 4;
                if ((u16)g_practice_menu_table[1].unk_6A == 0x21) {
                    func_80027A58((s32 *)partner);
                }
                func_80032854(1, 0x2D, (u8 *)&partner->unk_F4, (s16 *)0);
            } else if (g_practice_menu_table[1].unk_28C > g_practice_menu_table[0].unk_28C) {
                g_practice_menu_table[1].unk_286 = 3;
                g_practice_menu_table[0].unk_286 = 4;
                if ((u16)g_practice_menu_table[0].unk_6A == 0x21) {
                    func_80027A58((s32 *)record);
                }
                func_80032854(0, 0x2D, (u8 *)&record->unk_F4, (s16 *)0);
            }
            partner->unk_28C = 0;
            record->unk_28C = 0;
            D_800A389C = 0;
        }
        if (timer > 200) {
            record->unk_286 = 2;
            partner->unk_286 = 2;
        } else {
            s32 diff = record->unk_F4.y - partner->unk_F4.y;
            if (diff < 0) diff = -diff;
            if (diff >= 1000) {
                record->unk_286 = 2;
                partner->unk_286 = 2;
            }
        }
        if ((u16)record->unk_6A == 0x1D) {
            if (record->unk_2C & 0x8000) {
                if (partner->unk_2C & 0x8000) {
                    record->unk_286 = 2;
                    partner->unk_286 = 2;
                } else {
                    record->unk_286 = 2;
                    partner->unk_286 = 5;
                }
            } else if (partner->unk_2C & 0x8000) {
                record->unk_286 = 5;
                partner->unk_286 = 2;
            }
        }
    }
    /* FAKE: restore record zero at the three-path join after selected-pair
     * work; preserve the pointer reuse documented above.
     * SOTN: src/st/lib/unk_3B53C.c:41-55 @db41b28eee52969244a52cc269c8163d1ed8826a */
    record = g_practice_menu_table;
tail:
    /* FAKE: restore the second fixed record with the first at this join.
     * SOTN: src/st/lib/unk_3B53C.c:41-55 @db41b28eee52969244a52cc269c8163d1ed8826a */
    partner = record + 1;
    if (D_800A3910 == 0) {
        switch ((u16)g_practice_menu_table[0].unk_6A) {
        case 0xF:
            kind = -1;
            break;
        case 0x1C:
            kind = 0;
            break;
        case 0x21:
            kind = 1;
            break;
        case 0x20:
            kind = 2;
            break;
        case 0x1D:
            kind = 3;
            break;
        case 0x1E:
            kind = 4;
            break;
        case 0x1F:
            kind = 5;
            break;
        }
        if (kind >= 0) {
            pos[0] = (record->unk_F4.x + partner->unk_F4.x) / 2;
            pos[1] = (record->unk_D8.y + partner->unk_D8.y) / 2;
            pos[2] = (record->unk_F4.z + partner->unk_F4.z) / 2;
            pos[0] += (Judge[(u16)record->unk_1D8 & 0xFFF] * D_8008EB54[kind].unk0) >> 12;
            pos[1] += D_8008EB54[kind].unk2;
            pos[2] += (Judge[(record->unk_1D8 + 0x400) & 0xFFF] * D_8008EB54[kind].unk0) >> 12;
            func_80032854(0, D_8008EB6C[kind], (u8 *)pos, (s16 *)0);
        }
    } else {
        D_800A3910--;
    }
}
