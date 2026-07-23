/* func_8006156C (text1b.c) — MATCH, sandbox distance 0, ZERO rules / ZERO cheat-asm.
 * Closing lever: walking-pointer copy (*p++) of the 3 coordinate words + mask
 * store LAST — identical idiom to the COMPLETED-C sibling func_800618B4.
 * The *p++ pointer dependence serializes the loads (GCC keeps per-element lw/sw);
 * the mask (0xFF8080) store placed last lets its lui/ori schedule into the freed
 * load-delay slots of the 2nd/3rd loads, exactly as target does.
 * See .claude/rules/walking-pointer-serializes-parallel-loads.md */
void func_8006156C(s32 *arg0) {
    s32 *v1 = (s32 *)&D_800F116C;
    s32 *p;
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
    p = arg0;
    D_800F1140 = *p++;
    D_800F1144 = *p++;
    D_800F1148 = *p;
    D_800A3464 = 0xFF8080;
}
