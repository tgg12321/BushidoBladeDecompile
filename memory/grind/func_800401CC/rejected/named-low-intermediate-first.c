/* REJECTED s2-permuter 2026-08-11 — A2: real named intermediate
 * (low = ot[0x3FFC/4] & 0xFFFFFF;) set BEFORE the v mask set, hoping to birth
 * the li+ori 0xFFFFFF cluster before v's li without a bare constant holder.
 * sandbox 2 -> 8. Naming the whole AND hoists the ot-word lw AND the and
 * itself above the mask cluster; target keeps all four ANDs late (insns
 * 70-77). The li+ori must be born early WITHOUT its consumer moving — no
 * named-intermediate spelling of a real subexpression can do that, because
 * the intermediate's set necessarily contains the consumer. Killed the whole
 * "honest 4th-ref / named-subexpression" direction (s1 F1) for the emission-
 * order residual. */
        low = ot[0x3FFC / 4] & 0xFFFFFF;
        v = 0xFF000000;
        *pkt = (*pkt & v) | low;
        ot[0x3FFC / 4] = (ot[0x3FFC / 4] & v) | ((s32)pkt & 0xFFFFFF);
        D_800A3378 = (s32)(pkt + 6);
