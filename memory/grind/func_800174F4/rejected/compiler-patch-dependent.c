/* func_800174F4 — REJECTED 2026-09-26: compiler-patch-dependent (owner Q17).
 *
 * This is the body committed as COMPLETED-C by d9660d9b5 (2026-08-14, grinder
 * session 6), as it stood on main at the Q17 unpatch commit (identifier names
 * from later naming sweeps). It byte-matched ONLY because build/cc1 carried a
 * combine.c PLUS->IOR patch — the no-rewrite patch until 2026-09-25, then the
 * narrow one (tools/cc1-plus-to-ior-narrow.patch). Owner ruling 2026-09-26
 * Q17: "consider a compiler patch a cheat".
 *
 * Under the stock compiler (pinned upstream + host-only crash fix) the case-1/2
 * `h = v0 + 4;` (v0 = rand() & 3, so the operands share no bits) is rewritten
 * by combine.c's simplify_rtx to an IOR: we emit `ori s1,v0,0x4`, the target
 * has `addiu s1,v0,4`.
 * Honest floor under stock cc1: 1 (136 vs 136 insns),
 * `sandbox func_800174F4 --disable all --candidate` on this file.
 *
 * Everything else in this body is still the best-known form: its two
 * `do { ... } while (0);` wraps passed the 2026-08-14 Judge (decisions.md,
 * func_800174F4 final call). What is rejected is only the MATCH claim.
 */
/* kengo:HIGH  |  nm_cpu/cpu_set_move_command_and_dir_for_no_action_2  |  189i  |  x2 size collision */
void func_800174F4(void) {
    u8 sp18[8];
    u8 sp20[0x68];
    s32 env;
    unsigned short h;
    s32 prim;
    s32 mask;
    s32 mode;

    prim = (s32)(&D_800F33D8);
    if (g_disp_enable == DISP_DISABLED) {
        return;
    }
    env = (s32)sp20;
    h = 0xF0;
    mask = D_800A36AC & 1;
    mask = -mask;
    SetDefDrawEnv((u8 *)env, 0, mask & 0xF0, 0x280, h);
    sp20[0x18] = 0;
    PutDrawEnv((u8 *)env);
    g_gpu_ot_ptr = sp18;
    ClearOTagR(sp18, 2);
    mode = g_disp_enable;
    switch (mode) {
    case 1:
    case 2:
        prim = (s32)func_8005D46C((u8 *)prim);
        if (g_disp_fade != 0) {
            s32 v0;
            s32 i;
            /* FAKE: do-while(0) wrap, mechanism: flow.c loop-note reference
             * weighting (reg_n_refs += loop_depth) feeding
             * global.c:allocno_compare, lever-exhaustion: memory/grind/
             * func_800174F4/hypotheses.md K5/K10/K11/K12/H-S4-2/H-S5-1/H-S5-2.
             * Effect: seats the loop counter in $s0 and h in $s1 while the
             * counter is initialised before rand(). */
            do { i = 0; } while (0);
            v0 = rand();
            v0 &= 3;
            h = v0 + 4;
            if (h == 0) {
                break;
            }
        inner_loop:
            /* FAKE: do-while(0) wrap, mechanism: flow.c loop-note reference
             * weighting (reg_n_refs += loop_depth) feeding
             * global.c:allocno_compare, lever-exhaustion: as above.
             * Effect: the companion wrap for the counter's in-loop refs; it
             * must span the whole body so no code label lands between the
             * call and `i++` (that placement costs reorg.c the jal delay
             * slot). */
            do {
                prim = (s32)func_8005D554((u8 *)prim, g_disp_enable);
                i++;
            } while (0);
            if (i >= h) {
                break;
            }
            goto inner_loop;
        }
        else if ((rand() & 7) == 0) {
            func_8005D554((u8 *)prim, g_disp_enable);
        }
        break;
    case 10:
        func_8005E54C(D_800A3784, (u8 *)prim, 0);
        break;
    case 20:
        mode = D_800A37A0;
    {
        u8 a2_val = D_800A38F8;
        s32 a0_val = a2_val & 0xFF;
        s32 div_result;
        s32 counter;
        if (((u32)mode) < (u32)a0_val) {
            break;
        }
        div_result = h / (mode + 1);
        counter = D_800A37C0 + 1;
        D_800A37C0 = counter;
        if (div_result >= counter) {
            break;
        }
        if (a0_val == mode) {
            D_800A38F8 = a2_val + 1;
        } else {
            u8 new_val = a2_val + 1;
            D_800A38F8 = new_val;
            D_800A37C0 = 0;
            h = D_800A37A8[a0_val];
            if ((new_val & 0xFF) == mode) {
                h |= 0x8000;
            }
            func_80060414(h, (u8 *)prim, 0);
        }
        break;
    }
    }
    DrawOTag((u8 *)(g_gpu_ot_ptr + 4));
    DrawSync(0);
}
