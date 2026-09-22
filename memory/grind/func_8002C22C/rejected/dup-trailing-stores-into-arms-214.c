/* REJECTED s5 (2026-09-21): duplicating the trailing unconditional stores
 * (0x1F800368 = a2; 0x1F800374 = d_v1 (+d_v0); 0x1F800378 = d_a0 (+d_a1))
 * from after the first if/else join into EACH arm, using that arm's own
 * already-computed a2/d_v1/d_a0/d_v0/d_a1 — the s4 frontier's second item,
 * applying the same cse-block-extension mechanism that fixed the second
 * block's t0[0xC0] read (see candidate.c header). Measured WORSE:
 * sandbox --disable all 173 -> 214 (build_insns grew 240 -> 250 against an
 * unchanged 252-insn target). Unlike t0[0xC0] (which IS re-read across the
 * join, in the second if/else block's `sum_a1_2` computation), these five
 * trailing stores are never re-read after being written — there is no
 * shared-value re-materialization for cse1's block-extension to defeat, so
 * the duplication only doubled the emitted stores. KILLED (instance):
 * the cse-block-extension duplication lever does not generalize to a
 * write-only shared tail; it requires a value that is READ again across
 * the join, not merely written.  Reverted; candidate.c and src/code6cac_b.c
 * keep the single unconditional post-join store form (996-)-(970)-of the
 * s3/s4 chassis. */
void func_8002C22C_REJECTED_dup_trailing_stores(void) {
    /* Only the shape of the change, not a compilable standalone body —
     * see candidate.c for the full function. In each arm, appended:
     *   *(s32 *)0x1F800368 = a2;
     *   *(s32 *)0x1F800374 = d_v1;
     *   *(s32 *)0x1F800378 = d_a0;
     *   *(s32 *)0x1F800374 = d_v1 + d_v0;
     *   *(s32 *)0x1F800378 = d_a0 + d_a1;
     * and removed the single post-join copy of the same five statements. */
}
