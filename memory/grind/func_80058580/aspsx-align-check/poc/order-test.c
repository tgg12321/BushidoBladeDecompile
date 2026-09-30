int g(char *);
int f1(int x) { switch (x) { case 0: return g("one"); case 1: return 7; case 2: return 9; case 3: return 11; case 4: return 2; case 5: return 3; } return 0; }
int f2(void) { return g("hello"); }
int f3(int x) { switch (x) { case 0: return 5; case 1: return 71; case 2: return 19; case 3: return 1; case 4: return 22; case 5: return 33; } return g("three"); }
