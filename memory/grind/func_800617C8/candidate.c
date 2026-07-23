/* func_800617C8 — COMPLETED-C candidate (sandbox --disable all == 0, session s1)
 *
 * Floor 9 -> 0 in pure C. The inherited WIP claimed a "v0/v1 swap wall" proven
 * unreachable via cc1psx calibration. That was derived ONLY from arg0[i] INDEXING
 * forms. The plain WALKING-POINTER form (identical to the already-COMPLETED-C
 * sibling func_800618B4, same file/cluster) reaches score 0: the load temps land
 * in $v0 and the two-instruction mask (0xC06013 = lui+ori) lands in $v1, matching
 * target. cc1's first-pass scheduler defers the mask lui+ori+store into the third
 * load's delay-slot region on its own — no reorder, no staging local, no pins.
 *
 * Zero regfix/asmfix, zero cheat-asm, zero volatile. Only difference from
 * func_800618B4 is the mask constant (0xC06013 vs 0xFF0000) and the two-arg vs
 * one-arg prologue.
 */
void func_800617C8(s32 *arg0) {
    s32 *v1 = (s32 *)&D_800F116C;
    s32 *p;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    if (D_800F1160[0] != 0) {
        if (D_800F1160[1] != 0) {
            D_800F1160[1] = 0;
            D_800F1160[0] = 0;
        }
        if (D_800F1160[0] != 0) goto check_one_zero;
    }
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)D_800F1160;
    *(s32 *)D_800A3468 = 0x210010;
    goto end;
check_one_zero:
    if (D_800F1160[1] == 0) {
        D_800F1180 = (s32)(D_800F1160 + 1);
        *v1 = 0x210011;
    }
end:
    func_80060A68();
    p = arg0;
    D_800F1140 = *p++;
    D_800F1144 = *p++;
    D_800F1148 = *p;
    D_800A3464 = 0xC06013;
}
