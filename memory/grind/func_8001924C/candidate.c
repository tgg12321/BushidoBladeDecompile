/* func_8001924C (src/code6cac.c) — MATCH, sandbox --disable all = 0 (s2, 2026-07-23)
 *
 * Honest pure-C distance 0: 0 rules dropped, 49 == 49 insns, byte-exact.
 * NO cheat: no pad array, no dead local, no pin, no volatile, no asm.
 *
 * The +8 locals slot (frame 0x30 vs the phantom-less 0x28) that made the
 * structural twin func_80049A2C owner-gated is reproduced here in pure C by
 * the natural GUARDED-DO-WHILE form. Mechanism (measured, s2 greg + RTL dumps):
 *   - Writing the loop as `if (i < arg1) { init; do {..} while (i < arg1); }`
 *     makes GCC's loop pass (loop.c) generate the loop-entry guard by
 *     DUPLICATING the exit test against the counter pseudo:
 *         (insn 111  reg101 = (i < arg1))     ; slt into a pseudo
 *         (jump 112  if reg101==0 goto end)
 *   - cse2/combine later folds `0 < arg1` -> `blez $s2` (i is known 0), which
 *     KILLS reg101, but flow's reg_n_refs stays stale -> alter_reg reserves an
 *     8-byte stack slot for the now-dead guard pseudo = the target's phantom.
 *   - Spelling the guard as `if (arg1 > 0)` folds the constant DIRECTLY to blez
 *     (no intermediate pseudo) -> vars=0, no phantom (measured: variant v12).
 *   - This is the exact shape `for (i=0; i<arg1; i++)` rotates to; the plain
 *     for-loop reproduces the phantom too but hoists the table-base/new_var
 *     inits into the preheader BEFORE the guard (blez lands after `lui s3`),
 *     giving distance 7. Placing the inits AFTER the guard (inside the if)
 *     matches the target's post-guard preheader (blez before `lui s3`).
 *
 * s1's "phantom does not fire (aggregate-only), owner-gate" conclusion was
 * reached WITHOUT the guarded-do-while structural lever. It fires here.
 */
extern s32 g_file_data_buf;
void func_8001924C(s16 *arg0, s32 arg1) {
    s32 i = 0;
    s16 *s0;
    s32 new_var;

    if (i < arg1) {
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
}
