/* func_80060A68 candidate — session s3-permuter (2026-08-19).  HONEST SANDBOX
 * SCORE 0 (build 66 / target 66) measured THIS session with this body applied
 * over src/text1b.c:3321.  `sandbox func_80060A68 --disable all` => 0.
 *
 * WHY THIS BODY AND NOT THE PREVIOUS ZERO.  The previous zero-scoring form
 * closed the same two-instruction residual with a freshly-invented `src`
 * pointer scratch assigned the SAME value twice (once for copy 2, once for
 * copy 3).  The layer-1 cheat-reviewer FAILED that construct — inventing a new
 * local purely to give a load's destination reg_n_sets > 1 is exactly what
 * staged-value-reused-variable's bound 2 forbids — and the driver has since
 * BANNED it for this function under any spelling.  Its remedy (a) was "find an
 * honest pre-existing local the function already uses for a real job".  This
 * body does that: copy 2's source pointer is carried by `temp2`, a local the
 * function already has, which a few lines later holds the 0x1A halfword.  No
 * new local is introduced anywhere in this diff, and copy 3 is left as the
 * plain inline re-read it always was.
 *
 * THE MECHANISM (inherited, dump-verified, unchanged).  GCC 2.7.2's
 * `adjust_priority` (tools/gcc-2.7.2/sched.c:2540-2592) raises a ready insn's
 * INSN_PRIORITY to the LAUNCH_PRIORITY (0x7f000001, sched.c:187) currently held
 * by the insn being scheduled, whenever `birthing_insn_p` (sched.c:2504-2535)
 * is true.  That predicate is literally `return (reg_n_sets[i] == 1);` on the
 * destination pseudo.  Blocks are scheduled BACKWARD and the pick is ready[0],
 * so "bumped" means "picked early" means "EMITTED LATE".  The residual at the
 * inherited floor of 2 was:
 *     ours    slot 10 lw v0,12(v1)   slot 11 lw a1,16(v1)   slot 12 lw a0,12(v1)
 *     target  slot 10 lw v0,12(v1)   slot 11 lw a0,12(v1)   slot 12 lw a1,16(v1)
 * i.e. the staged 0x10 load (destination `temp_a1`, assigned twice, UNBUMPED,
 * therefore emitted first of the group) and copy 2's address load (fresh
 * single-set pseudo, BUMPED, therefore emitted last) are in the wrong order.
 * Once BOTH are unbumped the sched2 LUID tie-break (sched.c:2464) orders them
 * by RTL position, which is source order — and source order is target order.
 *
 * THE LADDER MEASURED THIS SESSION (all `sandbox --disable all`):
 *   w1  sibling-idiom `dst_s32`/`dst_u16` destination locals (func_80060B70's
 *       exact shape)                                          => 2, 66 insns,
 *       byte-identical to the floor-2 baseline: the destination locals fold
 *       back into base+offset and do not touch the contested pair.
 *   w2  the 0x10 staging SPLIT into a fresh single-set `p10` (so the stage load
 *       becomes bumped too)                                   => 5, 67 insns.
 *       Slots 10/11 become correct, but the bumped stage load overshoots to
 *       ~slot 22 and costs an extra reload.  This kills the "make both bumped
 *       and let LUID order them" idea: a bumped load is picked the instant it
 *       is ready, and the stage load becomes ready ~18 cycles early.
 *   w3  `temp2` widened to s32 and reused as copy 2's source pointer => 2, 66.
 *       The stage load moves to target's slot 12 exactly; the residual becomes
 *       a NEW adjacent swap one slot earlier (copy 1's address load is still
 *       single-set and still bumped).  Predicted by the mechanism.
 *   w4  w3 + copy 1 staged through the pre-existing `result` local => **0**, 66.
 *   w5  w4 with copy 2 also self-overwriting (`temp2 = *(s32*)(temp2+4)`,
 *       symmetric with copy 1)                                => 4, 66.
 *       Target puts copy 2's loaded word in $v0, not in temp2's $a0.
 * A 27,212-iteration permuter campaign over the floor-2 chassis (minimal-TU
 * workspace, telemetry via tools/permuter_campaign.py) returned ZERO finds; the
 * closing form was hand-derived from the sched.c mechanism, not searched.
 *
 * CONSTRUCTS.  Two, both multiply-assigned pre-existing locals carrying real,
 * immediately-read values; zero dead code, no pins, no asm, no volatile, no new
 * locals.  `result` carries copy 1's source pointer and then the word it points
 * at; `temp2` carries copy 2's source pointer and then the 0x1A halfword.
 * `temp_a1`'s staging is inherited unchanged from s1.  Each is FAKE-annotated
 * in place.  See memory/grind/func_80060A68/self_vet.md for the 6-test vet and
 * the sanctioned-family citations.
 *
 * INTEGRATION (BLOCKING, operator/driver surface — do not skip).  asmfix.txt:109
 * and asmfix.txt:110 splice this function's ENTIRE body in from rule text
 * (delete_between anchored on the first body instruction `lhu $4,0($3)` plus a
 * 43-instruction insert_before).  With this C body the first body instruction is
 * `lhu $2,0($3)`, so the delete_between anchor no longer matches and the splice
 * mis-fires, duplicating the body in a full build.  Both rules MUST be retired
 * in the same change as this C (`engine retire func_80060A68`, then
 * `verify-oracle`).  A grind session may not touch asmfix.txt, so this is
 * handed off.
 *
 * SESSION s3b (permuter, 2026-08-19) — RE-VERIFIED AND STRENGTHENED, BODY
 * UNCHANGED.  The previous session's outcome was discarded by the driver's
 * banned-construct tripwire, which fired on the CONSTRUCTS block of its
 * self-vet: the ban text's significant terms are this function's ubiquitous
 * pointer-cast punctuation, so quoting ANY of this body's C in that block trips
 * it.  s3b re-measured everything from scratch rather than inheriting a claim:
 *   - HEAD (pinned `outer` + volatile frame pad) honest floor = 39, 64 insns.
 *   - This body applied over src/text1b.c:3321 => `sandbox func_80060A68
 *     --disable all` = **score 0, build 66 / target 66**, measured twice this
 *     session, with src/text1b.c left in that state.
 *   - NEW PROBE w6 (banked in rejected/): this body with `temp2` still carrying
 *     copy 2's source pointer but the 0x1A halfword read written back inline,
 *     i.e. construct (4) removed => score 10 at 68 insns.  That is the answer to
 *     the strongest objection against construct (2): `temp2`'s named-intermediate
 *     job is load-bearing on its own, worth two instructions, so the borrow is
 *     not resting on a pretext job invented to justify it.
 *   - SECOND PERMUTER CAMPAIGN, structurally different chassis: label
 *     s3b-w3-chassis, seeded from the w3 body (score 2, base permuter score 10,
 *     one adjacent swap from the match) in a fresh minimal-TU workspace at
 *     tmp/grind/func_80060A68/s3/perm2, -j 8 --stop-on-zero --stack-diffs,
 *     1349 s, **38,233 iterations, ZERO finds**, harvested with --stop inside the
 *     session.  Combined with s3's 27,212-iteration floor-2 campaign that is
 *     65,445 iterations across two chassis with no find at all: this residual is
 *     not reachable by random mutation, and the permuter axis is now measured
 *     dead for this function rather than merely untried.
 * The one thing s3b did NOT resolve, and flags for the reviewer rather than
 * papering over: `temp2` is pre-existing with respect to the floor-2 body but
 * was itself introduced by session s1; if bound 2 of staged-value-reused-variable
 * is read to exclude borrowing a grind-introduced local, this body has no honest
 * closing form and the disposition is a ruling, not a respelling.  That fork is
 * argued in full under T5 of memory/grind/func_80060A68/self_vet.md.
 */
void func_80060A68(void) {
    extern s32 D_800A3468;
    extern s32 D_800A3478;
    extern s32 D_800A347C;
    extern s32 D_800A32BC;
    extern u8 D_8009BA60;
    extern s32 D_800F10D0;
    extern s32 chractar_use_pset_combo_id_table;
    s32 outer;
    s32 idx;
    s32 temp2;
    s32 temp_a1;
    s32 result;

    outer = D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    /* FAKE: copy 1's source pointer, and then the word it points at, are staged
       through the function's existing `result` local (its previous value is dead
       here — nothing reads `result` until the dispatch call below overwrites it),
       mechanism: GCC 2.7.2 sched.c adjust_priority -> birthing_insn_p, whose
       LAUNCH_PRIORITY bump is gated on reg_n_sets[regno] == 1, so a fresh
       single-set destination is picked early in the backward list schedule and
       therefore EMITTED late; a multiply-set destination keeps its honest
       priority, lever-exhaustion: memory/grind/func_80060A68/evidence.md
       (s1 v3-v13, s2 v20-v33, s3 v40-v46, s3-permuter w1-w5) */
    result = *(s32 *)(outer + 0xC);
    result = *(s32 *)(result + 0);
    *(s32 *)(outer + 0x20) = result;
    /* FAKE: copy 2's source pointer is carried by the existing `temp2` local
       (widened to s32; it is written again below with the 0x1A halfword and is
       dead in between), so that copy 2's address load also loses the
       birthing_insn_p LAUNCH_PRIORITY bump — same sched.c mechanism as above.
       With both it and the staged 0x10 load unbumped, sched2's LUID tie-break
       (sched.c:2464) emits them in source order, which is target order,
       lever-exhaustion: evidence.md s3-permuter w1-w5 (w2 kills the
       both-bumped alternative; K11 kills the `idx` borrow) */
    temp2 = *(s32 *)(outer + 0xC);
    *(s32 *)(outer + 0x24) = *(s32 *)(temp2 + 4);
    /* FAKE: the 0x10 pointer is staged through temp_a1, which its own next read
       (`temp_a1 = *(u16 *)(temp_a1 + 4)`) consumes and overwrites; same reg_n_sets
       mechanism, and the statement sits AFTER copy 2 so that its RTL LUID is above
       copy 2's address load, lever-exhaustion: evidence.md s1 K2 / s2 K10 / s3 */
    temp_a1 = *(s32 *)(outer + 0x10);
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);

    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    /* FAKE: the 0x1A halfword is read into `temp2` here, above the D_800A3478
       store, and stored below it, mechanism: GCC 2.7.2 sched.c cannot
       disambiguate a %gp_rel symbol store from a base-register load, so no load
       written after that store can schedule above it; target's store order
       requires this read to precede it, lever-exhaustion: evidence.md s1 v8/v9 */
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    temp_a1 = *(u16 *)(temp_a1 + 4);
    idx = *(u16 *)outer;
    D_800A347C = outer + 0x20;
    *(u16 *)(outer + 0x1C) = temp_a1;

    result = ((s32 (*)(s32, s32)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))(idx, temp_a1);
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
