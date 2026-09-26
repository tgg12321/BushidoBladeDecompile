extern s32 camera_GetBoneData(void);
extern u8 D_800A3208;
extern u8 *D_800A3894;

void func_8003993C(void) {
    s32 work[2][33];
    s32 sp120[34];
    s32 pos[3];
    s16 rot[3];
    s32 sp1C0[100];
    s32 out_b1;
    s32 out_b2;
    s32 idx;
    s32 prog;
    s32 i;
    u8 *p;
    PracticeMenuRec *rob;
    u8 *e;
    s32 window;
    u8 save40;
    s32 save58;

    if (D_800A3782 != 0) {
        idx = (D_800A36F8 + D_800A37D0) % 120;
        prog = (D_800A37D0 << 12) / 120;
    } else {
        idx = D_800A37D0;
        prog = (idx << 12) / D_800A36F8;
    }
    D_800A3778 = camera_GetBoneData();
    /* The frame record's address is written out at each argument (compound-address duplication,
     * no-new-park-categories 2026-08-18 F3): binding it to a `rec` local measures 20/526. */
    func_8001BAE4((u8 *)(D_800A36EC + idx * 56) + D_800A3748 * 28,
                  D_800A3748 == 0 ? (u8 *)(D_800A36EC + idx * 56) + 0x1C : (u8 *)(D_800A36EC + idx * 56), prog);
    func_8001BBD8((u8 *)(D_800A36EC + idx * 56) + D_800A3748 * 28,
                  D_800A3748 == 0 ? (u8 *)(D_800A36EC + idx * 56) + 0x1C : (u8 *)(D_800A36EC + idx * 56), prog);
    func_8001E6E4(prog);

    for (i = 0; i < 2; i++) {
        s32 sel;
        s32 i8;

        i8 = i * 8;
        p = (u8 *)(D_800A36EC + idx * 56) + (i8 - i) * 4;
        rob = &g_practice_menu_table[i];
        func_800198D0((*(s16 *)(p + 0xE) >> 14) & 3, *(s16 *)(p + 0xE) & 0x3FFF, work[0], sp1C0);
        func_800198D0((*(s16 *)(p + 0x10) >> 14) & 3, *(s16 *)(p + 0x10) & 0x3FFF, work[1], sp1C0);
        func_8001F1C4(rob, p, work[0], work[1]);
        func_80041188(i, work[0], work[1], *(s16 *)(p + 0x12), sp120);
        pos[0] = *(s16 *)(p + 4);
        pos[1] = *(s16 *)(p + 6);
        pos[2] = *(s16 *)(p + 8);
        rot[0] = 0;
        rot[1] = *(u16 *)(p + 0xA);
        rot[2] = 0;
        func_80040D48(i, 1, pos, rot, 0, *(s16 *)(p + 0xC));
        if (*(u8 *)(p + 0x18) & 1) {
            func_80049718(rob->unk_12, i * 2 + 0x8000, 0, 0);
        }
        if (*(u8 *)(p + 0x18) & 4) {
            func_80049718(D_8008EB80[rob->unk_14], i * 2 + 0x8001, 0, 0);
        }
        if (*(u8 *)(p + 0x18) & 2) {
            func_80049A2C(rob->unk_12, i * 2, (*(u8 *)(p + 0x18) >> 4) & 1);
        }
        if (*(u8 *)(p + 0x18) & 8) {
            func_80049A2C(D_8008EB80[rob->unk_14], i * 2 | 1, (*(u8 *)(p + 0x18) >> 5) & 1);
        }
        func_800207C8(rob, 0x1F8000A8 + i * 0x108, 0x1F800000 + (i8 + i) * 4, 0x1F800048 + i * 24);
        func_8003984C((s32 *)rob, &out_b1, &out_b2);
        if (out_b1 != 1) {
            *(u8 *)((u8 *)rob + 0xB1) = out_b1;
            if (out_b2 != -1) {
                *(u8 *)((u8 *)rob + 0xB2) = out_b2;
            }
        }
        save40 = *(u16 *)((u8 *)rob + 0x40);
        *(u16 *)((u8 *)rob + 0x40) = *(u8 *)(p + 0x19);
        save58 = *(s32 *)((u8 *)rob + 0x58);
        if (*(u8 *)(p + 0x17) & 1) {
            s32 entry_a;

            sel = i8; /* FAKE */
            entry_a = D_80102764 + *(u16 *)(*(s32 *)p + 4) * 4;
            *(s32 *)((u8 *)rob + 0x58) = D_80102768 + *(u16 *)(entry_a + 2);
        } else {
            s32 entry_b;

            sel = (*(u8 *)(p + 0x17) >> 1) & 1;
            entry_b = D_801027B0[sel][1] + *(u16 *)(*(s32 *)p + 4) * 4;
            *(s32 *)((u8 *)rob + 0x58) = D_801027B0[sel][2] + *(u16 *)(entry_b + 2);
        }
        if (*(u8 *)(p + 0x18) & 0x40) {
            cpu_check_same_dir_timer(rob);
        }
        *(u16 *)((u8 *)rob + 0x40) = save40;
        *(s32 *)((u8 *)rob + 0x58) = save58;
        func_80040304(i, ((*(u16 *)(p + 0xA) >> 12) & 7) + ((sel & 1) >> 1)); /* FAKE */
    }

    e = &D_80101BF0;
    for (i = 0; i < 0x20; i++, e += 0x10) {
        if (e[0] == idx) {
            pos[0] = *(s16 *)(e + 4);
            pos[1] = *(s16 *)(e + 6);
            pos[2] = *(s16 *)(e + 8);
            rot[0] = *(u16 *)(e + 0xA);
            rot[1] = *(u16 *)(e + 0xC);
            rot[2] = *(u16 *)(e + 0xE);
            D_800A3208 = 1;
            func_80032854(e[1], e[2], pos, rot);
            D_800A3208 = 0;
        }
    }

    if (D_800A3782 != 0) {
        window = 0x77 - D_800A37D0;
    } else {
        /* FAKE: named intermediate (Ruling 1, once-written). Unnamed, fold-const.c `associate`
         * (split_tree) rewrites D_800A36F8 - (D_800A37D0 + 1) as (D_800A36F8 - 1) - D_800A37D0 at
         * tree level; the target adds 1 to the counter first (addiu; subu). Without it: 2/526. */
        s32 next = D_800A37D0 + 1;
        window = D_800A36F8 - next;
    }
    e = (u8 *)D_800F68E0;
    for (i = 0; i < 0xB4; i++, e += 0x10) {
        if (*(s16 *)e >= window && *(s16 *)e - e[2] <= window) {
            pos[0] = *(s16 *)(e + 4);
            pos[1] = *(s16 *)(e + 6);
            pos[2] = *(s16 *)(e + 8);
            rot[0] = *(u16 *)(e + 0xA);
            rot[1] = *(u16 *)(e + 0xC);
            rot[2] = *(u16 *)(e + 0xE);
            func_80049718(e[3], 1, pos, rot);
        }
    }

    func_80046DA8(1);
    func_800335D8();
    D_800A37D0++;
    if ((D_800A3782 != 0 ? D_800A37D0 == 0x78 : D_800A37D0 == D_800A36F8) || (D_80102788.pressed & 0x400040)) {
        switch (D_800A38DC) {
        case 0:
            if (D_80101F5E == 0) {
                s32 valid = D_800A3836 != 0xFF;
                if (D_800A3712 != 0) {
                    D_800A38D4 = 2;
                    if (D_800A3836 != 0xFF) {
                        D_800A37A4 |= 1 << D_8008D538[(s8)D_80102778.unk_4[0]];
                    }
                    if (valid) {
                        func_8003B328();
                    }
                }
                func_8001DA2C();
                if (D_800A3894 != 0) {
                    if (D_800A3712 != 0) {
                        func_8003B534(valid ? 3 : 6);
                    } else {
                        func_8003B484(D_800A3894 + 6);
                        func_8003B534(valid ? 2 : 5);
                    }
                } else {
                    func_8003B5A4();
                }
                return;
            }
            D_800A3748 = 1;
            if (D_800A3836 == 0xFF) {
                D_800A3834 = 0xC;
                break;
            }
            func_8001DA2C();
            func_8003B328();
            func_8003AF40(0);
            func_8003AFFC();
            if (D_800A3712 != 0 && D_801023AA != 0) {
                func_8003B534(6);
            } else {
                func_8003B534(4);
            }
            return;
        case 3:
            if (D_80101F5E == 0) {
                D_800A3834 = 0x1E;
            } else {
                D_800A3834 = 0xC;
            }
            break;
        default:
            if (D_80101F5E == 0 || D_801023AA == 0) {
                D_800A3834 = 0x10;
            } else {
                D_800A3834 = 0xC;
            }
            break;
        }
    }
}
