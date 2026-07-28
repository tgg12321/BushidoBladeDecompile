/* MEASURED DEAD s1 2026-07-27: sandbox --disable all = 8 (worse than
 * floor 7). This is the layer-1 reviewer's suggested "direct rendering"
 * (drop the p-reassignment, store *bp straight into D_800F0D5C).
 * Why it fails: target's final load is `lw $v1, 0x8($v1)` hoisted ABOVE
 * the D_800F1114=1 / D_800F0BC6=0 stores, with the loaded value in $v1
 * (the base's own register). The direct-store form keeps the load
 * adjacent to the final sw and the value lands in $v0 — both the
 * schedule and the register miss. KILLS the hypothesis that the
 * reviewer-clean direct spelling can reach 0.
 */
void func_800651F0(void) {
    s32 *p = (s32 *)D_800A347C;
    s32 *ap = p;
    s32 *bp = p;
    s32 t;
    D_800F0D3C = *ap++;
    D_800F0D40 = *ap++;
    t = *ap;
    D_800F0BC2 = 0;
    D_800F0D44 = t;
    D_800F0D54 = *bp++;
    D_800F0D58 = *bp++;
    D_800F1114 = 1;
    D_800F0BC6 = 0;
    D_800F0D5C = *bp;
}
