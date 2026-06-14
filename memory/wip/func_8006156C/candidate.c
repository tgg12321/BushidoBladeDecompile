/* func_8006156C (text1b.c) — clean candidate, floor 6, ZERO rules / ZERO cheat-asm.
 * Resume: paste this over the function in src/text1b.c, confirm sandbox 6. */
void func_8006156C(s32 *arg0) {
    s32 *v1 = (s32 *)&D_800F116C;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    if ((&D_800F1154)[1] != 0) {
        if ((&D_800F1154)[2] != 0) {
            (&D_800F1154)[2] = 0;
            (&D_800F1154)[1] = 0;
        }
        if ((&D_800F1154)[1] != 0) goto check_one_zero;
    }
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)((&D_800F1154) + 1);
    *(s32 *)D_800A3468 = 0x210005;
    goto end;
check_one_zero:
    if ((&D_800F1154)[2] == 0) {
        D_800F1180 = (s32)((&D_800F1154) + 2);
        *v1 = 0x210006;
    }
end:
    func_80060A68();
    {
        s32 t;
        D_800A3464 = 0xFF8080;
        t = arg0[0]; D_800F1140 = t;
        t = arg0[1]; D_800F1144 = t;
        t = arg0[2]; D_800F1148 = t;
    }
}
