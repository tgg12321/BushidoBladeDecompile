/* func_800618B4 (text1b.c) — text transform-block dispatch. HEAD body.
 * CARRIES CHEATS (NOT a clean candidate): 2 register pins
 * `register s32 t asm("$2")` + `register s32 mask asm("$3")` AND 1 regfix
 * subst `$2 <-> $3 @42-46`. These force target's tail register-rotation
 * (arg0[0]/[1] in v0, arg0[2] in v1, mask in v0). No pure-C form found that
 * reaches that allocation without the pins. See notes.md. Shared cluster
 * infrastructure across ~10 text1b siblings. */
void func_800618B4(s32 *arg0, s32 arg1) {
    s32 *v1 = (s32 *)&D_800F116C;
    u8 *new_var;
    register s32 t asm("$2");
    register s32 mask asm("$3");
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    D_800F117C = arg1;
    new_var = D_800F1152;
    if (new_var[0] != 0) {
        if (D_800F1152[1] != 0) {
            D_800F1152[1] = 0;
            D_800F1152[0] = 0;
        }
        if (new_var[0] != 0) goto check_one_zero;
    }
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)D_800F1152;
    *(s32 *)D_800A3468 = 0x210002;
    goto end;
check_one_zero:
    if (D_800F1152[1] == 0) {
        D_800F1180 = (s32)(new_var + 1);
        *v1 = 0x210003;
    }
end:
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    mask = 0xFF0000;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
