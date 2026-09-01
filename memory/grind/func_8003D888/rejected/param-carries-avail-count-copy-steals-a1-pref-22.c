s32 func_8003D888(u32 *s, s32 n)
{
    s32 cnt = n;
    u32 r;

    n = s[2];
    if (n < cnt) {
        u32 m1, m2;
        s32 shift;

        cnt -= n;
        m1 = (1 << n) - 1;
        m2 = (1 << cnt) - 1;
        r = s[1] & m1;
        n = s[0];
        s[0] = n + 4;
        shift = 32 - cnt;
        n = *(u32 *)n;
        r <<= cnt;
        s[2] = shift;
        r |= ((u32)n >> shift) & m2;
        s[1] = n;
    } else {
        s[2] = n - cnt;
        r = (s[1] >> (n - cnt)) & ((1 << cnt) - 1);
    }
    return r;
}
