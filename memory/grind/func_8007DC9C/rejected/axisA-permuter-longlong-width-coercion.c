/* REJECTED — func_8007DC9C, s5 chassis-3 (whole-body named-temp) permuter find.
 *
 * output-630-1 (score 630 == baseline floor 9 — did NOT even improve the score).
 * The permuter widened the D_8009BF68 declaration to `unsigned long long[]` to
 * change the access shape at the axis-A materialization slot. This is a type
 * coercion of a game-state global (same cheat FAMILY as s4's volatile-BF68 and
 * long-long-holder junk, banked as permuter-alias-longlong-junk.c): no semantic
 * purpose, changes GCC's view of the memory access to fake target bytes, stripped
 * by engine cheat detectors, forbidden by [[inline-asm-policy]] expanded catalog /
 * cheats-by-any-spelling. Doubly dead: it is a coercion AND it does not lower the
 * floor. Recorded to prove chassis-3 produced no legitimate sub-baseline form.
 */
extern unsigned long long D_8009BF68[];   /* <-- the cheat: width coercion */
extern s32 D_8009BF6C;
extern s32 D_8009BF70;

/* body identical to candidate.c (floor 9) except arg_a = D_8009BF68[0] now reads
 * through the coerced long-long declaration. */
