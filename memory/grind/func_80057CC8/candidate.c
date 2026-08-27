/* BEST MEASURED FORM (grind s37, 2026-08-27, synthesis modality).
 * MEASURED THIS SESSION: `sandbox func_80057CC8 --disable all` -> score 16,
 * target_insns 111, build_insns 108, rules_dropped 0.  The inherited s34/s35/s36
 * candidate re-measured at 20 on this chassis first, so the floor moved 20 -> 16.
 *
 * WHAT CHANGED vs the score-20 predecessor: exactly ONE added statement --
 *     next_vert = &Judge;
 * placed after `scale = arg0[2] * 40;`, with the two sine-table reads then spelled
 * `*(next_vert + ...)` instead of `*(&Judge + ...)`.  Nothing else differs.  The
 * annotation-free score-20 predecessor is preserved verbatim at
 * tmp/grind/func_80057CC8/s37/v20.c (and in git history as the s34 candidate).
 *
 * /* FAKE * / STATUS -- READ BEFORE BUILDING ON THIS FILE.  The added statement is a
 * MATCH-HACK, not program logic: `next_vert` is the next-neighbour vertex pointer and
 * re-pointing it at the sine table has no semantic purpose (the loads are identical
 * either way).  Its family is "variable reuse for codegen control" (an EXISTING local
 * borrowed for a second unrelated value -- .claude/rules/defeat-licm-hoist-var-reuse.md,
 * borrows gated by .claude/rules/staged-value-reused-variable.md), which is on the FROZEN
 * SOTN-accepted list but REQUIRES a /* FAKE * / annotation plus that rule's prerequisites
 * (documented lever-exhaustion + named GCC-pass mechanism) before it may ever be
 * submitted.  s37 did NOT vet those prerequisites -- the floor is 16, not 0, so no
 * candidate-ready and no self-vet was in scope.  A future session that wants to SUBMIT
 * anything containing this statement must either clear the family prerequisites in a
 * self-vet or emit a ruling-request first.  It is banked here because it is the best
 * MEASURED form and because its MECHANISM (below) is the session's real product.
 *
 * THE MECHANISM (dump-verified, tmp/grind/func_80057CC8/dumps/text1b.lreg after this
 * form was applied).  The s34/s35 attribution of the register residual was:
 * "our next-neighbour address is a BLOCK-LOCAL call-crossing quantity (lreg: 'Register 88
 * used 3 times across 4 insns IN BLOCK 4; crosses 1 call'), so local-alloc hands it a
 * callee-save seat BEFORE the two centre twins are placed, pushing cxs to $18 and blocking
 * arg0 out of 16/17/18."  Adding a second SET of the same C variable in the FINAL basic
 * block removes the "in block 4" tag entirely -- lreg now prints
 *     Register 88 used 6 times across 4 insns; crosses 1 call; GR_REGS or none; pointer.
 * with no block tag, i.e. pseudo 88 is now a GLOBAL allocno and local-alloc no longer
 * pre-seats it.  Measured consequence, exactly as predicted by that attribution: cxs
 * moves out of $s2 into $s1 and now MATCHES the target ($s0 cys / $s1 cxs, target
 * asm/funcs/func_80057CC8.s:41-44 `sll $s1,$s4,16 / sra / sll $s0,$s5,16 / sra`).
 * That is the whole four-point gain; the instruction count is unchanged at 108.
 *
 * ISOLATION (all measured this session, all at 108 insns).  The permuter find that
 * exposed this carried three mutations; they were bisected:
 *   (b) `next_vert = &Judge;` reuse ................................. 16  <- the lever
 *   (c) multiply operand flip `(s32)(*..) * scale` on the *arg2 store  21  (worse)
 *   (a) `ang_prev = 0xFFF; ... & ang_prev;` constant holder .......... 20  (inert)
 *   (b)+(c) together ................................................ 17
 * And the lever was probed for a cheaper spelling:
 *   fresh local `s16 *jt; jt = &Judge;` (NOT a reuse) ................ 20  (no gain)
 *   same, array-index spelling `jt[...]` ............................ 20  (no gain)
 *   fresh local initialised at declaration `s16 *jt = &Judge;` ....... 48 at 110 insns
 *   reuse of `table` instead of `next_vert` ......................... 20  (no gain)
 *   reuse of `next_vert` consumed at only ONE of the two sites ...... 16  (same gain)
 *   reuse placed BEFORE `scale = arg0[2] * 40;` ..................... 16  (same gain)
 * The gain therefore comes from the extra SET of the next_vert pseudo (which promotes it
 * out of local-alloc), not from the pointer alias and not from which site consumes it.
 * A fresh pointer local cannot buy it -- that is precisely why no annotation-free
 * spelling of this lever exists.
 *
 * WHY IT IS STILL NOT 0 (unchanged in kind from s30b..s36).  The register map is now
 * $s0 cys, $s1 cxs, $s2 next-ADDRESS, $s3 arg0; the target is $s0 cys, $s1 cxs,
 * $s2 arg0, $s3 next-INDEX.  Only the $s2/$s3 pair is left, and it is the SAME wall:
 * pseudo 88's allocno_compare priority is floor_log2(6)*6/4 = 3.0 against arg0's
 * 2*5/54 = 0.185, so the address is always allocated first and takes the first free
 * callee-save seat.  It can only stop being a callee-save candidate by not crossing the
 * call, which requires post-call address formation, which requires the vertex-table base
 * to be available after the call, which -- with rematerialization (s36-E1), caller-saving
 * (s30) and a ninth callee-save seat (s33-E5, measures 33) all closed -- requires the
 * second source-level materialization the owner refused on 2026-07-20.  That residual is
 * a policy question, not a spelling.
 */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 new_var;
    s32 pi;
    u16 cy;
    s16 *table;
    s16 *next_vert;

    prev_idx = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        s32 off = tmp * 4;
        if ((s16) tmp >= (s32)arg0[3]) {
            off = 0;
        }
        next_vert = (s16 *)(off + (s32)table);
    }

    pi = (s16) prev_idx;
    ang_prev = ratan2(table[pi * 2] - (s16) cx, table[pi * 2 + 1] - (s16) cy) & 0xFFF;
    ang_next = ratan2(next_vert[0] - (s16) cx, next_vert[1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0[2] * 40;
    next_vert = &Judge;
    *arg2 = cx + ((scale * (s32)(*(next_vert + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(next_vert + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}