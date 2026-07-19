/* s45 REDERIVE PROBE - KILLED (HARMFUL, score=8, 86/83)
 *
 * Sub-axis 18: expand-path via array-subscript syntax inside the
 * first-loop body. Rewrite `s32 val = *(s32*)((u8*)&SYM + v1);` to
 * `s32 val = ((s32*)&SYM)[v1 >> 2];`. Semantically equivalent
 * (`(v1>>2)*4 == v1` for v1 a multiple of 4).
 *
 * Measured: score=8, target=83, build=86 -- HARMFUL. Adds 3 extra
 * insns per iteration. Unlike the s4-init variant (a0<<2 is a
 * constant-foldable expression, sub-axis 17 NEUTRAL), the loop-body
 * subscript on a RUNTIME variable v1 is NOT folded at fold-const:
 * GCC 2.7.2 expand_expr emits the srl v1,v1,2 explicitly, then
 * scales back by <<2 for the byte address. combine.c cannot cancel
 * the >>2/<<2 pair here because they surround a live pseudo used
 * elsewhere (v1 also indexes D_800EED14 and is the loop-carried
 * offset increment).
 *
 * Distinct expand-path measurement from sub-axis 17 (s4-init array
 * subscript on constant-foldable a0<<2, NEUTRAL). Adds an 18th
 * independent rederive sub-axis to the ledger tally: constant-fold
 * subscript is neutral, runtime-var subscript is harmful.
 * Further hardens s37's "no un-measured axis exists" closure.
 */
void func_80045294(s32 a0, s32 a1) {
    /* ... baseline decls unchanged ... */
    if (i < count) {
        do {
            s32 val = ((s32 *)&D_800EED18)[v1 >> 2];  /* KILLED - harmful */
            v1 += 0x10;
            i += 1;
            sum += val;
        } while (i < count);
    }
    /* ... rest baseline ... */
}
