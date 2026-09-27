extern u8 g_sc[2];
extern u8 g_tb[2];

void func_800340A0(void) {
    u8 p1, p2, round;
    u8 *p1s = &g_sc[0]; u8 *p2s = &g_sc[1]; u8 *p1t = &g_tb[0]; u8 *p2t = &g_tb[1];
    p1 = *p1s;
    if ((u8)p1 == D_800A37F8) {
        round = D_800A3874;
        D_800A377C[round] = 0;
    } else {
        p2 = *p2s;
        if ((u8)p2 == D_800A37F8) {
            round = D_800A3874;
            D_800A377C[round] = 1;
        } else if ((u8)p2 < (u8)p1) {
            round = D_800A3874;
            D_800A377C[round] = 0;
        } else if ((u8)p1 < (u8)p2) {
            round = D_800A3874;
            D_800A377C[round] = 1;
        } else {
            if ((u8)*p1t < (u8)*p2t) {
                *p1s = p1 + 1;
                round = D_800A3874;
                D_800A377C[round] = 0;
            } else if ((u8)*p2t < (u8)*p1t) {
                *p2s = p2 + 1;
                round = D_800A3874;
                D_800A377C[round] = 1;
            } else {
                round = D_800A3874;
                D_800A377C[round] = 2;
            }
        }
    }
    *(&D_800F65F8 + (D_800A3874 * 2)) = *p1s;
    *(&D_800F65F9 + (D_800A3874 * 2)) = *p2s;
    D_800A3874 = D_800A3874 + 1;
}
