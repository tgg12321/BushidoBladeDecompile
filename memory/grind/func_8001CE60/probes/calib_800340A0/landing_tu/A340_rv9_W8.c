extern u8 g_sc[2];
extern u8 g_tb[2];

void func_800340A0(void) {
    u8 p1, p2;
    s32 winner = 0;
    p1 = g_sc[winner];
    if ((u8)p1 != D_800A37F8) {
        winner = 1;
        p2 = g_sc[winner];
        if ((u8)p2 != D_800A37F8) {
            if ((u8)p2 < (u8)p1) {
                winner = 0;
            } else if ((u8)p1 == (u8)p2) {
                if ((u8)g_tb[0] < (u8)g_tb[1]) {
                    winner = 0;
                    g_sc[winner] = p1 + 1;
                } else if ((u8)g_tb[1] < (u8)g_tb[0]) {
                    g_sc[winner] = p2 + 1;
                } else {
                    winner = 2;
                }
            }
        }
    }
    D_800A377C[D_800A3874] = winner;
    *(&D_800F65F8 + (D_800A3874 * 2)) = g_sc[0];
    *(&D_800F65F9 + (D_800A3874 * 2)) = g_sc[1];
    D_800A3874 = D_800A3874 + 1;
}
