/* REJECTED s1: arg1 (as s32) reused as the sp20[1] repack staging temp
 * (the value that occupies $v1 in target's post-call block):
 *     arg1 = sp20[1];
 *     sp10[1] = arg1;
 * Goal: same as pointer-holder form - double-set arg1's pseudo to defeat
 * the sched1 birthing boost without hijacking the pointer temp.
 * Result: score 12 (93/93). Prologue copy lands at target slot 3 (boost
 * defeated, as predicted), BUT:
 *   - merged pseudo allocates $a3, not $v1;
 *   - the repack temp is no longer single-set, so IT loses its own boost:
 *     sh sp10[1] schedules LAST of the three, the pointer lui/addiu hoists
 *     up into arg1's now-longer live range, which excludes $v1 and rotates
 *     $v1/$a0/$a1 across the whole post-call block.
 * Both value-reuse spellings cascade ~10-26 distance. The multi-set lever
 * is real but needs a second set whose lifetime stays INSIDE the
 * entry->sh(0x1A) window, or some other non-value-carrying spelling. DEAD
 * as-is.
 */
