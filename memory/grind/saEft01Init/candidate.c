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
 *
 * SESSION-6 FORENSICS — READ BEFORE SPENDING ANY MORE TURNS ON (a):
 *   * The `arg4` local ABOVE IS STRUCTURALLY WRONG, even though it scores 8.
 *     From the `.rtl` (raw expand) dump: a NAMED argument's load is emitted at
 *     its statement position — `(set (reg/v 88) (mem (reg 93)))` — plus a copy
 *     `(set (reg a3) (reg/v 88))`, and `local_alloc` gives pseudo 88 `$a3` by
 *     copy preference, so the LOAD lands on `$a3` EARLY.  An INLINE argument's
 *     load is emitted by `expand_call` DIRECTLY into the hard reg as the LAST
 *     insn of the argument sequence — `(set (reg:SI 7 a3) (mem (reg 99)))` —
 *     which is target's shape (`lw a3,0(a0)` is target's last memory ref).
 *     combine plays no part (its `REG_USERVAR_P` guards are
 *     `SMALL_REGISTER_CLASSES`-gated; mips.h does not define it).
 *     The all-inline form is banked at
 *     `rejected/arg4-inline-is-target-expand-shape-but-14.c` at 14/91.
 *   * Source STATEMENT placement inside this basic block is byte-inert: four
 *     spellings with genuinely different expand LUID orders (including one
 *     that makes the `idx_1494[0]` `lbu` the block's first insn) all emit
 *     byte-identical code, because `rank_for_schedule` decides on
 *     `INSN_PRIORITY` and the dependence-class test (`sched.c:2412-2449`),
 *     never reaching the `INSN_LUID` tie-break at `sched.c:2452-2455`.
 *   * The `$a0` in target is not an allocator choice: it falls out of that
 *     chain being live across the WHOLE block, which is a sched1 consequence.
 *
 * ===========================================================================
 * SESSION-7 PROVENANCE WARNING — THIS FORM IS *NOT* THE ORIGINAL SOURCE
 * ===========================================================================
 * saEft01Init is Sony PsyQ LIBCD `CD_datasync`, and session 7 obtained the
 * matched C: memory/grind/saEft01Init/ref/sotn_libcd_bios_CD_datasync.c
 * (from Xeeynamo/sotn-decomp src/main/psxsdk/libcd/bios.c), with every symbol
 * mapped and independently confirmed against the target disassembly (the
 * clincher is `*D_800A14C0 & 0x1000000` = DMA3 CHCR channel-busy bit).
 *
 * The reference says plainly what the original body contains, and this file
 * disagrees with it in two places:
 *   * the original has NO named `arg4` intermediate — all four table lookups
 *     are written inline in the printf call (which session 6 had already
 *     deduced from the expand-time RTL shape, independently);
 *   * the original has NO `k` and no `cnt = k` staging — both compare
 *     constants are plain literals.
 *
 * So this 8/91 is a wrong-basin local optimum bought with two FAKE-family
 * levers.  Session 7 measured the honest cheat-free floor of this same chassis
 * with the reference's statements and zero levers: 32 / 96
 * (rejected/clean-no-levers-licm-hoists-both-constants-32.c).  It is retained
 * as `candidate.c` ONLY because it is still the lowest measured distance and
 * the driver asks for the best form; it must NOT be advanced toward completion
 * without cheat-review, and a reviewer should be shown the 32 as well.
 *
 * WHERE THE REAL WORK IS NOW (session-7 instrumented-cc1 forensics):
 * in the clean form the ALLOCDBG dispositions are ALREADY target's for the
 * first three pseudos — tbl_125c->$s0, idx_1494->$s1, the param->$s2 — with no
 * lever at all.  The single allocation defect is that loop.c's LICM hoists the
 * two loop-invariant compare constants into fresh pseudos (85 and 108) which
 * rank 5th/6th in global_alloc and push tbl_11dc from target's $s3 to $s5,
 * costing two extra callee-saves (+5 insns).  Suppressing exactly those two
 * hoists — and nothing else — reproduces target's map exactly
 * (rejected/cleank-licm-defeat-alone-lands-exact-callee-save-map-21.c).
 * Find a LEGITIMATE spelling for that suppression and the clean form, not this
 * one, becomes the match candidate.
 * ===========================================================================
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
