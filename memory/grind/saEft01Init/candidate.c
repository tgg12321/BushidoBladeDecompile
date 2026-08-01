/* saEft01Init — best SEMANTICALLY FAITHFUL form as of grind session 5.
 *
 * Honest pure-C distance (sandbox --disable all): 8   ·  91 build insns vs
 * 91 target — the instruction COUNT is exact, so the whole residual is
 * register choice and scheduling.
 *
 * *** READ THIS BEFORE "RESTORING" THE SESSION-4 FLOOR OF 7 ***
 * Session 4 banked a distance-7 form whose third lever re-based the table
 * pointer (`tbl_125c = &tbl_125c[idx_1494[0]];`) and then indexed the
 * RE-BASED pointer with the second index.  Session 5 read the target block
 * and found that is not a respelling, it is a different program:
 *
 *   target:  lbu a0,0(s1) / lbu v0,1(s1) / sll v0,v0,2 / addu v0,v0,s0 /
 *            sll a0,a0,2 / lw v1,0(v0) / addu a0,a0,s0 / sw v1,16(sp) /
 *            lw a3,0(a0)
 *
 * BOTH index chains are computed off the UNMODIFIED base `s0`, i.e.
 * arg4 = tbl[i0] and arg5 = tbl[i1].  The re-base emits `addu s0,s0,v1`
 * (an instruction target does not contain) and computes arg5 = tbl[i0+i1],
 * changing the value passed to debug_printf.  It is banked at
 * rejected/pointer-rebase-changes-arg5-semantics-7.c and must not come back.
 * The honest floor for a faithful form is 8, and this file is it.
 *
 * THE TWO SURVIVING LEVERS (sessions 2-4, both still load-bearing here):
 *
 *  (1) `k` holds BOTH loop-invariant compare constants (two non-consecutive
 *      sets), which denies loop.c:702 the hoist that would otherwise push
 *      them into $s4/$s5 and destroy target's $s0-$s3 callee-save map; and
 *      `cnt = k;` then stages the 0x1000000 mask out of `k` into the
 *      ALREADY-LIVE loop counter so the mask gets its own pseudo — target
 *      materialises the two constants in two DIFFERENT registers
 *      (`lui $v0,0x3c`, `lui $v1,0x100`).
 *      *** The staging holder MUST be an existing live local: the same stage
 *      through a FRESH local is byte-inert (session 4, H17).  That asymmetry
 *      is the [[defeat-licm-hoist-var-reuse]] / [[staged-value-reused-
 *      variable]] family signature, so this construct is NOT clean pure C by
 *      default — it needs a `/* FAKE *\/` annotation, the sessions 1-5
 *      lever-exhaustion record, and cheat-reviewer sign-off before any
 *      completion claim. ***
 *
 *  (2) `ret = *D_800A14C0 & cnt; if (ret == 0) break;` — naming the mask
 *      test's result instead of testing it inline (session 4, H18; the
 *      permuter's version of this carried a dead `ret = 0;` which measured
 *      byte-neutral and is correctly absent).
 *
 * ARGUMENT BLOCK (session 5, H20/H21 — cluster B / F6 is now closed at the
 * source-ordering level): ONE named intermediate, `arg4`, assigned BEFORE
 * the call, with the fifth argument written inline.  Measured cross-product:
 * arg4's assignment first = 8, arg5's first = 9; declaration order of two
 * named intermediates is byte-inert; naming arg5 instead of arg4 costs 6.
 *
 * INHERITED AND STILL TRUE (sessions 1-3): the real `do { } while (a0 == 0)`
 * loop is structurally required (only it emits NOTE_INSN_LOOP_BEG and gets
 * target's $s0-$s3 map); the three explicit table-pointer locals are
 * structurally required; the shared-`ret` exit beats inline returns; every
 * bare-goto exit form is 19/93.
 *
 * WHAT IS LEFT (8 at the exact instruction count), both register/schedule:
 *   (a) the fourth argument's address chain — ours runs through $v1 and
 *       issues `lw a3,0(v1)` early; target runs it through $a0 and defers
 *       `lw a3,0(a0)` past the `sw v1,16(sp)` / `lw a2` pair, and the
 *       `D_800F19C0` load moves with it;
 *   (b) session-3 F7 — target fills the `beqz $v0` delay slot with
 *       `move v0,zero`; we emit a `nop`.
 */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 ret;
    s32 k;
    s32 *tbl_11dc;
    u8 *idx_1494;
    s32 *tbl_125c;

    D_800F19B8 = sys_VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &D_800A1494;
    tbl_125c = D_800A125C;
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

    do {
        v0 = sys_VSync(-1);
        if (D_800F19B8 < v0) {
            goto do_timeout;
        }
        cnt = D_800F19BC;
        D_800F19BC = cnt + 1;
        k = 0x3C0000;
        if (!(k < cnt)) {
            goto success;
        }

    do_timeout:
        tslTm2LoadImage_2(&D_800161B8);
        {
            s32 arg4;
            arg4 = tbl_125c[idx_1494[0]];
            debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4, tbl_125c[idx_1494[1]]);
        }
        cdrom_ClearIrq();
        v0 = -1;
        goto check;

    success:
        v0 = 0;

    check:
        if (v0 != 0) {
            ret = -1;
            break;
        }
        k = 0x1000000;
        cnt = k;
        ret = *D_800A14C0 & cnt;
        if (ret == 0) {
            break;
        }
        ret = 1;
    } while (a0 == 0);
    return ret;
}
