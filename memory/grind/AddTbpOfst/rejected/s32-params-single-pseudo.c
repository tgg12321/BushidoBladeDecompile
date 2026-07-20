/* REJECTED (s1, measured 6): s32 AddTbpOfst(s32 a0, s32 a1).
 * With SI params there is ONE pseudo per param; ALL first uses (andi, both
 * sll-16 sign-extends) read the $a3/$t0 copies instead of raw $a0/$a1, and
 * the guard compare degrades to slti (mask result known non-negative).
 * Target needs the u16/s16 two-pseudo split: extensions read the SI incoming
 * copy (pseudo 73 -> $4), only the sb reads the var copy ($a3).
 */
s32 AddTbpOfst(s32 a0, s32 a1);
