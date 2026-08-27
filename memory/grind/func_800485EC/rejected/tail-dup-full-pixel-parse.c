/* REJECTED (s2, measured 16, build 72 insns vs target 68): duplicating the
 * ENTIRE pixel-block tail (p = tim + 2; x/y stores; h/w reads; pixdata;
 * GetTPage call) into both if/else arms, hoping jump2's cross_jump would
 * re-merge the folded copies and land the join label at target's position
 * (target's join label sits at `lh a0` with the clut arm's `j` delay slot
 * holding `sh v0,0x14` — a cross-jump-merge-shaped layout).
 *
 * Result: +5 insns survive — cross-jump does NOT merge the full suffixes
 * (the per-copy cse folds and scheduling leave the two copies non-identical
 * upstream of the common tail), and within each copy the reads still fold
 * onto tim (clut copy: fresh clut-arm tim quantity; else copy: entry tim),
 * so even a successful merge would have produced the folded 7-form reads,
 * not target's p-relative reads. Family dead for reaching 0 on two
 * independent grounds. Superseded by the F6 cancellation-pair candidate.c.
 */
