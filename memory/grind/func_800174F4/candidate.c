/* func_800174F4 — best form as of grind session 1 (recon).
 * Honest sandbox floor (`sandbox func_800174F4 --disable all`): 14
 * (session baseline was 25; build_insns == target_insns == 136, i.e. the
 * instruction SEQUENCE is now structurally identical to target — every
 * remaining diff is a register-NAME swap, no ins/del/reorder left).
 *
 * This form is 100% pure C: it deliberately drops the whole cheat scaffold
 * that the pre-session body carried (three `register T x asm("sN"/"a1")`
 * pins, a `if ((a1_val && a1_val) && a1_val) { }` empty-body dead-read, and
 * the `new_var`/`new_var2` constant-holder locals). Measured fact: that
 * scaffold was worth ZERO honest distance — the clean rewrite scored the
 * same 25 the scaffolded body did.
 *
 * Remaining 14 = exactly two register-name clusters (see hypotheses.md):
 *   (A) switch-selector web: ours $v1, target $a1  (insns 28,30,31,33,35,41)
 *   (B) callee-save split webs: ours i->$s1 / {loop-limit, table-value}->$s0,
 *       target i->$s0 / {loop-limit, table-value}->$s1
 *       (insns 50,52,53,58,60,116,119,120)
 * `h` is ONE C variable deliberately reused for 0xF0 / the loop limit / the
 * D_800A37A8[] table value — that reuse is load-bearing (splitting any of the
 * three webs into its own local measured WORSE; see rejected/).
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
            i = 0;
            v0 = rand();
            v0 &= 3;
            h = v0 + 4;
            if (h != 0) {
                inner_loop:
                prim = (s32)func_8005D554((u8 *)prim, g_disp_enable);
                i++;
                if (i >= h) {
                    break;
                }
                goto inner_loop;
            }
        } else if ((rand() & 7) == 0) {
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
