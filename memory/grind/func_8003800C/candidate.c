/* Candidate body for damage_DebugDisp — score 2 vs prior floor 6 (HEAD 9).
 * s10 (synthesis) dropped the floor 6 -> 2 by CLOSING Region B, previously
 * believed (s1-s9) to require a constant-holder cheat. Two independent levers:
 *
 *   1) for-loop fence on the 4-block CopyBlock loop
 *      (loop-exit-work-inside-loop-sched-fence; user-sanctioned).
 *   2) `do { sum = 0; } while (0);` — do-while(0) RA-weighting on the inner
 *      accumulator init. Raises sum's loop-depth-weighted reg_n_refs 10->11
 *      (=tie j), global.c allocno_compare breaks the tie by allocno number
 *      (sum pseudo < j) -> sum wins $a0 (== target). RESOLVES Region A.
 *      /* FAKE: no-semantic do-while(0) for RA weighting (do-while-zero-exception). */
 *   3) *** NEW s10 *** Region B closed: the k-loop is written INDEX-BASED
 *      (base + k*4 / base + k*2) instead of explicit ap/a2p walking pointers,
 *      PLUS `do { k = 0; } while (0);`.
 *      MECHANISM: with explicit `ap=base; a2p=base;` preheader moves, LICM
 *      (loop.c move_movables emit_insn_before(loop_start)) hoists the range-
 *      check constants (0x80000000, 0x1FFFFF) to the preheader END, AFTER the
 *      moves -> sched1 LUID tiebreak emits moves-first (target = consts-first).
 *      Removing the explicit moves makes ap/a2p strength-reduced GIVs, whose
 *      inits are created by strength_reduce (runs AFTER move_movables) -> land
 *      AFTER the hoisted consts -> consts emit FIRST (== target order).
 *      The `do{k=0}while(0)` (a) gives the biv a clean dominating const-0 init
 *      so strength_reduce FOLDS the giv inits to plain `move a0,t1; move a2,a0`
 *      (not `sll;addu` bloat), and (b) its loop-depth ref bump wins k the $a1
 *      register tiebreak over the a2p giv (== target k=$a1, a2p=$a2) — the exact
 *      Region A do-while(0) mechanism, applied to Region B's counter.
 *      /* FAKE: no-semantic do-while(0) for RA weighting + biv-init fold. */
 *      Region B now byte-exact: lui/lui/ori ; move a0,t1 ; move a2,a0.
 *
 * Remaining gap (score 2 = 2 objdump diffs) — Region A' ONLY:
 *   Inner-loop preheader emit ORDER. Target: `sum=0`($a0), `bp=base+offset`($v1),
 *   `j=0`($a1) — order sum,bp,j. Build (do-while(0) relocates sum=0 to the
 *   bracketed block-bottom = highest preheader LUID): j,bp,sum. sched1
 *   rank_for_schedule INSN_LUID tiebreak emits the block-bottom leaf (sum=0)
 *   LAST. IRREDUCIBLE on the do-while(0) chassis by def position (s6/s7),
 *   reconfirmed on this NEW index-B chassis (s10: sum-first plain=9, sum-first
 *   bracketed=9, co-bracket sum+bp=21). Target reaches sum=$a0 with sum=0 FIRST
 *   (low LUID) => sum has weighted-refs>=11 WITHOUT a bracket; that natural
 *   11th ref is the open question. Next: directed permuter on THIS score-2
 *   chassis (numbering changed vs the s4/s5 old-chassis campaigns).
 */
s32 damage_DebugDisp(s32 *arg0) {
    u8 *base = (u8 *)arg0;
    s32 i;
    s32 *chkptr;
    s32 offset;

    i = 0;
    chkptr = (s32 *)base;
    offset = 0;
    do {
        s32 sum;
        u8 *bp;
        u32 j;

        j = 0;
        bp = base + offset;
        do { sum = 0; } while (0); /* FAKE: do-while(0) RA weighting (Region A) */
        do {
            sum += *bp;
            bp++;
            j++;
        } while (j < 0x24U);
        if (sum == *(s32 *)((u8 *)chkptr + 0x6C)) {
            break;
        }
        chkptr++;
        i++;
        offset += 0x24;
    } while (i < 3);

    if (i == 3) {
        return 0;
    }

    if (D_800A31FC != 0) {
        return 1;
    }

    {
        u8 *src = base + i * 0x24;
        s32 k;

        if (!(*(src + 0x23) & 0x80)) {
            CopyBlock *dst = (CopyBlock *)&D_80106A50;
            CopyBlock *sp2 = (CopyBlock *)src;
            CopyBlock *end = (CopyBlock *)((u8 *)src + 0x20);
            for (;;) {
                *dst = *sp2;
                sp2++;
                dst++;
                if (sp2 != end) continue;
                *(s32 *)dst = *(s32 *)sp2;
                break;
            }
        }

        do { k = 0; } while (0); /* FAKE: do-while(0) RA weighting + biv-init fold (Region B) */
        do {
            u16 *ptr = *(u16 **)(base + k * 4 + 0x78);
            if ((u32)((u32)ptr - 0x80000000U) <= 0x1FFFFF) {
                *ptr = *(u16 *)(base + k * 2 + 0xD0);
            }
            k++;
        } while (k < 0x16);
    }

    return 1;
}
