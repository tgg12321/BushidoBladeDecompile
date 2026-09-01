s32 func_8003D888(u32 *s, s32 n)
{
    s32 t = s[2];
    u32 r;

    if (t < n) {
        u32 m1, m2;
        s32 shift;

        n -= t;
        m1 = (1 << t) - 1;
        m2 = (1 << n) - 1;
        r = s[1] & m1;
        t = s[0];
        s[0] = t + 4;
        shift = 32 - n;
        t = *(u32 *)t;
        r <<= n;
        s[2] = shift;
        r |= ((u32)t >> shift) & m2;
        s[1] = t;
    } else {
        s[2] = t - n;
        r = (s[1] >> (t - n)) & ((1 << n) - 1);
    }
    return r;
}
