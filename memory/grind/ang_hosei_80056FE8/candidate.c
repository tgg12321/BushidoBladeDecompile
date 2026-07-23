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
 * s6 FORENSICS (instrumented cc1, tmp/gccdbg/cc1; ALLOCDBG+FINDREGDBG): the
 * a1<->a2 swap is a global.c find_reg decision. allocno_compare priority
 * (floor_log2(nref)*nref/live_length*10000*size): var_v0(p82) 24000, a2local
 * (p73) 8571 [6 refs, len14], base(p77) 3809 [4 refs, len21], arg0(p72) 3333.
 * a2local is colored BEFORE base; with NO reg preference it takes the lowest
 * free reg = $a1; base then takes $a2. Target needs base->$a1: reachable ONLY
 * if base carries a full-preference for $a1, which prune_preferences would put
 * into a2local's regs_someone_prefers so a2local avoids $a1 (find_reg pass-0,
 * global.c:970). BUT set_preference (global.c:1591) only makes a hard-reg pref
 * from a reg<->hard-reg COPY insn, and $a1 has NO ABI anchor in this 1-arg leaf
 * (base is def'd by `sll` and consumed by `addu` -- no copy to a hard reg;
 * expand_preferences can't merge one in because base conflicts with var_v0).
 * => the copy-preference frontier is MECHANICALLY UNREACHABLE (KILLED s6).
 * Priority-flip is also dead: base MUST cross the join (len21) while a2local
 * MUST die at the join (the 43rd nop needs a FRESH *arg0 reload, not a2 reuse),
 * so a2local is inherently shorter-lived/higher-priority; cutting a2local refs
 * backfires (shortens its live range -> raises priority, sweep v3). do-while(0)
 * placements never shrink base's len21 nor flip priority (sweep_sched). A
 * diagnostic `register base asm("$5")` pin DOES yield base->$a1/a2local->$a2 but
 * RESCHEDULES to 41 insns (loses 2 nops) -- so even a blunt pin doesn't
 * reproduce the coupled {base$a1, a2local$a2, 43-insn} fixpoint; only the soft
 * preference (no anchor) would. Every sanctioned pure-C axis measured dead ->
 * OWNER-ESCALATION filed docs/grind/decisions.md (owner-gated).
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
