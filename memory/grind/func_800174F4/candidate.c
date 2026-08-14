/* func_800174F4 - best form as of grind session 4 (permuter).
 * Honest sandbox floor (`sandbox func_800174F4 --disable all`): 2
 * (sessions 1/2/3 left 25 -> 14 -> 8 -> 8; build_insns == target_insns == 136
 * throughout).
 *
 * SESSION 4 dropped the floor 8 -> 2 by closing cluster (A), the 6-point
 * switch-selector residue that had survived three sessions and is the exact
 * scope of the sole surviving regfix rule (`func_800174F4: $3 <-> $5 @ 27-41`).
 *
 * THE MOVE: one C variable, `mode`, holds the switch selector AND the case-20
 * limit value. `mode = g_disp_enable; switch (mode) { ... case 20: mode =
 * D_800A37A0; ...}` and every former `a1_val` use in case 20 now reads `mode`.
 * The case-20 block loses its separate `a1_val` local entirely, so the body is
 * SIMPLER than the floor-8 form, not more contrived - and it mirrors target's
 * own register economy exactly: target keeps the dispatch value and the case-20
 * limit in the SAME hardware register ($a1). Sessions 2 and 3 had measured a
 * selector local that was only ever the selector (ten spellings, all 8); what
 * they never tried was giving that local a SECOND live range in case 20. That
 * second def is what puts reg 4 into the selector allocno's exclusion set and
 * lets find_reg walk past $v1(3)/$a0(4) to $a1(5).
 *
 * PROVENANCE: proposed by a directed decomp-permuter campaign (chassis 3,
 * tmp/perm_ings3/output-145-1, 74 s after seed) and then MEASURED, reduced and
 * re-spelled by hand - the permuter's own find kept a redundant `s32 a1_val =
 * mode;` intermediate; dropping it scores the same 2 with one fewer local.
 * VETTING NOTE for the next session: `mode` serving two values is the SOTN
 * "variable reuse for codegen control" family (frozen sanctioned list,
 * .claude/rules/no-new-park-categories.md). It is NOT yet cleared by a
 * cheat-reviewer, because the floor is 2 and not 0 - it is banked here as the
 * measured best form, not as a submission.
 *
 * REMAINING 2 POINTS - unchanged from session 2/3, and the tension is unchanged
 * by this session's win: insns 50 and 54, the `rand()` jal delay slot. Target
 * fills it with `move s0,zero` (the `i = 0;`) and leaves a nop in the guard
 * branch's slot; ours is the mirror. Session 4 re-measured all three pre-call
 * `i = 0;` placements ON TOP of the new floor-2 base: pre-`rand()`, after
 * `rand()`, and after `h = v0 + 4;` all score 8 (136 insns), i.e. they still
 * cost exactly the 6-point callee-save cluster. The s3 inequality still governs
 * (counter allocno priority 6153 vs `h` 8750 once the counter's live range
 * crosses the call).
 *
 * Other load-bearing facts, all inherited and re-confirmed: the goto-form loop
 * with an explicit guard is required (natural for/while measure 39); `h` is ONE
 * variable deliberately reused for 0xF0 / the loop limit / the D_800A37A8[]
 * table value; `if (h == 0) break;` and `if (h != 0) { ... }` are exactly
 * equivalent spellings.
 *
 * SESSION 5 (permuter) left this body UNCHANGED and re-confirmed it at 2.
 * What it added: the floor-2 allocno table (counter pseudo 87 = nrefs 4 /
 * livelen 9 / pri 8888 -> $s0; `h` pseudo 73 = nrefs 7 / livelen 16 /
 * pri 8750 -> $s1 - the same 1.6% margin s3 measured on the floor-8 base), a
 * `-dg` reading showing neither allocno has a callee-save hard conflict (so
 * allocno_compare ORDER is the only lever), and kills for the last two open
 * axes: lowering `h`'s n_refs by folding the case-20 tail (21/21/41/26 - the
 * fold only reaches nrefs 6 / pri 8000 and its temp allocno steals reg 3,
 * pushing the selector off $a1), and every case-20 local merge including
 * reusing `env` there (6/2/23/12). Two more permuter campaigns, one of them
 * from the ALIGNED pre-call-init basin (base 80, 37k iterations, ZERO finds),
 * close the permuter modality. Note one equivalent respelling measured
 * neutral: `div_result` may be folded into its compare (`if (h / (mode + 1)
 * >= counter)`), one local fewer, still 2.
 *
 * 100% pure C. Zero pins, zero inline asm, zero dead code.
 */
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
    D_800A374C = sp18;
    ClearOTagR(sp18, 2);
    mode = g_disp_enable;
    switch (mode) {
    case 1:
    case 2:
        prim = (s32)func_8005D46C((u8 *)prim);
        if (g_disp_fade != 0) {
            s32 v0;
            s32 i;
            v0 = rand();
            v0 &= 3;
            h = v0 + 4;
            if (h == 0) {
                break;
            }
            i = 0;
        inner_loop:
            prim = (s32)func_8005D554((u8 *)prim, g_disp_enable);
            i++;
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
    DrawOTag((u8 *)(D_800A374C + 4));
    DrawSync(0);
}
