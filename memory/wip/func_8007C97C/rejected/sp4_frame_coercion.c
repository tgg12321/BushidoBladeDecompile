/* func_8007C97C — REJECTED FORM. DO NOT RE-DERIVE.
 *
 * This form was committed as 0e845d2 (Match func_8007C97C — retire 9 rules
 * via inverted-null-check + m2c-shape body) and REVERTED in 45a7bb0 because
 * the independent cheat-reviewer agent returned FAIL on the `s32 sp[4]; ...
 * sp[0..3] = temp_*;` write-only frame coercion.
 *
 * The mechanical detectors (find_unused_local_arrays etc.) missed it because
 * `sp` IS referenced in the body — it's being WRITTEN to four times. The
 * detector tests "name appears after declaration"; the cheat is "name is
 * never READ." This is a write-only-array variant of the forbidden frame-
 * coercion family.
 *
 * Why it's a cheat (the reviewer's 6-test verdict):
 *
 *   Test 1 — Semantic purpose: NONE. The return value is independent of
 *     sp[]. No caller reads sp[]. The four sw stores affect no observable
 *     output beyond the stack frame being deallocated on return.
 *
 *   Test 2 — Human-programmer test: NONE. No programmer writing 'pack
 *     color channels into a GPU command word' would write the channel
 *     values to a local array and then ignore them. The natural C is what's
 *     in candidate.c (no array, just compute the OR-expression).
 *
 *   Test 6 — Naming announces intent: 'sp' names the mechanism (stack
 *     pointer region), not the semantic content of the array. Same naming
 *     family as 'pad', 'buf', '_pad', 'pre_pad', 'spill' — all coercion
 *     announcements.
 *
 * Why it's the SAME family as the frame-coercion cheats in
 * .claude/rules/dead-vars-local-array.md, just spelled differently:
 *
 *   - dead-vars-local-array catches `s32 buf[N];` declared and NEVER
 *     referenced (the detector's check). This form is `s32 sp[N];` declared,
 *     written-only, never read — same intent (force GCC to reserve frame
 *     bytes for the array), different syntactic spelling.
 *
 *   - GCC's DCE removes the dead stores; the array existed to pin the
 *     frame size at 0x18 (sp/ra spill + this array). Without sp[4], the
 *     candidate's natural frame is presumably smaller and emits different
 *     prologue/epilogue.
 *
 * NEXT-DETECTOR-GAP (filed in the revert commit message): add a
 * write-only-array detector to engine/volatile_cheats.py that catches
 * `T name[N];` where every reference to `name` is on the LHS of an
 * assignment (write-only). Estimate 1-2 hours engine work.
 *
 * UNLEARN-AND-CONTINUE: the inverted-null-check + m2c-body parts of this
 * form ARE legitimate and ARE preserved in ../candidate.c. The lesson is
 * NOT "this whole approach was wrong" — the lesson is "the structural
 * lever was right, but the frame-coercion sneak-in was a cheat regardless
 * of how cleverly it was named."
 */

s32 func_8007C97C(u8 *arg0) {
    s32 sp[4];   /* CHEAT: write-only frame coercion. Naming announces intent. */
    u32 temp_a1;
    s32 temp_a2;
    u32 temp_v0;
    s32 temp_v1;
    if (arg0 != NULL) {
        temp_a1 = (u8) arg0[0] >> 3;
        sp[0] = temp_a1;   /* CHEAT: write-only */
        temp_a2 = (s32) (-*(s16 *)(arg0 + 4) & 0xFF) >> 3;
        sp[2] = temp_a2;   /* CHEAT: write-only */
        temp_v0 = (u8) arg0[2] >> 3;
        sp[1] = temp_v0;   /* CHEAT: write-only */
        temp_v0 = temp_v0 << 0xF;
        temp_v1 = (s32) (-*(s16 *)(arg0 + 6) & 0xFF) >> 3;
        sp[3] = temp_v1;   /* CHEAT: write-only */
        {
            u32 r_e2 = (temp_a1 << 0xA) | 0xE2000000;
            return temp_v0 | r_e2 | (temp_v1 << 5) | temp_a2;
        }
    }
    return 0;
}
