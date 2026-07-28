/* REJECTED (s1, floor 8 / 12): moving the `src_orig = src_base;` STATEMENT
 * before `orig_dest = dest_arr;` (or moving orig_dest's init after the count
 * loop) fixes the s5=s4-into-delay-slot placement but flips the s6/s7
 * ALLOCATION: src_orig's earlier copy lengthens its live range, its global.c
 * priority drops below sentinel's, and it lands in $s7 (sentinel takes $s6).
 * The correct lever is DECLARATION order, not statement order: declare
 * `s32 src_orig;` BEFORE `s32 sentinel;` (pseudo-creation order biases the
 * allocation tie), keep statements src_orig-then-orig_dest. */
    /* stmt order src_orig before orig_dest WITHOUT the decl reorder: */
    fp = frame_offsets;
    count = 0;
    scan = fp + 1;
    src_orig = src_base;   /* -> $s7, WRONG, when declared after sentinel */
    orig_dest = dest_arr;
