/* hirahira_w_frie — s2 best form, sandbox distance 0 (measured 2026-07-17).
 *
 * STATUS: RULING-PENDING — do NOT commit as-is.
 * Two steering constructs, each individually classified:
 *   1. `s32 *b = base;` — SINGLE literal param rename (forward order, offsets
 *      kept raw). Mechanism: base becomes single-use, so combine merges the
 *      entry copy (a0->pseudo) into the init insn at its LATER position; a1's
 *      entry copy then precedes it -> target's s3-pair-first prologue.
 *      Falls under the 2026-06-02 param-local-alias-prologue-pair-flip
 *      tombstone by intent (zero semantic purpose, GCC-internals-justified),
 *      though it is NOT the reversed pair. Ruling requested (s2 outcome).
 *   2. `s32 stop = -2;` — constant-holder local, named-local-fake-exception
 *      family (SOTN `s16 three = 3;`), FAKE-annotated. Closes the preheader
 *      pair (li s7,-2 must be emitted before move s1,s5; loop.c hoists the
 *      literal AFTER the walker stmt, and the 2-leaf priority tie keeps
 *      emission order).
 *
 * If ruling FORBIDS #1: fallback legal floor is 4 (this form minus `b`,
 * using `base` directly everywhere) — the 4-insn prologue pair is then
 * provably unreachable in sanctioned C (see evidence.md s2: sched1 priority
 * tie 4=4, rank_for_schedule LUID tie-break, entry-block bytes fix both
 * chains).
 */
s32 hirahira_w_frie(s32 *base, s16 *offsets) {
    s32 *b = base; /* FAKE: prologue pair order — see header comment */
    s32 *slots = b + 1;
    s32 count = 0;
    s32 *dest = (s32 *)((s32)b + b[1]);
    s32 v1;
    s32 *walker;
    s32 size;
    s32 cur_off;
    s32 ret;

    v1 = *offsets;
    offsets++;
    if (v1 == -2) {
        ret = (s32)dest;
        goto done;
    }

    {
    s32 stop = -2; /* FAKE: constant-holder, named-local-fake-exception */
    walker = slots;
    do {
        if (v1 >= 0) {
            size = walker[1];
            cur_off = walker[0];
            *slots = (s32)dest - (s32)b;
            slots++;
            count++;
            size = size - cur_off;
            func_800520B8((s32)b + cur_off, (s32)dest, size);
            size = (u32)size >> 2;
            size = size << 2;
            dest = (s32 *)((s32)dest + size);
        }
        walker++;
        v1 = *offsets;
        offsets++;
    } while (v1 != stop);
    }
    ret = (s32)dest;

done:
    v1 = ret - (s32)b;
    *b = count;
    *slots = v1;
    return ret;
}
