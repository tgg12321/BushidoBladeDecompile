/* saEft01Init — best form as of grind session 4 (permuter).
 *
 * Honest pure-C distance (sandbox --disable all): 7   [floor was 18 across
 * sessions 1-3]  ·  91 build insns vs 91 target — the instruction COUNT is
 * now exact, so the whole residual is register choice / scheduling.
 *
 * Session 4 ran two directed decomp-permuter campaigns (telemetry via
 * tools/permuter_campaign.py) on top of the session-3 chassis and screened
 * every find back through the ENGINE sandbox — the permuter's weighted score
 * and the engine's honest distance are only loosely correlated here (the
 * permuter's own best find, score 660, screened to sandbox 9, while a
 * score-1235 find screened to 13), so EVERY find must be re-measured.
 *
 * THREE LEVERS, each measured independently (see hypotheses.md H15-H18):
 *
 *  (1) `cnt = k;` — the 0x1000000 mask constant is staged out of the
 *      double-set holder `k` into the ALREADY-LIVE loop-counter local `cnt`.
 *      This is what finally gives target's TWO-REGISTER constant shape that
 *      session 3 (H11/F8) proved was required: `k` keeps its two sets so
 *      loop.c:702 still refuses to hoist either constant, and the extra copy
 *      gives the mask its own pseudo.  18 -> 11.
 *      *** The holder MUST be an existing live local: the same stage through
 *      a FRESH local (`s32 m; ... m = k;`) measures 18/92, i.e. no effect.
 *      That asymmetry is exactly the [[defeat-licm-hoist-var-reuse]] /
 *      [[staged-value-reused-variable]] family signature, so this construct
 *      is NOT clean pure C by default — it needs a `/* FAKE */` annotation,
 *      the lever-exhaustion record from sessions 1-4, and cheat-reviewer
 *      sign-off before any completion claim. ***
 *
 *  (2) `ret = *D_800A14C0 & cnt; if (ret == 0) break;` — the mask test's
 *      result is named instead of being tested inline.  11 -> 9, and it
 *      brings the build to target's exact 91 instructions.  The permuter's
 *      form of this carried a redundant `ret = 0;` inside the if-body (a
 *      dead store); dropping it is byte-neutral (both 9/91), so the dead
 *      store is NOT load-bearing and is correctly absent here.
 *
 *  (3) `tbl_125c = &tbl_125c[idx_1494[0]];` — the argument block re-bases the
 *      table pointer instead of indexing it twice ([[walking-pointer-
 *      serializes-parallel-loads]] family).  9 -> 7.
 *      SEMANTICS CHECKED: this mutation is unobservable on any later
 *      iteration — every path that reaches it sets `v0 = -1`, and the
 *      `if (v0 != 0)` join immediately `break`s out of the loop.  So the
 *      re-based pointer can never be re-used with the wrong base.
 *
 * A SEMANTICALLY-FAITHFUL, REUSE-FREE-ARG-BLOCK FALLBACK at distance 9/91 is
 * banked alongside this file as `alt_faithful_9.c` (levers 1+2 only).  It is
 * the form to fall back to if cheat-reviewer rejects lever (3).
 *
 * INHERITED AND STILL TRUE (sessions 1-3): the real `do { } while (a0 == 0)`
 * loop is structurally required (it is the only way to emit
 * NOTE_INSN_LOOP_BEG and get target's $s0-$s3 callee-save map); the three
 * explicit table-pointer locals are structurally required; the shared-`ret`
 * exit beats inline returns; the bare-goto exit forms are all 19/93.
 *
 * WHAT IS LEFT (7 at the exact instruction count): register choice and
 * schedule only.  See the session-4 frontier in hypotheses.md.
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
            s32 arg5, arg4;
            tbl_125c = &tbl_125c[idx_1494[0]];
            arg5 = tbl_125c[idx_1494[1]];
            arg4 = *tbl_125c;
            debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4, arg5);
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
