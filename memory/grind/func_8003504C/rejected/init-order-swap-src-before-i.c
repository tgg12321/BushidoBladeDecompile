/* REJECTED — floor 24 -> 25, allocation UNCHANGED (src still in $a3).
 * H1: the i/src register inversion is driven by init statement order.
 * Killed s1: statement order does not affect the allocno priority order at all;
 * it only reorders the emitted `move`s in the pre-loop block, which costs 1.
 * Do not re-propose. The allocation lever is reg_n_refs/live_length (see H3).
 */
    p = func_80077D00();
    src = (u8 *)p;      /* <-- moved ahead of i */
    i = 0;
    base = &D_80102785;
    ptr = (u8 *)(base - 9);
