/* REJECTED — `s32 pad[2];` unused-local-array frame coercion (HEAD's cheat).
 * Measured s1 2026-07-23: minrepro with pad -> `.frame $sp,48 # vars=8` = target
 * frame 0x30; byte-matches the full build (oracle green) but sandbox --disable all
 * strips it -> honest 12. FORBIDDEN per [[dead-vars-local-array]] /
 * [[inline-asm-policy]] expanded catalog (unused fixed-size local array to reserve
 * frame bytes). Target writes ZERO of the 8 hole bytes, so the 2026-07-01
 * written-never-read carve-out does NOT apply. Dead by policy, not by measurement.
 */
extern s32 g_file_data_buf;
void func_8001924C(s16 *arg0, s32 arg1) {
    s32 i;
    s16 *s0;
    s32 pad[2];   /* CHEAT: reserves the target's 8-byte phantom frame slot */
    s32 new_var;

    i = 0;
    if (arg1 <= 0) return;
    new_var = (s32)&g_file_data_buf;
    s0 = arg0;
    do {
        if (*(u8 *)((u8 *)s0 + 2) & 1) {
            s16 val = s0[0];
            func_80019310(s0, (s32 *)(val * 52 + new_var));
        } else {
            s16 val = s0[0];
            single_game_setModeRequest(s0, (s32 *)(val * 52 + new_var));
        }
        i++;
        s0 = (s16 *)((u8 *)s0 + 16);
    } while (i < arg1);
}
