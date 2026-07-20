/* s2 candidate — distance 2 (NEW attractor: post-call lw-swap residual).
 * Block 0 SOLVED: arg1 (retyped s32, byte-neutral per H2) double-set via the
 * arm-1 D_800A3468 reload -> birthing boost dead -> move v1,a1 at target slot 3.
 * val carries sp20[2] repack + arm-1 0x10016 const -> const gets $a0 (globalized
 * past local-alloc). All registers match target; sole residual: lw v1,0x24 /
 * lw a0,0x28 emitted swapped (val's load lost its LAUNCH boost; sched1 pick
 * {v1lw(LAUNCH) vs a0lw(1)} — proven immovable by source order).
 */
void func_80061C00(s32 arg0, s32 arg1, s32 arg2) {
    s16 sp10[4];
    s16 sp18[4];
    s32 sp20[3];
    u8 sp30[32];
    s32 sp50;
    s32 val;

    D_800A3468 = (s32)&D_800F116C;
    if (arg2 != 1) {
        arg2 = 0;
    }
    sp10[1] = -0xA00;
    sp10[0] = 0;
    sp10[2] = 0xA00;
    sp20[2] = 0;
    sp20[1] = 0;
    sp20[0] = 0;
    sp18[1] = arg1;
    sp18[2] = 0;
    sp18[0] = 0;
    motutil_GetWalkDir(sp18, sp30);
    *(s32 *)(sp30 + 0x1C) = 0;
    *(s32 *)(sp30 + 0x18) = 0;
    *(s32 *)(sp30 + 0x14) = 0;
    gte_SetRotMatrix(sp30);
    gte_SetTransVector(sp30);
    func_8007F2AC(sp10, sp20, &sp50);
    sp10[0] = (s16)sp20[0];
    sp10[1] = (s16)sp20[1];
    val = sp20[2];
    sp10[2] = (s16)val;
    *(s32 *)(D_800A3468 + 0xC) = arg0;
    *(s32 *)(D_800A3468 + 0x10) = (s32)sp10;
    if ((D_800F1164 + 2)[0] != 0) {
        if ((D_800F1164 + 2)[1] != 0) {
            (D_800F1164 + 2)[1] = 0;
            (D_800F1164 + 2)[0] = 0;
        }
        if ((D_800F1164 + 2)[0] != 0) goto check_one_zero;
    }
    arg1 = D_800A3468;
    *(s32 *)(arg1 + 0x14) = (s32)(D_800F1164 + 2);
    val = 0x10016;
    *(s32 *)arg1 = val;
    D_800A34F0 = arg2;
    goto end;
check_one_zero:
    if ((D_800F1164 + 2)[1] == 0) {
        *(s32 *)(D_800A3468 + 0x14) = (s32)(D_800F1164 + 3);
        *(s32 *)D_800A3468 = 0x10017;
        D_800A34F2 = arg2;
    }
end:
    func_80060A68();
}
