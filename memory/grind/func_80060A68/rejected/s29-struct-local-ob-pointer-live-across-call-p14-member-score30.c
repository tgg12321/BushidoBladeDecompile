void func_80060A68(void) {
    struct Ob {
        u16 idx;
        u16 u02;
        s32 u04;
        s32 u08;
        s32 *p0C;
        u16 *p10;
        s32 p14;
        u16 m18;
        u16 m1A;
        u16 m1C;
        u16 u1E;
        s32 m20;
        s32 m24;
        s32 m28;
    };
    extern s32 D_800A3468;
    extern s32 D_800A3478;
    extern s32 D_800A347C;
    extern s32 D_800A32BC;
    extern u8 D_8009BA60;
    extern s32 D_800F10D0;
    extern s32 chractar_use_pset_combo_id_table;
    struct Ob *ob;
    s32 result;

    ob = (struct Ob *)D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + ob->idx * 4) = 0;
    ob->m20 = ob->p0C[0];
    ob->m24 = ob->p0C[1];
    ob->m28 = ob->p0C[2];
    ob->m18 = ob->p10[0];
    ob->m1A = ob->p10[1];
    D_800A3478 = (s32)&ob->m18;
    ob->m1C = ob->p10[2];
    D_800A347C = (s32)&ob->m20;

    result = ((s32 (*)(void)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + ob->idx) + *(s32 *)((s32)&D_800F10D0 + ob->idx * 4)) * 4))();
    *(s8 *)ob->p14 = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
