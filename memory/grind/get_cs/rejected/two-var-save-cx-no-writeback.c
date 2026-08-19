/* REJECTED (s7, 2026-08-10): sandbox 12, build 50. The "same dataflow as the
 * banned x/tx pair but WITHOUT the writeback" attempt: save arg0 into x,
 * clamp into a fresh result variable cx whose in-range arm reads x, masks
 * read cx. GCC coalesces cx with x's home (no conflict: x's only read is
 * inside cx's in-range arm, and cx=x is a copy => preference, not conflict),
 * so no join temp and no `move a3,v0` copy materialize; the saved-copy read
 * changes nothing and the renames cascade back to the 12-level. Confirms the
 * WRITEBACK (x = tx re-defining the live-initialized variable) is the
 * load-bearing element of the banned construct, not the saved-copy read.
 */
s32 func_8007C7A0(s16 arg0, s16 arg1)
{
    s16 x = arg0;
    s16 cx;

    if (arg0 >= 0) {
        if ((D_8009BE78 - 1) < arg0) {
            cx = D_8009BE78 - 1;
        } else {
            cx = x;
        }
    } else {
        cx = 0;
    }
    /* Y clamp + per-arm returns identical to candidate.c, masks read cx */
}
