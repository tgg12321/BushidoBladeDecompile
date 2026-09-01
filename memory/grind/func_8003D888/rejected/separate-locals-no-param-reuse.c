/* REJECTED s1: fresh locals for need/p/word, no param reuse — floor 30 (and 26 with
   `n -= avail` + late shift + word local). Both leave n seated in $a1 via the entry-copy
   preference (greg: "73 preferences: 5"), tmp/s pushed to $a2/$a3, one arg-copy move missing
   (36/37 insns). The single reused `tmp` (avail→ptr→word) variant scored 22 with the same
   seat rotation. Mechanism: global.c find_reg skips $a1 for higher-priority allocnos because
   n's allocno copy-prefers it (regs_someone_prefers); target seating requires n's preference
   purged, which only happens when hard $a1 is live inside n's range (local-alloc seating the
   block-local ptr/word carrier in $a1 — i.e. the param-reuse form in candidate.c). */
s32 func_8003D888(u32 *s, s32 n)
{
    s32 avail = s[2];
    u32 r;

    if (avail < n) {
        s32 need = n - avail;
        u32 m1 = (1 << avail) - 1;
        u32 m2 = (1 << need) - 1;
        u32 *p;

        r = s[1] & m1;
        p = (u32 *)s[0];
        s[0] = (u32)(p + 1);
        s[2] = 32 - need;
        r = (r << need) | ((*p >> (32 - need)) & m2);
        s[1] = *p;
    } else {
        s[2] = avail - n;
        r = (s[1] >> (avail - n)) & ((1 << n) - 1);
    }
    return r;
}
