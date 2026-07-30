/* REJECTED (s2, structural) — declaration order and loop3-pointer identity are
 * BOTH completely inert for this function's register allocation.
 *
 * Measured with the cc1 -da `.greg` allocation dump (tmp/grind/func_800477E8/s2/
 * sweep.py + variants.py). "Inert" here is the strong form: the
 * `;; N regs to allocate:` priority order AND the `;; Register dispositions:`
 * map came back byte-identical to the HEAD baseline.
 *
 * Variants measured inert (dump identical to base, floor stays 17):
 *   decl-v1-first        : `s32 v1;` moved to the top of the declaration list
 *   decl-t1val-last      : `s32 t1val;` moved to the bottom
 *   ptr-split-loop3      : loop3 walks its own `s32 *q` instead of reusing `ptr`
 *   loop3-indexed        : loop3 rewritten as `for (a0=0;a0<0x11;a0++)
 *                          D_800EF558[a0] = (a0<<7)&0xFFF;`
 *   loop2-base-index     : `p = &ptr[0x10];` / `ptr = &ptr[0x11];` instead of
 *                          `p = ptr + 0x10;` / `ptr += 0x11;`
 *   a0-init-inside-outer : `a0 = 0;` moved from before the do-loop to the top of
 *                          the outer body (GCC hoists it straight back out)
 *
 * MECHANISM (why declaration order cannot matter here). GCC 2.7.2
 * global.c:allocno_compare sorts by
 *     floor_log2(n_refs) * n_refs * size / live_length
 * and only falls back to the allocno NUMBER (which is what declaration order
 * moves) when two allocnos have EXACTLY equal priority. None of this function's
 * contested pairs are exactly equal at HEAD, so renumbering changes nothing.
 * Declaration order only becomes a usable lever AFTER a split has equalised the
 * reference counts of the contestants -- and even then it was inert here
 * (see mid+val-declared-first, floor 17, seating unchanged).
 *
 * DO NOT re-probe declaration order, loop3's pointer identity, or loop3's loop
 * shape. They are dead.
 */
