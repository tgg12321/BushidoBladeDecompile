/* REJECTED — sandbox --disable all = 1 (87/87 insns).
 *
 * The idea: give the -1 holder unimpeachable semantic purpose by noticing that
 * the "empty slot" sentinel and the "no target found" return value are the same
 * -1. One variable `none = -1` used for the compare, the store, AND both
 * `return -1;` sites reads as ordinary C with no constant-holder smell.
 *
 * The single differing insn is at the shared exit (.L80030CE0): our build emits
 * `move $v0,$s3` where the target emits `addiu $v0,$zero,-1`. So the ORIGINAL C
 * re-materialized -1 at the returns rather than reusing the sentinel — i.e. the
 * original really did have a -1-holding local that is DISTINCT from the return
 * value. Useful corroboration of candidate.c's shape, but not a closing form.
 */
s32 func_80030BA8(u8 *arg0) {
    s32 i = 0;
    s32 none = -1;
    /* ... */
        if (sval == none) { goto next; }
        /* ... */
        if (func_80030B10(arg0, sval) == 0) {
            return none;              /* target: li v0,-1  ours: move v0,s3 */
        }
        old_val = (s32)(*(s16 *)p);
        *(s16 *)p = (s16)none;
    /* ... */
    return none;                      /* same shared exit */
}
