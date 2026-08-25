/* s16b KILLED (score 9, unchanged; ra_solver model exact 17/17).
 * PROBE A -- outer record loop respelled with goto/labels instead of do/while,
 * to strip the outer loop's NOTE_INSN_LOOP_BEG/END and drop the checksum loop
 * from loop_depth 2 to loop_depth 1.  Intent: cut j's flow-weighted reg_n_refs
 * from 11 to 7, which ra_solver's inverse search lists as a standalone 1-atom
 * vector (refs_down 79: 11->7) reaching the target disposition.
 * RESULT: the depth change is UNIFORM, not selective.  Measured priorities
 * before -> after:  j(79) 47142 -> 20000 (n_refs 11->7, exactly as predicted),
 * sum(77) 27272 -> 10909 (n_refs 10->6), bp(78) 41250 -> 17500 (n_refs 11->8).
 * The sort order 79 > 78 > 77 is PRESERVED, the seats are unchanged and the
 * score stays 9.  This RETRACTS the solver's refs_down(79) vector as a C lever:
 * it is reachable in honest C, but every C mechanism that reaches it (loop-note
 * removal / depth reduction) scales sum and bp by the same factor, so the
 * single-atom assumption behind the vector is violated.  Loop-depth weighting
 * cannot reorder allocnos that all live at the same depth.
 */
s32 damage_DebugDisp(s32 *arg0) {
    u8 *base = (u8 *)arg0;
    s32 i;
    s32 *chkptr;
    s32 offset;

    i = 0;
    chkptr = (s32 *)base;
    offset = 0;
rec_loop:
    {
        s32 sum;
        u8 *bp;
        u32 j;

        sum = 0;
        bp = base + offset;
        j = 0;
        do {
            sum += *bp;
            bp++;
            j++;
        } while (j < 0x24U);
        if (sum == *(s32 *)((u8 *)chkptr + 0x6C)) {
            goto rec_done;
        }
        chkptr++;
        i++;
        offset += 0x24;
    }
    if (i < 3) {
        goto rec_loop;
    }
rec_done:

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
