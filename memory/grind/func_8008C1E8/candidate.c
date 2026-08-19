/* candidate.c — func_8008C1E8 (SetPacketData), grind session s1 (2026-08-18)
 *
 * Honest floor with this body IN PLACE in src/main.c: sandbox 23
 * (target_insns 159, build_insns 160 — we carry one EXTRA insn pair at entry,
 * the `j main_work; move s2,zero` trampoline, offset by target's extra addiu
 * in the loop_continue address materialisation which this form now matches).
 *
 * This is a cheat-free body: no pins, no inline asm, no alias renames.
 * Volatile surfaces (all inherited, see evidence.md HAZARD + BLOCKER notes):
 *   - extern volatile on D_800F1AF0/AF4/AF8 (shared with COMPLETED-C
 *     func_8008C184 — do NOT de-volatilize, measured regression there)
 *   - volatile s32 *flag/loop_flag over D_800F1AEC — LOAD-BEARING
 *     (de-volatilizing measured 27->39 in the 2026-08-06 WIP session);
 *     needs the legitimate-volatile-interrupt-touched two-prong finding
 *     (identify the SPU/serial ISR mutating the D_800F1AEC block) before
 *     COMPLETED-C. Sibling grant precedent at main.c:3430 comment.
 *   - volatile u16 * derefs of D_800A3044+4/+0xA are SPU MMIO-register
 *     reads through a RAM-held pointer (hardware-register category).
 *   - the p_af4/p_af8/p_af4c/p_ae2 pointer-alias blocks reproduce target's
 *     full lui/addiu/lw 0(reg) address materialisation. They are C-level
 *     pointer aliases to globals — natural-read spelling, but classify
 *     before candidate-ready (pointer-alias family vs ordinary code).
 *
 * Depends on file-scope decls already in src/main.c (~3331-3435):
 *   extern volatile u16 D_800F1AE2; extern s32 (*D_800F1AE8)(s32, s32);
 *   extern s16 D_800A3074[4]; extern s32 D_800F1AEC;
 *   extern volatile s32 D_800F1AF0, D_800F1AF4, D_800F1AF8;
 *   extern s32 D_800A3044;
 */
s32 func_8008C1E8(u8 *arg0, s32 arg1) {
    volatile s32 *flag = &D_800F1AEC;
    s32 s0;
    s32 s5;
    s32 s1;
    void *spu;
    s32 (*fn)(s32, s32);
    s32 wait_val;

    if (*flag != 0) return -1;
    s0 = 0;
    goto main_work;

cleanup_A:
    DeliverEvent(0xF000000B, 0x100);
    goto return_val;

cleanup_B:
    DeliverEvent(0xF000000B, 0x100);
    {
        volatile s32 *p_af4 = &D_800F1AF4;
        return (arg1 - *p_af4) - 1;
    }

main_work:
    {
        volatile u16 *p_ae2 = &D_800F1AE2;
        u32 ae2_val;
        ae2_val = *p_ae2;
        s5 = *(s16 *)((s32)D_800A3074 + ((ae2_val & 0x300) >> 7));
    }
    D_800F1AF4 = arg1;
    D_800F1AF0 = (s32)arg0;
    s1 = 0;
    if (D_800F1AF4 == 0) goto return_val;

    {
        volatile s32 *loop_flag = flag;

outer_top:
        spu = (void *)D_800A3044;
        if ((*((volatile u16 *)(((s32)spu) + 4)) & 5) == 5) goto check_first;
        wait_val = 5;

inner_wait:
        fn = D_800F1AE8;
        if (fn != 0) {
            s32 prev = s0;
            s0 += 1;
            if (fn(2, prev) == 0) goto cleanup_A;
        }
        if ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 5) != wait_val) goto inner_wait;

check_first:
        if (s1 != 0) goto send_byte;
        D_800F1AF8 = (*((volatile u16 *)(((s32)D_800A3044) + 4))) & 0x80;

send_byte:
        *((u8 *)D_800A3044) = *((u8 *)D_800F1AF0);
        loop_flag[1] += 1;
        loop_flag[1];
        s1 += 1;
        loop_flag[2] -= 1;
        loop_flag[2];
        if (s1 != s5) goto loop_continue;
        if ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 0x80) != loop_flag[3]) {
            s1 = 0;
            goto loop_continue;
        }
        {
            volatile s32 *p_af8 = &D_800F1AF8;
inner_wait2:
            fn = D_800F1AE8;
            if (fn != 0) {
                s32 prev = s0;
                s0 += 1;
                if (fn(2, prev) == 0) goto cleanup_B;
            }
            if ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 0x80) == *p_af8) goto inner_wait2;
        }
        s1 = 0;

loop_continue:
        {
            volatile s32 *p_af4c = &D_800F1AF4;
            if (*p_af4c != 0) goto outer_top;
        }
    }

return_val:
    {
        volatile s32 *p_af4 = &D_800F1AF4;
        return arg1 - *p_af4;
    }
}
