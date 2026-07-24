/* REJECTED (distance 7, not 0) — plain for-loop, inits before the loop.
 *
 * Produces the phantom slot (vars=8, frame 0x30 correct) via loop.c's guard
 * pseudo, BUT the `s16 *s0 = arg0;` / `new_var = &buf` inits sit BEFORE the
 * for-loop in source, so GCC schedules `move s0,a0` and the `lui/addiu s3`
 * table-base setup into the prologue BEFORE the blez guard. The target places
 * both AFTER the guard (blez, then sw s0 in delay slot, then lui/addiu s3,
 * then move s0). Result: 7 prologue-scheduling diffs. Measured variants v14/v15
 * (base inline in arms / for-init comma) have the SAME defect.
 *
 * The fix is the guarded-do-while (candidate.c): put the inits AFTER an
 * explicit `if (i < arg1)` guard so they land in the post-guard preheader,
 * matching the target's structure exactly. Distance 0.
 */
extern s32 g_file_data_buf;
void func_8001924C(s16 *arg0, s32 arg1) {
    s32 i;
    s16 *s0 = arg0;
    s32 new_var = (s32)&g_file_data_buf;

    for (i = 0; i < arg1; i++) {
        if (*(u8 *)((u8 *)s0 + 2) & 1) {
            s16 val = s0[0];
            func_80019310(s0, (s32 *)(val * 52 + new_var));
        } else {
            s16 val = s0[0];
            single_game_setModeRequest(s0, (s32 *)(val * 52 + new_var));
        }
        s0 = (s16 *)((u8 *)s0 + 16);
    }
}
