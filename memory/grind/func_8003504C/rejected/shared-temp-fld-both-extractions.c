/* REJECTED — floor stays 24; cluster 2 RESHAPED but not closer.
 * H2: one shared temp for both bitfield extractions creates an anti-dependence
 * that denies sched1 the freedom to interleave the two chains, reproducing
 * target's serialized $v0-only form.
 * Killed s1 — cc1 does NOT keep the reuse as a single serializing pseudo: the
 * shared variable got its own hard reg ($a0) and the block came out as
 *   lui at; sh zero; srl a0,v0,4; lw v0,0x20(t3); andi v1,a0,63; ...
 * i.e. still interleaved, now across three registers instead of two.
 * Do not re-propose this spelling. Cluster 2's remaining untried axis is
 * STATEMENT ORDER of the three stores (see frontier), not variable reuse.
 */
    u32 fld;    /* extra local */

    fld = (u32)p[5] >> 4;
    D_80102784 = fld & 0x3F;
    D_800A36F6 = 0;
    fld = (u32)p[8] >> 3;
    D_80102786 = fld & 1;
