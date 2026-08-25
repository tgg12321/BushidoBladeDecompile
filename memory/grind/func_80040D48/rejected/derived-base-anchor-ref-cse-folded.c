/* REJECTED (s1 2026-08-24): the "derived-base anchor" spelling class for
 * bumping a pointer pseudo's reg_n_refs. Two instances, both measured NEUTRAL
 * (floor unchanged, emitted bytes identical, greg allocation order unchanged):
 *
 *   s3  = s5 + 0x68;    // instead of s3 = s4 + 0x94   (probe B, floor 31->31)
 *   a3p = a2p + 0x18;   // instead of a3p = s4 + 0x10EC (probe E, floor 24->24)
 *
 * Mechanism of death: cse1 runs BEFORE flow/life_analysis. cse knows the base
 * pointer is single-set (s5 = s4+0x2C / a2p = s4+0x10D4) and rewrites the
 * derived sum to the s4-relative form IN THE SAME EXTENDED BASIC BLOCK, so the
 * extra reference never survives to reg_n_refs counting. The F1
 * "combine-foldable chain-extender" family works only when the extender
 * survives cse and dies in combine (post-flow); a same-EBB derived-base anchor
 * is NOT that shape. Any future ref-bump attempt must either sit across a
 * cse block boundary (call / conditional with multi-use label) or use a
 * different mechanism entirely. */
