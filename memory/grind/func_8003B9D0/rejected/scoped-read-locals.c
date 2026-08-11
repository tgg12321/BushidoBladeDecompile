/* REJECTED — K3. Give each flag test its own scoped pointer local so the two reads
 * of ((u8 *)D_800A3878)[3] are textually and lexically distinct. MEASURED: region B
 * UNCHANGED — cc1 still emits ONE lui/lw/lbu block and reuses $v1 across both join
 * labels (LABEL_NUSES == 1 at each, so the extended basic block is not broken there).
 * Establishes that "separate the reads by control flow / scope" is not a lever for
 * region B; only a memory-invalidating store between them forces a re-read (see C1 in
 * hypotheses.md). Do not re-propose. */
    {
        u8 *r1 = (u8 *)D_800A3878;
        a3_arg = -1;
        if (r1[3] & 0x1) a3_arg = D_80101EDA;
    }
    {
        u8 *r2 = (u8 *)D_800A3878;
        a0_arg = -1;
        if (r2[3] & 0x2) a0_arg = D_80102326;
    }
