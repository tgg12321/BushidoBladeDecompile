/* func_80016E60 candidate - HONEST floor 25 on the asm-until-matched chassis
 * (INCLUDE_ASM main, 0 rules), down from 30. Pure C, ZERO FAKE constructs.
 * s5 revision (synthesis session).
 *
 * s5 CHANGE vs the s4 candidate: the bit arms are back to ALL-BLOCK-LOCAL
 * (`shift`, `mask`, `bits` declared inside each arm) and `shift` is declared
 * `u8` instead of `s32`. That single type change makes BOTH bit arms
 * BYTE-EXACT - emission order AND register seats - which is the residual
 * three sessions (s2/s3/s4) attacked from the scope/statement-order/permuter
 * directions without closing.
 *
 * MEASURED (s5): on the sanctioned do-while(0) env carrier this form scores 4
 * and its ENTIRE objdiff is the two-line `sw s5,44(sp) / move s5,a0` prologue
 * pair; the bit arms contribute nothing. Honest (wrap removed) it scores 25
 * and the whole residual is the env/select global.c seat swap plus that same
 * prologue pair.
 *
 * MECHANISM (hypothesis, attribution pending - see hypotheses.md [s5] H16):
 * s4 proved the bit-arm divergence is sched1's adjust_priority boost, which
 * fires via birthing_insn_p (tools/gcc-2.7.2/sched.c:2504) only for a SET
 * whose SET_DEST is a plain REG with reg_n_sets == 1. A `u8 shift` pseudo is
 * QImode, so the arm's `addiu` writes it through a subreg / QImode dest and
 * the `GET_CODE (SET_DEST (pat)) == REG` guard no longer selects it - the
 * boost never fires, the LUID tie-break emits `addiu, li, lbu`, local-alloc's
 * birth order becomes shift, mask, chain, and the hand-rolled 3-element sort
 * (local-alloc.c:1541-1553) hands out $v0, $v1, $a0 = the target seats. The
 * arm still costs exactly one `addiu` - insn count is unchanged at 211.
 *
 * REMAINING RESIDUALS (evidence.md E-s5-*):
 *  (1) env/select seat swap - global.c allocno priority (env 4615 vs select
 *      6878). The only measured lever remains the sanctioned do-while(0) wrap
 *      of {PutDispEnv, PutDrawEnv} (carrier_u8shift_dowhile_4.c, score 4).
 *  (2) the `sw s5,44(sp) / move s5,a0` prologue pair sits FIRST in our block 0
 *      and THIRD in the target. s5 re-measured this: contrary to E-s3-7 the
 *      pair is present in BOTH the honest and the carrier forms, i.e. it is an
 *      independent 2-insn sched2 defect, not a wrap artefact. tools/sched_solver
 *      says block 0's goal IS reachable and prints three vectors (see the
 *      frontier); four hand statement-order permutations all measured worse.
 */
void func_80016E60(u8 *arg0, s32 arg1) {
    u8 *ot[2];
    u8 *env;
    s32 select;
    s32 special;
    s32 limit;
    u32 fb_base;
    s32 idx;
    u32 pad;

    select = 0;
    special = 0;
    if (D_800A38DC == 2) {
        special = D_800A389A < 1;
    }
    limit = 3;
    if (special != 0) {
        limit = 6;
    }

    D_800A36B0 = 1;
    func_8005C650(3, 0x7F, 0x7F);
    fb_base = (&D_800A3770)[D_800A36AC & 1];

    while (1) {
        idx = D_800A36AC & 1;
        D_800A38B4 = fb_base + (idx * 0x9A00);
        D_800A374C = (u8 *)&ot[idx];
        env = &D_800F7438 + (idx * 0x4090);

        ClearOTagR(D_800A374C, 1);
        func_80019568();
        if (special != 0) {
            func_8005C8A8(2, select | (D_800A3788 << 16), D_800A38B4, 0);
        } else {
            func_8005C8A8(0, select, D_800A38B4, 0);
        }
        func_80036940();
        func_8005C6D0();
        DrawSync(0);
        VSync(2);
        PutDispEnv(env + 0x5C);
        PutDrawEnv(env);
        DrawOTag(arg0 + 0x408C);
        DrawOTag(D_800A374C);
        D_800A36AC++;

        pad = D_80102794;
        if (pad & 0x100010) {
            func_8005C650(1, 0x7F, 0x7F);
            select = 0;
            break;
        }
        if (pad & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            break;
        }
        if (pad & 0x10001000) {
            func_8005C650(0, 0x7F, 0x7F);
            select = (select == 0) ? limit - 1 : select - 1;
        } else if (pad & 0x40004000) {
            func_8005C650(0, 0x7F, 0x7F);
            select = (select == limit - 1) ? 0 : select + 1;
        }

        if ((special != 0) && (select >= 3)) {
            if (D_80102794 & 0x80008000) {
                u8 shift;
                s32 mask;
                s32 bits;
                func_8005C650(0, 0x7F, 0x7F);
                shift = select - 3;
                mask = 1;
                mask <<= shift;
                bits = D_800A3788;
                bits |= mask;
                D_800A3788 = bits;
            } else if (D_80102794 & 0x20002000) {
                u8 shift;
                s32 mask;
                s32 bits;
                func_8005C650(0, 0x7F, 0x7F);
                shift = select - 3;
                mask = 1;
                mask <<= shift;
                bits = D_800A3788;
                bits &= ~mask;
                D_800A3788 = bits;
            }
        }
    }

    if (select != 0) {
        if (select == 1) {
            D_800A31DA = 1;
            D_800A3834 = 8;
            func_800372C0();
        } else if (select == 2) {
            D_800A3834 = 8;
            func_800372C0();
        }
    }

    DrawSync(0);
    ResetRCnt(0xF2000001);
    D_800A36B0 = 1;
}
