/* REJECTED — round 14 directed-PERM_GENERAL find. cheat-reviewer FAIL 2026-06-06.
 *
 * Measured sandbox: score=26, build_insns=25, target_insns=33 (1-point lower
 * than round-7 candidate's 27).
 *
 * Cheat-reviewer verdict: FAIL on:
 *   - OR-pack reorder/reparenthesize: `(((0xE2000000u | b1<<F) | r<<A) | b2<<5) | g`
 *     vs round-7's `b1<<F | r<<A | 0xE2000000u | b2<<5 | g`. OR is associative+
 *     commutative; both forms produce byte-identical observable output. The
 *     specific parenthesization only changes cc1's RTL expression-tree shape
 *     and thus INSN_PRIORITY in the scheduler. Same family as the FAIL on
 *     func_8007CBB0 round-14 (commit d4872471, 2026-06-06, same day):
 *     RTL-tree-shape-shift via parenthesization in associative expression.
 *     Forbidden by cheats-by-any-spelling (no semantic purpose; GCC-internals
 *     justification only).
 *   - Declaration order: g, b2, r, b1 with no semantic motivation. Channels
 *     are independent (no data dependency). Natural orderings would be:
 *     by name (r, g, b1, b2), by source field offset (r=0, b1=2, g=4, b2=6),
 *     or by target computation order (r, g, b1, b2 — target's lbu sequence).
 *     Round-14's g, b2, r, b1 is none of these — it's a permuter-found order
 *     that lowers the score by biasing LUID/allocno priority. Test 2 (human-
 *     programmer) + Test 3 (GCC-internals justification) fail.
 *
 * Isolation test: applying ONLY the early-return inversion (round-7 body
 * unchanged except `if (arg0 != 0) { body } return 0;` -> `if (arg0 == 0)
 * return 0; { body }`) scores 28 — REGRESSES +1 from round-7's 27.
 * Confirms: the 1-point apparent gain was entirely from the OR-reorder +
 * decl-reorder, not the early-return inversion.
 *
 * Standing forbidden constructs in this rejection:
 *   - OR-tree-shape-shift via parenthesization in associative expression
 *   - Non-semantic declaration ordering (permuter-selected for LUID bias)
 *
 * See cheat-reviewer evidence in meta.json sessions[round=14].reviewer.
 *
 * Floor remains 27 (round-7 candidate). The function continues INCOMPLETE
 * per [[no-new-park-categories]] standing posture.
 */

s32 func_8007C97C(u8 *arg0) {
    if (arg0 == 0) return 0;
    {
        u8 g  = (-*(s16 *)(arg0 + 4)) >> 3;
        u8 b2 = (-*(s16 *)(arg0 + 6)) >> 3;
        u8 r  = arg0[0] >> 3;
        u8 b1 = arg0[2] >> 3;
        return (((0xE2000000u | ((u32)b1 << 0xF)) | ((u32)r << 0xA)) | ((u32)b2 << 5)) | (u32)g;
    }
}
