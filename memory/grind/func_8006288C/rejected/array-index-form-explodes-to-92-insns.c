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
    src_a = (s32 *)D_800A347C;
    src_b = (u16 *)D_800A3478;
    D_800F1138 = 1;
loop_top:
    mask = 1 << i;
    if (!(D_800A3460 & mask)) {
        (&D_800F0FB8)[i * 3 + 0] = src_a[0];
        (&D_800F0FB8)[i * 3 + 1] = src_a[1];
        (&D_800F0FB8)[i * 3 + 2] = src_a[2];
        ((u16 *)&D_800F10A0)[i * 4 + 0] = src_b[0];
        ((u16 *)&D_800F10A0)[i * 4 + 1] = src_b[1];
        D_800A3460 |= mask;
        ((u16 *)&D_800F10A0)[i * 4 + 2] = src_b[2];
        (&D_800F0C04)[i] = 0;
    } else {
        i++;
        if (i < 6) goto loop_top;
    }
    return 1;
}
