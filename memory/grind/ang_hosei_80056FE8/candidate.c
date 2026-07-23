/* ang_hosei_80056FE8 (text1b.c) — s5 candidate, ZERO rules, ZERO cheat-asm.
 * FLOOR IMPROVED 10 -> 9 this session (sandbox --disable all: score 9,
 * build_insns 43 == target 43). Applied to src/text1b.c.
 *
 * THE CHASSIS (new this session, contradicts the s1-s4 "single-BB stuck at 42"
 * framing): the DOUBLE variable-reuse tail —
 *     base += var_v0;            // partial computed IN base's register (target: $a1)
 *     var_v0 = *((s32 *) arg0);  // arg0 reload reuses var_v0's dying register (target: $v0)
 * This is TARGET-FAITHFUL, not a gratuitous coercion: the target asm tail is
 *     addu a1,a1,v0 ; lw v0,0(a0) ; nop ; lh v0,1034(v0) ; nop ; addu v0,a1,v0
 * i.e. partial lands in base's reg ($a1, base dead) and the reload lands in
 * var_v0's reg ($v0, var_v0 dead) — EXACTLY this dataflow. Reaching build_insns
 * 43 (the load-delay nop) REQUIRES this reuse (single-BB non-reuse forms = 42,
 * per s2/s3). Both reuses are load-bearing: dropping `base +=` -> diff 16;
 * dropping the var_v0 reload-reuse -> diff 17 (measured s5).
 *
 * THE RESIDUAL (score 9): a PURE register-allocation tiebreak. Target puts the
 * first-load pointer a2local (*arg0) in $a2 and base (a3*40) in $a1; our build
 * swaps them (a2local->$a1, base->$a2). All 9 differing instructions are that
 * one $a1<->$a2 swap propagating through the field reads + the tail adds.
 * greg proof (tmp/grind/.../s5/f_doublereuse.c.greg): 4 pseudos colored in
 * priority order 82,73,77,72; pseudo 73 = a2local (5 refs) is colored BEFORE
 * pseudo 77 = base (2 refs), so a2local grabs $a1 and base is forced to $a2.
 * Flipping requires base to out-prioritize a2local, i.e. base needs MORE refs
 * (priority ~ floor_log2(n_refs)/live_length; base's 2 refs lose to a2local's 5).
 * The s5 permuter's ONLY weighted-0 closure was `base++; base--;` (adds 2 dead
 * base refs -> flips the tiebreak) = the SAME forbidden dead-op cheat s4 found,
 * reconfirmed on this better chassis. No target-faithful structure supplies the
 * extra refs. NOT candidate-ready (score 9, not 0).
 *
 * FAKE note: `base += var_v0` / `var_v0 = *arg0` are variable-reuse (SOTN
 * defeat-licm/RA-reuse family). Here they are target-faithful (the asm proves
 * the reuse), so they are the honest floor-9 form rather than a coercion. */
s32 ang_hosei_80056FE8(s32 arg0) {
    s32 a2 = *((s32 *) arg0);
    s32 a3 = *((u8 *) ((*((s32 *) (a2 + 0x58))) + 3));
    s32 base = a3 * 40;
    s32 var_v0;
    if ((*((u8 *) (a2 + 0xA3))) != 0xFF) {
        if ((var_v0 = *((s16 *) (arg0 + 0x5E))) == 0) {
            var_v0 = (*((u8 *) (((s32) (&D_8009A830)) + (*((s16 *) (a2 + 0xE)))))) * 2;
        } else {
            var_v0 = (*((s8 *) (((s32) (&D_8009A838)) + (*((s16 *) (a2 + 0xE)))))) * 8;
        }
    } else {
        var_v0 = (*((u8 *) (((s32) (&D_8009A840)) + (*((s16 *) (a2 + 0x14)))))) * 2;
    }
    {
        base += var_v0;
        var_v0 = *((s32 *) arg0);
        return base + (*((s16 *) (var_v0 + 0x40A))) + 0x12C;
    }
}
