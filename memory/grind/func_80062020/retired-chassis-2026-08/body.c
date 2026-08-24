void func_80062020(s32 *arg0) {
    register s32 *a0 asm("$4") = arg0;
    register s32 i asm("$5");
    register s32 ofs asm("$3");
    register s32 t asm("$2");
    s32 *p;
    s32 i12;
    t = a0[0];
    D_800A32B8 = 0;
    i = 0;
    if ((t & 1) == 0) goto end;
    ofs = 0;
    do {
        t = a0[0];
        *(s32 *)((u8 *)&D_800F1198 + ofs) = t;
        t = a0[1];
        i = i + 1;
        *(s32 *)((u8 *)&D_800F119C + ofs) = t;
        t = a0[2];
        a0 = (s32 *)((u8 *)a0 + 12);
        *(s32 *)((u8 *)&D_800F11A0 + ofs) = t;
        t = a0[0];
        ofs = ofs + 12;
    } while ((t & 1) != 0);
end:
    i12 = i + i;
    i12 = i12 + i;
    i12 = i12 << 2;
    p = (s32 *)((u8 *)&D_800F1198 + i12);
    p[2] = 0;
    p[1] = 0;
    *(s32 *)((u8 *)&D_800F1198 + i12) = 0;
}
