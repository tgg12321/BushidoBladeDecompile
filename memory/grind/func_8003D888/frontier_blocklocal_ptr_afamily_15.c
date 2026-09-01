/* s3 (structural, 2026-09-01) FRONTIER FORM -- score 15 / 36 insns (NOT the floor; the
   floor is candidate.c at 13/37).  This is the STRUCTURALLY CLOSEST form known: 5 of the 6
   target register roles are correct.
     seats: s -> $t0 (target $a2)   avail -> $a2 (target $a1)   count(param n) -> $a3 OK
            ptr/word local p -> $a1 OK   r -> $v1 OK   const 1 -> $a0 OK
   Shape: `avail` is a GLOBAL allocno, the ptr/word carrier `p` is a BLOCK-LOCAL of the
   if-arm.  local_alloc seats p in hard $a1 inside block 1, which makes the count allocno
   conflict with hard $a1 and therefore PURGES its $a1 copy-preference (global.c:908-910).
   That is what finally lets the count land in $a3 instead of the s2 3-cycle seat rotation.
   Residual: avail's allocno also conflicts with hard $a1 (sched1 puts the `p = s[0]` load at
   block-1 index 1, before avail's last use at index 3), so avail cannot share $a1 with p the
   way the target does; and allocno_compare still orders the count ahead of s in this
   association.  See evidence.md s3. */
s32 func_8003D888(u32 *s, s32 n)
{
    s32 avail = s[2];
    u32 r;

    if (avail < n) {
        u32 m1, m2;
        s32 shift;
        u32 p;

        n -= avail;
        m1 = (1 << avail) - 1;
        m2 = (1 << n) - 1;
        r = s[1] & m1;
        p = s[0];
        s[0] = p + 4;
        shift = 32 - n;
        p = *(u32 *)p;
        s[2] = shift;
        s[1] = p;
        r = (r << n) | (((u32)p >> shift) & m2);
    } else {
        s[2] = avail - n;
        r = (s[1] >> (avail - n)) & ((1 << n) - 1);
    }
    return r;
}
