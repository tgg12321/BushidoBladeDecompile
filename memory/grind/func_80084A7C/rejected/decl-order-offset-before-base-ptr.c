/* REJECTED — session 1 (recon), 2026-08-17.  Declaration-order probe.
 * Form: declare `s32 offset = (s16)a1 * 0xB0;` BEFORE `s32 *base_ptr = ...`
 * (everything else identical to candidate.c).
 *
 * MEASURED: sandbox --disable all = 32 (with the `shifted` local) / 33
 * (without it), vs 26 for base_ptr-first.  REGRESSION.
 *
 * WHY IT IS DEAD, and this is the load-bearing finding — the disassembly diff
 * shows declaration order changes ONLY the emission order of the two entry
 * chains (the *0xB0 multiply chain moves ahead of the table-index chain).  It
 * does NOT change the hard-register assignment at all: base_ptr still lands in
 * $a2 (target wants $a3), offset still lands in $a3 with the extra
 * `move $a1,$a3` coalesce failure, and every downstream flag site still reads
 * `lw $v1, 0($a2)` / `addu $v1,$a1,$v1` exactly as in the base_ptr-first form.
 * The regression is purely the entry chains being emitted in the wrong order
 * (5 extra mismatching instructions at the top) on top of the unchanged
 * register residual.
 *
 * => The "named-intermediate declaration order" / LUID-bias lever is KILLED
 * for the $a2<->$a3 assignment on this function.  Do not re-propose it.
 */
void func_80084A7C(s16 a0, s16 a1) {
    s32 offset = (s16)a1 * 0xB0;
    s32 *base_ptr = (s32 *)((u8 *)&D_80106F28 + ((a0 << 16) >> 14));
    u8 *base = (u8 *)(*base_ptr + offset);
    /* ... body identical to memory/grind/func_80084A7C/candidate.c ... */
}
