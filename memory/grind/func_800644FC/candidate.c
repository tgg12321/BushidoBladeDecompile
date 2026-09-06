/* Declarations this body relies on (src/text1b.c): file-scope `extern s32 D_800A3454[];`
 * (was `extern s32 D_800A3454;`; func_80060C60 now writes `D_800A3454[0] = 0;`, measured 0)
 * and `extern SVECTOR D_800F1000[][10];` (the unused `extern s16 D_800F1000;` was removed). */
/* func_800644FC (src/text1b.c) -- MATCHING FORM.  Honest distance 0 / 45
 * (sandbox --disable all, zero cheat-asm) measured in grind session s1
 * (2026-09-06, recon modality) with this exact body in src/text1b.c.
 *
 * Rotates one matrix per enabled bit: for every i < *count whose bit is set
 * in D_800A3454[idx], RotMatrix(&D_800F1000[idx][i], &m[i]).
 *
 * Shape notes (each ordinary alternative was measured -- see
 * memory/grind/func_800644FC/hypotheses.md):
 *  - goto loop, not for/do/while: with a loop note present, loop.c
 *    move_movables hoists the `1` of `1 << i` and the D_800F1000 address
 *    out of the loop and strength-reduces the i*8 giv; the target keeps all
 *    three inside the loop (li/lui/addiu/sll every iteration).
 *  - `i = 0` before the guard, guard written on i: the i=0 lands in the blez
 *    delay slot and the folded guard-compare pseudo reserves the target's
 *    8 phantom frame bytes (vars=8, frame 0x30) -- phantom-slot producer 1.
 *  - `ptr++` before `i++`: bottom-block LUID order (lw *count schedules
 *    before addiu i, and the ptr increment fills the bnez delay slot).
 */
void func_800644FC(s32 *count, MATRIX *m, s32 idx) {
    s32 i;
    MATRIX *ptr;
    s32 *bits;
    s32 vec_off;
    s32 *base; /* FAKE: second handle to D_800A3454 so its address is materialized (la) before the idx<<2 shift, mechanism: expr.c expand_binop force_reg emits the symbol_ref la AFTER the index shift for every single-expression spelling and sched.c rank_for_schedule breaks the equal-priority tie by INSN_LUID, lever-exhaustion: memory/grind/func_800644FC/hypotheses.md H4 (direct-global forms D/F/G/H/I/K/N measured 4..22) */
    i = 0;
    if (i < *count) {
        base = D_800A3454;
        bits = base + idx;
        vec_off = idx * 0x50;
        ptr = m;
    top:
        {
            s32 mask = 1 << i;
            if (*bits & mask) {
                RotMatrix((s16 *)((u8 *)D_800F1000 + vec_off + (i << 3)), (u8 *)ptr);
            }
        }
        ptr++;
        i++;
        if (i < *count) goto top;
    }
}
