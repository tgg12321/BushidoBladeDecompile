/* REJECTED s2-permuter 2026-08-11 — A1-flip: stmt1 OR-operand flip in the
 * v-staged (score-2) context. sandbox 2 -> 8. Same failure as s1 K1a/K3:
 * GCC reuses the FIRST or-operand's register as the destination, so putting
 * the ot-word term first restructures the or-dest and the addiu/sw placement.
 * Confirms target stmt1 operand order is (*pkt & mask_hi) | (ot & mask_lo)
 * in every staging context, not just floor-7. */
        v = 0xFF000000;
        *pkt = (ot[0x3FFC / 4] & 0xFFFFFF) | (*pkt & v);
        ot[0x3FFC / 4] = (ot[0x3FFC / 4] & v) | ((s32)pkt & 0xFFFFFF);
        D_800A3378 = (s32)(pkt + 6);
