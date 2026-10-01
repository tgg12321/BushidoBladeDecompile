extern int ratan2(int, int);
extern void use(short *);
extern short X0;
extern short X1;
static short S0;
short *fe(int a, int b, short s0) {
    X0 = -ratan2(a, b);
    X1 = s0;
    return &X0;
}
void fe2(int a) {
    X0 = a;
    use(&X0);
    X1 = a;
    use(&X0);
}
void fs2(int a) {
    S0 = a;
    use(&S0);
    S0 = a + 1;
    use(&S0);
}
