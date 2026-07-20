/* KILLED s2: hoist lbu's OUT of both branches into shared prologue.
 * Hypothesis: Ternary/consolidation restructures CFG; cross-jump may unify tails.
 * Result: score EXPLODED 2 -> 32, +2 build_insns. Target has TWO SEPARATE lbu
 * blocks (one before jal func_8004881C @ 80041770-78, one at .L80041798) with
 * duplicated loads at 0x18/0x19/0x1A per branch. Hoisting collapses this to a
 * single lbu triple that both branches share, destroying target's block structure.
 * The two-block form is load-bearing; do not consolidate. */
r = *((u8 *)player + 0x18);
g = *((u8 *)player + 0x19);
b = *((u8 *)player + 0x1A);
if (func_800486FC()) { v = func_8004881C(b,g,r); gnd_load_tex((v<<16)|(v<<8)|v); }
else                 { gnd_load_tex(b | ((r<<16)|(g<<8))); }
