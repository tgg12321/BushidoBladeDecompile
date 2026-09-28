extern u8 g_sc[2];
extern u8 g_tb[2];
void func_800340A0(void) {
    u8 p1, p2, round, w;

    p1 = g_sc[0];
    w = D_800A37F8;
    if ((u8)p1 == w || ((p2 = g_sc[1]) != w && (u8)p2 < (u8)p1)) {
        round = D_800A3874;
        D_800A377C[round] = 0;
    } else if ((u8)p2 == w || (u8)p1 < (u8)p2) {
        round = D_800A3874;
        D_800A377C[round] = 1;
    } else {
        if ((u8)g_tb[0] < (u8)g_tb[1]) {
            g_sc[0] = p1 + 1;
            round = D_800A3874;
            D_800A377C[round] = 0;
        } else if ((u8)g_tb[1] < (u8)g_tb[0]) {
            g_sc[1] = p2 + 1;
            round = D_800A3874;
            D_800A377C[round] = 1;
        } else {
            round = D_800A3874;
            D_800A377C[round] = 2;
        }
    }
    *(&D_800F65F8 + (D_800A3874 * 2)) = g_sc[0];
    *(&D_800F65F9 + (D_800A3874 * 2)) = g_sc[1];
    D_800A3874 = D_800A3874 + 1;
}
