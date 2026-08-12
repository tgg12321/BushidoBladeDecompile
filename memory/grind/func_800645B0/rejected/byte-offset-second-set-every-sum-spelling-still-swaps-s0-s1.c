/* REJECTED — func_800645B0, grind session 6 (forensics), 2026-08-12.
 *
 * WHY IT IS DEAD (measured, sandbox func_800645B0 --disable all):
 *   FA (this file, *3 sum as an UNNAMED temp inside the byte-offset
 *       expression)                                            12 / 78
 *   FD (same, sum spelled `idx * 3`)                           12 / 78
 *   FB (SB chassis + byte offset back into `idx`, three sets)  10 / 78
 *   DA (sum in its own `wid`, byte offset into `idx`)          12 / 78
 *   CA control                                                  3 / 78
 *   SB control (the shipped form)                               1 / 78
 *
 * These four are the complete spelling family for "give `idx` a second,
 * non-foldable set whose value is the 12-byte byte offset".  That second set
 * is exactly what denies sched.c's birthing_insn_p max_priority lift to the
 * loop-top `addu idx,i,j` and fixes the inner-loop emission order — and every
 * member of the family pays the same 12 points, for a reason that is now
 * proven from the dumps rather than inferred:
 *
 *   tmp/grind/func_800645B0/s6/dump_CA/f_greg.txt  ;; Register dispositions:
 *       74 (idx) in 16   75 (idx2) in 17   76 (wid) in 16   85 (byte off) in 16
 *   tmp/grind/func_800645B0/s6/dump_DA/f_greg.txt  ;; Register dispositions:
 *       74 (idx) in 17   75 (idx2) in 16   76 (wid) in 3    (no pseudo 85)
 *
 * In CA the byte offset is its own BLOCK-LOCAL pseudo (85) that crosses two
 * calls, so local-alloc — which runs before global-alloc — hands it $s0 and
 * pushes the block-local `idx2` to $s1; the multi-block `idx` then gets $s0
 * from global-alloc.  That is the target's allocation exactly.  Routing the
 * byte offset through `idx` deletes pseudo 85, so when local-alloc reaches
 * `idx2` (the only remaining block-local call-crossing quantity) $s0 is free
 * and reg_alloc_order hands it over (local-alloc.c:2250-2270, find_free_reg
 * scans reg_alloc_order and $s0 precedes $s1 among the callee-saves).  The
 * multi-block `idx` is then forced to $s1 by global-alloc.
 *
 * The swap is therefore not a tunable priority — it is the direct consequence
 * of the byte offset and `idx` being the SAME pseudo.  No spelling of the sum
 * changes it.  Do not re-propose this family.
 */
extern s32 rand(void);
extern void *D_800A347C;
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 wid;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                idx = (idx2 + idx) << 2;
                *((s32 *)(((s32)(&D_800F0D78)) + idx)) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + idx)) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + idx)) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
