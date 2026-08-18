/* CANDIDATE - func_80017848, s8 (2026-08-18, rederive).  sandbox --disable all = 5
 * FLOOR 11 -> 5 (variant Z1).  Residual is 5 differing instructions, all of them
 * inside the two scan-loop preheaders; the rest of the function is exact.
 *
 * THREE INDEPENDENT LEVERS WERE STACKED THIS SESSION.  Each was measured on its
 * own; the order below is the order they were found, and the score after each:
 *
 *  (1) 11 -> 9  THE LOOP BASE'S ADDEND IS ITS OWN FRESH READ OF ctx+0xC, NOT THE
 *      TOP-GUARD `slots` READ (s8 variant B_basefresh).  s7 had concluded the
 *      opposite ("source the addend from `slots` so the two addends are
 *      un-equatable"), but that conclusion was measured on the s6 chassis.  On
 *      the s7 chassis a per-loop fresh read is worth 2 points: loop 2's preheader
 *      becomes instruction-exact with target (9 insns) because loop 2's block is
 *      a cse JOIN (loop 1's `blez` branches into the middle of it, past the
 *      `lw a0,12(s2)`), so cse's table is reset and the redundant read survives
 *      as a real `lw`.  With `slots` the whole preheader was 2 insns short.
 *
 *  (2) 9 -> 6  ONE LOCAL SERVES AS BOTH LOOP 1'S GUARD ADDRESS AND THE COUNT IT
 *      LOADS (`t = sh + (s32)p; t = *(s32 *)(t + 0x1C); if (i < t)`).  This is
 *      the single highest-value discovery of the session and the mechanism is
 *      exact: cse.c hashes the guard's address expression `(plus shift p)` and
 *      records the pseudo holding it.  Overwriting that SAME pseudo with the
 *      loaded count INVALIDATES the hash entry (cse_insn -> invalidate on the
 *      SET_DEST), so when the loop base recomputes `(plus shift p)` in the
 *      preheader cse can no longer fold it to a copy of the guard address.
 *      Without the reuse, loop 1's base collapsed to `addu a0,a1,zero` (a copy of
 *      the still-live guard address) and the preheader was 1 insn short; with it,
 *      the base is genuinely recomputed as `addu a0,a1,a0`.
 *      This lever is loop-1 ONLY.  Applying it to loop 2 as well costs 26 points
 *      (P_treuse_both = 32) and applying it to the s7 O chassis costs 31
 *      (S_treuse_O = 37) - it is chassis- and loop-specific.
 *
 *  (3) 6 -> 5  AN EXPLICIT `sh = slot_a << 6;` LOCAL, SHARED BY BOTH LOOPS.
 *      s7 killed the explicit shift local (variants K=16, X=17) - on the s7
 *      chassis.  On this chassis it is worth 1 point: it fixes loop 1's links
 *      register (ours `lw a1,16(s2)` -> target's `lw a2,16(s2)`, which also fixes
 *      the dependent `addu v0,v0,a2` in loop 1's body).  Recomputing it per loop
 *      (Z5) or using two shift locals (Z7) both LOSE the point.
 *
 * THE REMAINING 5 (s8/T.txt vs s8/B.txt - the complete residual):
 *   loop 1:  target  addu a3,a0,zero  /  addu a0,a1,a3
 *            ours    addu a0,a1,a0
 *   loop 2:  target  sll a1,s4,6  /  addu a3,a0,zero  /  addu a0,a1,a3
 *            ours    lw v0,12(s2)  /  addu a0,a1,v0
 * i.e. the SAME unexplained artifact s7 already isolated: target routes each
 * preheader's base addend through a dead reg-reg copy of the pre-guard pointer.
 * Plus, now, loop 2's `sll` which our shared `sh` local hoists away (recomputing
 * it costs more than it buys - Z5 = 6).
 *
 * WHAT IS NOW PROVEN ABOUT THAT COPY (do not re-derive):
 *   - It is NOT a source-level copy statement.  `q = p; base = sh + (s32)q;`
 *     was re-measured on THIS chassis (W1_qcopy) and costs 6 points (11).  s6's
 *     pass-level kill therefore survives the two chassis changes since.
 *   - It is NOT a cse-created copy from a redundant load placed in the guard
 *     block.  ZB_readinguard_l1 puts a second `*(u8 **)(ctx + 0xC)` read BEFORE
 *     the guard branch (so the copy would land in a different basic block from
 *     the base add, where combine.c cannot reach across to propagate it); the
 *     read is folded away entirely and the emitted code is byte-identical to Z1.
 *   - It is not reachable by re-routing the addend: `p` (U2 = 11), `slots`
 *     (U3 = 11, U4 = 9), a fresh read (this candidate), a named intermediate
 *     read in the preheader (I/J8 = 9), the base written inside the loop body
 *     (F/U6, no change) all leave it untouched.
 *   - loop.c is ruled out as the emitter by source reading: the only two places
 *     loop.c emits a preheader copy are `move_movables`'s `m->move_insn` path
 *     (requires a REG_EQUIV note or a CONSTANT REG_EQUAL - our addend is a
 *     memory load, neither applies) and the `m->partial && m->match` path
 *     (zero/sign-extension movable combining, which needs a `reg = 0` movable
 *     whose next insn sets a SUBREG of it - not our shape).
 */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *slots;
    u8 *p;
    u8 *base;
    u8 *rec_a;
    u8 *rec_b;
    s32 sh;
    s32 t;
    s32 i;
    s32 dist;

    if (slot_a == slot_b) {
        return 0;
    }

    slots = *(u8 **)(ctx + 0xC);
    if (*(s32 *)((slot_a << 6) + (s32)slots + 0x18) >= 0) {
        if (*(s32 *)((slot_b << 6) + (s32)slots + 0x18) >= 0) {
            return 0;
        }
    }

    p = *(u8 **)(ctx + 0xC);
    sh = slot_a << 6;
    i = 0;
    t = sh + (s32)p;
    t = *(s32 *)(t + 0x1C);
    if (i < t) {
        base = (u8 *)(sh + (s32) * (u8 **)(ctx + 0xC));
        do {
            if (*(u16 *)((*(u8 *)(base + i + 0x24) << 4) +
                         (s32) * (u8 **)(ctx + 0x10) + 0x4) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(base + 0x1C));
        p = *(u8 **)(ctx + 0xC);
    }

    i = 0;
    if (i < *(s32 *)(sh + (s32)p + 0x20)) {
        base = (u8 *)(sh + (s32) * (u8 **)(ctx + 0xC));
        do {
            if (*(s16 *)((*(u8 *)(base + i + 0x2C) << 4) +
                         (s32) * (u8 **)(ctx + 0x10) + 0x6) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(base + 0x20));
    }

    dist = math_Distance3D((s32 *)(*(u8 **)(ctx + 0xC) + (slot_a << 6)),
                           (s32 *)(*(u8 **)(ctx + 0xC) + (slot_b << 6)));
    link = *(u8 **)(ctx + 0x10) + (*(s16 *)(ctx + 0x6) << 4);
    *(s32 *)(link + 0x0) = dist;
    *(s32 *)(link + 0x8) = dist * 3;
    *(s32 *)(link + 0xC) = arg1;
    *(s32 *)(link + 0x4) = (slot_a << 16) | slot_b;

    rec_a = (u8 *)((slot_a << 6) + (s32) * (u8 **)(ctx + 0xC));
    i = *(s32 *)(rec_a + 0x1C);
    *(s32 *)(rec_a + 0x1C) = i + 1;
    *(u8 *)(rec_a + i + 0x24) = *(u16 *)(ctx + 0x6);

    rec_b = (u8 *)((slot_b << 6) + (s32) * (u8 **)(ctx + 0xC));
    i = *(s32 *)(rec_b + 0x20);
    *(s32 *)(rec_b + 0x20) = i + 1;
    *(u8 *)(rec_b + i + 0x2C) = *(u16 *)(ctx + 0x6);

    *(s16 *)(ctx + 0x6) = *(u16 *)(ctx + 0x6) + 1;
    return 1;
}
