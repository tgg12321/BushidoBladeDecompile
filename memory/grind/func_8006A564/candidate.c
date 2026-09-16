/* func_8006A564 -- src/text1b.c
 * Session 8 (solver). Chassis at session start: the working tree's
 * src/text1b.c already carried an UNBANKED body from a discarded earlier
 * session-8 process (its scratch is tmp/grind/func_8006A564/s8/, artifacts
 * timestamped 2026-09-16 14:08-14:16, no candidate.c / outcome written).
 * That inherited body measured `sandbox --disable all` score 7 (target_insns
 * 199, build_insns 199, exact parity) -- i.e. the ledger's recorded floor of
 * 29 was already stale at dispatch. This session BANKS that body and takes it
 * from 7 to 3 (exact parity held, 199 == 199, throughout).
 *
 * What the inherited (unbanked) body had changed relative to the s7
 * candidate.c: store-order reshuffles inside blocks 1-3 (the tile+0xC /
 * tile+0xE pair ordering), tile[6] = v0 moved inside both arms of block 3,
 * and the final record-copy region split into two separate `{ s32 v0; ... }`
 * scopes (one per func_8007352C call). It is preserved verbatim here except
 * for this session's one edit.
 *
 * SESSION 8 METHOD (solver modality, pass-attributed):
 *   1. `inverse_compose.py classify` (from the discarded process's run,
 *      tmp/grind/func_8006A564/s8/classify.txt) typed the residual as RA:
 *      same instructions, different registers, concentrated in the tail.
 *   2. A register-normalized objdump diff (tmp/grind/func_8006A564/s8/
 *      objdiff.sh, ours vs build/src/text1b.o) localized the whole score-7
 *      residual to exactly two adjacent clusters in the final block, 4 + 3
 *      mismatched instructions:
 *        A) ours  lw v0,28(s1) / lw v1,0(s1) / sw zero,24(s1) /
 *                 addiu v0,v0,15 / addiu v1,v1,12
 *           tgt   lw v1,0(s1)  / lw v0,28(s1) / sw zero,24(s1) /
 *                 addiu v1,v1,12 / addiu v0,v0,15
 *        B) ours  sw v0,0(s1) / addiu v0,v0,12 / sw v0,4(s1)
 *           tgt   addiu v1,v0,12 / sw v0,0(s1) / sw v1,4(s1)
 *   3. Cluster A CLOSED by rewriting its group as a compound-assignment
 *      split with the arg1+0x18 store hoisted to the head of the group:
 *        *(arg1+0x18) = 0; v0 = *(arg1+0); v0 += 0xC;
 *        *(arg1+0x1C) += 0xF; *(arg1+4) = v0;
 *      This is ordinary C under .claude/rules/ordinary-c-judge-decidable.md
 *      Ruling 4 (compound-assignment splits on the same variable). It also
 *      DELETED the two dead reads (`v0 = *(arg1+0); v0 = *(arg1+0x1C);`)
 *      that every body since session 2 had carried -- the current body has
 *      zero dead stores and zero FAKE constructs.  7 -> 3.
 *   4. Cluster B (3 insns) survives: 28 measured spellings of that group all
 *      score 3 (see hypotheses.md [s8] H-B1..H-B4 for the enumeration).
 *
 * PASS ATTRIBUTION for cluster B (read, not guessed -- dumps regenerated this
 * session from this body via `pwsh tools/grinder/dump.ps1 func_8006A564`):
 *   - .rtl and .combine keep the add BEFORE the store when the C names the
 *     +0xC value (`v1 = v0 + 0xC;` then the two stores): insn order
 *     436(lw) 439(add) 442(sw 0) 445(sw 4) -- i.e. TARGET's order already
 *     exists at combine time.
 *   - sched1 (.sched) reorders it to 436 442 439 445. Its trace:
 *       ;; ready list at T-25: 442 (4) 445 (4), now 445 442
 *       ;; ready list at T-26: 442 (4) 439 (7f000001), now 439 442
 *     0x7f000001 is sched.c's LAUNCH_PRIORITY (sched.c:187, set at
 *     sched.c:4049). The add is released by the scheduling of its consumer
 *     (sw 4(s1)) and enters the ready list carrying that boost, so
 *     rank_for_schedule's FIRST test (INSN_PRIORITY, sched.c:2418) picks it
 *     over the equal-class store -- the dependence-class and LUID tests
 *     (sched.c:2420-2463) are never reached. Because schedule_block is a
 *     BACKWARD scheduler, being picked at T-26 places the add AFTER the
 *     store in program order.
 *   - Post-reload (sched2) the two values have been coalesced into v0, so
 *     `addiu v0,v0,12` carries a REG_DEP_ANTI on `sw v0,0(s1)` and the order
 *     is then dependence-FORCED (every sched2 ready list in that region has
 *     exactly one member). The register coalescing is a CONSEQUENCE of the
 *     sched1 order, not an independent RA decision -- which is why every
 *     two-named-locals spelling is inert.
 *
 * Matched-sibling ground truth for cluster B's target shape: func_8006A1A0
 * (same file, COMPLETED-C) emits `lw v0,12(s3); addiu v1,v0,12; sw v0,24(sp);
 * sw v1,28(sp)` from `p1 = ptr[3]; tbl = p1 + 0xC; s.sp18 = p1; s.sp1C = tbl;`
 * -- the same source shape as our b01/d01..d05 variants. The difference is
 * that the sibling's destination is a STACK STRUCT (sw ...(sp), fixed
 * address) while ours is a pointer parameter (sw ...(s1)); transplanting the
 * sibling's spelling onto our chassis measured 3 (no change) in 5 variants.
 *
 * KILLED THIS SESSION (all instance kills, all measured on this chassis with
 * zero FAKE constructs present) -- see hypotheses.md [s8] for the full grids.
 */
extern s32 D_800A34F8;
void func_8006A564(u8 *arg0, u8 *arg1, s32 arg2) {
    u8 *tile;
    u8 *obj2;
    s32 s4;

    tile = *(u8 **)(arg0 + 0x18);
    SetTile(tile);
    {
        s32 v0;
        if ((D_800A34F8 & 0xF) == arg2) {
            v0 = *(u8 *)(arg1 + 0x29);
            tile[5] = 0;
            tile[4] = v0;
            v0 = *(u8 *)(arg1 + 0x2B);
            s4 = 0;
        } else {
            s4 = 0x20;
            v0 = 0x50;
            tile[4] = v0;
            tile[5] = v0;
        }
        tile[6] = v0;
        *(s16 *)(tile + 8) = (0x5F);
        *(s16 *)(tile + 0xA) = ((*(s32 *)(arg1 + 0x1C)) + 0xF);
        *(s16 *)(tile + 0xC) = ((*(s32 *)(arg1 + 0x18)) + 0x19);
        *(s16 *)(tile + 0xE) = 1;
    }
    SetSemiTrans(tile, *(s32 *)(arg1 + 0x10));
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);
    tile += 0x10;

    SetTile(tile);
    {
        s32 v0;
        if ((D_800A34F8 & 0xF) == arg2) {
            v0 = *(u8 *)(arg1 + 0x29);
            tile[5] = 0;
            tile[4] = v0;
            v0 = *(u8 *)(arg1 + 0x2B);
            tile[6] = v0;
        } else {
            v0 = 0x20;
            tile[4] = v0;
            tile[5] = v0;
            tile[6] = v0;
        }
        *(s16 *)(tile + 8) = (*(s32 *)(arg1 + 0x18));
        *(s16 *)(tile + 0xA) = ((*(s32 *)(arg1 + 0x1C)) + 0xE);
        *(s16 *)(tile + 0xC) = 0x78;
        *(s16 *)(tile + 0xE) = 1;
    }
    SetSemiTrans(tile, 1);
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);
    tile += 0x10;

    SetTile(tile);
    {
        s32 v0;
        if ((D_800A34F8 & 0xF) == arg2) {
            v0 = *(u8 *)(arg1 + 0x29);
            tile[5] = 0;
            v0 = (u32)v0 >> 1;
            tile[4] = v0;
            v0 = *(u8 *)(arg1 + 0x2B);
            v0 = (u32)v0 >> 1;
            tile[6] = v0;
        } else {
            v0 = 0x10;
            tile[4] = v0;
            tile[5] = v0;
            tile[6] = v0;
        }
        *(s16 *)(tile + 8) = ((*(s32 *)(arg1 + 0x18)) + 0x40);
        *(s16 *)(tile + 0xA) = ((*(s32 *)(arg1 + 0x1C)) + 0xD);
        *(s16 *)(tile + 0xC) = 0x38;
        *(s16 *)(tile + 0xE) = 1;
    }
    SetSemiTrans(tile, 1);
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), tile);

    obj2 = *(u8 **)(arg0 + 4);
    tile = tile + 0x10;
    *(u8 **)(arg0 + 0x18) = tile;
    tile = *(u8 **)(obj2 + 0x1C);
    {
        s32 v0;
        *(s32 *)(arg1 + 0) = *(s32 *)(tile + 0x28);

        if ((D_800A34F8 & 0xF) == arg2) {
            *(u8 *)(arg1 + 0x2A) = 0;
            *(u8 *)(arg1 + 0x29) = (u32)(*(u8 *)(arg1 + 0x29)) >> 1;
            *(u8 *)(arg1 + 0x2B) = (u32)(*(u8 *)(arg1 + 0x2B)) >> 1;
        } else {
            *(u8 *)(arg1 + 0x2B) = 0x28;
            *(u8 *)(arg1 + 0x2A) = 0x28;
            *(u8 *)(arg1 + 0x29) = 0x28;
        }

        *(s32 *)(arg1 + 0x18) = 0;
        v0 = *(s32 *)(arg1 + 0);
        v0 += 0xC;
        *(s32 *)(arg1 + 0x1C) += 0xF;
        *(s32 *)(arg1 + 4) = v0;

        *(s32 *)(arg1 + 8) = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg0 + 0x14) = func_8007352C((s32)arg1);
    }
    {
        s32 v0;
        v0 = *(s32 *)(tile + 0x2C);
        *(s32 *)(arg1 + 0) = v0;
        *(s32 *)(arg1 + 4) = v0 + 0xC;
        *(s32 *)(arg1 + 8) = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg0 + 0x14) = func_8007352C((s32)arg1);
    }

    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0,
                func_8006E480(*(s32 *)(arg1 + 0), s4), 0);
    AddPrim(D_800A374C + (*(s32 *)(arg1 + 0x14) << 2), *(u8 **)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) = *(s32 *)(arg0 + 0x1C) + 0xC;
}
