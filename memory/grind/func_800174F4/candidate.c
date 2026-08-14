/* func_800174F4 - best form as of grind session 3 (structural).
 * Honest sandbox floor (`sandbox func_800174F4 --disable all`): 8
 * (session 1 left 14; build_insns == target_insns == 136 throughout).
 *
 * SESSION 3 did not move the floor: seventeen further structural forms all
 * measured >= 8 (ladder in evidence.md). What it DID establish is the exact
 * arithmetic of the remaining 8 points, and it CORRECTS session 2's K4:
 *
 *  - The $s0/$s1 assignment is decided by `global.c:allocno_compare`, not by
 *    local-alloc. The cc1 `-dg` headers of this form and the floor-14 form
 *    differ in exactly one place, the allocno ORDER list; the conflict lists
 *    are identical. Counter pseudo 85: n_refs 4, live_length 9, priority 8888.
 *    `h` pseudo 73: n_refs 7, live_length 16, priority 8750. The whole
 *    callee-save match rests on that 1.6% margin - ONE unit of the counter's
 *    live length. Do not lengthen it.
 *  - The 2-point delay-slot residue costs 6 to buy 2: putting `i = 0;` before
 *    the `rand()` call (the only placement reorg.c can pull into the jal
 *    delay slot) takes the counter's live_length to ~13 and its priority to
 *    6153, so `h` wins $s0.
 *  - The 6-point switch-selector residue needs regs 3 AND 4 in the selector
 *    allocno's `hard_reg_conflicts U regs_someone_prefers`; instrumented
 *    find_reg shows conflicts {2,29} and all three preference sets EMPTY.
 *
 * An exactly equivalent spelling of this body (8, 136 insns, byte-identical
 * output) inverts the guard to an early exit: `if (h == 0) break;` followed by
 * the unindented `i = 0; inner_loop: ...`.
 *
 * 100% pure C. Session 2's single change over the session-1 form is the
 * placement of `i = 0;`: it moved from the loop PRE-HEADER (before the
 * rand() call) INTO the `if (h != 0)` guard block. That one statement move
 * resolved the entire callee-save cluster (B) - the counter now takes $s0
 * and the loop-limit / D_800A37A8[] table-value webs both take $s1, exactly
 * as target does - worth 6 of the 14 points.
 *
 * Remaining 8 points, both measured this session:
 *  (A) switch-selector web, 6 pts, insns 28/30/31/33/35/41: ours reads
 *      `lbu v1,%gp_rel(D_800A3768)` and does the whole dispatch in $v1;
 *      target does it in $a1. This is the exact scope of the sole surviving
 *      regfix rule (`func_800174F4: $3 <-> $5 @ 27-41`, regfix.txt:11).
 *  (B') delay-slot placement, 2 pts, insns 50 and 54: target fills the
 *      rand() `jal` delay slot with `move s0,zero` (the `i = 0;`) and leaves
 *      a `nop` in the guard branch's delay slot; ours is the mirror image.
 *      Target's `i = 0;` therefore sits BEFORE the call in the insn stream -
 *      but every spelling that puts it before the call measured back at 14
 *      because the counter then loses $s0 to the `h` web (see
 *      rejected/i-init-before-call.c). Those two goals are in tension and
 *      the tension is the next session's whole problem.
 *
 * `h` is ONE C variable deliberately reused for 0xF0 / the loop limit / the
 * D_800A37A8[] table value - that reuse is load-bearing (session 1 measured
 * every split WORSE; see rejected/).
 */
void func_800174F4(void) {
    u8 sp18[8];
    u8 sp20[0x68];
    s32 env;
    unsigned short h;
    s32 prim;
    s32 mask;

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
    D_800A374C = sp18;
    ClearOTagR(sp18, 2);
    switch (g_disp_enable) {
    case 1:
    case 2:
        prim = (s32)func_8005D46C((u8 *)prim);
        if (g_disp_fade != 0) {
            s32 v0;
            s32 i;
            v0 = rand();
            v0 &= 3;
            h = v0 + 4;
            if (h != 0) {
                i = 0;
                inner_loop:
                prim = (s32)func_8005D554((u8 *)prim, g_disp_enable);
                i++;
                if (i >= h) {
                    break;
                }
                goto inner_loop;
            }
        }
        else if ((rand() & 7) == 0) {
            func_8005D554((u8 *)prim, g_disp_enable);
        }
        break;
    case 10:
        func_8005E54C(D_800A3784, (u8 *)prim, 0);
        break;
    case 20:
    {
        u8 a2_val = D_800A38F8;
        s32 a1_val = D_800A37A0;
        s32 a0_val = a2_val & 0xFF;
        s32 div_result;
        s32 counter;
        if (((u32)a1_val) < (u32)a0_val) {
            break;
        }
        div_result = h / (a1_val + 1);
        counter = D_800A37C0 + 1;
        D_800A37C0 = counter;
        if (div_result >= counter) {
            break;
        }
        if (a0_val == a1_val) {
            D_800A38F8 = a2_val + 1;
        } else {
            u8 new_val = a2_val + 1;
            D_800A38F8 = new_val;
            D_800A37C0 = 0;
            h = D_800A37A8[a0_val];
            if ((new_val & 0xFF) == a1_val) {
                h |= 0x8000;
            }
            func_80060414(h, (u8 *)prim, 0);
        }
        break;
    }
    }
    DrawOTag((u8 *)(D_800A374C + 4));
    DrawSync(0);
}
