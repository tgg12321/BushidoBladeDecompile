/* REJECTED (s2, 2026-09-01) — floor 26 (169 insns) vs the 12 chassis it was applied to.
 * WHY DEAD: a second C-level handle for the func_8006E49C result (here: reusing `r`
 * as an int handle, with p_old = (s32 *)r) collapses the build from 174 to 169 insns —
 * cse/combine forward the handle and delete the loop-preheader D_800A35D0 reload.
 * This is the SAME collapse s1 measured as K1 (q-split-handle-call-result.c) on the
 * s1 chassis, so the kill is chassis-independent: the target's `sw $zero,0x30($v0)` /
 * `sh $zero,0x34($v0)` (raw call-result pseudo held live for two stores) is NOT
 * reachable by naming the call result twice at C level, in either direction.
 */
        r = (s32)func_8006E49C(r, D_800A35D8);
        p_old = (s32 *)r;
        D_800A36A0 = (u8 *)p_old;
        *(s32 *)((u8 *)p_old + 4) = (s32)prev;
        *(s32 *)(r + 0x30) = 0;
        *(s16 *)(r + 0x34) = 0;
