/* s12 KILLED (neutral): introduce local `s32 aa = a0;` and derive both v1 = aa<<4
 * and i = aa from the alias. Result: score=2 NEUTRAL. cse.c places {a0, aa, i}
 * in the same value-equivalence class as expected; adding a named intermediate
 * pseudo does not shift LUID between the sll insn and the move16 insn. Sched2
 * tie still resolved by INSN_LUID with sll (LUID 14) < move16 (LUID 22).
 * Rules out the "named-alias intermediate" spelling as a LUID lever.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 aa = a0;
    s32 v1 = aa << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 i = aa;
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;
    /* ... rest identical to candidate.c ... */
}
