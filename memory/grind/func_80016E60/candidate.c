/* func_80016E60 candidate - honest floor 30 on the asm-until-matched chassis
 * (INCLUDE_ASM main, 0 rules). Pure C, no FAKE construct. s4 revision.
 *
 * s4 CHANGE vs the s3 "q2" candidate: `shift` and `mask` are FUNCTION-SCOPE
 * (written in both bit arms) instead of block-scoped per arm. Same score (30)
 * but STRICTLY closer: the s3 form's bit arms emitted `li` before `addiu`
 * (an emission-ORDER divergence in both arms); the s4 form emits the target's
 * `addiu; li; lbu` order in arm A and `addiu; li; sllv; lbu` in arm B, so the
 * whole bit-arm residual is now pure REGISTER NAMING with zero insn movement.
 *
 * MECHANISM (s4, measured - tools/gcc-2.7.2/sched.c:2504 birthing_insn_p /
 * :2586 adjust_priority): a block-local `shift` pseudo has reg_n_sets == 1, so
 * birthing_insn_p() is true for its `addiu` and adjust_priority() raises that
 * insn's INSN_PRIORITY to max_priority (0x7F000001 here, inherited from the
 * block-ending jump). sched1 then picks it FIRST in its bottom-up scan, which
 * emits it LAST of the three ready insns -> `li, lbu, addiu`. Writing `shift`
 * in BOTH arms through one function-scope variable makes reg_n_sets == 2, the
 * boost never fires, all three insns stay at priority 1, and rank_for_schedule
 * falls through to the INSN_LUID tie-break, which reproduces the target order
 * exactly. Full trace: tmp/grind/func_80016E60/s4/sched.log.
 *
 * Residuals vs target (evidence.md E-s4-*):
 *  (1) env/select seat swap - global.c priority (every s0/s1 line of the diff).
 *      Honest split-init lever measures 33; the sanctioned do-while(0) carrier
 *      (carrier_n4_dowhile.c) measures 11.
 *  (2) bit arms, REGISTERS ONLY: making `shift` function-scope also makes it a
 *      global.c allocno, and global.c seats it in $a0 (target: $v0) while the
 *      block-local arm-A chain takes $v0 (target: $a0). Arm B's chain is
 *      already byte-correct ($v0). The wanted allocation - shift $v0, mask
 *      $v1, chain $a0/$v0 - is what the ALL-BLOCK-LOCAL form produces once the
 *      birth order is shift, mask, chain, so the open question is a block-local
 *      `shift` pseudo with reg_n_sets != 1 (cse folds every split-init
 *      spelling measured so far).
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
    s32 shift;
    s32 mask;

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
                s32 bits;
                func_8005C650(0, 0x7F, 0x7F);
                shift = select - 3;
                mask = 1;
                mask <<= shift;
                bits = D_800A3788;
                bits |= mask;
                D_800A3788 = bits;
            } else if (D_80102794 & 0x20002000) {
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
