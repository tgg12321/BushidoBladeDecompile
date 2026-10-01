extern int ratan2(int, int);
static short D_pair[2];
short *f(int a, int b, short s0) {
    short *p = D_pair;
    *p++ = -ratan2(a, b);
    *p = s0;
    return D_pair;
}
