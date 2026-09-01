/* s3 (structural, 2026-09-01) FRONTIER FORM -- score 19 / 36 insns (NOT the floor).
   Same block-local-ptr shape as frontier_blocklocal_ptr_afamily_15.c but with the final OR
   written `(masked_word) | (r << n)` instead of `(r << n) | (masked_word)`.  That single
   association change raises the count allocno's live length 25 -> 27 and thereby flips
   allocno_compare into the TARGET allocation order 74(avail) 72(s) 73(count) 75(r), which
   seats s -> $a2 and count -> $a3 exactly as the target does.  Its cost is that local_alloc
   then puts the const-1 pseudo in $v1 instead of $a0, so r is pushed off $v1 and avail off
   $a0/$a1.  The two knobs (allocation ORDER via the OR association, local seating via the
   same association) are currently coupled; decoupling them is the s4 frontier.
   See evidence.md s3. */
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
        r = (((u32)p >> shift) & m2) | (r << n);
    } else {
        s[2] = avail - n;
        r = (s[1] >> (avail - n)) & ((1 << n) - 1);
    }
    return r;
}
