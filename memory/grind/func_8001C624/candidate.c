/* func_8001C624 — BEST HONEST FORM (s4, 2026-07-13).  127/127 insns, sandbox 61.
 *
 * s4 REPLACES s3's candidate (the Blk16/Blk12 aggregate-typing form, sandbox 73)
 * with this one.  s3 saved the aggregate form because it was the only SEMANTICALLY
 * CORRECT form measured; s4 proved with isolated compiler probes that the aggregate
 * family is STRUCTURALLY INCAPABLE of ever reaching the target, on two independent
 * counts, so it can never close and is no longer the right thing to carry forward.
 * It now lives in rejected/aggregate-any-spelling-loses-offset0-atform.c.
 *
 * WHY THIS CHASSIS IS THE RIGHT ONE — it is the only one whose ADDRESSING matches
 * the target exactly.  The target (asm/6CAC.s:5019-5147) simultaneously contains:
 *
 *   lui $at,%hi(D_80101FB0) ; sw $zero,%lo(D_80101FB0)($at)   <- at-form offset-0 store
 *   lui $a1,%hi(D_80101FB0) ; addiu $a1,$a1,%lo(D_80101FB0)   <- independent copy base
 *   lw,lw,sw,sw / lw,nop,sw                                   <- block copy, num_regs=2
 *
 * i.e. GCC did NOT common the store's address with the copy's base register.  That
 * is the signature of CONSTANT-ADDRESS MEMs — separate scalar symbols.  Any real
 * aggregate (see the rejected file) forces a shared base register and loses BOTH the
 * three at-form `lui`s AND the num_regs=2 chunking.  This chassis reproduces both.
 *
 * THE ONE REMAINING GAP: the SCHEDULE.  Each block copy's only dependence is the
 * store to its OWN base word (verified in the -dS RTL: LOG_LINKS = {offset-0 store,
 * ANTI on the call}), because sched.c's memrefs_conflict_p short-circuits on
 * GET_MODE_SIZE(BLKmode)==0 only when the canonicalised bases are rtx_equal — so the
 * +4/+8 stores are different symbol_refs and GCC sees NO dependence on them.  sched1
 * therefore glues each copy directly behind its base store, ABOVE the +4/+8 stores
 * that feed it, and the emitted code copies stale data.  That is a GCC 2.7.2 alias
 * DEFICIENCY miscompiling legal C — this source's MEANING is correct; GCC breaks it.
 * (Reproduced in 12 lines: tmp/grind/func_8001C624/s4/probe_offset0.c, f_scalar.)
 *
 * DO NOT "fix" the schedule with a do{}while(0) wrapper: in this straight-line
 * function that is purely a sched.c:2066 total barrier (flush_pending_lists), the
 * forbidden family — killed for sanction in s2, and s4's permuter campaign shows it
 * is the ONLY thing the search space contains (27,651 iterations, 1,517 finds; every
 * single top find is a do-while(0) barrier or a relocated call).
 *
 * The open question is now a RULING question, not a search question — see
 * hypotheses.md §s4 and the outcome's ruling_question.
 */
void func_8001C624(void) {
    typedef struct { s32 a, b, c, d; } Blk16;
    typedef struct { s32 a, b, c; } Blk12;
    s32 local[3];
    s32 x;
    s32 y;
    s32 z;
    s32 i;

    /* `i` must stay a VARIABLE: written as a literal 0x36, combine folds the
     * address into the symbol D_80101FA0 and the build loses 2 insns (125/127). */
    i = 0x36;
    func_80021D10(0, &((s32 *)&D_80101EC8)[i], (s32)D_800A38E0);
    func_80021D10(1, local, (s32)D_800A38E0);
    D_80101FB0 = 0;
    x = ((s32 *)&D_80101EC8)[0x36];
    y = D_80101FA4;
    z = D_80101FA8;
    D_80101FB4 = -0x384;
    D_80101FB8 = 0;
    D_80101FBC = x;
    D_80101FC0 = y - 0x384;
    D_80101FC4 = z;
    D_80101F80 = x;
    D_80101F84 = y;
    D_80101F88 = z;
    *((Blk16 *)&D_80101F90) = *((Blk16 *)&D_80101F80);
    *((Blk12 *)&D_801020C0) = *((Blk12 *)&D_80101FB0);
    D_80101FCC = 0;
    D_80101FD0 = 0;
    D_80101FD4 = 0;
    *((Blk16 *)&D_80102114) = *((Blk16 *)&D_80101FCC);
    /* setVector idiom (Judge-approved respell of the t/u round-trip): the target
     * genuinely contains the self-copy stores — `local` is address-taken. */
    local[0] = local[0], local[1] = local[1] - 0x384, local[2] = local[2];
    D_80101FDC = 0;
    D_80101FE0 = 0;
    D_80101FE4 = 0;
    D_80101FEC = 0;
    D_80101FF0 = 0;
    D_80101FF4 = 0;
    D_80101FFC = 0;
    D_80102000 = 0;
    D_80102004 = 0;
    D_8010200C = 0;
    D_80102014 = 0;
    D_80102018 = 0;
    D_8010201A = 0;
    D_80102016 = 0;
    D_80102010 = D_80101F84;
    func_8003FFE0(0);
}
