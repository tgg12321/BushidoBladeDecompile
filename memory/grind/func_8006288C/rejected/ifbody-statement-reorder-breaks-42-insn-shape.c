typedef signed int s32;
typedef unsigned short u16;
typedef signed short s16;

extern s32 D_800A3460;
extern s32 D_800A347C;
extern s32 D_800A3478;
extern s16 D_800F0C04;
extern s32 D_800F0FB8;
extern s32 D_800F0FBC;
extern s32 D_800F0FC0;
extern s32 D_800F10A0;
extern s32 D_800F10A2;
extern s32 D_800F10A4;
extern s32 D_800F1138;

s32 func_8006288C(void) {
    int new_var;
    s16 *flag_p;
    s32 *src_a;
    u16 *src_b;
    s32 i;
    s32 off_s32;
    s32 off_s16;
    s32 mask;

    i = 0;
    flag_p = &D_800F0C04;
    off_s16 = 0;
    off_s32 = 0;
    src_a = (s32 *)D_800A347C;
    src_b = (u16 *)D_800A3478;
    D_800F1138 = 1;
    new_var = 1;
loop_top:
    mask = new_var << i;
    if (!(D_800A3460 & mask)) {
        *(u16 *)((s32)&D_800F10A0 + off_s16) = src_b[0];
        *(u16 *)((s32)&D_800F10A2 + off_s16) = src_b[1];
        *(u16 *)((s32)&D_800F10A4 + off_s16) = src_b[2];
        *(s32 *)((s32)&D_800F0FB8 + off_s32) = src_a[0];
        *(s32 *)((s32)&D_800F0FBC + off_s32) = src_a[1];
        *(s32 *)((s32)&D_800F0FC0 + off_s32) = src_a[2];
        D_800A3460 |= mask;
        *flag_p = 0;
    } else {
        flag_p++;
        off_s16 += 8;
        i++;
        off_s32 += 0xC;
        if (i < 6) goto loop_top;
    }
    return 1;
}
