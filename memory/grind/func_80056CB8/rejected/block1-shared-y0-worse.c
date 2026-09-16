/* s39 REJECTED: sharing the identical `*(s32 *)(obj + 0xBC) - 0x320` expression
 * (computed twice, once into pt0[1] once into pt1[1]) into a single named
 * local `y0` consumed at both stores.
 *
 * measured_on: s39 chassis (candidate.c's s22-s38-banked 38/204 body +
 * func_80053614 s32-return prerequisite + 5-line extern header, block1's
 * duplicate pt0[1]/pt1[1] expression replaced by a shared local, no FAKE
 * constructs present)
 *
 * result: score 38 -> 73/204, build_insns 198 -> 196 (-2 real instructions,
 * i.e. GCC's own CSE already merges the duplicate expression into one
 * computation at baseline -- forcing it into an explicit named local changes
 * *which* register/instruction sequence carries the shared value and
 * regresses the match, it does not add or remove real work). KILLED.
 *
 * Snippet (full context in candidate.c / src/text1b.c chassis):
 */
        {
            s32 y0 = *(s32 *)(obj + 0xBC) - 0x320;
            pt0[0] = *(s32 *)(obj + 0xB8);
            pt0[1] = y0;
            pt0[2] = *(s32 *)(obj + 0xC0);
            pt1[0] = x;
            pt1[1] = y0;
            pt1[2] = z;
        }
