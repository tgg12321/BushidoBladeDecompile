extern void *D_800A347C;
extern s32 D_800F0CA0;
extern s32 D_800F0CA4;
extern s32 D_800F0CA8;
extern s32 D_800F10E0;
extern s16 D_800F0BA8;
void func_80064E90(void) {
    s32 *p = (s32 *)D_800A347C;
    s32 t;
    t = p[0]; D_800F0CA0 = t;
    t = p[1]; D_800F0CA4 = t;
    p = (s32 *)p[2];
    D_800F10E0 = 1;
    D_800F0BA8 = 0;
    D_800F0CA8 = (s32)p;
}
