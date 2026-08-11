/* REJECTED s2-permuter 2026-08-11 — A6: stmt1 all-literal, v's set moved
 * BETWEEN stmt1 and stmt2 (hoping stmt1's expansion births the masks in
 * floor-7 order and CSE folds v's set onto stmt1's 0xFF000000 temp).
 * sandbox 2 -> 12 with build_insns 79 (+1): CSE does NOT fold the second
 * 0xFF000000 materialization into a clean reuse — an extra instruction
 * appears and the tail restructures. Killed the "set v late via CSE" path. */
        *pkt = (*pkt & 0xFF000000) | (ot[0x3FFC / 4] & 0xFFFFFF);
        v = 0xFF000000;
        ot[0x3FFC / 4] = (ot[0x3FFC / 4] & v) | ((s32)pkt & 0xFFFFFF);
        D_800A3378 = (s32)(pkt + 6);
