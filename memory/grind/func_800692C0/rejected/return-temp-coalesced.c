/* REJECTED (s2 structural) — explicit return temp to shorten sum live-range.
 * (measured on top of the shared-accumulation form; score stayed 16.)
 *
 *     } while (i < 2);
 *     { s32 ret = sum; return ret; }
 *
 * INTENT (from inherited frontier #2): end sum's live range before `return`
 * so its live_length no longer beats the num-8 group. RESULT: no change — GCC
 * copy-propagates/coalesces `ret` with `sum` (ret = sum right after the loop),
 * so sum still lives to the move-into-$v0. live_length unchanged; sum stays
 * $t5. KILLED — a post-loop copy temp does not shorten the return value's
 * live range under GCC 2.7.2. NB: this also proves the inherited frontier #2
 * direction ("shorten sum") is BACKWARDS — see evidence.md s2.
 */
