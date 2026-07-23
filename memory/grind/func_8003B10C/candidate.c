/* func_8003B10C — floor candidate, sandbox --disable all = 8 (HEAD was 10).
 *
 * This is the HOISTED family (61 insns; ED6 read ONCE, cross-jump-merged into
 * the shared tail). Pure C, no cheats: e2/v0/tbl all live and used, ED6 read
 * inline. It is the lowest sandbox NUMBER but a MATCHING DEAD-END: target is
 * 64 insns and duplicates the ED6 read per-arm, so any single-read (61-insn)
 * form is structurally 3 insns short of 0 and can never byte-match.
 *
 * THE 0-PATH IS NOT THIS FORM. The 0-path is the DUPLICATED chassis saved at
 * tmp/grind/func_8003B10C/s2/dup13_form.c (sandbox = 13, 64 insns, ED6 read
 * per-arm). Its ENTIRE residual is two register assignments: tbl lands in a1
 * (target a0) and ED6 lands in a0 (target v1), because sched1 hoists the ED6
 * load ABOVE the ED2 multiply (where v1 still holds ED2) instead of after it
 * (target reads ED6 after the mult, reusing the freed v1). See the frontier.
 *
 * KEY STRUCTURAL WIN THIS SESSION: recompute-inline of `arg0 * 1100` at every
 * offset use (no `s0` variable) ELIMINATES the cse.c canon_reg web split that
 * cost v9 the `move s2,s0` copy + frame -40. With recompute-inline the offset
 * is a single CSE temp (s0), frame is -32, and the 493E4 delay-slot nop is
 * restored. The `e2` temp fixes the if-arm ED2->v1 + mult register order.
 */
void func_8003B10C(s32 arg0) {
    s32 addr = (s32)0x80190800;
    s32 v0;
    s32 e2;
    u8 *tbl;

    gpu_EnableDisplay();
    EndADRSound();
    gnd_close_8004939C();

    func_800493E4(*(s16 *)((u8 *)&D_80101EDA + arg0 * 1100));

    if (D_800A38DC == 5) {
        tbl = &D_8008E6A4;
        e2 = *(s16 *)((u8 *)&D_80101ED2 + arg0 * 1100);
        v0 = e2 * 6;
        func_800494D4(arg0, *(tbl + v0 + *(s16 *)((u8 *)&D_80101ED6 + arg0 * 1100)));
    } else {
        tbl = &D_8008E5CC;
        e2 = *(s16 *)((u8 *)&D_80101ED2 + arg0 * 1100);
        v0 = e2 * 8;
        func_800494D4(arg0, *(tbl + v0 + *(s16 *)((u8 *)&D_80101ED6 + arg0 * 1100)));
    }
    func_80049584(addr);
}
