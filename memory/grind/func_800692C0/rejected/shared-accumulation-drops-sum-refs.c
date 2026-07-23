/* REJECTED (s2 structural) — shared-accumulation via goto tail. score 16-19.
 *
 * Rewrite the two mutually-exclusive sval branches to a SINGLE shared
 * `sum += c; *arg3=0; *arg2=0;` (matching target's merged .L8006938C addu),
 * reached by goto from each arm:
 *     if (sval >= 6)      { c = 1 << bitpos; }
 *     else if (sval < -5) { c = 2; c <<= bitpos; }   // c=2 kept separate so 2
 *     else                { goto tail; }             //   is NOT LICM-hoisted
 *     sum += c; *arg3 = 0; *arg2 = 0;
 *   tail: ...loop tail...
 *
 * RESULT: the shared structure DID flip bitpos -> $t1 ($9, = target). BUT it
 * drops sum's RTL n_refs from 6 (def + 2 duplicated += + return) to 4 (def +
 * ONE shared += + return). global.c allocno pri = floor_log2(nrefs)*nrefs/
 * live_length: sum num 8 (was 12), == p/a3_off num 8, and BELOW i num 10.
 * sum's live range still extends past the loop to `return`, so its
 * live_length is the LONGEST of the num-8 group -> LOWEST pri -> sum lands in
 * $t5 (last), rotating i->$t2, p->$t3, a3_off->$t4. sum can NEVER beat i in
 * this form (8 < 10 numerator, and sum's live_length only makes it worse).
 * Variant with `c = 2 << bitpos` (single stmt) additionally LICM-hoists the 2
 * to $t7 (+1 preheader li, wrong reg in sllv) -> score 19.
 * KILLED: sharing overshoots; sum must keep 6 refs (duplicated accumulation).
 */
