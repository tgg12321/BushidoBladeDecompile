/* REJECTED (s2 structural) — `s32 one = 1;` opaque var (slot-4) + bitpos=0
 * moved to last init. sandbox --disable all = 5 (DOWN from 9), build_insns=68.
 *
 *   s32 one; ... arg1<<=4; one = 1; a3_off=0; p=&D_800A32D0; bitpos=0;
 *   ... sum += one << bitpos;   (branchB unchanged: c=2; sum += c<<bitpos)
 *
 * WHY IT FLIPS: greg ALLOCDBG proved the sum/bitpos tie is razor-thin —
 *   base:  sum(pseudo76) nrefs=10 livelen=62 pri=4838 -> $t1($9)
 *          bitpos(79)    nrefs=9  livelen=57 pri=4736 -> $t2($10)
 *   allocno_compare (global.c): pri = flog2(nrefs)*nrefs*size/live_length,
 *   ties broken by allocno# (sum lower# -> wins). Alloc order == reg order.
 *   `one=1` injects one extra preheader `li` INSIDE sum's live range ->
 *   sum livelen 62->63 -> pri 4761. bitpos=0 last -> livelen 57->56 ->
 *   pri 4821. Now bitpos 4821 > sum 4761 -> bitpos gets $t1, sum $t2 =
 *   TARGET RA. Return becomes `move $2,$10` (target). Score 9->5.
 *
 * WHY REJECTED: it is a CHEAT. build_insns=68 vs target 67 — the redundant
 * `li 1` ($15) has NO semantic purpose (the `1<<bitpos` already hoists its
 * own const1 to $14); it exists ONLY to lengthen sum's live range and steer
 * RA. Target is 67 insns / ONE li — this form is provably not the original.
 * Fails cheat-reviewer tests #1 (no semantic purpose), #3 (GCC-internals
 * justification), #6 (literal named `one`). The score-5 is cheat-assisted;
 * the honest floor stays 9.
 *
 * VALUE: proves the RA-flip is worth ~4 points (9->5) and that the ONLY
 * obstacle to a CLEAN 67-insn flip is a +1 sum-livelen (or -1 bitpos-livelen)
 * at constant instruction count — which init/increment reordering provably
 * cannot supply (10 orderings measured: sum ll pinned 62, bitpos ll floor 56).
 */
