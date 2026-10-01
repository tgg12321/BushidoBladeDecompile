void func_80026DA4(void) {
    extern void func_80027A58(s32 *);
    extern void func_80032854(s32, s32, u8 *, s16 *);
    PracticeMenuRec *s0;
    PracticeMenuRec *s1;
    PracticeMenuRec *p;
    s32 timer;
    s32 i;
    s32 idx;
    s32 dir;
    s32 kind;
    s32 pos[3];

    timer = func_8002BEA0();
    s0 = g_practice_menu_table;
    D_800A3824 = -1;
    if (g_practice_menu_table[0].unk_6A == 0x1C) {
        if (g_practice_menu_table[0].unk_40 != 4) goto tail;
        idx = D_800A3876;
        if (idx == -1) goto tail;
        s0 = &g_practice_menu_table[idx];
        s1 = g_practice_menu_table;
        if (idx == 0) {
            s1++;
        }
        s0->unk_286 = 3;
        s1->unk_286 = 4;
    } else if (g_practice_menu_table[0].unk_6A != 0xF) {
        for (i = 0; i < 2; i++) {
            p = s0 + i;
            if (p->unk_30 & 0x20) {
                p->unk_28C += p->unk_20;
            }
            if (p->unk_30 & 0x40) {
                p->unk_28C += p->unk_20;
            }
            if (p->unk_6A == 0x1D || p->unk_6A == 0x1E ||
                p->unk_6A == 0x20) {
                if (p->unk_2C & 0x1000) {
                    dir = 1;
                } else if (p->unk_2C & 0x4000) {
                    dir = -1;
                } else {
                    dir = 0;
                }
                p->unk_134.vx -= (Judge[(p->unk_1C8.vy + 0x400) & 0xFFF] * dir) / 256;
                p->unk_134.vz += (Judge[(u16)p->unk_1C8.vy & 0xFFF] * dir) / 256;
            }
        }
        s0 = g_practice_menu_table;
        s1 = s0 + 1;
        D_800A389C++;
        if ((s16)D_800A389C >= 0x2B) {
            if (g_practice_menu_table[0].unk_28C > g_practice_menu_table[1].unk_28C) {
                g_practice_menu_table[0].unk_286 = 3;
                g_practice_menu_table[1].unk_286 = 4;
                if (g_practice_menu_table[1].unk_6A == 0x21) {
                    func_80027A58((s32 *)s1);
                }
                func_80032854(1, 0x2D, (u8 *)&s1->unk_F4, 0);
            } else if (g_practice_menu_table[1].unk_28C > g_practice_menu_table[0].unk_28C) {
                g_practice_menu_table[1].unk_286 = 3;
                g_practice_menu_table[0].unk_286 = 4;
                if (g_practice_menu_table[0].unk_6A == 0x21) {
                    func_80027A58((s32 *)s0);
                }
                func_80032854(0, 0x2D, (u8 *)&s0->unk_F4, 0);
            }
            s1->unk_28C = 0;
            s0->unk_28C = 0;
            D_800A389C = 0;
        }
        if (timer > 200) {
            s0->unk_286 = 2;
            s1->unk_286 = 2;
        } else {
            s32 diff = s0->unk_F4.y - s1->unk_F4.y;
            if (diff < 0) diff = -diff;
            if (diff >= 1000) {
                s0->unk_286 = 2;
                s1->unk_286 = 2;
            }
        }
        if (s0->unk_6A == 0x1D) {
            if (s0->unk_2C & 0x8000) {
                if (s1->unk_2C & 0x8000) {
                    s0->unk_286 = 2;
                    s1->unk_286 = 2;
                } else {
                    s0->unk_286 = 2;
                    s1->unk_286 = 5;
                }
            } else if (s1->unk_2C & 0x8000) {
                s0->unk_286 = 5;
                s1->unk_286 = 2;
            }
        }
    }
    s0 = g_practice_menu_table;
tail:
    s1 = s0 + 1;
    if (D_800A3910 == 0) {
        switch (g_practice_menu_table[0].unk_6A) {
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
            pos[0] = (s0->unk_F4.x + s1->unk_F4.x) / 2;
            pos[1] = (s0->unk_D8.y + s1->unk_D8.y) / 2;
            pos[2] = (s0->unk_F4.z + s1->unk_F4.z) / 2;
            pos[0] += (Judge[(u16)s0->unk_1D8 & 0xFFF] * D_8008EB54[kind].unk0) >> 12;
            pos[1] += D_8008EB54[kind].unk2;
            pos[2] += (Judge[(s0->unk_1D8 + 0x400) & 0xFFF] * D_8008EB54[kind].unk0) >> 12;
            func_80032854(0, D_8008EB6C[kind], (u8 *)pos, 0);
        }
    } else {
        D_800A3910--;
    }
}
INCLUDE_RODATA("asm/rodata", D_80010478);
