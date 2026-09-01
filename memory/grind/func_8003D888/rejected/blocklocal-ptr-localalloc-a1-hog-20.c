s32 func_8003D888(u32 *s, s32 n)
{
    s32 avail = s[2];
    u32 r;

    if (avail < n) {
        u32 m1, m2;
        s32 shift;
        s32 p;

        n -= avail;
        m1 = (1 << avail) - 1;
        m2 = (1 << n) - 1;
        r = s[1] & m1;
        p = s[0];
        s[0] = p + 4;
        shift = 32 - n;
        p = *(u32 *)p;
        r <<= n;
        s[2] = shift;
        r |= ((u32)p >> shift) & m2;
        s[1] = p;
    } else {
        s[2] = avail - n;
        r = (s[1] >> (avail - n)) & ((1 << n) - 1);
    }
    return r;
}
