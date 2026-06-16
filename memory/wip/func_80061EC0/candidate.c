/* WIP candidate for func_80061EC0 — t_for_mask form, score 5
 * cheat-reviewer: PASS (2026-06-16)
 * Restoration: replace lines 14077–14104 of src/text1b.c with this body */
void func_80061EC0(s32 *arg0) {
    s32 *v1 = (s32 *)&D_800F116C;
    s32 t;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    if (D_800F1160[2] != 0) {
        if (D_800F1160[3] != 0) {
            D_800F1160[3] = 0;
            D_800F1160[2] = 0;
        }
        if (D_800F1160[2] != 0) goto check_one_zero;
    }
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)(D_800F1160 + 2);
    *(s32 *)D_800A3468 = 0x210012;
    goto end;
check_one_zero:
    if (D_800F1160[3] == 0) {
        D_800F1180 = (s32)(D_800F1160 + 3);
        *v1 = 0x210013;
    }
end:
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    t = 0xFF00FF; D_800A3464 = t;
    t = arg0[2]; D_800F1148 = t;
}
