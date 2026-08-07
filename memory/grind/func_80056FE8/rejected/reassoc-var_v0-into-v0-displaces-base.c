/* ang_hosei_80056FE8 — REJECTED structural class (s2, RTL-proven dead).
 *
 * Goal was {base->$a1, var_v0->$v0, partial-add scheduled BEFORE the *arg0
 * reload}. RTL (.combine vs .sched) proves the divergence is sched1 hoisting
 * the reload+lookup load-chain above the partial-add: source order (.combine)
 * is ALREADY partial-first, but sched1's list scheduler pulls the reload up
 * because reload->lookup->sum is intrinsically ONE LOAD longer than
 * partial->sum, so it always has higher INSN_PRIORITY. No reassociation of the
 * final adds can flip this.
 *
 * Forms that push var_v0 into $v0 do so only by displacing base to $a2 (never
 * $a1) and fill BOTH delay slots -> build_insns 41 (2 short), masked score 14.
 * The target fixpoint {a2=$a2, base=$a1, var_v0=$v0, partial-first} is not
 * reachable by any structural transform (12 new forms this session + ~20 prior).
 *
 * Representative form (F8 grouped): score 14, build_insns 41. */
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
    /* base += var_v0 (score 15, base->$a2 var_v0->$v1) and
       base+var_v0+0x12C into partial (var_v0->$v0 base->$a2) both fail here. */
    return (base + var_v0) + ((*((s16 *) ((*((s32 *) arg0)) + 0x40A))) + 0x12C);
}
