void func_8003CF84(void) {
    /* FAKE: unwritten leading pad ([[dead-vars-local-array]] re-scoped carve-out, owner rulings 2026-08-17 + 2026-08-18): reconstructs the original frame's 16-byte allocated-but-untouched leading region (compiled-out >=7-word call, frame forensics in memory/wip/func_8003CF84/notes.md). SOTN-master precedent: volatile u32 pad[4]; // FAKE at st/sel/stream.c:80. */
    volatile u32 pre_pad[4];
    s32 vec[3];
    /* FAKE: unwritten TRAILING pad (owner ruling 2026-08-18, this function only): the target frame's census shows a second 8-byte allocated-but-untouched object above vec; 14 honest spellings + all 3 phantom-slot producers measured inert (notes.md). */
    volatile u32 pad2[2];
    s32 *a;
    s32 *b;
    s32 flag = 0;
    s8 p;
    s16 stage;

    func_800335D8();
    p = D_800A3748;
    stage = g_practice_menu_table[p].unk_0A;
    if (D_800A37B8 == D_8008EAC0[stage]) {
        func_8005C650(40 * p + 0x2D, 0x7F, 0x7F);
    }
    if (D_800A37B8 == D_8008EB04) {
        func_8005C650(40 * D_800A3748 + 0x31, 0x7F, 0x7F);
    }
    if (D_800A37B8 == D_8008EB06) {
        func_8005C650(40 * D_800A3748 + 0x36, 0x7F, 0x7F);
    }
    if (D_800A37B8 == D_8008EB08) {
        if (D_800A3748 == 0) {
            func_8005C650(0x53, 0x7F, 0x7F);
        } else {
            func_8005C650(0x2B, 0x7F, 0x7F);
        }
    }
    if (D_800A37B8 == D_8008EB0A) {
        func_8005C650(0x71, 0x7F, 0x7F);
    }
    if (D_800A37B8 == D_8008EB0C) {
        func_80021D10(0, vec, D_800A3818);
        vec[0] += D_8008EB10;
        vec[1] += D_8008EB14;
        vec[2] += D_8008EB18;
        func_800618B4(vec, &D_800A312C);
    }
    a = func_8005507C();
    b = func_8005508C();
    func_80061064(a, b);
    if (func_80054F68() == 0) {
        flag = 1;
    }
    if (flag != 0 || (D_80102788.pressed & 0x400040) != 0) {
        func_800548DC();
        if (D_800A38DC == 4 || D_800A38DC == 6) {
            /* FAKE: indexes past D_800A37D2 into D_800A37D3 by player number (owner Q63, this
             * byte pair only): the target also reaches each byte through its own symbol, which
             * no single array or struct gives (proof: memory/grind/func_8001C8DC/evidence.md
             * s1, s2-struct). */
            (&D_800A37D2)[D_800A3748] = (&D_800A37D2)[D_800A3748] + 1;
        }
        func_8001979C(0, (u32 *)D_80102770);
        func_8001979C(1, (u32 *)D_801027B0[0][4]);
        func_8001979C(2, (u32 *)D_801027B0[1][4]);
        if (D_800A38DC == 0 && (u8)D_800A3836 != 0xFF) {
            func_8001DA2C();
            func_8003B328();
            func_8003AF40(0);
            func_8003AFFC();
            func_8003B534(4);
        } else {
            D_800A3834 = 0x18;
        }
    }
    D_800A37B8 = D_800A37B8 + 1;
}
