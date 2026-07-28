/* MEASURED s1 2026-07-27: sandbox --disable all = 4 — a NEW UNCONTESTED
 * FLOOR (old floor 7), but not 0. Reuses the pass1 temp t for the final
 * p[2] load (symmetric with pass1's BC2-interposed load/store split;
 * ap/bp walking pointers for both passes; no p-reassignment). Residual 4:
 * the final value lands in $v0 (t's register) where target wants $v1
 * (`lw $v1, 0x8($v1)` — value overwrites the base register). Only the
 * p-reassignment spelling (candidate.c, sandbox 0, ruling-pending)
 * reaches $v1, because the same C variable keeps the same pseudo/hardreg.
 * Kept in rejected/ as a measured non-zero form, NOT as a cheat verdict —
 * this is the fallback floor if the owner rules against candidate.c.
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
    t = *bp;
    D_800F1114 = 1;
    D_800F0BC6 = 0;
    D_800F0D5C = t;
}
