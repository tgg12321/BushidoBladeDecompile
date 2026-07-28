/* REJECTED: chained-zeroing-order (session s1, 2026-07-28)
 * Form: s32 s1, s2, s3;  ...  s1 = s2 = s3 = 0;   (decl order UNCHANGED)
 * Chained assignment expands right-to-left -> zero-inits emit s3,s2,s1 ->
 * prologue save/init pairs come out in target order s0,s3,s2,s1,ra.
 * MEASURED: floor 1 with this form (and that 1 is the D_8008F19C+1 vs
 * D_8008F19D symbol-vs-addend text artifact — linked bytes identical, verified
 * word-level vs asm/funcs raw words; only the jal reloc word differs pre-link).
 *
 * Layer-1 cheat-reviewer verdict: FAIL (agent a2676f6e1138797c0, this session).
 * Reasoning: same family as rejected/decl-order-prologue-flip.c — "swapping the
 * lever from declaration order to statement-evaluation order to reach the
 * identical documented effect is precisely the 'different spelling of the same
 * intent' the family check exists to catch"; tests 3 (GCC-internals
 * justification) + 5 (family). next_action: revert to s1=0; s2=0; s3=0; and
 * do NOT re-derive under a third spelling.
 *
 * s1 outcome escalates the underlying question via ruling-request: the init
 * ORDER is semantically arbitrary live-store order (not dead code); statement-
 * order levers on live stores are sanctioned elsewhere (defer-store-past-later-
 * compute, loop-counter-fills-load-delay, hoist-call-arg-local). If ALL
 * orderings that match are forbidden BECAUSE they match, no pure-C spelling can
 * reach COMPLETED-C for this function. Owner ruling needed; do not re-propose
 * any order-steering spelling until it lands.
 */

void func_80038170(u8 *out) {
    s32 s1, s2, s3;
    s32 i;
    s32 mask;
    s32 bit;

    s1 = s2 = s3 = 0;   /* REJECTED: right-to-left expansion -> s3,s2,s1 emission */
    mask = D_80106A50;
    /* ... rest identical to candidate.c */
}
