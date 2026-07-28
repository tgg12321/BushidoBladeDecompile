/* REJECTED (s2) -- CHEAT, DO NOT LAND, a future permuter session WILL
 * rediscover this (exact precedent: func_80037540 s4 `volatile char new_var`).
 *
 * The s2 campaign's ONLY honest-score find (output-202-1, score 202 vs base
 * 266 under the stack-diffs scorer, 6.7 min in): add a never-written,
 * never-read `volatile int new_var;` local. volatile denies the object a
 * register AND deletion, so get_frame_size grows with ZERO emitted
 * instructions. It is the volatile-coercion cheat family
 * ([[inline-asm-policy]] expanded catalog); the engine's cheat-invisible
 * sandbox strips the qualifier, the form degenerates to vars=0, and the
 * score does not move. Score-inert, forbidden, pointless.
 *
 * Note: `volatile int` gives vars=8 -> frame 0x30... campaign variants
 * stacked 4-5 volatile decls to reach vars=32/frame 0x48 (all 6 vars=32
 * hits in tmp/grind/func_800481E8/s2/perm/hits are this family).
 */
void func_800481E8_volatile_cheat(s32 arg0, s32 arg1)
{
    volatile int new_var;   /* CHEAT: never touched, frame coercion only */
    /* ... body identical to candidate.c ... */
}
