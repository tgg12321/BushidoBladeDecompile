typedef int s32; typedef unsigned char u8;
extern s32 D_800F1198; extern s32 D_800F119C; extern s32 D_800F11A0;
extern s32 D_800A32B8;
void func_80062020(s32 *arg0) {
    s32 i, k, ofs, t; s32 *p;

    t = *(s32 *)((u8 *)arg0 + 0);
    D_800A32B8 = 0;
    i = 0;
    if ((t & 1) == 0) goto end;
    ofs = 0;
    do {
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
        *(s32 *)((u8 *)&D_800F1198 + ofs) = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 4);
        i = i + 1;
        *(s32 *)((u8 *)&D_800F119C + ofs) = t;
        t = *(s32 *)((u8 *)arg0 + ofs + 8);
        *(s32 *)((u8 *)&D_800F11A0 + ofs) = t;
        ofs = ofs + 12;
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
    } while ((t & 1) != 0);
end:
    ofs = i + i; ofs = ofs + i; ofs = ofs << 2;
    *(s32 *)((u8 *)&D_800F1198 + ofs) = *(s32 *)((u8 *)&D_800F119C + ofs) = *(s32 *)((u8 *)&D_800F11A0 + ofs) = 0;

}
