/* REJECTED (s2, structural) — score 8, build_insns 79 (NO change). KILLED.
 * Region A hypothesis #2 (block-local sum split): accumulate into a
 * short-lived inner `acc`, then `sum = acc;` right after the loop, so acc
 * dies at loop exit (same live range as j) and its priority rises to match
 * j; the tie would then break by qty number toward sum.
 *   s32 acc = 0; do { acc += *bp; bp++; j++; } while (j<0x24U);
 *   sum = acc; if (sum == chk) break;
 * WHY DEAD: GCC copy-propagates/coalesces acc into sum (single qty), so the
 * RTL is byte-identical to baseline (79 insns, score 8). The intended
 * live-range shortening never materialises — there is no separate acc pseudo
 * after coalescing. Hypothesis #2 is measured dead.
 */
        s32 acc;
        j = 0;
        bp = base + offset;
        acc = 0;
        do {
            acc += *bp;
            bp++;
            j++;
        } while (j < 0x24U);
        sum = acc;
        if (sum == *(s32 *)((u8 *)chkptr + 0x6C)) {
            break;
        }
