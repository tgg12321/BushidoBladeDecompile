/* func_8006288C — best form as of grind session 1 (2026-08-13).
   honest sandbox distance (--disable all): 2  (was 23)
   The lever: spelling the slot scan as a real do/while with an early exit
   out of the taken-slot body (instead of a goto-label loop with the latch
   in an else arm) reproduces all 9 of the target's register assignments.
   Residual: our `li $t3,1` (the sllv shift base) lands at prologue slot 5;
   target has it at slot 1.  See memory/grind/func_8006288C/evidence.md. */
s32 func_8006288C(void) {
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
    do {
        mask = 1 << i;
        if (!(D_800A3460 & mask)) {
            *(s32 *)((s32)&D_800F0FB8 + off_s32) = src_a[0];
            *(s32 *)((s32)&D_800F0FBC + off_s32) = src_a[1];
            *(s32 *)((s32)&D_800F0FC0 + off_s32) = src_a[2];
            *(u16 *)((s32)&D_800F10A0 + off_s16) = src_b[0];
            *(u16 *)((s32)&D_800F10A2 + off_s16) = src_b[1];
            D_800A3460 |= mask;
            *(u16 *)((s32)&D_800F10A4 + off_s16) = src_b[2];
            *flag_p = 0;
            goto out;
        }
        flag_p++;
        off_s16 += 8;
        i++;
        off_s32 += 0xC;
    } while (i < 6);
out:
    return 1;
}
