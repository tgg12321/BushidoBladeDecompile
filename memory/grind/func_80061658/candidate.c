/* func_80061658 — best NON-CHEAT form. Floor 7 (honest sandbox --disable all),
 * improved from s1 floor 9 via Lever A (block-local split, register-alloc-pure-c.md).
 * Pure C, zero pins/rules. Apply this body to src/text1b.c to resume from floor 7.
 *
 * MECHANISM (s2, RTL-confirmed): the honest gap is a register-allocation choice.
 *   - HEAD byte-match needs pins t->$2(v0), mask->$3(v1) (cheat, stripped by sandbox).
 *   - Unpinned single-temp form (s1 floor-9): load-temp t=$3(v1), mask const=$2(v0)
 *     — a PURE v0<->v1 swap, target-identical schedule. Root cause: mask's two sets
 *     (li + ori) are RMW-chained => ONE contiguous quantity => handled by local_alloc,
 *     which runs BEFORE global_alloc and grabs v0. t's three loads are DISJOINT ranges
 *     => t goes to global_alloc => gets leftover v1. local-before-global seats mask@v0.
 *   - This Lever-A form (below): splitting the MIDDLE load into a block-local `u`
 *     makes `u` a local pseudo born before mask => u grabs v0, mask correctly takes v1.
 *     But the shared t (loads 1&3, still 2 disjoint global ranges) scatters to a0, and
 *     the block-local (no anti-dep) lets the scheduler hoist load2, DISTORTING the
 *     interleave. Net: mask@v1 (target-correct) but load1/load3@a0 + schedule shift = 7.
 *
 * TENSION (the wall to 0): single-t gives the TARGET SCHEDULE but wrong RA (mask@v0);
 * any split gives better RA (mask@v1) but breaks the SCHEDULE. To reach 0 you need BOTH
 * — the single reused load-temp @ v0 (for the interleave) AND mask @ v1. That requires
 * the disjoint-range shared load-temp to win v0 over the RMW-chained local mask, which
 * local-before-global allocation forbids for these value shapes. No grouping-preserving
 * structural transform flips the local/global classification. Path to 0 = directed
 * permuter over the tail RA structure (different modality), NOT more structural splits.
 *
 * ALTERNATIVE PERMUTER BASE (floor-9, pure RA swap, CORRECT target schedule) — likely a
 * cleaner permuter start than the 7-form since only RA differs (see evidence.md):
 *   func_80060A68();
 *   t = arg0[0]; D_800F1140 = t;
 *   t = arg0[1]; D_800F1144 = t;
 *   mask = 0x10FFFF; D_800A3464 = mask;
 *   t = arg0[2]; D_800F1148 = t;
 */
void func_80061658(s32 *arg0, s32 arg1) {
    s32 *v1 = (s32 *)&D_800F116C;
    s32 t;
    s32 mask;
    u8 *p;
    s32 val;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    switch (arg1) {
    case 0:
        val = 0x21000C;
        p = &D_800F115C;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = val;
        break;
    case 1:
        val = 0x21000D;
        p = &D_800F115C + 1;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = val;
        break;
    }
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    { s32 u = arg0[1]; D_800F1144 = u; }   /* Lever A: block-local split -> mask@v1 */
    mask = 0x10FFFF;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
