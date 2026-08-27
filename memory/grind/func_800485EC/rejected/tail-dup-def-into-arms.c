/* REJECTED (s1, measured 3, build 69 insns vs target 68): duplicating
 * `p = tim + 2;` into both if/else arms so the def sits before the join
 * label (cse tables die at labels, so the fold cannot fire):
 *
 *     if (flag & 8) { ... GetClut(...); p = tim + 2; }
 *     else          { spr->clut = 0;    p = tim + 2; }
 *     <reads via p>
 *
 * The join reads DO come out v1-relative (fold blocked as predicted), but
 * jump2's cross-jump does NOT merge the two addius (the arm suffixes differ:
 * the clut arm's addiu is scheduled above the j whose delay slot holds
 * sh v0,0x14) — both copies survive: one extra insn, and target's single
 * addiu sits INSIDE the join after lh a0, which a cross-jump merge could
 * never produce (a merge target label would precede the addiu, but the clut
 * arm jumps to the lh). Would also have needed the duplicated-statement
 * FAKE family had it worked. Superseded by the natural candidate.c form.
 */
