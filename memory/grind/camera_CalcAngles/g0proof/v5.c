extern int ratan2(int, int);
static short D_pair[2];
short *f(int a, int b, short s0) {
    short t = -ratan2(a, b);
    D_pair[0] = t;
    D_pair[1] = s0;
    return D_pair;
}
