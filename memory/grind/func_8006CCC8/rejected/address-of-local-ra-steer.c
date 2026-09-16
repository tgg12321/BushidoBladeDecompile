/* REJECTED (s5, permuter modality) — cheat-shaped, no semantic purpose.
 * Directed decomp-permuter finding output-1130-1 (score 1130, worse than the
 * accepted split-read fix's 870): introduces a fresh `s32 *new_var;` pointer
 * local whose ONLY purpose is `new_var = &masked; ... *new_var` in place of
 * plain `masked` at one of the two i==0/else arm use sites. No family in
 * .claude/rules/no-new-park-categories.md covers "take the address of an
 * existing scalar purely to force a different addressing mode/register
 * choice at one read site" — this is the same INTENT as the forbidden
 * Lever-D dead-param-assign family (steer RA with no observable behavior
 * change), just spelled as an address-of instead of a dead store. Never
 * proposed to the Judge; recorded here per the vetting-checklist discipline
 * in no-new-park-categories.md ("recognize the find as a cheat AND NOT
 * surface it").
 *
 *   s32 *new_var;
 *   ...
 *   for (j = 0; j < 3; j++) {
 *       rec = (u8 *)D_800A3524 + j;
 *       masked = *(rec + 0x1A) & (nib << fade);
 *       masked = *(rec + 0x1A) & masked;   // permuter's own redundant
 *                                          // restatement, also pointless
 *       byte17 = *(rec + 0x17);
 *       new_var = &masked;
 *       if (i == 0) {
 *           *(rec + 0x17) = (u8)((byte17 & 0xF0) + masked);
 *       } else {
 *           *(rec + 0x17) = (u8)((byte17 & 0xF) + (*new_var));
 *       }
 *   }
 */
