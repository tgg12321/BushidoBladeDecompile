void func_80016E60(u8 *arg0, s32 arg1) {
    u8 *ot[2];
    u8 *env;
    s32 select;
    s32 special;
    s32 limit;
    u32 fb_base;
    s32 idx;
    u32 padbits;
    u8 *ot_base;

    select = 0;
    special = 0;
    /* FAKE: pass-through local handle on the parameter (pointer-alias family,
       .claude/rules/pointer-alias-fake-exception.md exact-scope bullet 3),
       consumed once at the DrawOTag call site; effect: the prologue's
       (save,init) group for $s5 emits third instead of first.
       mechanism: combine.c's i2/i3 merge folds `p1 = a0` into this later copy
       and leaves it at the LATER position, so sched.c:3256's parameter-copy pin
       (leading run of hard-register-source SETs) no longer applies, sched1's
       birthing_insn_p boost (sched.c:2504) emits it after the two init insns,
       and sched2's INSN_LUID tie-break (sched.c:2462) orders the groups s1,s2,s5.
       lever-exhaustion: memory/grind/func_80016E60/hypotheses.md (s1-s5 bare-
       parameter forms, [s6] E-s6-4 hard mechanism, [s6] E-s6-7 honest env route). */
    ot_base = arg0;
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
        /* FAKE: single-level do { } while (0) wrap around the two env
           publishes (do-while-zero family, .claude/rules/do-while-zero-exception.md);
           effect: env is seated in $s0 and select in $s1, the target's assignment.
           mechanism: the wrap's loop notes make flow.c weight env's three in-loop
           references at loop_depth 3 instead of 2, lifting its global.c
           allocno_compare priority above select's.
           lever-exhaustion: memory/grind/func_80016E60/hypotheses.md ([s2] H6 and
           [s6] E-s6-7/E-s6-8 - the honest env split-init routes measure 22 vs 21). */
        do {
            PutDispEnv(env + 0x5C);
            PutDrawEnv(env);
        } while (0);
        DrawOTag(ot_base + 0x408C);
        DrawOTag(D_800A374C);
        D_800A36AC++;

        padbits = D_80102794;
        if (padbits & 0x100010) {
            func_8005C650(1, 0x7F, 0x7F);
            select = 0;
            break;
        }
        if (padbits & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            break;
        }
        if (padbits & 0x10001000) {
            func_8005C650(0, 0x7F, 0x7F);
            select = (select == 0) ? limit - 1 : select - 1;
        } else if (padbits & 0x40004000) {
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
