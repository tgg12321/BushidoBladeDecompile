static int si = 5;
static char sc[3] = {1,2,3};
static char sd[3] = {4,5,6};
static int sz = 0;
int gi = 7;
static int big[4] = {1,2,3,4};
static char s8[8] = "abcdefg";
static char s4[4] = "abc";
static char q2[2] = "\001";
static short hs = 3;
static char *sp8 = "xy";
int f(void) { return si + sc[1] + sd[1] + sz + gi + big[2] + s8[3] + s4[1] + q2[0] + hs + sp8[0]; }
