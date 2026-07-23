/* REJECTED (s2 structural) — shared accumulate via goto into the else-if arm,
 * matching target's merged .L8006938C `addu $t2,$t2,$v0`. sandbox = 16.
 *
 *   if (sval >= 6)      { c = 1 << bitpos; goto accum; }
 *   else if (sval < -5) { c = 2 << bitpos; accum: sum += c; *arg3=0; *arg2=0; }
 *
 * KEY REALIZATION (why this is the WRONG source): target's SINGLE `addu $t2`
 * is produced by the jump2/cross-jump pass, which runs AFTER register
 * allocation. At allocation time target still had TWO separate accumulates
 * (= our candidate, sum nrefs=10). Writing the merge in SOURCE reduces sum's
 * ALLOCATION-time nrefs from 10 to 6.
 *
 * greg ALLOCDBG: bitpos(79) -> $t1($9) pri 5000 (flips, good) BUT sum(76)
 * nrefs 10->6, pri 4838->2033 -> $t5($13) (`move $2,$13`). sum falls below
 * i/p/a3_off. Also LICM-hoists the `2` (li $15,2 preheader) which target
 * keeps in-loop, adding a 16th pseudo. KILLED: source-level tail-sharing
 * overshoots (sum nrefs too low) AND fights the post-alloc cross-jump.
 * Confirms the inherited shared-accumulation-drops-sum-refs.c finding with
 * exact allocno numbers.
 */
