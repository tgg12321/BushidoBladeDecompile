void func_80060A68(void) {
    struct Ob {
        u16 idx;
        u16 u02;
        s32 u04;
        s32 u08;
        s32 p0C;
        s32 p10;
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
    s32 idx;
    u16 temp0;
    u16 temp2;
    s32 p10;
    u16 temp_a1;
    s32 result;

    ob = (struct Ob *)D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + ob->idx * 4) = 0;
    ob->m20 = *(s32 *)(ob->p0C + 0);
    ob->m24 = *(s32 *)(ob->p0C + 4);
    ob->m28 = *(s32 *)(ob->p0C + 8);
    ob->m18 = *(u16 *)(ob->p10 + 0);
    ob->m1A = *(u16 *)(ob->p10 + 2);
    ob->m1C = *(u16 *)(ob->p10 + 4);
    D_800A3478 = (s32)&ob->m18;
    idx = ob->idx;
    D_800A347C = (s32)&ob->m20;

    result = ((s32 (*)(void)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))();
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
