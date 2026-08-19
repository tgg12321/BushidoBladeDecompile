/* s8 REJECTED: cc1 SIGSEGV. `(u16 *)&D_800A9A24 + off` (off = idx << 4) makes the
 * D_800A9A24 address a SCALED pointer add inside the inner loop. GCC 2.7.2 cc1
 * SEGFAULTS (exit 139) compiling text1a_post.c in that form; -fno-strength-reduce
 * is the ONLY flag that avoids it (measured: -fno-schedule-insns / -fno-schedule-insns2
 * / -fno-rerun-cse-after-loop all still segfault), so the crash is in loop.c's
 * strength reduction. The build flags are frozen, so this whole family is
 * MECHANICALLY UNAVAILABLE, not merely worse. */
    LoadImage((s32)rect, (s32)((u16 *)&D_800A9A24 + (idx << 4)));
