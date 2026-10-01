extern int ratan2(int, int);
static short D_pair[2];
short *f(int a, int b, short s0) {
    short *p;
    short t = -ratan2(a, b);
    p = D_pair;
    *p++ = t;
    *p = s0;
    return D_pair;
}
